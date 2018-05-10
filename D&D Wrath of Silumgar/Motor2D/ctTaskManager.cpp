#include "ctTaskManager.h"
#include "ctApp.h"
#include "ctEntities.h"
#include "ctInput.h"
#include "ctCombat.h"
#include "Particle.h"
#include "ParticlePool.h"
#include "Emitter.h"
#include "Cleric.h"
#include "Dwarf.h"
#include "Elf.h"
#include "Warrior.h"
//randomize libs
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */


bool MoveToEntity::Execute()
{
	bool ret = false;

	if (entity_to_move->IsStunned())
			return true;

	if (entity_to_move->GetCurrentHealthPoints() != 0) {
		if (entity_to_go->GetCurrentHealthPoints() == 0) {
			if (entity_to_move->type == ELF || entity_to_move->type == CLERIC || entity_to_move->type == WARRIOR || entity_to_move->type == DWARF) {
				for (int i = 0; i < App->combat->enemies.size(); i++)
				{
					entity_to_go = App->combat->enemies.at(i);
					if (entity_to_go->GetCurrentHealthPoints() != 0)
						break;
				}
				if(entity_to_go->GetCurrentHealthPoints() == 0)
					return true;
			}
			else {
				for (int i = 0; i < App->combat->heroes.size(); i++)
				{
					entity_to_go = App->combat->heroes.at(i);
					if (entity_to_go->GetCurrentHealthPoints() != 0)
						break;
				}
				if (entity_to_go->GetCurrentHealthPoints() == 0)
					return true;
			}
		}


		int x_objective = (entity_to_go->position.x + offset);

		if (((entity_to_move->position.x + 25) >= x_objective && (entity_to_move->position.x - 25) <= x_objective) && ((entity_to_move->position.y + 25) >= entity_to_go->position.y && (entity_to_move->position.y - 25) <= entity_to_go->position.y))
		{
			ret = true;
		}
		else {
			if (entity_to_move->position.x < entity_to_go->position.x)
			{
				entity_to_move->animation = &entity_to_move->run_forward;
				entity_to_move->position.x += BASE_SPEED;
			}

			else if (entity_to_move->position.x > entity_to_go->position.x)
			{
				entity_to_move->animation = &entity_to_move->run_forward;
				entity_to_move->position.x -= BASE_SPEED;
			}

			if (entity_to_move->position.y < entity_to_go->position.y)
			{
				entity_to_move->animation = &entity_to_move->run_forward;
				entity_to_move->position.y += BASE_SPEED;
			}

			else if (entity_to_move->position.y > entity_to_go->position.y)
			{
				entity_to_move->animation = &entity_to_move->run_forward;
				entity_to_move->position.y -= BASE_SPEED;
			}
		}
	}
	else
		ret = true;

	

	return ret;
}

bool MoveToInitialPosition::Execute()
{
	bool ret = false;

	if(entity_to_move->IsStunned())
			return true;

	if (entity_to_move->GetCurrentHealthPoints() != 0) {
		if (entity_to_move->flip_texture != true) {
			entity_to_move->flip_texture = true;
		}

		if (entity_to_move->position.x == entity_to_move->initial_position.x && entity_to_move->position.y == entity_to_move->initial_position.y)
		{
			entity_to_move->animation = &entity_to_move->idle;
			entity_to_move->flip_texture = false;
			ret = true;
		}
		else {
			if (entity_to_move->position.x < entity_to_move->initial_position.x)
			{
				entity_to_move->animation = &entity_to_move->run_forward;
				entity_to_move->position.x += BASE_SPEED;
			}

			else if (entity_to_move->position.x > entity_to_move->initial_position.x)
			{
				entity_to_move->animation = &entity_to_move->run_forward;
				entity_to_move->position.x -= BASE_SPEED;
			}

			if (entity_to_move->position.y < entity_to_move->initial_position.y)
			{
				entity_to_move->animation = &entity_to_move->run_forward;
				entity_to_move->position.y += BASE_SPEED;
			}

			else if (entity_to_move->position.y > entity_to_move->initial_position.y)
			{
				entity_to_move->animation = &entity_to_move->run_forward;
				entity_to_move->position.y -= BASE_SPEED;
			}
		}
	}
	else
		ret = true;

	//todo check this
	if (ret) {
		App->combat->draw_turn_priority_entity.erase(App->combat->draw_turn_priority_entity.cbegin());
		App->combat->draw_turn_priority_entity.shrink_to_fit();
	}
	

	return ret;
}



bool PerformActionToEntity::Execute()
{

	bool ret = false;

	if (actioner_entity->IsStunned())
			return true;

	if (actioner_entity->GetCurrentHealthPoints() != 0) {
		switch (action_to_perform.type)
		{
		case DEFAULT_ATTACK: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->attack;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->attack.Reset();


				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal =  action_to_perform.health_points_effect * actioner_entity->GetCurrentStrengthPoints();
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}
						

						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Attack();

			}
		}
		break;

		case VOID_CANNON: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->void_cannon;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->void_cannon.Reset();
				actioner_entity->animation = &actioner_entity->idle;

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS


						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
					
						
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
					
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
						//TODO SITO
						fPoint posP;
						posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };

						App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_MINDBLOWN);
						App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);

						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability2T1();

			}
		}
	 break;
		case KICK: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->kick;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->SetCurrentManaPoints(actioner_entity->GetCurrentManaPoints() - action_to_perform.mana_points_effect_to_himself);
				App->combat->UpdateManaBarOfEntity(actioner_entity, (-action_to_perform.mana_points_effect_to_himself));

				actioner_entity->kick.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity && !receiver_entity->IsStunned()) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;
						
						int damage_to_deal = App->entities->GetWarrior()->GetCurrentStrengthPoints();
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;
						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical)
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
						else
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);

						receiver_entity->Damaged();


						//TO TEST 
						Altered_Stat stun;
						stun.stun = true;
						stun.turn_left = 2;

						receiver_entity->AddAlteredStat(stun);
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				fPoint  posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };

				App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_KICK);

				actioner_entity->Ability1T1();

			}
		}
		break;
		case GUARD: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->guard;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->guard.Reset();


				
				std::string tmp_dmg = "STRENGTH UP";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
				tmp_dmg = "INTELLIGENCE UP";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);


				Altered_Stat defense;
				defense.turn_left = 1;
				defense.stat_effect_magical_defense = 1;
				defense.stat_effect_physical_defense = 1;
				receiver_entity->AddAlteredStat(defense);
			

				actioner_entity->Ability2T1();

			}
		}
		break;

		case HEAVY_SLASH: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->heavy_slash;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->heavy_slash.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = 10 + App->entities->GetWarrior()->GetCurrentStrengthPoints();
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}

						if (!receiver_entity->IsBleeding()) {
							Altered_Stat bleed;
							bleed.bleeding = true;
							bleed.turn_left = 3;

							receiver_entity->AddAlteredStat(bleed);
							std::string bleed_string = "BLEEDING";
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, bleed_string, { 127,0,8,255 }, 16, nullptr, nullptr);

						}

						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability3T1();

			}
		}
		break;

		case KICK_PLUS: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->kick;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->SetCurrentManaPoints(actioner_entity->GetCurrentManaPoints() - action_to_perform.mana_points_effect_to_himself);
				App->combat->UpdateManaBarOfEntity(actioner_entity, (-action_to_perform.mana_points_effect_to_himself));

				actioner_entity->kick.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity && !receiver_entity->IsStunned()) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = App->entities->GetWarrior()->GetCurrentStrengthPoints();
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;
						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical)
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
						else
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);

						receiver_entity->Damaged();


						//TO TEST 
						Altered_Stat stun;
						stun.stun = true;
						stun.turn_left = 2;
						stun.stat_effect_physical_defense = -1;

						receiver_entity->AddAlteredStat(stun);
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				fPoint  posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };

				App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_KICK);

				actioner_entity->Ability1T1();

			}
		}
		break;

		case GUARD_PLUS: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->guard;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->guard.Reset();



				std::string tmp_dmg = "STRENGTH UP";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
				tmp_dmg = "INTELLIGENCE UP";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);


				Altered_Stat defense;
				defense.turn_left = 2;
				defense.stat_effect_magical_defense = 2;
				defense.stat_effect_physical_defense = 2;
				receiver_entity->AddAlteredStat(defense);


				actioner_entity->Ability2T1();

			}
		}
		break;

		case HEAVY_SLASH_PLUS: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->heavy_slash;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->heavy_slash.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = 10 + App->entities->GetWarrior()->GetCurrentStrengthPoints();
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);

							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}

						if (!receiver_entity->IsBleeding()) {
							Altered_Stat bleed;
							bleed.bleeding = true;
							bleed.turn_left = 3;
							bleed.stat_effect_physical_defense = 1;
							receiver_entity->AddAlteredStat(bleed);
							std::string bleed_string = "BLEEDING";
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, bleed_string, { 127,0,8,255 }, 16, nullptr, nullptr);

						}

						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability3T1();

			}
		}
		 break;

		case CHARGE: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->charge;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->charge.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = 15 + App->entities->GetWarrior()->GetCurrentStrengthPoints();
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO

							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);

							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}

						
						Altered_Stat judg;
						judg.bleeding = true;
						judg.turn_left = 2;
						judg.stat_effect_judgement = -2;
						receiver_entity->AddAlteredStat(judg);
						std::string bleed_string = "DEBUFFED JUDGEMENT";
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, bleed_string, { 127,0,8,255 }, 16, nullptr, nullptr);

						

						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability3T1();

			}
		}
		break;

		case TAUNT: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->taunt;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->taunt.Reset();

				std::string tmp_dmg = "STRENGTH UP";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
				tmp_dmg = "PHYSICAL DEFENSE REDUCED";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h-10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
				tmp_dmg = "MAGICAL DEFENSE REDUCED";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 20, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);

				Altered_Stat defense;
				defense.turn_left = 1;
				defense.stat_effect_strength = 1;
				defense.stat_effect_magical_defense = -2;
				defense.stat_effect_physical_defense = -2;
				receiver_entity->AddAlteredStat(defense);


				actioner_entity->Ability2T1();				

			}
		}
		break;

		case WHIRLWIND: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->whirlwind;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->whirlwind.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = 30 + App->entities->GetWarrior()->GetCurrentStrengthPoints();
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);

							damage_to_deal -=20;
							damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
							damage_to_deal = damage_to_deal - damage_reduction;
							receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
							receiver_entity->animation = &receiver_entity->hit;
							App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
							tmp_dmg = std::to_string(damage_to_deal);

							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 15, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);

							if(damage_to_deal > 20)
								damage_to_deal -= 20;
							else
								damage_to_deal -= 10;

							if (damage_to_deal < 0)
								damage_to_deal = 0;

							damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
							damage_to_deal = damage_to_deal - damage_reduction;
							receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
							receiver_entity->animation = &receiver_entity->hit;
							App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
							tmp_dmg = std::to_string(damage_to_deal);

							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 20, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);


							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}




						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability3T2();

			}
		}
		break;


		
		case HIGH_AXE: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->high_axe;

			

			ret = actioner_entity->animation->Finished();

			if (ret == true) {

				actioner_entity->SetCurrentManaPoints(actioner_entity->GetCurrentManaPoints() - action_to_perform.mana_points_effect_to_himself);
				App->combat->UpdateManaBarOfEntity(actioner_entity, (-action_to_perform.mana_points_effect_to_himself));

				actioner_entity->high_axe.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity || receiver_entity->IsStunned()) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;
						int damage_to_deal = App->entities->GetDwarf()->GetCurrentStrengthPoints();
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;
						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						
						if (receiver_entity->IsStunned())
							damage_to_deal = damage_to_deal * 2;
						
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (critical)
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
						else if(receiver_entity->IsStunned())
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 253,103,6,255 }, 16, nullptr, nullptr);
						else
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 },14, nullptr, nullptr);

						Altered_Stat defense;
						defense.turn_left = 1;
						defense.stat_effect_magical_defense = -1;
						defense.stat_effect_physical_defense = -1;
						actioner_entity->AddAlteredStat(defense);

						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				fPoint  posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };

				App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIGH_AXE);
				actioner_entity->Ability2T1();

			}
		}
		break;

		case MORALE_BOOST: {

			if (!HaveTeamObjective())
				return true;

			actioner_entity->animation = &actioner_entity->morale_boost;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->morale_boost.Reset();

			
				std::string tmp_dmg = "STRENGTH UP";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
				tmp_dmg = "INTELLIGENCE UP";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 20, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);

				Altered_Stat strength_up;
				strength_up.turn_left = 2;
				strength_up.stat_effect_strength = 1;
				strength_up.stat_effect_intelligence = 1;
				receiver_entity->AddAlteredStat(strength_up);

				//animate the receiver to hit + audio or smth
				actioner_entity->Ability1T1();

			}
		}
		break;

		case DWARFS_WRATH: {

			if (!HaveTeamObjective())
				return true;

			actioner_entity->animation = &actioner_entity->morale_boost;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->morale_boost.Reset();


				std::string tmp_dmg = "DEXTERITY X 1.5";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
				
				//TODO IMPLEMENT LOSE HP
				Altered_Stat dexterity;
				dexterity.turn_left = 2;
				dexterity.stat_effect_dexterity = 2;

				receiver_entity->AddAlteredStat(dexterity);

				//animate the receiver to hit + audio or smth
				actioner_entity->Ability1T1();

			}
		}
		break;

		case HIGH_AXE_PLUS: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->high_axe;



			ret = actioner_entity->animation->Finished();

			if (ret == true) {

				actioner_entity->SetCurrentManaPoints(actioner_entity->GetCurrentManaPoints() - action_to_perform.mana_points_effect_to_himself);
				App->combat->UpdateManaBarOfEntity(actioner_entity, (-action_to_perform.mana_points_effect_to_himself));

				actioner_entity->high_axe.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity || receiver_entity->IsStunned()) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;
						int dwarfdmg = App->entities->GetDwarf()->GetCurrentStrengthPoints();
						int damage_to_deal = 20+ dwarfdmg;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;
						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}

						if (receiver_entity->IsStunned())
							damage_to_deal = damage_to_deal * 2;

						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (critical)
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
						else if (receiver_entity->IsStunned())
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 253,103,6,255 }, 16, nullptr, nullptr);
						else
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);

						Altered_Stat defense;
						defense.turn_left = 1;
						defense.stat_effect_magical_defense = -1;
						defense.stat_effect_physical_defense = -1;
						actioner_entity->AddAlteredStat(defense);

						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				fPoint  posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };

				App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIGH_AXE);
				actioner_entity->Ability2T1();

			}
		}
		break;

		case MORALE_BOOST_PLUS: {

			if (!HaveTeamObjective())
				return true;

			actioner_entity->animation = &actioner_entity->morale_boost;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->morale_boost.Reset();


				std::string tmp_dmg = "STRENGTH UP x2";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
				tmp_dmg = "INTELLIGENCE UP";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 20, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);

				Altered_Stat strength_up;
				strength_up.turn_left = 3;
				strength_up.stat_effect_strength = 2;
				strength_up.stat_effect_intelligence = 1;
				receiver_entity->AddAlteredStat(strength_up);

				//animate the receiver to hit + audio or smth
				actioner_entity->Ability1T1();

			}
		}
		break;

		case DWARFS_WRATH_PLUS: {

			if (!HaveTeamObjective())
				return true;

			actioner_entity->animation = &actioner_entity->morale_boost;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->morale_boost.Reset();


				std::string tmp_dmg = "DEXTERITY X 2.25";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);

				//TODO IMPLEMENT LOSE HP
				Altered_Stat dexterity;
				dexterity.turn_left = 3;
				dexterity.stat_effect_dexterity = 2;

				receiver_entity->AddAlteredStat(dexterity);
				receiver_entity->AddAlteredStat(dexterity);
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability1T1();

			}
		}
		 break;

		case RAGING_MOCK: {

			if (!HaveTeamObjective())
				return true;

			actioner_entity->animation = &actioner_entity->raging_mock;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->raging_mock.Reset();


				std::string tmp_dmg = "DEBUFFED JUDGEMENT";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
				tmp_dmg = "DEBUFFED STRENGHT";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 20, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
				tmp_dmg = "DEBUFFED INTELLIGENCE";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 30, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);

				//TODO IMPLEMENT LOSE HP
				Altered_Stat debuff;
				debuff.turn_left = 3;
				debuff.stat_effect_judgement= 2;
				debuff.stat_effect_strength = 2;
				debuff.stat_effect_intelligence = 2;
				receiver_entity->AddAlteredStat(debuff);

				//animate the receiver to hit + audio or smth
				actioner_entity->Ability1T1();

			}
		}
		break;

		case METEOR: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->high_axe;



			ret = actioner_entity->animation->Finished();

			if (ret == true) {

				actioner_entity->SetCurrentManaPoints(actioner_entity->GetCurrentManaPoints() - action_to_perform.mana_points_effect_to_himself);
				App->combat->UpdateManaBarOfEntity(actioner_entity, (-action_to_perform.mana_points_effect_to_himself));

				actioner_entity->high_axe.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity || receiver_entity->IsStunned()) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;
						int dwarfdmg = App->entities->GetDwarf()->GetCurrentStrengthPoints();

						int damage_to_deal = 50 + dwarfdmg;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;
						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}

						if (receiver_entity->IsStunned())
							damage_to_deal = damage_to_deal * 1.5;

						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (critical)
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
						else if (receiver_entity->IsStunned())
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 253,103,6,255 }, 16, nullptr, nullptr);
						else
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);

						Altered_Stat defense;
						defense.turn_left = 1;
						defense.stat_effect_magical_defense = -1;
						defense.stat_effect_physical_defense = -1;
						actioner_entity->AddAlteredStat(defense);

						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				fPoint  posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };

				App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIGH_AXE);
				actioner_entity->Ability2T1();

			}
		}
		break;

		case COUNTER: {

			if (!HaveTeamObjective())
				return true;

			actioner_entity->animation = &actioner_entity->raging_mock;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->raging_mock.Reset();


				std::string tmp_dmg = "BUFFED PHYSICAL DEFENSE";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
				tmp_dmg = "BUFFED MAGICAL DEFENSE";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 20, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
				tmp_dmg = "DEBUFFED STRENGTH x2.25";
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 30, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);

				//TODO IMPLEMENT LOSE HP
				Altered_Stat debuff;
				debuff.turn_left = 3;
				debuff.stat_effect_magical_defense = 2;
				debuff.stat_effect_physical_defense = 2;
				debuff.stat_effect_strength = 2;
				receiver_entity->AddAlteredStat(debuff);

				//animate the receiver to hit + audio or smth
				actioner_entity->Ability1T1();

			}
		}
		break;





		case HEAL: {
			
			if (!HaveTeamObjective())
				return true;

			actioner_entity->animation = &actioner_entity->heal;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {

				actioner_entity->SetCurrentManaPoints(actioner_entity->GetCurrentManaPoints() - action_to_perform.mana_points_effect_to_himself);
				App->combat->UpdateManaBarOfEntity(actioner_entity, (-action_to_perform.mana_points_effect_to_himself));

				actioner_entity->heal.Reset();

				bool critical = false;
				App->entities->GetElf()->GetCurrentIntelligencePoints();
				int damage_to_deal = action_to_perform.health_points_effect;

				damage_to_deal = App->entities->GetElf()->GetCurrentIntelligencePoints();
				receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
				//receiver_entity->animation = &receiver_entity->hit;
				App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
				std::string tmp_dmg = std::to_string(damage_to_deal);

				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 0,255,0,255 }, 14, nullptr, nullptr);
				
				
				//TODO SITO
				fPoint  posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
				
				App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_LOWER_HEALTH);
				
				actioner_entity->Ability1T1();

			}
		}
		break;

		case THUNDER_PUNCH: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->thunder_punch;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->SetCurrentManaPoints(actioner_entity->GetCurrentManaPoints() - action_to_perform.mana_points_effect_to_himself);
				App->combat->UpdateManaBarOfEntity(actioner_entity, (-action_to_perform.mana_points_effect_to_himself));

				actioner_entity->thunder_punch.Reset();
				actioner_entity->animation = &actioner_entity->idle;

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity && !receiver_entity->IsStunned()) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;
						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical)
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
						else
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);

						receiver_entity->Damaged();

						Altered_Stat stun;
						stun.stun = true;
						stun.turn_left = 1;

						receiver_entity->AddAlteredStat(stun);
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				fPoint  posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };

				App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_KICK);

				actioner_entity->Ability1T1(); 

			}
		}
		break;


		case INSIGNIFICANT_MORTALS: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->insignificant_mortals;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->base_stats.agility += 5;
				
				App->gui->AddUIFloatingValue(actioner_entity->position.x + (actioner_entity->animation->GetCurrentFrame().w / 2), actioner_entity->position.y - actioner_entity->animation->GetCurrentFrame().h, "Stats Up!", { 0,255,0,255 }, 14, nullptr, nullptr);
				//animate the receiver to hit + audio or smth
				//fPoint  posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };

				//App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_KICK);

				actioner_entity->Ability3T1();

			}
		}
		break;

		case CLAW_ATTACK:
		{
			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->attack_1;
			ret = actioner_entity->animation->Finished();
			if (ret)
			{
				actioner_entity->SetCurrentManaPoints(actioner_entity->GetCurrentManaPoints() - action_to_perform.mana_points_effect_to_himself);
				App->combat->UpdateManaBarOfEntity(actioner_entity, (-action_to_perform.mana_points_effect_to_himself));

				actioner_entity->attack_1.Reset();
				actioner_entity->animation = &actioner_entity->idle;

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity && !receiver_entity->IsStunned()) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;
						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical)
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
						else
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);

						receiver_entity->Damaged();


					}
				}
			}

		}
		break;

		case BLOCK:
		{
			// TO GUILLERMO

			actioner_entity->animation = &actioner_entity->attack;
			
			ret = actioner_entity->animation->Finished();
			if (ret = true)
			{

				actioner_entity->attack.Reset();
				actioner_entity->animation = &actioner_entity->idle;

				App->gui->AddUIFloatingValue(actioner_entity->position.x + (actioner_entity->animation->GetCurrentFrame().w / 2), actioner_entity->position.y - actioner_entity->animation->GetCurrentFrame().h, "Block Physical Damage", { 0,255,0,255 }, 14, nullptr, nullptr);

				Altered_Stat block;

				block.stat_effect_physical_defense = 2;
				block.turn_left = 2;

				receiver_entity->AddAlteredStat(block);
				receiver_entity->AddAlteredStat(block);
				receiver_entity->AddAlteredStat(block);
				receiver_entity->AddAlteredStat(block);

			}
		}
		break;

		case INFESTED_CLAW:
		{

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->attack;
			ret = actioner_entity->animation->Finished();
			if (ret)
			{
				actioner_entity->SetCurrentManaPoints(actioner_entity->GetCurrentManaPoints() - action_to_perform.mana_points_effect_to_himself);
				App->combat->UpdateManaBarOfEntity(actioner_entity, (-action_to_perform.mana_points_effect_to_himself));

				actioner_entity->attack.Reset();
				actioner_entity->animation = &actioner_entity->idle;

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity && !receiver_entity->IsStunned()) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;
						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical)
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
						else
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);

						receiver_entity->Damaged();


					}
				}
			}
		}
		break;
		case RUSH:
		{
			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->attack;
			ret = actioner_entity->animation->Finished();
			if (ret)
			{
				actioner_entity->SetCurrentManaPoints(actioner_entity->GetCurrentManaPoints() - action_to_perform.mana_points_effect_to_himself);
				App->combat->UpdateManaBarOfEntity(actioner_entity, (-action_to_perform.mana_points_effect_to_himself));

				actioner_entity->attack.Reset();
				actioner_entity->animation = &actioner_entity->idle;

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity && !receiver_entity->IsStunned()) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;
						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical)
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
						else
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);

						receiver_entity->Damaged();


					}
				}
			}
		}
		break;

		case INFERNAL_FIRE:
		{
			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->infernal_fire_animation;
			ret = actioner_entity->animation->Finished();
			if (ret)
			{
				actioner_entity->SetCurrentManaPoints(actioner_entity->GetCurrentManaPoints() - action_to_perform.mana_points_effect_to_himself);
				App->combat->UpdateManaBarOfEntity(actioner_entity, (-action_to_perform.mana_points_effect_to_himself));

				actioner_entity->infernal_fire_animation.Reset();
				actioner_entity->animation = &actioner_entity->idle;

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity && !receiver_entity->IsStunned()) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;
						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical)
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
						else
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);

						receiver_entity->Damaged();


					}
				}
			}
		}
		break;

		case SHADOW_JAB:
		{
			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->shadow_jab_animation;
			ret = actioner_entity->animation->Finished();
			if (ret)
			{
				actioner_entity->SetCurrentManaPoints(actioner_entity->GetCurrentManaPoints() - action_to_perform.mana_points_effect_to_himself);
				App->combat->UpdateManaBarOfEntity(actioner_entity, (-action_to_perform.mana_points_effect_to_himself));

				actioner_entity->shadow_jab_animation.Reset();
				actioner_entity->animation = &actioner_entity->idle;

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity && !receiver_entity->IsStunned()) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;
						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical)
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
						else
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);

						receiver_entity->Damaged();


					}
				}
			}
		}
		break;

		case CALL_OF_THE_DEAD:
		{
			//NO IDEA
			/*if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->attack;
			ret = actioner_entity->animation->Finished();
			if (ret)
			{
				actioner_entity->SetCurrentManaPoints(actioner_entity->GetCurrentManaPoints() - action_to_perform.mana_points_effect_to_himself);
				App->combat->UpdateManaBarOfEntity(actioner_entity, (-action_to_perform.mana_points_effect_to_himself));

				actioner_entity->attack.Reset();
				actioner_entity->animation = &actioner_entity->idle;

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity && !receiver_entity->IsStunned()) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;
						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical)
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
						else
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);

						receiver_entity->Damaged();


					}
				}
			}*/
		}
		break;

		case BROTHERLY_RAGE:
		{
			if (!HaveObjective())
				return true;
			//TO GUILLERMO
			actioner_entity->animation = &actioner_entity->attack;
			ret = actioner_entity->animation->Finished();
			if (ret)
			{
				actioner_entity->SetCurrentManaPoints(1000);
				actioner_entity->attack_2.Reset();
				actioner_entity->animation = &actioner_entity->idle;
				App->gui->AddUIFloatingValue(actioner_entity->position.x + (actioner_entity->animation->GetCurrentFrame().w / 2), actioner_entity->position.y - actioner_entity->animation->GetCurrentFrame().h, "BUFF STRENGHT++ ", { 0,255,0,255 }, 14, nullptr, nullptr);
				App->gui->AddUIFloatingValue(actioner_entity->position.x + (actioner_entity->animation->GetCurrentFrame().w / 2), actioner_entity->position.y - actioner_entity->animation->GetCurrentFrame().h+10, "BUFF CONSTITUTION+ ", { 0,255,0,255 }, 14, nullptr, nullptr);
				App->gui->AddUIFloatingValue(actioner_entity->position.x + (actioner_entity->animation->GetCurrentFrame().w / 2), actioner_entity->position.y - actioner_entity->animation->GetCurrentFrame().h+20, "RESTORED ALL MANA ", { 0,255,0,255 }, 14, nullptr, nullptr);
				App->gui->AddUIFloatingValue(actioner_entity->position.x + (actioner_entity->animation->GetCurrentFrame().w / 2), actioner_entity->position.y - actioner_entity->animation->GetCurrentFrame().h+30, "JUDGEMENT -- ", { 0,255,0,255 }, 14, nullptr, nullptr);
				Altered_Stat stats_up;

				stats_up.stat_effect_constitution = 1;
				stats_up.stat_effect_judgement = -2;
				stats_up.stat_effect_strength = 2;
				stats_up.turn_left = 2;

				receiver_entity->AddAlteredStat(stats_up);

			}

		}
		break;

		case LIGHT_STRIKE: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->light_strike;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->light_strike.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}




						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability2T1();

			}
		}
		break;

		case MACE_THROW: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->mace_throw;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->mace_throw.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}




						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability3T1();

			}
		}
		break;

		case HEAL_PLUS: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->heal_plus;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->heal_plus.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}




						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability1T1();

			}
		}
		break;

		case LIGHT_STRIKE_PLUS: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->light_strike_plus;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->light_strike_plus.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}




						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability2T1();

			}
		}
		break;

		case MACE_THROW_PLUS: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->mace_throw_plus;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->mace_throw_plus.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}




						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability3T1();

			}
		}
		break;

		case BLESSING: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->blessing;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->blessing.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}




						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability1T2();

			}
		}
		break;

		case HARDEN_SKIN: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->harder_skin;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->harder_skin.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}




						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability2T2();

			}
		}
		break;

		case CLARITY: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->clarity;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->clarity.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}




						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability3T2();

			}
		}
		 break;


		case BLIZZARD: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->blizzard;



			ret = actioner_entity->animation->Finished();

			if (ret == true) {

				actioner_entity->SetCurrentManaPoints(actioner_entity->GetCurrentManaPoints() - action_to_perform.mana_points_effect_to_himself);
				App->combat->UpdateManaBarOfEntity(actioner_entity, (-action_to_perform.mana_points_effect_to_himself));

				actioner_entity->blizzard.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity || receiver_entity->IsStunned()) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentMagicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;
						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}

						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical)
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
						else
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);

						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				fPoint  posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };

				App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_MINDBLOWN);
				actioner_entity->Ability2T2();

			}
		}
		break;

		case SEED_OF_LIFE: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->seed_of_life;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->seed_of_life.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}




						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability1T1();

			}
		}
		break;

		case FIREBALL: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->fireball;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->fireball.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}




						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability2T1();

			}
		}
		break;

		case LIGHTNING_BOLT: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->lightning_bolt;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->lightning_bolt.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}




						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability3T1();

			}
		}
		break;

		case SEED_OF_LIFE_PLUS: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->seed_of_life_plus;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->seed_of_life_plus.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}




						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability1T1();

			}
		}
		 break;

		case FIREBALL_PLUS: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->fireball_plus;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->fireball_plus.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}




						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability2T1();

			}
		}
		 break;

		case LIGHTNING_BOLT_PLUS: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->lightning_bol_plus;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->lightning_bol_plus.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}




						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability3T1();

			}
		}
		break;

		case REVIVE: {
			if (!HaveDeadTeamObjective())
				return true;

			actioner_entity->animation = &actioner_entity->revive;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->revive.Reset();


				bool critical = false;

				int damage_to_deal = action_to_perform.health_points_effect;
						
				receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
					
				App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
				std::string tmp_dmg = std::to_string(damage_to_deal);
						
				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 8,160,8,255 }, 14, nullptr, nullptr);
				
				fPoint posP;

				posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
				App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HEALTH_AREA);

				actioner_entity->Ability1T2();
			}
		}
		break;

		case FIRE_DJINN: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->fire_djinn;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->fire_djinn.Reset();

				int actioner_dexterity = BASE_DEXTERITY + actioner_entity->GetCurrentDexterityPoints();

				int random_thousand_faces_die = (rand() % 100) + 1;

				if (random_thousand_faces_die <= actioner_dexterity) {// THE ACTIONER HITS THE RECEIVER
					int receiver_agility = BASE_AGILITY + receiver_entity->GetCurrentAgilityPoints();

					random_thousand_faces_die = (rand() % 100) + 1;
					if (random_thousand_faces_die <= receiver_agility && !receiver_entity->IsStunned()) {// THE RECEIVER DODGES THE ATTACK
						App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Dodge", { 204,204,0,255 }, 14, nullptr, nullptr);
						receiver_entity->animation = &receiver_entity->dodge;
					}
					else {// THE ATTACK HITS

						bool critical = false;

						int damage_to_deal = action_to_perform.health_points_effect;
						float damage_reduction = (float)receiver_entity->GetCurrentPhysicalDefensePoints() / 100 * (float)damage_to_deal;
						actioner_dexterity = actioner_dexterity / 10;

						random_thousand_faces_die = (rand() % 100) + 1;
						if (random_thousand_faces_die <= actioner_dexterity) {
							damage_to_deal = damage_to_deal * CRITICAL_VALUE;
							critical = true;
						}
						damage_to_deal = damage_to_deal - damage_reduction;
						receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
						receiver_entity->animation = &receiver_entity->hit;
						App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
						std::string tmp_dmg = std::to_string(damage_to_deal);
						if (!critical) {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,0,255 }, 14, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_ENEMY);
							}
						}
						else {
							App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,0,255,255 }, 16, nullptr, nullptr);
							//TODO SITO
							fPoint posP;
							if (receiver_entity->type == CLERIC || receiver_entity->type == WARRIOR || receiver_entity->type == ELF || receiver_entity->type == DWARF)
							{
								posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_HEROES);
							}
							else
							{
								posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
								App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HIT_CRITICAL_ENEMY);
							}
						}




						receiver_entity->Damaged();
					}
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
				//animate the receiver to hit + audio or smth
				actioner_entity->Ability3T2();

			}
		}
						 break;




		case LOW_HEALTH_RECOVER_ACTION: {

			if (!HaveTeamObjective())
				return true;

			actioner_entity->animation = &actioner_entity->throw_object;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {

				//todo reducir la quantity
				for (int i = 0; i < actioner_entity->usable_items.size(); i++)
				{
					if (actioner_entity->usable_items.at(i).action.type == action_to_perform.type) {
						actioner_entity->usable_items.at(i).quantity--;
						if (actioner_entity->usable_items.at(i).quantity == 0)
							actioner_entity->usable_items.erase(actioner_entity->usable_items.cbegin() + i);
						break;
					}
				}

				actioner_entity->throw_object.Reset();

				bool critical = false;

				int damage_to_deal = action_to_perform.health_points_effect;

				damage_to_deal = damage_to_deal;
				receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
				//receiver_entity->animation = &receiver_entity->hit;
				App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
				std::string tmp_dmg = std::to_string(damage_to_deal);

				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 0,255,0,255 }, 14, nullptr, nullptr);


				//TODO SITO
				fPoint  posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
				App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_LOWER_HEALTH);
				

				//actioner_entity->Ability1();

			}
		}
		break;

		case HIGH_HEALTH_RECOVER_ACTION: { 

			if (!HaveTeamObjective())
				return true;

			actioner_entity->animation = &actioner_entity->throw_object;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {

				//todo reducir la quantity
				for (int i = 0; i < actioner_entity->usable_items.size(); i++)
				{
					if (actioner_entity->usable_items.at(i).action.type == action_to_perform.type) {
						actioner_entity->usable_items.at(i).quantity--;
						if (actioner_entity->usable_items.at(i).quantity == 0)
							actioner_entity->usable_items.erase(actioner_entity->usable_items.cbegin() + i);
						break;
					}
				}

				actioner_entity->throw_object.Reset();

				bool critical = false;

				int damage_to_deal = action_to_perform.health_points_effect;

				damage_to_deal = damage_to_deal;
				receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);
				//receiver_entity->animation = &receiver_entity->hit;
				App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
				std::string tmp_dmg = std::to_string(damage_to_deal);

				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 0,255,0,255 }, 14, nullptr, nullptr);


				fPoint  posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
				App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_LOWER_HEALTH);

			}
		}
		break;

		case LOW_MANA_RECOVER_ACTION: {

			if (!HaveTeamObjective())
				return true;

			actioner_entity->animation = &actioner_entity->throw_object;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {

				//todo reducir la quantity
				for (int i = 0; i < actioner_entity->usable_items.size(); i++)
				{
					if (actioner_entity->usable_items.at(i).action.type == action_to_perform.type) {
						actioner_entity->usable_items.at(i).quantity--;
						if (actioner_entity->usable_items.at(i).quantity == 0)
							actioner_entity->usable_items.erase(actioner_entity->usable_items.cbegin() + i);
						break;
					}
				}

				actioner_entity->throw_object.Reset();

				bool critical = false;

				int damage_to_deal = action_to_perform.mana_points_effect;
				damage_to_deal = damage_to_deal;
				receiver_entity->SetCurrentManaPoints(receiver_entity->GetCurrentManaPoints()+damage_to_deal);
				//receiver_entity->animation = &receiver_entity->hit;
				App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
				std::string tmp_dmg = std::to_string(damage_to_deal);

				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 0,0,255,255 }, 14, nullptr, nullptr);


				fPoint  posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
				App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_MANA_POTION);

			}
		}
		break;

		case HIGH_MANA_RECOVER_ACTION: {

			if (!HaveTeamObjective())
				return true;

			actioner_entity->animation = &actioner_entity->throw_object;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {

				//todo reducir la quantity
				for (int i = 0; i < actioner_entity->usable_items.size(); i++)
				{
					if (actioner_entity->usable_items.at(i).action.type == action_to_perform.type) {
						actioner_entity->usable_items.at(i).quantity--;
						if (actioner_entity->usable_items.at(i).quantity == 0)
							actioner_entity->usable_items.erase(actioner_entity->usable_items.cbegin() + i);
						break;
					}
				}

				actioner_entity->throw_object.Reset();

				bool critical = false;

				int damage_to_deal = action_to_perform.mana_points_effect;
				damage_to_deal = damage_to_deal;
				receiver_entity->SetCurrentManaPoints(receiver_entity->GetCurrentManaPoints() + damage_to_deal);
				//receiver_entity->animation = &receiver_entity->hit;
				App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
				std::string tmp_dmg = std::to_string(damage_to_deal);

				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 0,0,255,255 }, 14, nullptr, nullptr);


				fPoint  posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
				App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_MANA_POTION);

			}
		}
		break;

		case POISONED_DAGGER_ACTION: {

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->throw_object;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {

				if (!receiver_entity->IsPoisoned()) {
					//todo reducir la quantity
					for (int i = 0; i < actioner_entity->usable_items.size(); i++)
					{
						if (actioner_entity->usable_items.at(i).action.type == action_to_perform.type) {
							actioner_entity->usable_items.at(i).quantity--;
							if (actioner_entity->usable_items.at(i).quantity == 0)
								actioner_entity->usable_items.erase(actioner_entity->usable_items.cbegin() + i);
							break;
						}
					}

					actioner_entity->throw_object.Reset();

					std::string tmp_dmg = "POISONED";

					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 127,0,85,255 }, 16, nullptr, nullptr);


					Altered_Stat poison;
					poison.poison = true;
					poison.turn_left = 3;

					receiver_entity->AddAlteredStat(poison);

					//TODO SITO
					fPoint  posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
					App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_POISON);

				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}
			}
		}
		break;

		case BURN_TARGET_ACTION:
		{

			if (!HaveObjective())
				return true;

			actioner_entity->animation = &actioner_entity->throw_object;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->throw_object.Reset();
				//todo reducir la quantity
				for (int i = 0; i < actioner_entity->usable_items.size(); i++)
				{
					if (actioner_entity->usable_items.at(i).action.type == action_to_perform.type) {
						actioner_entity->usable_items.at(i).quantity--;
						if (actioner_entity->usable_items.at(i).quantity == 0)
							actioner_entity->usable_items.erase(actioner_entity->usable_items.cbegin() + i);
						break;
					}
				}

				if (!receiver_entity->IsBurning()) {
					
					std::string tmp_dmg = "BURNING";

					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 255,80,80,255 }, 16, nullptr, nullptr);


					fPoint  posP = { (float)(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2)), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
					App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_BURNING);
					//actioner_entity->Ability1();

					Altered_Stat burn;
					burn.burn = true;
					burn.turn_left = 3;

					receiver_entity->AddAlteredStat(burn);
				}
				else {//ACTIONER MISSES!
					App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, "Miss", { 0,102,204,255 }, 14, nullptr, nullptr);
				}

			}
		}
		break;
		case REVIVE_AN_ALLY_ACTION:
		{

			if (!HaveDeadTeamObjective())
				return true;

			actioner_entity->animation = &actioner_entity->throw_object;

			ret = actioner_entity->animation->Finished();

			if (ret == true) {
				actioner_entity->throw_object.Reset();


				bool critical = false;

				int damage_to_deal = action_to_perform.health_points_effect;

				receiver_entity->SetCurrentHealthPoints(receiver_entity->GetCurrentHealthPoints() + damage_to_deal);

				App->combat->UpdateHPBarOfEntity(receiver_entity, damage_to_deal);
				std::string tmp_dmg = std::to_string(damage_to_deal);

				App->gui->AddUIFloatingValue(receiver_entity->position.x + (receiver_entity->animation->GetCurrentFrame().w / 2), receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h - 10, tmp_dmg, { 8,160,8,255 }, 14, nullptr, nullptr);

				fPoint posP;

				posP = { (float)(receiver_entity->position.x), (float)(receiver_entity->position.y - receiver_entity->animation->GetCurrentFrame().h / 2) };
				App->psystem->AddEmiter(posP, EmitterType::EMITTER_TYPE_HEALTH_AREA);

			}
		}
		break;


		default:
			break;
		}
	}
	else {
		ret = true;
	}
		

	return ret;
}

bool PerformActionToEntity::HaveObjective()
{

	if (receiver_entity->GetCurrentHealthPoints() == 0) {
		if (actioner_entity->type == ELF || actioner_entity->type == CLERIC || actioner_entity->type == WARRIOR || actioner_entity->type == DWARF) {
			for (int i = 0; i < App->combat->enemies.size(); i++)
			{
				receiver_entity = App->combat->enemies.at(i);
				if (receiver_entity->GetCurrentHealthPoints() != 0)
					break;
			}
			if (receiver_entity->GetCurrentHealthPoints() == 0)
				return false;
		}
		else {
			for (int i = 0; i < App->combat->heroes.size(); i++)
			{
				receiver_entity = App->combat->heroes.at(i);
				if (receiver_entity->GetCurrentHealthPoints() != 0)
					break;
			}
			if (receiver_entity->GetCurrentHealthPoints() == 0)
				return false;
		}
	}

	return true;
}

bool PerformActionToEntity::HaveTeamObjective()
{
	if (receiver_entity->GetCurrentHealthPoints() == 0) {

		for (int i = 0; i < App->combat->heroes.size(); i++)
		{
			receiver_entity = App->combat->heroes.at(i);
			if (receiver_entity->GetCurrentHealthPoints() != 0)
				break;
		}
		if (receiver_entity->GetCurrentHealthPoints() == 0)
			return false;

	}
	return true;
}

bool PerformActionToEntity::HaveDeadTeamObjective()
{
	if (receiver_entity->GetCurrentHealthPoints() != 0) {

		for (int i = 0; i < App->combat->heroes.size(); i++)
		{
			receiver_entity = App->combat->heroes.at(i);
			if (receiver_entity->GetCurrentHealthPoints() == 0)
				break;
		}
		if (receiver_entity->GetCurrentHealthPoints() != 0)
			return false;

	}
	return true;
}

bool ctTaskManager::Update(float dt)
{
	TaskOrderer();

	bool ret = false;

	ret = DoTask();

	return ret;
}

bool ctTaskManager::Start()
{
	/* initialize random seed: */
	srand(time(NULL));
	return true;
}

bool ctTaskManager::CleanUp()
{
	while (TaskQueue.size() != 0)
	{
		TaskQueue.pop_front();
	}
	return true;
}

void ctTaskManager::PerformAllTheTasks()
{
	if (aux_task == nullptr && TaskQueue.size() != 0)
	{
		aux_task = TaskQueue.front();
		TaskQueue.pop_front();
	}
}

void ctTaskManager::OrderTasksByEntities(std::vector<Entity*> turn_priority_entity)
{
	std::list<Task*> task_tmp;
	
	int i = 0;
	
	while (i < turn_priority_entity.size())
	{
		for (std::list<Task*>::const_iterator it_list = TaskQueue.begin(); it_list != TaskQueue.end(); ++it_list)
		{
			if ((*it_list)->performed_by == turn_priority_entity[i])
			{
				task_tmp.push_back((*it_list));
			}

		}

		i++;
	}
	TaskQueue = task_tmp;
}

void ctTaskManager::DeleteTasksFromEntity(Entity * entity)
{
	std::list<Task*>::const_iterator it_list = TaskQueue.begin();

	while (it_list != TaskQueue.end()) {
		if ((*it_list)->performed_by == entity) {
			TaskQueue.remove((*it_list));
		}
		it_list++;
	}
}

bool ctTaskManager::AddTask(Task * task)
{
	TaskQueue.push_back(task);
	return true;
}

bool ctTaskManager::DoTask()
{
	if (aux_task != nullptr)
	{

		if (aux_task->Execute())
		{
			if (TaskQueue.size() != 0)
			{
				aux_task = TaskQueue.front();
				TaskQueue.pop_front();
			}
			else 
				aux_task = nullptr;

		}
	}
	return true;
}

bool ctTaskManager::TaskOrderer()
{
	
	return true;
}

bool MoveAvatarsToPosition::Execute()
{
	bool ret = false;


	if (((entity_to_move->position.x + 10) >= position_to_go.x && (entity_to_move->position.x - 10) <= position_to_go.x) && ((entity_to_move->position.y + 10) >= position_to_go.y && (entity_to_move->position.y - 10) <= position_to_go.y))
	{
		//entity_to_move->animation = &entity_to_move->idle;
		ret = true;
	}
	else {
		if (entity_to_move->position.x < position_to_go.x)
		{
			entity_to_move->animation = &entity_to_move->run_forward;
			entity_to_move->position.x += 1;
		}

		else if (entity_to_move->position.x > position_to_go.x)
		{
			entity_to_move->animation = &entity_to_move->run_forward;
			entity_to_move->position.x -= 1;
		}

		if (entity_to_move->position.y < position_to_go.y)
		{
			entity_to_move->animation = &entity_to_move->run_forward;
			entity_to_move->position.y += 1;
		}

		else if (entity_to_move->position.y > position_to_go.y)
		{
			entity_to_move->animation = &entity_to_move->run_forward;
			entity_to_move->position.y -= 1;
		}
	}




	return ret;
}
