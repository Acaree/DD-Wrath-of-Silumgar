#include "ctDefs.h"
#include "ctLog.h"
#include "ctApp.h"
#include "ctInput.h"
#include "ctTextures.h"
#include "ctAudio.h"
#include "ctRender.h"
#include "ctWindow.h"
#include "ctEntities.h"

#include "ctMainMenu.h"

#include "ctFadeToBlack.h"
#include "ctWorldMap.h"
#include "ctTabernScene.h"
#include "ctSettings.h"
#include "ctSkillTree.h"
#include "ctAbout.h"
#include "j1Language.h"
#include "j1Map.h"



ctMainMenu::ctMainMenu() : ctModule()
{
	name = "main_menu";
}

// Destructor
ctMainMenu::~ctMainMenu()
{}

// Called before render is available

bool ctMainMenu::Awake()
{
	LOG("Loading Main Menu");
	bool ret = true;

	return ret;
}

// Called before the first frame
bool ctMainMenu::Start()
{
	bool ret = true;

	pugi::xml_document	data_file;
	pugi::xml_node* node = &App->LoadData(data_file);

	is_new_game = false;
	is_continue = false;

	App->map->actual_tier = TIER_MAP_1;
	App->items->warrior_equip.clear();
	App->items->cleric_equip.clear();
	App->items->elf_equip.clear();
	App->items->dwarf_equip.clear();

	background = App->gui->AddUIImage(0, 0, { 337, 479, 484, 324 }, this);
	labels_bg = App->gui->AddUIImage(15, 0, { 220, 1044, 80, 115 }, this);
	if (node->attribute("continue").as_int()) 
		continue_label = App->gui->AddUILabel(35, 10, App->language->GetDictionary().MM_continue_btn.c_str(), { 255,255,255,255 }, 25, this);
	else
		continue_label = App->gui->AddUILabel(35, 10, App->language->GetDictionary().MM_continue_btn.c_str(), { 255,0,0,255 }, 25, this);
	new_game_label = App->gui->AddUILabel(35, 30, App->language->GetDictionary().MM_new_game_btn.c_str(), { 255,255,255,255 }, 25, this);
	settings_label = App->gui->AddUILabel(35, 50, App->language->GetDictionary().MM_settings_btn.c_str(), { 255,255,255,255 }, 25, this);
	about_label = App->gui->AddUILabel(35, 70, App->language->GetDictionary().MM_about_btn.c_str(), { 255,255,255,255 }, 25, this);
	quit_label = App->gui->AddUILabel(35, 90, App->language->GetDictionary().MM_quit_btn.c_str(), { 255,255,255,255 }, 25, this);
	arrow = App->gui->AddUIImage(-10, 0, { 1333, 272, 7, 14 }, this);

	if (node->attribute("continue").as_int()) {
		labels.push_back(continue_label);
		continue_label->current_state = STATE_FOCUSED;
		arrow->SetParent(continue_label);
	}
	else {
		new_game_label->current_state = STATE_FOCUSED;
		arrow->SetParent(new_game_label);
	}

	labels.push_back(new_game_label);
	labels.push_back(settings_label);
	labels.push_back(about_label);
	labels.push_back(quit_label);

	if (App->audio->device_connected) {
		Mix_VolumeMusic(App->settings->music_volume_value);
		Mix_Volume(-1, App->settings->fx_volume_value);
	}

	if (!App->audio->PlayMusic(App->audio->MainMenuBSO.c_str(),1)) {
		
		LOG("Error playing music in ctMainMenu Start");
	}
	music_is_playing = false;

	if(key_select == -1)
		LoadButtonsInteractions();
	
	return ret;
}

// Called each loop iteration
bool ctMainMenu::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool ctMainMenu::Update(float dt)
{


	//Go down
	if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_DOWN || App->input->gamepad.CROSS_DOWN == GAMEPAD_STATE::PAD_BUTTON_DOWN || App->input->gamepad.JOYSTICK_DOWN == GAMEPAD_STATE::PAD_BUTTON_DOWN) {
		App->audio->PlayFx(App->audio->mm_movement_fx);
		NavigateDown(labels);
	}
	//Go up
	if (App->input->GetKey(SDL_SCANCODE_UP) == KEY_DOWN || App->input->gamepad.CROSS_UP == GAMEPAD_STATE::PAD_BUTTON_DOWN || App->input->gamepad.JOYSTICK_UP == GAMEPAD_STATE::PAD_BUTTON_DOWN) {
		App->audio->PlayFx(App->audio->mm_movement_fx);
		NavigateUp(labels);
	}
	//Execute
	if (App->input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN && App->fadeToBlack->FadeIsOver() == true || App->input->GetGamepadButton(key_select) == GAMEPAD_STATE::PAD_BUTTON_DOWN && App->fadeToBlack->FadeIsOver() == true) {
		ExecuteComand(labels);
	}
	
	return true;
}

// Called each loop iteration
bool ctMainMenu::PostUpdate()
{
	bool ret = true;

	if (quit_pressed ||App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool ctMainMenu::CleanUp()
{
	LOG("Freeing main_menu");

	App->audio->PauseMusic();

	App->gui->DeleteUIElement(*pauseMenu);
	pauseMenu = nullptr;

	App->gui->DeleteUIElement(*arrow);
	arrow = nullptr;
	App->gui->DeleteUIElement(*background);
	background = nullptr;
	App->gui->DeleteUIElement(*continue_label);
	continue_label = nullptr;
	App->gui->DeleteUIElement(*new_game_label);
	new_game_label = nullptr;
	App->gui->DeleteUIElement(*settings_label);
	settings_label = nullptr;
	App->gui->DeleteUIElement(*about_label);
	about_label = nullptr;
	App->gui->DeleteUIElement(*quit_label);
	quit_label = nullptr;
	App->gui->DeleteUIElement(*labels_bg);
	labels_bg = nullptr;

	for (int i = 0; i < labels.size(); i++)
	{
		App->gui->DeleteUIElement(*labels[i]);

	}
	labels.clear();
	
	App->psystem->RemoveAllEmitters();
	return true;
}

bool ctMainMenu::Load(pugi::xml_node& load)
{
	bool ret = true;

	return ret;
}

bool ctMainMenu::Save(pugi::xml_node& save) const
{
	bool ret = true;

	return ret;
}

void ctMainMenu::OnUITrigger(UIElement* elementTriggered, UI_State ui_state)
{

}

void ctMainMenu::NavigateDown(std::vector<UIElement*> &current_vector) {
	std::vector<UIElement*>::const_iterator it_vector = current_vector.begin();
	while (it_vector != current_vector.end()) {
		if ((*it_vector)->current_state == STATE_FOCUSED) {
			if ((*it_vector) != current_vector.back()) {
				(*it_vector)->current_state = STATE_NORMAL;
				it_vector++;
				(*it_vector)->current_state = STATE_FOCUSED;
				arrow->SetParent((*it_vector));
				break;
			}
			else
			{
				(*it_vector)->current_state = STATE_NORMAL;
				it_vector = current_vector.begin();
				(*it_vector)->current_state = STATE_FOCUSED;
				arrow->SetParent((*it_vector));
			}
		}
		it_vector++;
	}
}

void ctMainMenu::NavigateUp(std::vector<UIElement*> &current_vector) {
	std::vector<UIElement*>::const_iterator it_vector = current_vector.begin();
	while (it_vector != current_vector.end()) {
		if ((*it_vector)->current_state == STATE_FOCUSED) {
			if ((*it_vector) != current_vector.front()) {
				(*it_vector)->current_state = STATE_NORMAL;
				it_vector--;
				(*it_vector)->current_state = STATE_FOCUSED;
				arrow->SetParent((*it_vector));
				break;
			}
			else
			{
				(*it_vector)->current_state = STATE_NORMAL;
				it_vector = current_vector.end()-1;
				(*it_vector)->current_state = STATE_FOCUSED;
				arrow->SetParent((*it_vector));
			}
		}
		it_vector++;
	}
}

void ctMainMenu::ExecuteComand(std::vector<UIElement*> &current_vector) {
	for (int i = 0; i < current_vector.size(); i++) {
		if (current_vector.at(i)->current_state == STATE_FOCUSED) {
			current_vector.at(i)->current_state = STATE_EXECUTED;
		}
	}
	
	if (continue_label->current_state == STATE_EXECUTED) {
		is_continue = true;
		LOG("continue_label pressed");
		App->audio->PlayFx(App->audio->mm_select_fx);


		if (!App->audio->PlayMusic(App->audio->MainMenuVoice.c_str(),0.0f)) {

			LOG("Error playing music in ctMainMenu Start");
		}

		if (App->fadeToBlack->FadeIsOver())
			App->fadeToBlack->FadeToBlackBetweenModules(this, App->world_map, 5.0f);

		
	}

	if (new_game_label->current_state == STATE_EXECUTED) {
		LOG("new_game_label pressed");

		/*chapusa*/
		pugi::xml_document	data_file;
		pugi::xml_node* node = &App->LoadData(data_file);
		node = &node->child("heroes");

		for (pugi::xml_node heroe = node->child("heroe"); heroe; heroe = heroe.next_sibling("heroe"))
		{
			std::string tmp(heroe.attribute("name").as_string());

			heroe.child("items").attribute("boot").set_value("");
			heroe.child("items").attribute("ring").set_value("");
			heroe.child("items").attribute("helmet").set_value("");
			heroe.child("items").attribute("shield").set_value("");
			heroe.child("items").attribute("weapon").set_value("");
			heroe.child("items").attribute("gauntlet").set_value("");
			heroe.child("items").attribute("chest").set_value("");
			heroe.child("items").attribute("accessory").set_value("");

			if (tmp == "cleric") {
				for (pugi::xml_node skill = heroe.child("skills").child("skill"); skill; skill = skill.next_sibling("skill")) {
					skill.attribute("owned").set_value(0);
				}

				for (pugi::xml_node item = heroe.child("items").child("item"); item; item = item.next_sibling("item")) {
					item.attribute("quantity").set_value(0);
				}

			}
			else if (tmp == "warrior") {
				for (pugi::xml_node skill = heroe.child("skills").child("skill"); skill; skill = skill.next_sibling("skill")) {
					skill.attribute("owned").set_value(0);
				}

				for (pugi::xml_node item = heroe.child("items").child("item"); item; item = item.next_sibling("item")) {
					item.attribute("quantity").set_value(0);
				}
			}
			else if (tmp == "dwarf") {
				for (pugi::xml_node skill = heroe.child("skills").child("skill"); skill; skill = skill.next_sibling("skill")) {
					skill.attribute("owned").set_value(0);

				}
				for (pugi::xml_node item = heroe.child("items").child("item"); item; item = item.next_sibling("item")) {
					item.attribute("quantity").set_value(0);
				}
			}
			else if (tmp == "elf") {
				for (pugi::xml_node skill = heroe.child("skills").child("skill"); skill; skill = skill.next_sibling("skill")) {
					skill.attribute("owned").set_value(0);
				}
				for (pugi::xml_node item = heroe.child("items").child("item"); item; item = item.next_sibling("item")) {
					item.attribute("quantity").set_value(0);
				}
			}

		}

		data_file.save_file("data.xml");
		data_file.reset();

		App->psystem->AddEmiter({ 180,175 }, EmitterType::EMITTER_TYPE_NEW_GAME);
		

		if (!music_is_playing) {

			if (!App->audio->PlayMusic(App->audio->MainMenuVoice.c_str(), 0.0f)) {

				LOG("Error playing music in ctMainMenu Start");
			}
			music_is_playing = true;
		}
		if (App->fadeToBlack->FadeIsOver()) {
			is_new_game = true;
			App->fadeToBlack->FadeToBlackBetweenModules(this, App->tabern_scene, 5.0f); //5.0f 
		}
	}
	if (settings_label->current_state == STATE_EXECUTED) {
		LOG("settings_label pressed");
		if(App->fadeToBlack->FadeIsOver())
			App->fadeToBlack->FadeToBlackBetweenModules(this, App->settings, 1.0f);
		settings_label->current_state = STATE_FOCUSED;
	}
	if (about_label->current_state == STATE_EXECUTED) {
		LOG("about_label pressed");
		if (App->fadeToBlack->FadeIsOver())
			App->fadeToBlack->FadeToBlackBetweenModules(this, App->about, 1.0f);
		about_label->current_state = STATE_FOCUSED;
	}
	if (quit_label->current_state == STATE_EXECUTED) {
		LOG("quit_label pressed");
		this->quit_pressed = true;
	}

}

void ctMainMenu::LoadButtonsInteractions()
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file("data.xml");

	if (result != NULL)
	{
		pugi::xml_node node = doc.child("data").child("settings");

		key_select = node.attribute("select").as_int();

		key_back = node.attribute("back").as_int();

	}
}