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
#include "../config.h"
#include "../GUI_Elements/GUI_Window.h"
#include "../GUI_Elements/GUI_Panel.h"
#include "../GUI_Elements/GUI_PanelWindow.h"
#include "../GUI_Elements/GUI_Container.h"
#include "../GUI_Elements/GUI_Button.h"
#include "../GUI_Elements/GUI_Label.h"
#include "../GUI_Elements/GUI_StaticImage.h"
#include "../GUI_Elements/GUI_Separator.h"
#include "../GUI_Elements/GUI_Icon.h"
#include "../GUI_Elements/GUI_TextBox.h"
#include "../game.h"
#include "../map.h"
#include "../creature.h"
#include "../container.h"
#include "itemUI.h"

#define TRAINER_TITLE "Trainer"
#define TRAINER_MAXIMINI_EVENTID 1000
#define TRAINER_CLOSE_EVENTID 1001
#define TRAINER_RESIZE_WIDTH_EVENTID 1002
#define TRAINER_RESIZE_HEIGHT_EVENTID 1003
#define TRAINER_EXIT_WINDOW_EVENTID 1004
#define TRAINER_CONTAINER_EVENTID 1005
#define TRAINER_CONTENT_HEIGHT 190

#define TRAINER_SETTINGS_TITLE "Trainer Settings"
#define TRAINER_SETTINGS_WIDTH 260
#define TRAINER_SETTINGS_HEIGHT 230
#define TRAINER_SETTINGS_CANCEL_EVENTID 1100
#define TRAINER_SETTINGS_OK_EVENTID 1101
#define TRAINER_SETTINGS_MANA_SPELL_EVENTID 3000
#define TRAINER_SETTINGS_MANA_MANA_EVENTID 3001
#define TRAINER_SETTINGS_RUNE_SPELL_EVENTID 3002
#define TRAINER_SETTINGS_RUNE_MANA_EVENTID 3003

#define TRAINER_CAST_DELAY 2000
#define TRAINER_EAT_DELAY 5000
#define TRAINER_EAT_BELOW_SECONDS 600
#define TRAINER_ANTI_IDLE_DELAY 300000
#define TRAINER_BLANK_RUNE_ID 3147

extern Engine g_engine;
extern Game g_game;
extern Map g_map;
extern Uint32 g_frameTime;

bool g_haveTrainerOpen = false;

namespace
{
	struct TrainerSettings
	{
		bool manaTraining = false;
		std::string manaSpell = "utevo lux";
		Uint32 manaMana = 100;
		bool runeMaker = false;
		std::string runeSpell = "adori gran";
		Uint32 runeMana = 100;
		bool antiIdle = false;
		bool autoEat = false;
	};

	TrainerSettings g_trainer;
	bool g_trainerOn = false;
	Uint32 g_trainerNextCast = 0;
	Uint32 g_trainerNextEat = 0;
	Uint32 g_trainerNextAntiIdle = 0;

	//Food objects of the 7.72 objects.srv (Flags = {Food,...})
	const Uint16 g_trainerFoods[] = {3577, 3578, 3579, 3580, 3581, 3582, 3583, 3584, 3585, 3586, 3587, 3588, 3589, 3590, 3591, 3592, 3593, 3594, 3595, 3596, 3597, 3598, 3599, 3600, 3601, 3602, 3606, 3607, 3723, 3724, 3725, 3726, 3727, 3728, 3729, 3730, 3731, 3732};
}

class GUI_TrainerToggle : public GUI_Element
{
	public:
		GUI_TrainerToggle(iRect boxRect) {setRect(boxRect);}

		void onLMouseDown(Sint32, Sint32) {m_pressed = true;}
		void onLMouseUp(Sint32 x, Sint32 y)
		{
			if(m_pressed && m_tRect.isPointInside(x, y))
			{
				g_trainerOn = !g_trainerOn;
				g_trainerNextCast = g_trainerNextEat = g_frameTime;
				g_trainerNextAntiIdle = g_frameTime + TRAINER_ANTI_IDLE_DELAY;
			}
			m_pressed = false;
		}

		void render()
		{
			auto& renderer = g_engine.getRender();
			Uint8 red = (g_trainerOn ? 40 : 120), green = (g_trainerOn ? 100 : 30), blue = (g_trainerOn ? 40 : 30);
			renderer->fillRectangle(m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2, red, green, blue, 255);
			if(m_pressed)
			{
				renderer->drawRectangle(m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2, 1, 30, 30, 30, 255);
				renderer->drawRectangle(m_tRect.x1 + 1, m_tRect.y1 + 1, m_tRect.x2 - 2, m_tRect.y2 - 2, 1, 70, 70, 70, 255);
			}
			else
			{
				renderer->drawRectangle(m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2, 1, 30, 30, 30, 255);
				renderer->drawRectangle(m_tRect.x1 + 1, m_tRect.y1 + 1, m_tRect.x2 - 2, m_tRect.y2 - 2, 1, 160, 160, 160, 255);
			}
			Sint32 offset = (m_pressed ? 1 : 0);
			g_engine.drawFont(CLIENT_FONT_NONOUTLINED, m_tRect.x1 + m_tRect.x2 / 2 + offset, m_tRect.y1 + (m_tRect.y2 - 10) / 2 + offset, (g_trainerOn ? "TRAINER ON" : "TRAINER OFF"), 255, 255, 255, CLIENT_FONT_ALIGN_CENTER);
		}

	protected:
		bool m_pressed = false;
};

class GUI_TrainerCheck : public GUI_Element
{
	public:
		GUI_TrainerCheck(iRect boxRect, const std::string& label, bool* value) : m_label(label), m_value(value) {setRect(boxRect);}

		void onLMouseUp(Sint32 x, Sint32 y)
		{
			if(m_tRect.isPointInside(x, y))
			{
				*m_value = !(*m_value);
				g_engine.saveCFG();
			}
		}

		void render()
		{
			auto& renderer = g_engine.getRender();
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_CHECKBOX_UNCHECKED_X, ((*m_value) ? GUI_UI_CHECKBOX_CHECKED_Y : GUI_UI_CHECKBOX_UNCHECKED_Y), m_tRect.x1, m_tRect.y1 + 1, GUI_UI_CHECKBOX_CHECKED_W, GUI_UI_CHECKBOX_CHECKED_H);
			g_engine.drawFont(CLIENT_FONT_NONOUTLINED, m_tRect.x1 + 18, m_tRect.y1 + 2, m_label, 180, 180, 180, CLIENT_FONT_ALIGN_LEFT);
		}

	protected:
		std::string m_label;
		bool* m_value;
};

//Shows a value like a textbox, clicking it opens the trainer settings
//(windows in the side panels don't get keyboard input)
class GUI_TrainerField : public GUI_Element
{
	public:
		GUI_TrainerField(iRect boxRect, std::string* text, Uint32* number) : m_text(text), m_number(number) {setRect(boxRect);}

		void onLMouseUp(Sint32 x, Sint32 y)
		{
			if(m_tRect.isPointInside(x, y))
				UTIL_trainerSettings();
		}

		void render()
		{
			auto& renderer = g_engine.getRender();
			renderer->fillRectangle(m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2, 54, 54, 54, 255);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_DARK_X, GUI_UI_ICON_HORIZONTAL_LINE_DARK_Y, GUI_UI_ICON_HORIZONTAL_LINE_DARK_W, GUI_UI_ICON_HORIZONTAL_LINE_DARK_H, m_tRect.x1, m_tRect.y1, m_tRect.x2, 1);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_DARK_X, GUI_UI_ICON_VERTICAL_LINE_DARK_Y, GUI_UI_ICON_VERTICAL_LINE_DARK_W, GUI_UI_ICON_VERTICAL_LINE_DARK_H, m_tRect.x1, m_tRect.y1 + 1, 1, m_tRect.y2 - 1);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_X, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_Y, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_W, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_H, m_tRect.x1 + 1, m_tRect.y1 + m_tRect.y2 - 1, m_tRect.x2 - 1, 1);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_X, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_Y, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_W, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_H, m_tRect.x1 + m_tRect.x2 - 1, m_tRect.y1 + 1, 1, m_tRect.y2 - 2);
			if(m_text)
				g_engine.drawFont(CLIENT_FONT_NONOUTLINED, m_tRect.x1 + 4, m_tRect.y1 + 3, *m_text, 180, 180, 180, CLIENT_FONT_ALIGN_LEFT);
			else if(m_number)
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", *m_number);
				g_engine.drawFont(CLIENT_FONT_NONOUTLINED, m_tRect.x1 + 4, m_tRect.y1 + 3, std::string(g_buffer, SDL_static_cast(size_t, len)), 180, 180, 180, CLIENT_FONT_ALIGN_LEFT);
			}
		}

	protected:
		std::string* m_text;
		Uint32* m_number;
};

void trainer_Events(Uint32 event, Sint32 status)
{
	switch(event)
	{
		case TRAINER_MAXIMINI_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_TRAINER);
			if(pPanel)
			{
				GUI_Panel* parent = pPanel->getParent();
				if(!parent)
					break;

				iRect& pRect = pPanel->getOriginalRect();
				GUI_Container* pContainer = SDL_static_cast(GUI_Container*, pPanel->getChild(TRAINER_CONTAINER_EVENTID));
				if(pRect.y2 > 19)
				{
					pPanel->setCachedHeight(pRect.y2);
					pPanel->setSize(pRect.x2, 19);
					parent->checkPanels();
					if(pContainer)
						pContainer->makeInvisible();
				}
				else
				{
					Sint32 cachedHeight = pPanel->getCachedHeight();
					parent->tryFreeHeight(cachedHeight - pRect.y2);
					pPanel->setSize(pRect.x2, cachedHeight);
					parent->checkPanels();
					if(pContainer)
						pContainer->makeVisible();
				}
			}
		}
		break;
		case TRAINER_CLOSE_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_TRAINER);
			if(pPanel)
			{
				GUI_Panel* parent = pPanel->getParent();
				if(parent)
					parent->removePanel(pPanel);
			}
		}
		break;
		case TRAINER_RESIZE_HEIGHT_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_TRAINER);
			if(pPanel)
			{
				GUI_Container* pContainer = SDL_static_cast(GUI_Container*, pPanel->getChild(TRAINER_CONTAINER_EVENTID));
				if(pContainer)
				{
					iRect cRect = pContainer->getRect();
					cRect.y2 = status - 19;
					pContainer->setRect(cRect);
				}
			}
		}
		break;
		case TRAINER_EXIT_WINDOW_EVENTID: g_haveTrainerOpen = false; break;
		case TRAINER_SETTINGS_CANCEL_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_TRAINER_SETTINGS)
				g_engine.removeWindow(pWindow);
		}
		break;
		case TRAINER_SETTINGS_OK_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_TRAINER_SETTINGS)
			{
				GUI_TextBox* pTextBox = SDL_static_cast(GUI_TextBox*, pWindow->getChild(TRAINER_SETTINGS_MANA_SPELL_EVENTID));
				if(pTextBox)
					g_trainer.manaSpell = pTextBox->getActualText();
				pTextBox = SDL_static_cast(GUI_TextBox*, pWindow->getChild(TRAINER_SETTINGS_MANA_MANA_EVENTID));
				if(pTextBox)
					g_trainer.manaMana = SDL_static_cast(Uint32, SDL_strtoul(pTextBox->getActualText().c_str(), NULL, 10));
				pTextBox = SDL_static_cast(GUI_TextBox*, pWindow->getChild(TRAINER_SETTINGS_RUNE_SPELL_EVENTID));
				if(pTextBox)
					g_trainer.runeSpell = pTextBox->getActualText();
				pTextBox = SDL_static_cast(GUI_TextBox*, pWindow->getChild(TRAINER_SETTINGS_RUNE_MANA_EVENTID));
				if(pTextBox)
					g_trainer.runeMana = SDL_static_cast(Uint32, SDL_strtoul(pTextBox->getActualText().c_str(), NULL, 10));
				g_engine.removeWindow(pWindow);
				g_engine.saveCFG();
			}
		}
		break;
		default: break;
	}
}

void UTIL_trainerSettings()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_TRAINER_SETTINGS);
	if(pWindow)
		g_engine.removeWindow(pWindow);

	GUI_Window* newWindow = new GUI_Window(iRect(0, 0, TRAINER_SETTINGS_WIDTH, TRAINER_SETTINGS_HEIGHT), TRAINER_SETTINGS_TITLE, GUI_WINDOW_TRAINER_SETTINGS);
	const char* labels[4] = {"Mana Training spell:", "Mana Training at mana:", "Rune Maker spell:", "Rune Maker at mana:"};
	Uint32 ids[4] = {TRAINER_SETTINGS_MANA_SPELL_EVENTID, TRAINER_SETTINGS_MANA_MANA_EVENTID, TRAINER_SETTINGS_RUNE_SPELL_EVENTID, TRAINER_SETTINGS_RUNE_MANA_EVENTID};
	std::string values[4];
	values[0] = g_trainer.manaSpell;
	values[1] = std::to_string(g_trainer.manaMana);
	values[2] = g_trainer.runeSpell;
	values[3] = std::to_string(g_trainer.runeMana);
	for(Sint32 i = 0; i < 4; ++i)
	{
		GUI_Label* newLabel = new GUI_Label(iRect(18, 32 + i * 38, 0, 0), labels[i]);
		newWindow->addChild(newLabel);
		GUI_TextBox* newTextBox = new GUI_TextBox(iRect(16, 46 + i * 38, TRAINER_SETTINGS_WIDTH - 32, 16), values[i], ids[i]);
		newTextBox->setMaxLength((i % 2) == 0 ? 64 : 5);
		newTextBox->startEvents();
		newWindow->addChild(newTextBox);
	}
	GUI_Button* newButton = new GUI_Button(iRect(TRAINER_SETTINGS_WIDTH - 56, TRAINER_SETTINGS_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Cancel", CLIENT_GUI_ESCAPE_TRIGGER);
	newButton->setButtonEventCallback(&trainer_Events, TRAINER_SETTINGS_CANCEL_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(TRAINER_SETTINGS_WIDTH - 109, TRAINER_SETTINGS_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Ok", CLIENT_GUI_ENTER_TRIGGER);
	newButton->setButtonEventCallback(&trainer_Events, TRAINER_SETTINGS_OK_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	GUI_Separator* newSeparator = new GUI_Separator(iRect(13, TRAINER_SETTINGS_HEIGHT - 40, TRAINER_SETTINGS_WIDTH - 26, 2));
	newWindow->addChild(newSeparator);
	g_engine.addWindow(newWindow);
}

void UTIL_toggleTrainerWindow()
{
	GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_TRAINER);
	if(pPanel)
	{
		g_engine.removePanelWindow(pPanel);
		return;
	}

	//The trainer window has a fixed size: everything fits without scrolling
	GUI_PanelWindow* newWindow = new GUI_PanelWindow(iRect(0, 0, GAME_PANEL_FIXED_WIDTH - 4, TRAINER_CONTENT_HEIGHT + 19), true, GUI_PANEL_WINDOW_TRAINER, true);
	newWindow->setEventCallback(&trainer_Events, TRAINER_RESIZE_WIDTH_EVENTID, TRAINER_RESIZE_HEIGHT_EVENTID, TRAINER_EXIT_WINDOW_EVENTID);
	newWindow->setMinHeight(TRAINER_CONTENT_HEIGHT + 19);
	newWindow->setMaxHeight(TRAINER_CONTENT_HEIGHT + 19);
	GUI_StaticImage* newImage = new GUI_StaticImage(iRect(2, 0, GUI_UI_ICON_BATTLELIST_W, GUI_UI_ICON_BATTLELIST_H), GUI_UI_IMAGE, GUI_UI_ICON_BATTLELIST_X, GUI_UI_ICON_BATTLELIST_Y);
	newWindow->addChild(newImage);
	GUI_Icon* newIcon = new GUI_Icon(iRect(147, 0, GUI_UI_ICON_MINIMIZE_WINDOW_UP_W, GUI_UI_ICON_MINIMIZE_WINDOW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MINIMIZE_WINDOW_UP_X, GUI_UI_ICON_MINIMIZE_WINDOW_UP_Y, GUI_UI_ICON_MINIMIZE_WINDOW_DOWN_X, GUI_UI_ICON_MINIMIZE_WINDOW_DOWN_Y, TRAINER_MAXIMINI_EVENTID, "Maximise or minimise window");
	newIcon->setButtonEventCallback(&trainer_Events, TRAINER_MAXIMINI_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(159, 0, GUI_UI_ICON_CLOSE_WINDOW_UP_W, GUI_UI_ICON_CLOSE_WINDOW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_CLOSE_WINDOW_UP_X, GUI_UI_ICON_CLOSE_WINDOW_UP_Y, GUI_UI_ICON_CLOSE_WINDOW_DOWN_X, GUI_UI_ICON_CLOSE_WINDOW_DOWN_Y, 0, "Close this window");
	newIcon->setButtonEventCallback(&trainer_Events, TRAINER_CLOSE_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	GUI_DynamicLabel* newLabel = new GUI_DynamicLabel(iRect(19, 2, 100, 14), TRAINER_TITLE, 0, 144, 144, 144);
	newWindow->addChild(newLabel);

	GUI_Container* newContainer = new GUI_Container(iRect(2, 13, 168, TRAINER_CONTENT_HEIGHT), newWindow, TRAINER_CONTAINER_EVENTID);
	Sint32 posY = -4;
	GUI_Element* element = new GUI_TrainerToggle(iRect(5, posY, 146, 26));
	element->startEvents();
	newContainer->addChild(element);
	posY += 34;
	element = new GUI_TrainerCheck(iRect(5, posY, 146, 14), "Mana Training", &g_trainer.manaTraining);
	element->startEvents();
	newContainer->addChild(element);
	posY += 17;
	element = new GUI_TrainerField(iRect(5, posY, 146, 16), &g_trainer.manaSpell, NULL);
	element->startEvents();
	newContainer->addChild(element);
	posY += 18;
	element = new GUI_TrainerField(iRect(5, posY, 146, 16), NULL, &g_trainer.manaMana);
	element->startEvents();
	newContainer->addChild(element);
	posY += 24;
	element = new GUI_TrainerCheck(iRect(5, posY, 146, 14), "Rune Maker", &g_trainer.runeMaker);
	element->startEvents();
	newContainer->addChild(element);
	posY += 17;
	element = new GUI_TrainerField(iRect(5, posY, 146, 16), &g_trainer.runeSpell, NULL);
	element->startEvents();
	newContainer->addChild(element);
	posY += 18;
	element = new GUI_TrainerField(iRect(5, posY, 146, 16), NULL, &g_trainer.runeMana);
	element->startEvents();
	newContainer->addChild(element);
	posY += 24;
	element = new GUI_TrainerCheck(iRect(5, posY, 146, 14), "Anti-Idle", &g_trainer.antiIdle);
	element->startEvents();
	newContainer->addChild(element);
	posY += 18;
	element = new GUI_TrainerCheck(iRect(5, posY, 146, 14), "Auto-Eat Food", &g_trainer.autoEat);
	element->startEvents();
	newContainer->addChild(element);
	newContainer->startEvents();
	newWindow->addChild(newContainer);

	Sint32 preferredPanel = g_engine.getContentWindowParent(GUI_PANEL_WINDOW_TRAINER);
	bool added = g_engine.addToPanel(newWindow, preferredPanel);
	if(!added && preferredPanel != GUI_PANEL_RANDOM)
		added = g_engine.addToPanel(newWindow, GUI_PANEL_RANDOM);

	g_haveTrainerOpen = added;
	if(!added)
		delete newWindow;
}

static bool trainerHasBlankRuneInHand()
{
	for(Uint8 slot : {SDL_static_cast(Uint8, SLOT_LEFT), SDL_static_cast(Uint8, SLOT_RIGHT)})
	{
		ItemUI* item = g_game.getInventoryItem(slot);
		if(item && item->getID() == TRAINER_BLANK_RUNE_ID)
			return true;
	}
	return false;
}

static Uint16 trainerFindFood()
{
	auto isFood = [](ItemUI* item) -> bool
	{
		if(!item)
			return false;

		Uint16 itemId = item->getID();
		for(Uint16 foodId : g_trainerFoods)
		{
			if(foodId == itemId)
				return true;
		}
		return false;
	};

	for(Uint8 slot = SLOT_HEAD; slot < SLOT_LAST; ++slot)
	{
		ItemUI* item = g_game.getInventoryItem(slot);
		if(isFood(item))
			return item->getID();
	}

	for(Uint8 cid = 0; cid < GAME_MAX_CONTAINERS; ++cid)
	{
		Container* container = g_game.findContainer(cid);
		if(!container)
			continue;

		for(ItemUI* item : container->getItems())
		{
			if(isFood(item))
				return item->getID();
		}
	}
	return 0;
}

void UTIL_trainerThink()
{
	if(!g_trainerOn || !g_engine.isIngame())
		return;

	if(g_trainer.antiIdle && g_frameTime >= g_trainerNextAntiIdle)
	{
		//Turning counts as an action for the server's idle check
		Creature* localCreature = g_map.getLocalCreature();
		if(localCreature)
			g_game.sendTurn(localCreature->getDirection());
		g_trainerNextAntiIdle = g_frameTime + TRAINER_ANTI_IDLE_DELAY;
	}

	if(g_trainer.autoEat && g_frameTime >= g_trainerNextEat)
	{
		g_trainerNextEat = g_frameTime + TRAINER_EAT_DELAY;
		bool hungry = true;
		if(g_game.hasGameFeature(GAME_FEATURE_REGENERATION_TIME))
			hungry = (g_game.getPlayerRegeneration() < TRAINER_EAT_BELOW_SECONDS);
		else
			g_trainerNextEat = g_frameTime + 60000;

		if(hungry)
		{
			//The server looks for the food anywhere in the inventory (hotkey style)
			Uint16 foodId = trainerFindFood();
			if(foodId != 0)
				g_game.sendUseItem(Position(0xFFFF, 0, 0), foodId, 0, 0);
		}
	}

	if(g_frameTime < g_trainerNextCast)
		return;

	Uint32 mana = g_game.getPlayerMana();
	if(g_trainer.runeMaker && !g_trainer.runeSpell.empty() && mana >= g_trainer.runeMana && trainerHasBlankRuneInHand())
	{
		g_game.sendSay(MessageSay, 0, std::string(), g_trainer.runeSpell);
		g_trainerNextCast = g_frameTime + TRAINER_CAST_DELAY;
	}
	else if(g_trainer.manaTraining && !g_trainer.manaSpell.empty() && mana >= g_trainer.manaMana)
	{
		g_game.sendSay(MessageSay, 0, std::string(), g_trainer.manaSpell);
		g_trainerNextCast = g_frameTime + TRAINER_CAST_DELAY;
	}
}

void UTIL_trainerLoadCFG(Config& cfg)
{
	std::string data = cfg.fetchKey("TrainerManaTraining");
	if(!data.empty())
		g_trainer.manaTraining = (data == "yes");
	data = cfg.fetchKey("TrainerManaSpell");
	if(!data.empty())
		g_trainer.manaSpell = data;
	data = cfg.fetchKey("TrainerManaMana");
	if(!data.empty())
		g_trainer.manaMana = SDL_static_cast(Uint32, SDL_strtoul(data.c_str(), NULL, 10));
	data = cfg.fetchKey("TrainerRuneMaker");
	if(!data.empty())
		g_trainer.runeMaker = (data == "yes");
	data = cfg.fetchKey("TrainerRuneSpell");
	if(!data.empty())
		g_trainer.runeSpell = data;
	data = cfg.fetchKey("TrainerRuneMana");
	if(!data.empty())
		g_trainer.runeMana = SDL_static_cast(Uint32, SDL_strtoul(data.c_str(), NULL, 10));
	data = cfg.fetchKey("TrainerAntiIdle");
	if(!data.empty())
		g_trainer.antiIdle = (data == "yes");
	data = cfg.fetchKey("TrainerAutoEat");
	if(!data.empty())
		g_trainer.autoEat = (data == "yes");
}

void UTIL_trainerSaveCFG(Config& cfg)
{
	cfg.insertKey("TrainerManaTraining", (g_trainer.manaTraining ? "yes" : "no"));
	cfg.insertKey("TrainerManaSpell", g_trainer.manaSpell);
	cfg.insertKey("TrainerManaMana", std::to_string(g_trainer.manaMana));
	cfg.insertKey("TrainerRuneMaker", (g_trainer.runeMaker ? "yes" : "no"));
	cfg.insertKey("TrainerRuneSpell", g_trainer.runeSpell);
	cfg.insertKey("TrainerRuneMana", std::to_string(g_trainer.runeMana));
	cfg.insertKey("TrainerAntiIdle", (g_trainer.antiIdle ? "yes" : "no"));
	cfg.insertKey("TrainerAutoEat", (g_trainer.autoEat ? "yes" : "no"));
}
