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

#include "GUI_UTIL.h"
#include "../engine.h"
#include "../GUI_Elements/GUI_Window.h"
#include "../GUI_Elements/GUI_Button.h"
#include "../GUI_Elements/GUI_CheckBox.h"
#include "../GUI_Elements/GUI_Label.h"
#include "../GUI_Elements/GUI_Separator.h"
#include "../GUI_Elements/GUI_TextBox.h"
#include "../GUI_Elements/GUI_ScrollBar.h"
#include "../game.h"
#include "itemUI.h"

#define AUTOLOOT_TITLE "Auto Loot"
#define AUTOLOOT_WIDTH 390
#define AUTOLOOT_HEIGHT 352
#define AUTOLOOT_CLOSE_EVENTID 1000
#define AUTOLOOT_ADD_EVENTID 1001
#define AUTOLOOT_ON_EVENTID 1002
#define AUTOLOOT_FALLBACK_EVENTID 1003
#define AUTOLOOT_ITEM_EVENTID 1004
#define AUTOLOOT_BACKPACK_EVENTID 1005
#define AUTOLOOT_LIST_EVENTID 1006
#define AUTOLOOT_ROW_HEIGHT 36

extern Engine g_engine;
extern Game g_game;

// A text box that shows a gray hint (e.g. "<item name>") while it is empty,
// until it is clicked to type.
class GUI_HintTextBox : public GUI_TextBox
{
	public:
		GUI_HintTextBox(iRect boxRect, const std::string& hint, Uint32 internalID = 0) : GUI_TextBox(boxRect, "", internalID), m_hint(hint) {}

		void onLMouseDown(Sint32 x, Sint32 y)
		{
			m_clicked = true;
			GUI_TextBox::onLMouseDown(x, y);
		}

		void render()
		{
			GUI_TextBox::render();
			if(getActualText().empty() && !(m_clicked && isActive()))
				g_engine.drawFont(CLIENT_FONT_NONOUTLINED, m_tRect.x1 + 4, m_tRect.y1 + 3, m_hint, 140, 140, 140, CLIENT_FONT_ALIGN_LEFT);
			if(!isActive())
				m_clicked = false;
		}

	protected:
		std::string m_hint;
		bool m_clicked = false;
};

// The list of auto loot items: item -> backpack, with a button to remove it.
class GUI_AutoLootList : public GUI_Element
{
	public:
		GUI_AutoLootList(iRect boxRect, Uint32 internalID = 0) : m_scrollBar(iRect(0, 0, 0, 0), 0, 0)
		{
			setRect(boxRect);
			m_internalID = internalID;
			refresh();
		}
		~GUI_AutoLootList() {clearItems();}

		// non-copyable
		GUI_AutoLootList(const GUI_AutoLootList&) = delete;
		GUI_AutoLootList& operator=(const GUI_AutoLootList&) = delete;

		// non-moveable
		GUI_AutoLootList(GUI_AutoLootList&&) = delete;
		GUI_AutoLootList& operator=(GUI_AutoLootList&&) = delete;

		void setRect(iRect& NewRect)
		{
			if(m_tRect == NewRect)
				return;

			iRect nRect = iRect(NewRect.x1 + NewRect.x2 - 13, NewRect.y1 + 1, 12, NewRect.y2 - 2);
			m_scrollBar.setRect(nRect);
			m_tRect = NewRect;
			m_maxDisplay = (m_tRect.y2 - 4) / AUTOLOOT_ROW_HEIGHT;
			m_scrollBar.setScrollSize(SDL_static_cast(Sint32, m_items.size()) - m_maxDisplay);
		}

		void refresh()
		{
			clearItems();
			for(AutoLootEntry& entry : g_game.getAutoLootEntries())
			{
				// Stackables are shown as a pile (e.g. gold coins), without a number.
				ItemUI* item = (entry.itemId ? ItemUI::createItemUI(entry.itemId, 100) : NULL);
				if(item)
					item->setSubtype(100, false);
				m_items.push_back(item);
				m_backpacks.push_back(entry.backpackId ? ItemUI::createItemUI(entry.backpackId, 1) : NULL);
			}
			m_scrollBar.setScrollSize(SDL_static_cast(Sint32, m_items.size()) - m_maxDisplay);
		}

		void onLMouseDown(Sint32 x, Sint32 y)
		{
			if(m_scrollBar.getRect().isPointInside(x, y))
			{
				m_scrollBar.onLMouseDown(x, y);
				return;
			}

			Sint32 index = m_scrollBar.getScrollPos();
			Sint32 posY = m_tRect.y1 + 2;
			std::vector<AutoLootEntry>& entries = g_game.getAutoLootEntries();
			for(; index < SDL_static_cast(Sint32, entries.size()) && posY + AUTOLOOT_ROW_HEIGHT <= m_tRect.y1 + m_tRect.y2; ++index)
			{
				iRect removeRect = getRemoveRect(posY);
				if(removeRect.isPointInside(x, y))
				{
					g_game.sendAutoLootRemove(SDL_static_cast(Uint8, index));
					return;
				}
				posY += AUTOLOOT_ROW_HEIGHT;
			}
		}

		void onLMouseUp(Sint32 x, Sint32 y) {m_scrollBar.onLMouseUp(x, y);}
		void onMouseMove(Sint32 x, Sint32 y, bool isInsideParent) {m_scrollBar.onMouseMove(x, y, isInsideParent);}
		void onWheel(Sint32, Sint32, bool wheelUP) {m_scrollBar.setScrollPos(m_scrollBar.getScrollPos() + (wheelUP ? -1 : 1));}

		void render()
		{
			auto& renderer = g_engine.getRender();
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_DARK_X, GUI_UI_ICON_HORIZONTAL_LINE_DARK_Y, GUI_UI_ICON_HORIZONTAL_LINE_DARK_W, GUI_UI_ICON_HORIZONTAL_LINE_DARK_H, m_tRect.x1, m_tRect.y1, m_tRect.x2, 1);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_DARK_X, GUI_UI_ICON_VERTICAL_LINE_DARK_Y, GUI_UI_ICON_VERTICAL_LINE_DARK_W, GUI_UI_ICON_VERTICAL_LINE_DARK_H, m_tRect.x1, m_tRect.y1 + 1, 1, m_tRect.y2 - 1);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_X, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_Y, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_W, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_H, m_tRect.x1 + 1, m_tRect.y1 + m_tRect.y2 - 1, m_tRect.x2 - 1, 1);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_X, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_Y, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_W, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_H, m_tRect.x1 + m_tRect.x2 - 1, m_tRect.y1 + 1, 1, m_tRect.y2 - 2);
			renderer->fillRectangle(m_tRect.x1 + 1, m_tRect.y1 + 1, m_tRect.x2 - 14, m_tRect.y2 - 2, 64, 64, 64, 255);
			renderer->setClipRect(m_tRect.x1 + 2, m_tRect.y1 + 2, m_tRect.x2 - 16, m_tRect.y2 - 4);

			std::vector<AutoLootEntry>& entries = g_game.getAutoLootEntries();
			Sint32 index = m_scrollBar.getScrollPos();
			Sint32 posY = m_tRect.y1 + 2;
			Sint32 rowWidth = m_tRect.x2 - 16;
			for(; index < SDL_static_cast(Sint32, entries.size()) && posY < m_tRect.y1 + m_tRect.y2; ++index)
			{
				AutoLootEntry& entry = entries[index];
				if(index % 2)
					renderer->fillRectangle(m_tRect.x1 + 2, posY, rowWidth, AUTOLOOT_ROW_HEIGHT, 72, 72, 72, 255);

				Sint32 x = m_tRect.x1 + 4;
				renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_INVENTORY_EMPTY_X, GUI_UI_INVENTORY_EMPTY_Y, x, posY + 1, 34, 34);
				if(SDL_static_cast(size_t, index) < m_items.size() && m_items[index])
					m_items[index]->render(x + 1, posY + 2, 32);
				g_engine.drawFont(CLIENT_FONT_NONOUTLINED, x + 40, posY + 13, entry.itemName, 175, 175, 175, CLIENT_FONT_ALIGN_LEFT);

				x = m_tRect.x1 + 4 + (rowWidth - 24) / 2;
				g_engine.drawFont(CLIENT_FONT_NONOUTLINED, x - 14, posY + 13, "->", 175, 175, 175, CLIENT_FONT_ALIGN_LEFT);
				renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_INVENTORY_EMPTY_X, GUI_UI_INVENTORY_EMPTY_Y, x, posY + 1, 34, 34);
				if(SDL_static_cast(size_t, index) < m_backpacks.size() && m_backpacks[index])
					m_backpacks[index]->render(x + 1, posY + 2, 32);
				g_engine.drawFont(CLIENT_FONT_NONOUTLINED, x + 40, posY + 13, entry.backpackName, 175, 175, 175, CLIENT_FONT_ALIGN_LEFT);

				iRect removeRect = getRemoveRect(posY);
				renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_CLOSE_CHANNEL_UP_X, GUI_UI_ICON_CLOSE_CHANNEL_UP_Y, removeRect.x1, removeRect.y1, removeRect.x2, removeRect.y2);
				posY += AUTOLOOT_ROW_HEIGHT;
			}

			if(entries.empty())
				g_engine.drawFont(CLIENT_FONT_NONOUTLINED, m_tRect.x1 + 6, m_tRect.y1 + 6, "Add an item name and a backpack name above.", 145, 145, 145, CLIENT_FONT_ALIGN_LEFT);

			renderer->disableClipRect();
			m_scrollBar.render();
		}

	protected:
		iRect getRemoveRect(Sint32 posY)
		{
			return iRect(m_tRect.x1 + m_tRect.x2 - 16 - GUI_UI_ICON_CLOSE_CHANNEL_UP_W - 6, posY + (AUTOLOOT_ROW_HEIGHT - GUI_UI_ICON_CLOSE_CHANNEL_UP_H) / 2, GUI_UI_ICON_CLOSE_CHANNEL_UP_W, GUI_UI_ICON_CLOSE_CHANNEL_UP_H);
		}

		void clearItems()
		{
			for(ItemUI* item : m_items)
				delete item;
			for(ItemUI* item : m_backpacks)
				delete item;
			m_items.clear();
			m_backpacks.clear();
		}

		std::vector<ItemUI*> m_items;
		std::vector<ItemUI*> m_backpacks;
		GUI_VScrollBar m_scrollBar;
		Sint32 m_maxDisplay = 0;
};

void autoloot_Events(Uint32 event, Sint32 status)
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_AUTOLOOT);
	switch(event)
	{
		case AUTOLOOT_CLOSE_EVENTID:
		{
			if(pWindow)
				g_engine.removeWindow(pWindow);
		}
		break;
		case AUTOLOOT_ADD_EVENTID:
		{
			if(!pWindow)
				break;

			GUI_TextBox* pItem = SDL_static_cast(GUI_TextBox*, pWindow->getChild(AUTOLOOT_ITEM_EVENTID));
			GUI_TextBox* pBackpack = SDL_static_cast(GUI_TextBox*, pWindow->getChild(AUTOLOOT_BACKPACK_EVENTID));
			if(!pItem || !pBackpack)
				break;

			if(pItem->getActualText().empty() || pBackpack->getActualText().empty())
				break;

			g_game.sendAutoLootAdd(pItem->getActualText(), pBackpack->getActualText());
			pItem->setText("");
		}
		break;
		// The check boxes also call this when the server list updates them, so
		// only a change is sent.
		case AUTOLOOT_ON_EVENTID:
			if((status != 0) != g_game.isAutoLootOn())
				g_game.sendAutoLootSettings(status != 0, g_game.isAutoLootFallback());
			break;
		case AUTOLOOT_FALLBACK_EVENTID:
			if((status != 0) != g_game.isAutoLootFallback())
				g_game.sendAutoLootSettings(g_game.isAutoLootOn(), status != 0);
			break;
		default: break;
	}
}

void UTIL_toggleAutoLootWindow()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_AUTOLOOT);
	if(pWindow)
	{
		g_engine.removeWindow(pWindow);
		return;
	}

	GUI_Window* newWindow = new GUI_Window(iRect(0, 0, AUTOLOOT_WIDTH, AUTOLOOT_HEIGHT), AUTOLOOT_TITLE, GUI_WINDOW_AUTOLOOT);
	GUI_CheckBox* newCheckBox = new GUI_CheckBox(iRect(16, 32, AUTOLOOT_WIDTH - 32, 22), "Auto Loot on", g_game.isAutoLootOn(), AUTOLOOT_ON_EVENTID);
	newCheckBox->setBoxEventCallback(&autoloot_Events, AUTOLOOT_ON_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);

	GUI_TextBox* newTextBox = new GUI_HintTextBox(iRect(16, 62, 134, 16), "<item name>", AUTOLOOT_ITEM_EVENTID);
	newTextBox->setMaxLength(40);
	newTextBox->startEvents();
	newWindow->addChild(newTextBox);
	newTextBox = new GUI_HintTextBox(iRect(156, 62, 124, 16), "<backpack name>", AUTOLOOT_BACKPACK_EVENTID);
	newTextBox->setMaxLength(40);
	newTextBox->startEvents();
	newWindow->addChild(newTextBox);
	// The 75px button is not in the 8.6 Tibia.pic, the 86px one is.
	GUI_Button* newButton = new GUI_Button(iRect(AUTOLOOT_WIDTH - 16 - GUI_UI_BUTTON_86PX_GRAY_UP_W, 60, GUI_UI_BUTTON_86PX_GRAY_UP_W, GUI_UI_BUTTON_86PX_GRAY_UP_H), "Add Item", CLIENT_GUI_ENTER_TRIGGER);
	newButton->setButtonEventCallback(&autoloot_Events, AUTOLOOT_ADD_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);

	GUI_AutoLootList* newList = new GUI_AutoLootList(iRect(16, 88, AUTOLOOT_WIDTH - 32, 4 + AUTOLOOT_ROW_HEIGHT * 5), AUTOLOOT_LIST_EVENTID);
	newList->startEvents();
	newWindow->addChild(newList);

	newCheckBox = new GUI_CheckBox(iRect(16, AUTOLOOT_HEIGHT - 70, AUTOLOOT_WIDTH - 32, 22), "Use main backpack as fallback", g_game.isAutoLootFallback(), AUTOLOOT_FALLBACK_EVENTID);
	newCheckBox->setBoxEventCallback(&autoloot_Events, AUTOLOOT_FALLBACK_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);

	newButton = new GUI_Button(iRect(AUTOLOOT_WIDTH - 56, AUTOLOOT_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Close", CLIENT_GUI_ESCAPE_TRIGGER);
	newButton->setButtonEventCallback(&autoloot_Events, AUTOLOOT_CLOSE_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	GUI_Separator* newSeparator = new GUI_Separator(iRect(13, AUTOLOOT_HEIGHT - 40, AUTOLOOT_WIDTH - 26, 2));
	newWindow->addChild(newSeparator);
	g_engine.addWindow(newWindow);

	// The list comes from the server.
	g_game.sendAutoLootRequest();
}

void UTIL_refreshAutoLootWindow()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_AUTOLOOT);
	if(!pWindow)
		return;

	GUI_CheckBox* pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(AUTOLOOT_ON_EVENTID));
	if(pCheckBox && pCheckBox->isChecked() != g_game.isAutoLootOn())
		pCheckBox->setChecked(g_game.isAutoLootOn());
	pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(AUTOLOOT_FALLBACK_EVENTID));
	if(pCheckBox && pCheckBox->isChecked() != g_game.isAutoLootFallback())
		pCheckBox->setChecked(g_game.isAutoLootFallback());

	GUI_AutoLootList* pList = SDL_static_cast(GUI_AutoLootList*, pWindow->getChild(AUTOLOOT_LIST_EVENTID));
	if(pList)
		pList->refresh();
}
