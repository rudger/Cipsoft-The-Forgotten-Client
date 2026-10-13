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
#include "../GUI_Elements/GUI_Separator.h"
#include "../GUI_Elements/GUI_CheckBox.h"
#include "../game.h"

#define GENERAL_OPTIONS_TITLE "General Options"
#define GENERAL_OPTIONS_WIDTH 246
#define GENERAL_OPTIONS_HEIGHT 642
#define GENERAL_OPTIONS_HELP_LINK "http://www.tibia.com/gameguides/?subtopic=manual&section=options"
#define GENERAL_OPTIONS_HELP_EVENTID 1000
#define GENERAL_OPTIONS_CANCEL_EVENTID 1001
#define GENERAL_OPTIONS_OK_EVENTID 1002
#define GENERAL_OPTIONS_CLASSIC_CONTROL_TEXT "Classic Control"
#define GENERAL_OPTIONS_CLASSIC_CONTROL_X 18
#define GENERAL_OPTIONS_CLASSIC_CONTROL_Y 32
#define GENERAL_OPTIONS_CLASSIC_CONTROL_W 210
#define GENERAL_OPTIONS_CLASSIC_CONTROL_H 22
#define GENERAL_OPTIONS_CLASSIC_CONTROL_EVENTID 1003
#define GENERAL_OPTIONS_CHASE_OFF_TEXT "Auto Chase Off"
#define GENERAL_OPTIONS_CHASE_OFF_X 18
#define GENERAL_OPTIONS_CHASE_OFF_Y 65
#define GENERAL_OPTIONS_CHASE_OFF_W 210
#define GENERAL_OPTIONS_CHASE_OFF_H 22
#define GENERAL_OPTIONS_CHASE_OFF_EVENTID 1004
#define GENERAL_OPTIONS_WASD_WALKING_TEXT "WASD Walking"
#define GENERAL_OPTIONS_WASD_WALKING_X 18
#define GENERAL_OPTIONS_WASD_WALKING_Y 98
#define GENERAL_OPTIONS_WASD_WALKING_W 210
#define GENERAL_OPTIONS_WASD_WALKING_H 22
#define GENERAL_OPTIONS_WASD_WALKING_EVENTID 1014
#define GENERAL_OPTIONS_SMART_WALKING_TEXT "Smart Walking"
#define GENERAL_OPTIONS_SMART_WALKING_X 18
#define GENERAL_OPTIONS_SMART_WALKING_Y 131
#define GENERAL_OPTIONS_SMART_WALKING_W 210
#define GENERAL_OPTIONS_SMART_WALKING_H 22
#define GENERAL_OPTIONS_SMART_WALKING_EVENTID 1015
#define GENERAL_OPTIONS_MANA_BAR_TEXT "Show player mana bar"
#define GENERAL_OPTIONS_MANA_BAR_X 18
#define GENERAL_OPTIONS_MANA_BAR_Y 164
#define GENERAL_OPTIONS_MANA_BAR_W 210
#define GENERAL_OPTIONS_MANA_BAR_H 22
#define GENERAL_OPTIONS_MANA_BAR_EVENTID 1016
#define GENERAL_OPTIONS_TAB_ATTACK_TEXT "TAB to attack nearest monster"
#define GENERAL_OPTIONS_TAB_ATTACK_X 18
#define GENERAL_OPTIONS_TAB_ATTACK_Y 164
#define GENERAL_OPTIONS_TAB_ATTACK_W 210
#define GENERAL_OPTIONS_TAB_ATTACK_H 22
#define GENERAL_OPTIONS_TAB_ATTACK_EVENTID 1017
#define GENERAL_OPTIONS_LOOT_MESSAGE_TEXT "Show monster loot message"
#define GENERAL_OPTIONS_LOOT_MESSAGE_X 18
#define GENERAL_OPTIONS_LOOT_MESSAGE_Y 197
#define GENERAL_OPTIONS_LOOT_MESSAGE_W 210
#define GENERAL_OPTIONS_LOOT_MESSAGE_H 22
#define GENERAL_OPTIONS_LOOT_MESSAGE_EVENTID 1018
#define GENERAL_OPTIONS_PRIVATE_CHAT_TEXT "Open private chat on new message"
#define GENERAL_OPTIONS_PRIVATE_CHAT_X 18
#define GENERAL_OPTIONS_PRIVATE_CHAT_Y 230
#define GENERAL_OPTIONS_PRIVATE_CHAT_W 210
#define GENERAL_OPTIONS_PRIVATE_CHAT_H 22
#define GENERAL_OPTIONS_PRIVATE_CHAT_EVENTID 1019
#define GENERAL_OPTIONS_MOVE_STACKS_TEXT "Move stacks directly"
#define GENERAL_OPTIONS_MOVE_STACKS_X 18
#define GENERAL_OPTIONS_MOVE_STACKS_Y 263
#define GENERAL_OPTIONS_MOVE_STACKS_W 210
#define GENERAL_OPTIONS_MOVE_STACKS_H 22
#define GENERAL_OPTIONS_MOVE_STACKS_EVENTID 1020
#define GENERAL_OPTIONS_SHOW_PING_TEXT "Show ping"
#define GENERAL_OPTIONS_SHOW_PING_X 18
#define GENERAL_OPTIONS_SHOW_PING_Y 296
#define GENERAL_OPTIONS_SHOW_PING_W 210
#define GENERAL_OPTIONS_SHOW_PING_H 22
#define GENERAL_OPTIONS_SHOW_PING_EVENTID 1021
#define GENERAL_OPTIONS_SHOW_FPS_TEXT "Show FPS"
#define GENERAL_OPTIONS_SHOW_FPS_X 18
#define GENERAL_OPTIONS_SHOW_FPS_Y 329
#define GENERAL_OPTIONS_SHOW_FPS_W 210
#define GENERAL_OPTIONS_SHOW_FPS_H 22
#define GENERAL_OPTIONS_SHOW_FPS_EVENTID 1022
#define GENERAL_OPTIONS_SHOW_NAMES_TEXT "Show Names of Creatures"
#define GENERAL_OPTIONS_SHOW_NAMES_X 18
#define GENERAL_OPTIONS_SHOW_NAMES_Y 362
#define GENERAL_OPTIONS_SHOW_NAMES_W 210
#define GENERAL_OPTIONS_SHOW_NAMES_H 22
#define GENERAL_OPTIONS_SHOW_NAMES_EVENTID 1005
#define GENERAL_OPTIONS_SHOW_MARKS_TEXT "Show Marks on Creatures"
#define GENERAL_OPTIONS_SHOW_MARKS_X 18
#define GENERAL_OPTIONS_SHOW_MARKS_Y 395
#define GENERAL_OPTIONS_SHOW_MARKS_W 210
#define GENERAL_OPTIONS_SHOW_MARKS_H 22
#define GENERAL_OPTIONS_SHOW_MARKS_EVENTID 1006
#define GENERAL_OPTIONS_SHOW_ICONS_TEXT "Show Icons on NPCs"
#define GENERAL_OPTIONS_SHOW_ICONS_X 18
#define GENERAL_OPTIONS_SHOW_ICONS_Y 428
#define GENERAL_OPTIONS_SHOW_ICONS_W 210
#define GENERAL_OPTIONS_SHOW_ICONS_H 22
#define GENERAL_OPTIONS_SHOW_ICONS_EVENTID 1008
#define GENERAL_OPTIONS_SHOW_COOLDOWN_TEXT "Show Cooldown Bar"
#define GENERAL_OPTIONS_SHOW_COOLDOWN_X 18
#define GENERAL_OPTIONS_SHOW_COOLDOWN_Y 461
#define GENERAL_OPTIONS_SHOW_COOLDOWN_W 210
#define GENERAL_OPTIONS_SHOW_COOLDOWN_H 22
#define GENERAL_OPTIONS_SHOW_COOLDOWN_EVENTID 1010
#define GENERAL_OPTIONS_SHOW_ACTION1_TEXT "Show Action Bar 1"
#define GENERAL_OPTIONS_SHOW_ACTION1_X 18
#define GENERAL_OPTIONS_SHOW_ACTION1_Y 494
#define GENERAL_OPTIONS_SHOW_ACTION1_W 210
#define GENERAL_OPTIONS_SHOW_ACTION1_H 22
#define GENERAL_OPTIONS_SHOW_ACTION1_EVENTID 1011
#define GENERAL_OPTIONS_SHOW_ACTION2_TEXT "Show Action Bar 2"
#define GENERAL_OPTIONS_SHOW_ACTION2_X 18
#define GENERAL_OPTIONS_SHOW_ACTION2_Y 527
#define GENERAL_OPTIONS_SHOW_ACTION2_W 210
#define GENERAL_OPTIONS_SHOW_ACTION2_H 22
#define GENERAL_OPTIONS_SHOW_ACTION2_EVENTID 1012
#define GENERAL_OPTIONS_ASK_BUY_PRODUCTS_TEXT "Ask Before Buying Products"
#define GENERAL_OPTIONS_ASK_BUY_PRODUCTS_X 18
#define GENERAL_OPTIONS_ASK_BUY_PRODUCTS_Y 560
#define GENERAL_OPTIONS_ASK_BUY_PRODUCTS_W 210
#define GENERAL_OPTIONS_ASK_BUY_PRODUCTS_H 22
#define GENERAL_OPTIONS_ASK_BUY_PRODUCTS_EVENTID 1013

extern Engine g_engine;
extern Game g_game;

void general_options_Events(Uint32 event, Sint32)
{
	switch(event)
	{
		case GENERAL_OPTIONS_HELP_EVENTID: UTIL_OpenURL(GENERAL_OPTIONS_HELP_LINK); break;
		case GENERAL_OPTIONS_CANCEL_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_GENERALOPTIONS)
				g_engine.removeWindow(pWindow);
		}
		break;
		case GENERAL_OPTIONS_OK_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_GENERALOPTIONS)
			{
				GUI_CheckBox* pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GENERAL_OPTIONS_CLASSIC_CONTROL_EVENTID));
				if(pCheckBox)
					g_engine.setClassicControl(pCheckBox->isChecked());
				pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GENERAL_OPTIONS_CHASE_OFF_EVENTID));
				if(pCheckBox)
					g_engine.setAutoChaseOff(pCheckBox->isChecked());
				pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GENERAL_OPTIONS_WASD_WALKING_EVENTID));
				if(pCheckBox)
					g_engine.setWasdWalking(pCheckBox->isChecked());
				pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GENERAL_OPTIONS_SMART_WALKING_EVENTID));
				if(pCheckBox)
					g_engine.setSmartWalking(pCheckBox->isChecked());
				pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GENERAL_OPTIONS_MANA_BAR_EVENTID));
				if(pCheckBox)
					g_engine.setShowManaBar(pCheckBox->isChecked());
				pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GENERAL_OPTIONS_TAB_ATTACK_EVENTID));
				if(pCheckBox)
					g_engine.setTabAttack(pCheckBox->isChecked());
				pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GENERAL_OPTIONS_LOOT_MESSAGE_EVENTID));
				if(pCheckBox)
					g_engine.setShowLootMessage(pCheckBox->isChecked());
				pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GENERAL_OPTIONS_PRIVATE_CHAT_EVENTID));
				if(pCheckBox)
					g_engine.setAutoOpenPrivateChat(pCheckBox->isChecked());
				pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GENERAL_OPTIONS_MOVE_STACKS_EVENTID));
				if(pCheckBox)
					g_engine.setMoveStacksDirectly(pCheckBox->isChecked());
				pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GENERAL_OPTIONS_SHOW_PING_EVENTID));
				if(pCheckBox)
					g_engine.setShowPing(pCheckBox->isChecked());
				pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GENERAL_OPTIONS_SHOW_FPS_EVENTID));
				if(pCheckBox)
					g_engine.setShowFPS(pCheckBox->isChecked());
				pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GENERAL_OPTIONS_SHOW_NAMES_EVENTID));
				if(pCheckBox)
					g_engine.setShowNames(pCheckBox->isChecked());
				pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GENERAL_OPTIONS_SHOW_MARKS_EVENTID));
				if(pCheckBox)
					g_engine.setShowMarks(pCheckBox->isChecked());
				pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GENERAL_OPTIONS_SHOW_ICONS_EVENTID));
				if(pCheckBox)
					g_engine.setShowIcons(pCheckBox->isChecked());
				pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GENERAL_OPTIONS_SHOW_COOLDOWN_EVENTID));
				if(pCheckBox)
					g_engine.setShowCooldown(pCheckBox->isChecked());

				g_engine.removeWindow(pWindow);
			}
		}
		break;
		default: break;
	}
}

void UTIL_generalOptions()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_GENERALOPTIONS);
	if(pWindow)
		g_engine.removeWindow(pWindow);

	//The mana bar option is only shown when the mana bar is enabled in features.lua
	const bool manaBarOption = g_game.hasGameFeature(GAME_FEATURE_MANA_BAR);
	const Sint32 offsetY = (manaBarOption ? 33 : 0);
	const Sint32 windowHeight = GENERAL_OPTIONS_HEIGHT + offsetY;
	GUI_Window* newWindow = new GUI_Window(iRect(0, 0, GENERAL_OPTIONS_WIDTH, windowHeight), GENERAL_OPTIONS_TITLE, GUI_WINDOW_GENERALOPTIONS);
	GUI_CheckBox* newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_CLASSIC_CONTROL_X, GENERAL_OPTIONS_CLASSIC_CONTROL_Y, GENERAL_OPTIONS_CLASSIC_CONTROL_W, GENERAL_OPTIONS_CLASSIC_CONTROL_H), GENERAL_OPTIONS_CLASSIC_CONTROL_TEXT, g_engine.hasClassicControl(), GENERAL_OPTIONS_CLASSIC_CONTROL_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_CLASSIC_CONTROL_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_CHASE_OFF_X, GENERAL_OPTIONS_CHASE_OFF_Y, GENERAL_OPTIONS_CHASE_OFF_W, GENERAL_OPTIONS_CHASE_OFF_H), GENERAL_OPTIONS_CHASE_OFF_TEXT, g_engine.hasAutoChaseOff(), GENERAL_OPTIONS_CHASE_OFF_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_CHASE_OFF_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_WASD_WALKING_X, GENERAL_OPTIONS_WASD_WALKING_Y, GENERAL_OPTIONS_WASD_WALKING_W, GENERAL_OPTIONS_WASD_WALKING_H), GENERAL_OPTIONS_WASD_WALKING_TEXT, g_engine.hasWasdWalking(), GENERAL_OPTIONS_WASD_WALKING_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_WASD_WALKING_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_SMART_WALKING_X, GENERAL_OPTIONS_SMART_WALKING_Y, GENERAL_OPTIONS_SMART_WALKING_W, GENERAL_OPTIONS_SMART_WALKING_H), GENERAL_OPTIONS_SMART_WALKING_TEXT, g_engine.hasSmartWalking(), GENERAL_OPTIONS_SMART_WALKING_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_SMART_WALKING_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	if(manaBarOption)
	{
		newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_MANA_BAR_X, GENERAL_OPTIONS_MANA_BAR_Y, GENERAL_OPTIONS_MANA_BAR_W, GENERAL_OPTIONS_MANA_BAR_H), GENERAL_OPTIONS_MANA_BAR_TEXT, g_engine.hasShowManaBar(), GENERAL_OPTIONS_MANA_BAR_EVENTID);
		newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_MANA_BAR_EVENTID);
		newCheckBox->startEvents();
		newWindow->addChild(newCheckBox);
	}
	newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_TAB_ATTACK_X, GENERAL_OPTIONS_TAB_ATTACK_Y + offsetY, GENERAL_OPTIONS_TAB_ATTACK_W, GENERAL_OPTIONS_TAB_ATTACK_H), GENERAL_OPTIONS_TAB_ATTACK_TEXT, g_engine.hasTabAttack(), GENERAL_OPTIONS_TAB_ATTACK_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_TAB_ATTACK_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_LOOT_MESSAGE_X, GENERAL_OPTIONS_LOOT_MESSAGE_Y + offsetY, GENERAL_OPTIONS_LOOT_MESSAGE_W, GENERAL_OPTIONS_LOOT_MESSAGE_H), GENERAL_OPTIONS_LOOT_MESSAGE_TEXT, g_engine.hasShowLootMessage(), GENERAL_OPTIONS_LOOT_MESSAGE_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_LOOT_MESSAGE_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_PRIVATE_CHAT_X, GENERAL_OPTIONS_PRIVATE_CHAT_Y + offsetY, GENERAL_OPTIONS_PRIVATE_CHAT_W, GENERAL_OPTIONS_PRIVATE_CHAT_H), GENERAL_OPTIONS_PRIVATE_CHAT_TEXT, g_engine.hasAutoOpenPrivateChat(), GENERAL_OPTIONS_PRIVATE_CHAT_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_PRIVATE_CHAT_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_MOVE_STACKS_X, GENERAL_OPTIONS_MOVE_STACKS_Y + offsetY, GENERAL_OPTIONS_MOVE_STACKS_W, GENERAL_OPTIONS_MOVE_STACKS_H), GENERAL_OPTIONS_MOVE_STACKS_TEXT, g_engine.hasMoveStacksDirectly(), GENERAL_OPTIONS_MOVE_STACKS_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_MOVE_STACKS_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_SHOW_PING_X, GENERAL_OPTIONS_SHOW_PING_Y + offsetY, GENERAL_OPTIONS_SHOW_PING_W, GENERAL_OPTIONS_SHOW_PING_H), GENERAL_OPTIONS_SHOW_PING_TEXT, g_engine.hasShowPing(), GENERAL_OPTIONS_SHOW_PING_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_SHOW_PING_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_SHOW_FPS_X, GENERAL_OPTIONS_SHOW_FPS_Y + offsetY, GENERAL_OPTIONS_SHOW_FPS_W, GENERAL_OPTIONS_SHOW_FPS_H), GENERAL_OPTIONS_SHOW_FPS_TEXT, g_engine.hasShowFPS(), GENERAL_OPTIONS_SHOW_FPS_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_SHOW_FPS_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_SHOW_NAMES_X, GENERAL_OPTIONS_SHOW_NAMES_Y + offsetY, GENERAL_OPTIONS_SHOW_NAMES_W, GENERAL_OPTIONS_SHOW_NAMES_H), GENERAL_OPTIONS_SHOW_NAMES_TEXT, g_engine.hasShowNames(), GENERAL_OPTIONS_SHOW_NAMES_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_SHOW_NAMES_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_SHOW_MARKS_X, GENERAL_OPTIONS_SHOW_MARKS_Y + offsetY, GENERAL_OPTIONS_SHOW_MARKS_W, GENERAL_OPTIONS_SHOW_MARKS_H), GENERAL_OPTIONS_SHOW_MARKS_TEXT, g_engine.hasShowMarks(), GENERAL_OPTIONS_SHOW_MARKS_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_SHOW_MARKS_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_SHOW_ICONS_X, GENERAL_OPTIONS_SHOW_ICONS_Y + offsetY, GENERAL_OPTIONS_SHOW_ICONS_W, GENERAL_OPTIONS_SHOW_ICONS_H), GENERAL_OPTIONS_SHOW_ICONS_TEXT, g_engine.hasShowIcons(), GENERAL_OPTIONS_SHOW_ICONS_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_SHOW_ICONS_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_SHOW_COOLDOWN_X, GENERAL_OPTIONS_SHOW_COOLDOWN_Y + offsetY, GENERAL_OPTIONS_SHOW_COOLDOWN_W, GENERAL_OPTIONS_SHOW_COOLDOWN_H), GENERAL_OPTIONS_SHOW_COOLDOWN_TEXT, g_engine.hasShowCooldown(), GENERAL_OPTIONS_SHOW_COOLDOWN_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_SHOW_COOLDOWN_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_SHOW_ACTION1_X, GENERAL_OPTIONS_SHOW_ACTION1_Y + offsetY, GENERAL_OPTIONS_SHOW_ACTION1_W, GENERAL_OPTIONS_SHOW_ACTION1_H), GENERAL_OPTIONS_SHOW_ACTION1_TEXT, false, GENERAL_OPTIONS_SHOW_ACTION1_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_SHOW_ACTION1_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_SHOW_ACTION2_X, GENERAL_OPTIONS_SHOW_ACTION2_Y + offsetY, GENERAL_OPTIONS_SHOW_ACTION2_W, GENERAL_OPTIONS_SHOW_ACTION2_H), GENERAL_OPTIONS_SHOW_ACTION2_TEXT, false, GENERAL_OPTIONS_SHOW_ACTION2_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_SHOW_ACTION2_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(GENERAL_OPTIONS_ASK_BUY_PRODUCTS_X, GENERAL_OPTIONS_ASK_BUY_PRODUCTS_Y + offsetY, GENERAL_OPTIONS_ASK_BUY_PRODUCTS_W, GENERAL_OPTIONS_ASK_BUY_PRODUCTS_H), GENERAL_OPTIONS_ASK_BUY_PRODUCTS_TEXT, false, GENERAL_OPTIONS_ASK_BUY_PRODUCTS_EVENTID);
	newCheckBox->setBoxEventCallback(&general_options_Events, GENERAL_OPTIONS_ASK_BUY_PRODUCTS_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	GUI_Button* newButton = new GUI_Button(iRect(GENERAL_OPTIONS_WIDTH - 56, windowHeight - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Cancel", CLIENT_GUI_ESCAPE_TRIGGER);
	newButton->setButtonEventCallback(&general_options_Events, GENERAL_OPTIONS_CANCEL_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(GENERAL_OPTIONS_WIDTH - 109, windowHeight - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Ok", CLIENT_GUI_ENTER_TRIGGER);
	newButton->setButtonEventCallback(&general_options_Events, GENERAL_OPTIONS_OK_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(GENERAL_OPTIONS_WIDTH - 162, windowHeight - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Help");
	newButton->setButtonEventCallback(&general_options_Events, GENERAL_OPTIONS_HELP_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	GUI_Separator* newSeparator = new GUI_Separator(iRect(13, windowHeight - 40, GENERAL_OPTIONS_WIDTH - 26, 2));
	newWindow->addChild(newSeparator);
	g_engine.addWindow(newWindow, true);
}
