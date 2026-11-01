/*
  The Forgotten Client
  Copyright (C) 2020 Saiyans King

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "spriteManager.h"
#include "game.h"
#include "GUI/GUI_UTIL.h"
#include "util.h"
#include "json/json.h"
#include "lzma/LzmaLib.h"
#include <zlib.h>

SpriteManager g_spriteManager;

extern Game g_game;
extern Uint32 g_datRevision;
extern Uint32 g_sprRevision;
extern Uint32 g_spriteCounts;

SpriteManager::~SpriteManager()
{
	unloadSprites();
}

void SpriteManager::unloadSprites()
{
	if(m_cachedSprites && m_sprCached)
	{
		SDL_free(m_cachedSprites->hidden.mem.base);//Our pointer saved in rwops - let's free it
		SDL_RWclose(m_cachedSprites);
		m_cachedSprites = NULL;
	}

	m_spriteData.clear();
	m_spriteSheets.clear();
	m_spriteOffsets.clear();
	m_sprLoaded = false;
	unloadHDSprites();
}

void SpriteManager::unloadHDSprites()
{
	if(m_hdSprites)
	{
		SDL_RWclose(m_hdSprites);
		m_hdSprites = NULL;
	}
	m_hdOffsets.clear();
	m_hdSpriteSize = 32;
}

// HD sprites file (made by build_hdspr.py):
//   u32 signature (same as Tibia.spr), u32 magic 'HSPR', u16 version, u16 sprite size,
//   u32 count, u32 offset[count] (0 = empty), per sprite: u32 size + zlib(size*size RGBA).
bool SpriteManager::loadHDSprites(const char* filename)
{
	unloadHDSprites();
	SDL_RWops* file = SDL_RWFromFile(filename, "rb");
	if(!file)
		return false;

	Uint32 signature = SDL_ReadLE32(file);
	Uint32 magic = SDL_ReadLE32(file);
	Uint16 version = SDL_ReadLE16(file);
	Uint16 spriteSize = SDL_ReadLE16(file);
	Uint32 count = SDL_ReadLE32(file);
	if(magic != 0x52505348 || version != 1 || spriteSize <= 32 || spriteSize > 256 || (spriteSize % 32) != 0)
	{
		UTIL_messageBox("The Forgotten Client - Error", "The HD sprites file is not valid, the normal sprites are used.");
		SDL_RWclose(file);
		return false;
	}

	if(signature != g_sprRevision || count != g_spriteCounts)
	{
		UTIL_messageBox("The Forgotten Client - Error", "The HD sprites file does not belong to this Tibia.spr, the normal sprites are used.");
		SDL_RWclose(file);
		return false;
	}

	m_hdOffsets.resize(count);
	for(Uint32 i = 0; i < count; ++i)
		m_hdOffsets[i] = SDL_ReadLE32(file);

	// An incomplete download makes every sprite empty: check that the last
	// sprite is inside the file.
	Sint64 fileSize = SDL_RWsize(file);
	Uint32 lastOffset = 0;
	for(Uint32 i = 0; i < count; ++i)
		lastOffset = UTIL_max<Uint32>(lastOffset, m_hdOffsets[i]);
	bool complete = (fileSize > 0 && lastOffset + 4 <= SDL_static_cast(Uint64, fileSize));
	if(complete && lastOffset > 0)
	{
		SDL_RWseek(file, lastOffset, RW_SEEK_SET);
		Uint32 lastSize = SDL_ReadLE32(file);
		complete = (SDL_static_cast(Uint64, lastOffset) + 4 + lastSize <= SDL_static_cast(Uint64, fileSize));
	}

	if(!complete)
	{
		SDL_snprintf(g_buffer, sizeof(g_buffer), "The HD sprites file '%s' is incomplete or damaged (%lld bytes), the normal sprites are used.\n\nDownload Tibia96.spr again.", filename, SDL_static_cast(long long, fileSize));
		UTIL_messageBox("The Forgotten Client - Error", g_buffer);
		m_hdOffsets.clear();
		SDL_RWclose(file);
		return false;
	}

	m_hdSprites = file;
	m_hdSpriteSize = spriteSize;
	return true;
}

// Small inflate (zlib stream) for the HD sprites, so they don't depend on the
// zlib library that is installed with the client.
namespace
{
	struct HDInflateState
	{
		const unsigned char* in;
		size_t inLen;
		size_t inPos;
		unsigned char* out;
		size_t outLen;
		size_t outPos;
		Uint32 bitBuf;
		Sint32 bitCount;
		bool error;
	};

	struct HDHuffman
	{
		Uint16 counts[16];
		Uint16 symbols[288];
	};

	inline Sint32 hdBits(HDInflateState& s, Sint32 need)
	{
		while(s.bitCount < need)
		{
			if(s.inPos >= s.inLen)
			{
				s.error = true;
				return 0;
			}
			s.bitBuf |= SDL_static_cast(Uint32, s.in[s.inPos++]) << s.bitCount;
			s.bitCount += 8;
		}
		Sint32 value = SDL_static_cast(Sint32, s.bitBuf & ((1u << need) - 1));
		s.bitBuf >>= need;
		s.bitCount -= need;
		return value;
	}

	bool hdBuildHuffman(HDHuffman& h, const Uint8* lengths, Sint32 n)
	{
		SDL_memset(h.counts, 0, sizeof(h.counts));
		for(Sint32 i = 0; i < n; ++i)
			++h.counts[lengths[i]];
		h.counts[0] = 0;

		Sint32 left = 1;
		for(Sint32 len = 1; len < 16; ++len)
		{
			left <<= 1;
			left -= h.counts[len];
			if(left < 0)
				return false;
		}

		Uint16 offsets[16];
		offsets[1] = 0;
		for(Sint32 len = 1; len < 15; ++len)
			offsets[len + 1] = offsets[len] + h.counts[len];
		for(Sint32 i = 0; i < n; ++i)
		{
			if(lengths[i] != 0)
				h.symbols[offsets[lengths[i]]++] = SDL_static_cast(Uint16, i);
		}
		return true;
	}

	Sint32 hdDecode(HDInflateState& s, const HDHuffman& h)
	{
		Sint32 code = 0, first = 0, index = 0;
		for(Sint32 len = 1; len < 16; ++len)
		{
			code |= hdBits(s, 1);
			if(s.error)
				return -1;
			Sint32 count = h.counts[len];
			if(code - count < first)
				return h.symbols[index + (code - first)];
			index += count;
			first += count;
			first <<= 1;
			code <<= 1;
		}
		s.error = true;
		return -1;
	}

	bool hdInflateCodes(HDInflateState& s, const HDHuffman& lencode, const HDHuffman& distcode)
	{
		static const Uint16 lbase[29] = {3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258};
		static const Uint8 lext[29] = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};
		static const Uint16 dbase[30] = {1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577};
		static const Uint8 dext[30] = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};
		for(;;)
		{
			Sint32 symbol = hdDecode(s, lencode);
			if(symbol < 0)
				return false;
			if(symbol < 256)
			{
				if(s.outPos >= s.outLen)
					return false;
				s.out[s.outPos++] = SDL_static_cast(unsigned char, symbol);
			}
			else if(symbol == 256)
				return true;
			else
			{
				symbol -= 257;
				if(symbol >= 29)
					return false;
				size_t len = lbase[symbol] + hdBits(s, lext[symbol]);
				Sint32 dsym = hdDecode(s, distcode);
				if(dsym < 0 || dsym >= 30)
					return false;
				size_t dist = dbase[dsym] + hdBits(s, dext[dsym]);
				if(s.error || dist > s.outPos || s.outPos + len > s.outLen)
					return false;
				for(size_t i = 0; i < len; ++i, ++s.outPos)
					s.out[s.outPos] = s.out[s.outPos - dist];
			}
		}
	}

	bool hdInflateFixed(HDInflateState& s)
	{
		static HDHuffman lencode, distcode;
		static bool built = false;
		if(!built)
		{
			Uint8 lengths[288];
			Sint32 i = 0;
			for(; i < 144; ++i) lengths[i] = 8;
			for(; i < 256; ++i) lengths[i] = 9;
			for(; i < 280; ++i) lengths[i] = 7;
			for(; i < 288; ++i) lengths[i] = 8;
			hdBuildHuffman(lencode, lengths, 288);
			for(i = 0; i < 30; ++i) lengths[i] = 5;
			hdBuildHuffman(distcode, lengths, 30);
			built = true;
		}
		return hdInflateCodes(s, lencode, distcode);
	}

	bool hdInflateDynamic(HDInflateState& s)
	{
		static const Uint8 order[19] = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
		Sint32 nlen = hdBits(s, 5) + 257;
		Sint32 ndist = hdBits(s, 5) + 1;
		Sint32 ncode = hdBits(s, 4) + 4;
		if(s.error || nlen > 286 || ndist > 30)
			return false;

		Uint8 lengths[320];
		SDL_memset(lengths, 0, sizeof(lengths));
		for(Sint32 i = 0; i < ncode; ++i)
			lengths[order[i]] = SDL_static_cast(Uint8, hdBits(s, 3));
		HDHuffman lencode, distcode;
		if(s.error || !hdBuildHuffman(lencode, lengths, 19))
			return false;

		Sint32 index = 0;
		while(index < nlen + ndist)
		{
			Sint32 symbol = hdDecode(s, lencode);
			if(symbol < 0)
				return false;
			if(symbol < 16)
				lengths[index++] = SDL_static_cast(Uint8, symbol);
			else
			{
				Uint8 len = 0;
				Sint32 repeat;
				if(symbol == 16)
				{
					if(index == 0)
						return false;
					len = lengths[index - 1];
					repeat = 3 + hdBits(s, 2);
				}
				else if(symbol == 17)
					repeat = 3 + hdBits(s, 3);
				else
					repeat = 11 + hdBits(s, 7);
				if(s.error || index + repeat > nlen + ndist)
					return false;
				while(repeat--)
					lengths[index++] = len;
			}
		}

		if(lengths[256] == 0)
			return false;
		if(!hdBuildHuffman(lencode, lengths, nlen))
			return false;
		// An incomplete distance code is allowed (e.g. only one distance).
		hdBuildHuffman(distcode, lengths + nlen, ndist);
		return hdInflateCodes(s, lencode, distcode);
	}

	// Returns the number of bytes written, or -1 on error.
	Sint64 hdInflate(const unsigned char* in, size_t inLen, unsigned char* out, size_t outLen)
	{
		if(inLen < 2 || (in[0] & 0x0F) != 8 || ((SDL_static_cast(Uint32, in[0]) << 8) | in[1]) % 31 != 0 || (in[1] & 0x20))
			return -1;

		HDInflateState s;
		s.in = in; s.inLen = inLen; s.inPos = 2;
		s.out = out; s.outLen = outLen; s.outPos = 0;
		s.bitBuf = 0; s.bitCount = 0; s.error = false;

		Sint32 last;
		do
		{
			last = hdBits(s, 1);
			Sint32 type = hdBits(s, 2);
			if(s.error)
				return -1;

			bool ok;
			if(type == 0)
			{
				s.bitBuf = 0;
				s.bitCount = 0;
				if(s.inPos + 4 > s.inLen)
					return -1;
				size_t len = s.in[s.inPos] | (SDL_static_cast(size_t, s.in[s.inPos + 1]) << 8);
				size_t nlen = s.in[s.inPos + 2] | (SDL_static_cast(size_t, s.in[s.inPos + 3]) << 8);
				s.inPos += 4;
				if(len != (~nlen & 0xFFFF) || s.inPos + len > s.inLen || s.outPos + len > s.outLen)
					return -1;
				SDL_memcpy(s.out + s.outPos, s.in + s.inPos, len);
				s.inPos += len;
				s.outPos += len;
				ok = true;
			}
			else if(type == 1)
				ok = hdInflateFixed(s);
			else if(type == 2)
				ok = hdInflateDynamic(s);
			else
				ok = false;
			if(!ok || s.error)
				return -1;
		} while(!last);
		return SDL_static_cast(Sint64, s.outPos);
	}
}

unsigned char* SpriteManager::LoadSpriteHD(Uint32 spriteId, bool bgra)
{
	if(!m_hdSprites || spriteId == 0 || spriteId > m_hdOffsets.size())
		return NULL;

	size_t pixelsSize = SDL_static_cast(size_t, m_hdSpriteSize) * m_hdSpriteSize * 4;
	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_calloc(pixelsSize, sizeof(unsigned char)));
	if(!pixels)
		return NULL;

	Uint32 offset = m_hdOffsets[spriteId - 1];
	if(offset == 0)
		return pixels;

	// A sprite that can't be read gives NULL, then the normal sprite is used.
	// The first failure is shown once so the cause can be found.
	static bool reported = false;
	#define HD_SPRITE_FAILED(reason, value) do { \
		if(!reported) { \
			reported = true; \
			SDL_snprintf(g_buffer, sizeof(g_buffer), "HD sprite %u can't be read: %s (%d, offset %u).\n\nThe normal sprites are used for sprites that can't be read.", spriteId, reason, SDL_static_cast(int, value), offset); \
			UTIL_messageBox("The Forgotten Client - Error", g_buffer); \
		} \
		SDL_free(pixels); \
		return NULL; \
	} while(0)

	if(SDL_RWseek(m_hdSprites, offset, RW_SEEK_SET) < 0)
		HD_SPRITE_FAILED("seek failed", 0);
	Uint32 compressedSize = SDL_ReadLE32(m_hdSprites);
	if(compressedSize == 0 || compressedSize > pixelsSize * 2)
		HD_SPRITE_FAILED("wrong size", compressedSize);

	std::vector<unsigned char> compressed(compressedSize);
	size_t readBytes = SDL_RWread(m_hdSprites, &compressed[0], 1, compressedSize);
	if(readBytes != compressedSize)
		HD_SPRITE_FAILED("read failed", readBytes);

	Sint64 destSize = hdInflate(&compressed[0], compressedSize, pixels, pixelsSize);
	if(destSize < 0)
		HD_SPRITE_FAILED("damaged data", destSize);
	if(destSize != SDL_static_cast(Sint64, pixelsSize))
		HD_SPRITE_FAILED("wrong pixel count", destSize);
	#undef HD_SPRITE_FAILED

	if(bgra)
	{
		for(size_t i = 0; i < pixelsSize; i += 4)
		{
			unsigned char r = pixels[i];
			pixels[i] = pixels[i + 2];
			pixels[i + 2] = r;
		}
	}
	return pixels;
}

void SpriteManager::manageSprites(std::vector<Uint32>& fromSprites, std::vector<Uint32>& toSprites, Uint8& width, Uint8& height)
{
	if(fromSprites.empty())
		return;
	if(fromSprites[0] < m_spriteOffsets.size())
	{
		SpriteOffset& currentOffset = m_spriteOffsets[fromSprites[0]];
		switch(currentOffset.spriteType)
		{
			case 0:
			{
				width = 1;
				height = 1;
			}
			break;
			case 1:
			{
				width = 1;
				height = 2;
			}
			break;
			case 2:
			{
				width = 2;
				height = 1;
			}
			break;
			case 3:
			{
				width = 2;
				height = 2;
			}
			break;
			default: return;
		}
	}
	for(std::vector<Uint32>::iterator it = fromSprites.begin(), end = fromSprites.end(); it != end; ++it)
	{
		Uint32 currentSprite = (*it);
		if(currentSprite < m_spriteOffsets.size())
		{
			SpriteOffset& currentOffset = m_spriteOffsets[currentSprite];
			switch(currentOffset.spriteType)
			{
				case 0:
				{
					toSprites.emplace_back(currentOffset.spriteIndex);
				}
				break;
				case 1:
				{
					toSprites.emplace_back(currentOffset.spriteIndex + 0);
					toSprites.emplace_back(currentOffset.spriteIndex + 1);
				}
				break;
				case 2:
				{
					toSprites.emplace_back(currentOffset.spriteIndex + 0);
					toSprites.emplace_back(currentOffset.spriteIndex + 1);
				}
				break;
				case 3:
				{
					toSprites.emplace_back(currentOffset.spriteIndex + 0);
					toSprites.emplace_back(currentOffset.spriteIndex + 1);
					toSprites.emplace_back(currentOffset.spriteIndex + 2);
					toSprites.emplace_back(currentOffset.spriteIndex + 3);
				}
				break;
				default: return;
			}
		}
	}
	toSprites.shrink_to_fit();
}

unsigned char* SpriteManager::LoadSpriteSheet_BMP(const std::string& spriteFile, size_t& outputSize)
{
	size_t fileSize = spriteFile.size();
	if(spriteFile[fileSize - 1] != 'a' || spriteFile[fileSize - 2] != 'm' || spriteFile[fileSize - 3] != 'z' || spriteFile[fileSize - 4] != 'l' || spriteFile[fileSize - 5] != '.')
	{
		#if CLIENT_OVVERIDE_VERSION > 0
		SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s%c%s.lzma", g_basePath.c_str(), ASSETS_CATALOG, PATH_PLATFORM_SLASH, spriteFile.c_str());
		#else
		SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s%c%u%c%s.lzma", g_basePath.c_str(), ASSETS_CATALOG, PATH_PLATFORM_SLASH, g_clientVersion, PATH_PLATFORM_SLASH, spriteFile.c_str());
		#endif
	}
	else
	{
		#if CLIENT_OVVERIDE_VERSION > 0
		SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s%c%s", g_basePath.c_str(), ASSETS_CATALOG, PATH_PLATFORM_SLASH, spriteFile.c_str());
		#else
		SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s%c%u%c%s", g_basePath.c_str(), ASSETS_CATALOG, PATH_PLATFORM_SLASH, g_clientVersion, PATH_PLATFORM_SLASH, spriteFile.c_str());
		#endif
	}
	
	SDL_RWops* fp = SDL_RWFromFile(g_buffer, "rb");
	if(!fp)
		return NULL;

	size_t sizeData = SDL_static_cast(size_t, SDL_RWsize(fp));
	if(sizeData <= 43)
	{
		SDL_RWclose(fp);
		return NULL;
	}

	unsigned char* srcData = SDL_reinterpret_cast(unsigned char*, SDL_malloc(sizeData));
	if(!srcData)
	{
		SDL_RWclose(fp);
		return NULL;
	}

	SDL_RWread(fp, srcData, 1, sizeData);
	SDL_RWclose(fp);

	//Cipsoft specific header
	size_t position = 0;
	while(srcData[position++] == 0x00) {}
	position += 4;
	while((srcData[position++] & 0x80) == 0x80) {}

	//LZMA file
	position += 13;
	unsigned char* destData = SDL_reinterpret_cast(unsigned char*, SDL_malloc(1048576));
	if(!destData)
	{
		SDL_free(srcData);
		return NULL;
	}

	SizeT procOutSize = 1048576, procInSize = SDL_static_cast(SizeT, sizeData - position);
	Sint32 status = LzmaUncompress(destData, &procOutSize, &srcData[position], &procInSize, &srcData[position - 13], LZMA_PROPS_SIZE);
	if(status == SZ_OK)
	{
		outputSize = SDL_static_cast(size_t, procOutSize);
		SDL_free(srcData);
		return destData;
	}
	SDL_free(destData);
	SDL_free(srcData);
	return NULL;
}

void SpriteManager::SplitSpriteSheet(SDL_Surface* sheet, unsigned char* destData, Sint32 x, Sint32 y)
{
	Uint32* dp = SDL_reinterpret_cast(Uint32*, SDL_reinterpret_cast(Uint8*, sheet->pixels) + (y * sheet->w + x) * 4);
	Sint32 dgap = (sheet->pitch / 4);
	for(Sint32 h = 0; h < 32; h += 4)
	{
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, destData + ((h + 0) * 128)), SDL_reinterpret_cast(const Uint8*, dp), 128); dp += dgap;
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, destData + ((h + 1) * 128)), SDL_reinterpret_cast(const Uint8*, dp), 128); dp += dgap;
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, destData + ((h + 2) * 128)), SDL_reinterpret_cast(const Uint8*, dp), 128); dp += dgap;
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, destData + ((h + 3) * 128)), SDL_reinterpret_cast(const Uint8*, dp), 128); dp += dgap;
	}
}

bool SpriteManager::LoadSpriteSheet(Uint32 spriteId, bool bgra)
{
	for(std::vector<SpriteSheet>::iterator it = m_spriteSheets.begin(), end = m_spriteSheets.end(); it != end; ++it)
	{
		SpriteSheet& spriteSheet = (*it);
		if(spriteId >= spriteSheet.firstSpriteId && spriteId <= spriteSheet.lastSpriteId)
		{
			size_t bmpSize = 0;
			unsigned char* bmpData = LoadSpriteSheet_BMP(spriteSheet.spriteFile, bmpSize);
			if(!bmpData)
				return false;

			SDL_Surface* bmpSurface = SDL_LoadBMP_RW(SDL_RWFromMem(bmpData, SDL_static_cast(Sint32, bmpSize)), 1);
			SDL_free(bmpData);
			if(!bmpSurface)
				return false;

			Uint32 wantedFormat = (bgra ? SDL_PIXELFORMAT_BGRA32 : SDL_PIXELFORMAT_RGBA32);
			if(wantedFormat != bmpSurface->format->format)
			{
				SDL_Surface* s = SDL_ConvertSurfaceFormat(bmpSurface, wantedFormat, SDL_SWSURFACE);
				SDL_FreeSurface(bmpSurface);
				if(!s)
					return false;

				bmpSurface = s;
			}

			Uint32 currentSprite = spriteSheet.firstSpriteId;
			Uint32 spriteCount = currentSprite + (spriteSheet.lastSpriteId - spriteSheet.firstSpriteId);
			switch(spriteSheet.spriteType)
			{
				case 0://1x1
				{
					for(Sint32 y = 0; y < 12; ++y)
					{
						for(Sint32 x = 0; x < 12; ++x)
						{
							if(currentSprite > spriteCount)
								goto Exit_Nest_Loop;

							SpriteData& newSpriteData0 = m_spriteData[currentSprite++];
							newSpriteData0.bgra = bgra;
							SplitSpriteSheet(bmpSurface, newSpriteData0.data, x * 32, y * 32);
						}
					}
				}
				break;
				case 1://1x2
				{
					for(Sint32 y = 0; y < 12; y += 2)
					{
						for(Sint32 x = 0; x < 12; ++x)
						{
							if(currentSprite > spriteCount)
								goto Exit_Nest_Loop;

							SpriteData& newSpriteData0 = m_spriteData[currentSprite++];
							newSpriteData0.bgra = bgra;
							SplitSpriteSheet(bmpSurface, newSpriteData0.data, x * 32, (y + 1) * 32);

							SpriteData& newSpriteData1 = m_spriteData[currentSprite++];
							newSpriteData1.bgra = bgra;
							SplitSpriteSheet(bmpSurface, newSpriteData1.data, x * 32, y * 32);
						}
					}
				}
				break;
				case 2://2x1
				{
					for(Sint32 y = 0; y < 12; ++y)
					{
						for(Sint32 x = 0; x < 12; x += 2)
						{
							if(currentSprite > spriteCount)
								goto Exit_Nest_Loop;

							SpriteData& newSpriteData0 = m_spriteData[currentSprite++];
							newSpriteData0.bgra = bgra;
							SplitSpriteSheet(bmpSurface, newSpriteData0.data, (x + 1) * 32, y * 32);

							SpriteData& newSpriteData1 = m_spriteData[currentSprite++];
							newSpriteData1.bgra = bgra;
							SplitSpriteSheet(bmpSurface, newSpriteData1.data, x * 32, y * 32);
						}
					}
				}
				break;
				case 3://2x2
				{
					for(Sint32 y = 0; y < 12; y += 2)
					{
						for(Sint32 x = 0; x < 12; x += 2)
						{
							if(currentSprite > spriteCount)
								goto Exit_Nest_Loop;

							SpriteData& newSpriteData0 = m_spriteData[currentSprite++];
							newSpriteData0.bgra = bgra;
							SplitSpriteSheet(bmpSurface, newSpriteData0.data, (x + 1) * 32, (y + 1) * 32);

							SpriteData& newSpriteData1 = m_spriteData[currentSprite++];
							newSpriteData1.bgra = bgra;
							SplitSpriteSheet(bmpSurface, newSpriteData1.data, x * 32, (y + 1) * 32);

							SpriteData& newSpriteData2 = m_spriteData[currentSprite++];
							newSpriteData2.bgra = bgra;
							SplitSpriteSheet(bmpSurface, newSpriteData2.data, (x + 1) * 32, y * 32);

							SpriteData& newSpriteData3 = m_spriteData[currentSprite++];
							newSpriteData3.bgra = bgra;
							SplitSpriteSheet(bmpSurface, newSpriteData3.data, x * 32, y * 32);
						}
					}
				}
				break;
				default:
				{
					SDL_FreeSurface(bmpSurface);
					return false;
				}
			}
			Exit_Nest_Loop:
			SDL_FreeSurface(bmpSurface);
			return true;
		}
	}
	return false;
}

unsigned char* SpriteManager::LoadSprite_NEW(Uint32 spriteId, bool bgra)
{
	//Not the best solution - let's think of better way of caching later
	std::map<Uint32, SpriteData>::iterator it = m_spriteData.find(spriteId);
	if(it == m_spriteData.end())
	{
		if(!LoadSpriteSheet(spriteId, bgra))
			return NULL;

		it = m_spriteData.find(spriteId);
	}

	SpriteData& spriteData = it->second;
	if(!spriteData.data)
		return NULL;

	if(spriteData.bgra != bgra)
	{
		for(Sint32 i = 0; i < 4096; i += 4)
		{
			unsigned char temp1 = spriteData.data[i + 0];
			unsigned char temp2 = spriteData.data[i + 2];
			spriteData.data[i + 0] = temp2;
			spriteData.data[i + 2] = temp1;
		}
		spriteData.bgra = bgra;
	}

	unsigned char* newSpriteData = SDL_reinterpret_cast(unsigned char*, SDL_malloc(4096));
	if(newSpriteData)
	{
		UTIL_FastCopy(SDL_reinterpret_cast(Uint8*, newSpriteData), SDL_reinterpret_cast(const Uint8*, spriteData.data), 4096);
		return newSpriteData;
	}
	return NULL;
}

unsigned char* SpriteManager::LoadSprite_RGBA(Uint32 spriteId)
{
	SDL_RWops* sprites = m_cachedSprites;
	if(!sprites)
		return LoadSprite_NEW(spriteId, false);

	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_calloc(4096, sizeof(unsigned char)));
	if(!pixels)
		return NULL;

	Sint64 offset = (g_game.hasGameFeature(GAME_FEATURE_EXTENDED_SPRITES) ? 8 : 6);
	SDL_RWseek(sprites, (spriteId - 1) * 4 + offset, RW_SEEK_SET);

	Uint32 sprLoc = SDL_ReadLE32(sprites);
	SDL_RWseek(sprites, sprLoc, RW_SEEK_SET);
	SDL_RWseek(sprites, 3, RW_SEEK_CUR);//ignore colorkey

	Uint16 sprSize = SDL_ReadLE16(sprites);
	if(sprSize == 0)
		return pixels;

	Uint32 writeData = 0, readData = 0;
	Uint16 numPix;
	while(readData < sprSize)
	{
		numPix = SDL_ReadLE16(sprites);
		writeData += numPix * 4;
		numPix = SDL_ReadLE16(sprites);
		readData += 4;
		for(Uint16 i = 0; i < numPix && writeData <= 4092; ++i)
		{
			pixels[writeData] = SDL_ReadU8(sprites);
			pixels[writeData + 1] = SDL_ReadU8(sprites);
			pixels[writeData + 2] = SDL_ReadU8(sprites);
			#if defined(__ALPHA_SPRITES__)
			pixels[writeData + 3] = SDL_ReadU8(sprites);
			readData += 4;
			#else
			pixels[writeData + 3] = SDL_ALPHA_OPAQUE;
			readData += 3;
			#endif
			writeData += 4;
		}
	}
	return pixels;
}

unsigned char* SpriteManager::LoadSprite_BGRA(Uint32 spriteId)
{
	SDL_RWops* sprites = m_cachedSprites;
	if(!sprites)
		return LoadSprite_NEW(spriteId, true);

	unsigned char* pixels = SDL_reinterpret_cast(unsigned char*, SDL_calloc(4096, sizeof(unsigned char)));
	if(!pixels)
		return NULL;

	Sint64 offset = (g_game.hasGameFeature(GAME_FEATURE_EXTENDED_SPRITES) ? 8 : 6);
	SDL_RWseek(sprites, (spriteId - 1) * 4 + offset, RW_SEEK_SET);

	Uint32 sprLoc = SDL_ReadLE32(sprites);
	SDL_RWseek(sprites, sprLoc, RW_SEEK_SET);
	SDL_RWseek(sprites, 3, RW_SEEK_CUR);//ignore colorkey

	Uint16 sprSize = SDL_ReadLE16(sprites);
	if(sprSize == 0)
		return pixels;

	Uint32 writeData = 0, readData = 0;
	Uint16 numPix;
	while(readData < sprSize)
	{
		numPix = SDL_ReadLE16(sprites);
		writeData += numPix * 4;
		numPix = SDL_ReadLE16(sprites);
		readData += 4;
		for(Uint16 i = 0; i < numPix && writeData <= 4092; ++i)
		{
			pixels[writeData + 2] = SDL_ReadU8(sprites);
			pixels[writeData + 1] = SDL_ReadU8(sprites);
			pixels[writeData] = SDL_ReadU8(sprites);
			#if defined(__ALPHA_SPRITES__)
			pixels[writeData + 3] = SDL_ReadU8(sprites);
			readData += 4;
			#else
			pixels[writeData + 3] = SDL_ALPHA_OPAQUE;
			readData += 3;
			#endif
			writeData += 4;
		}
	}
	return pixels;
}

bool SpriteManager::loadSprites(const char* filename)
{
	SDL_RWops* sprites = SDL_RWFromFile(filename, "rb");
	if(!sprites)
		return false;

	size_t sizeSprites = SDL_static_cast(size_t, SDL_RWsize(sprites));
	unsigned char* data = SDL_reinterpret_cast(unsigned char*, SDL_malloc(sizeSprites));
	if(!data)
	{
		m_cachedSprites = sprites;
		g_sprRevision = SDL_ReadLE32(m_cachedSprites);
		g_spriteCounts = (g_game.hasGameFeature(GAME_FEATURE_EXTENDED_SPRITES) ? SDL_ReadLE32(m_cachedSprites) : SDL_static_cast(Uint32, SDL_ReadLE16(m_cachedSprites)));
		m_sprLoaded = true;
		m_sprCached = false;
		return true;
	}

	SDL_RWread(sprites, data, 1, sizeSprites);
	SDL_RWclose(sprites);
	
	m_cachedSprites = SDL_RWFromMem(data, SDL_static_cast(Sint32, sizeSprites));
	if(!m_cachedSprites)
	{
		SDL_free(data);
		return false;
	}
	g_sprRevision = SDL_ReadLE32(m_cachedSprites);
	g_spriteCounts = (g_game.hasGameFeature(GAME_FEATURE_EXTENDED_SPRITES) ? SDL_ReadLE32(m_cachedSprites) : SDL_static_cast(Uint32, SDL_ReadLE16(m_cachedSprites)));
	m_sprLoaded = true;
	m_sprCached = true;
	return true;
}

bool SpriteManager::loadCatalog(const char* filename)
{
	Uint32 startSpriteIndex = 1;
	SDL_RWops* fp = SDL_RWFromFile(filename, "rb");
	if(!fp)
		return false;

	size_t sizeData = SDL_static_cast(size_t, SDL_RWsize(fp));
	if(sizeData == 0)
		return false;

	std::vector<char> msgData(sizeData);
	SDL_RWread(fp, &msgData[0], 1, sizeData);
	SDL_RWclose(fp);

	std::unique_ptr<JSON_VALUE> dataJson(JSON_VALUE::decode(&msgData[0]));
	JSON_VALUE* catalogJson = dataJson.get();
	if(catalogJson && catalogJson->IsArray())
	{
		size_t catalogSize = catalogJson->size();
		for(size_t i = 0; i < catalogSize; ++i)
		{
			JSON_VALUE* fileJson = catalogJson->find(i);
			if(fileJson && fileJson->IsObject())
			{
				JSON_VALUE* fileType = fileJson->find("type");
				JSON_VALUE* fileName = fileJson->find("file");
				if(fileType && fileName && fileType->IsString() && fileName->IsString())
				{
					const std::string& fileTypeName = fileType->AsString();
					if(fileTypeName == "appearances")
					{
						g_datPath = fileName->AsString();

						JSON_VALUE* fileVersion = fileJson->find("version");
						if(fileVersion && fileVersion->IsNumber())
							g_datRevision = SDL_static_cast(Uint32, fileVersion->AsNumber());
						else
							g_datRevision = 0x64434654;
					}
					else if(fileTypeName == "staticdata")
					{
						//save staticdata filename to parse it later
					}
					else if(fileTypeName == "map")
					{
						//save map filename to parse it later
					}
					else if(fileTypeName == "sprite")
					{
						JSON_VALUE* spriteTypeJson = fileJson->find("spritetype");
						JSON_VALUE* firstSpriteIdJson = fileJson->find("firstspriteid");
						JSON_VALUE* lastSpriteIdJson = fileJson->find("lastspriteid");
						if(spriteTypeJson && firstSpriteIdJson && lastSpriteIdJson && spriteTypeJson->IsNumber() && firstSpriteIdJson->IsNumber() && lastSpriteIdJson->IsNumber())
						{
							Uint32 spriteIndex = startSpriteIndex;
							Uint32 spriteType = SDL_static_cast(Uint32, spriteTypeJson->AsNumber());
							Uint32 firstSpriteId = SDL_static_cast(Uint32, firstSpriteIdJson->AsNumber());
							Uint32 lastSpriteId = SDL_static_cast(Uint32, lastSpriteIdJson->AsNumber());
							for(Uint32 j = 0; j <= (lastSpriteId - firstSpriteId); ++j)
							{
								Uint32 spriteOffset = firstSpriteId + j;
								if(spriteOffset >= m_spriteOffsets.size())
									m_spriteOffsets.resize(lastSpriteId + j + 1);

								SpriteOffset& sprOffset = m_spriteOffsets[spriteOffset];
								sprOffset.spriteIndex = startSpriteIndex;
								sprOffset.spriteType = spriteType;
								switch(spriteType)
								{
									case 0: startSpriteIndex += 1; break;
									case 1: startSpriteIndex += 2; break;
									case 2: startSpriteIndex += 2; break;
									case 3: startSpriteIndex += 4; break;
									default: break;
								}
							}

							m_spriteSheets.emplace_back(fileName->AsString(), spriteIndex, startSpriteIndex - 1, spriteType);
						}
					}
				}
				else
					return false;
			}
			else
				return false;
		}
	}
	else
		return false;

	m_spriteSheets.shrink_to_fit();
	m_spriteOffsets.shrink_to_fit();

	g_sprRevision = 0x73434654;
	g_spriteCounts = startSpriteIndex - 1;
	m_sprLoaded = true;
	return true;
}
