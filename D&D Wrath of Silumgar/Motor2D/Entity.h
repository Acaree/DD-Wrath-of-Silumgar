#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "ctPoint.h"
#include "ctAnimation.h"
#include "ctEntities.h"
#include "ctTextures.h"
#include "SDL/include/SDL.h"

#include <vector>

struct SDL_Texture;

struct Stats {
	int base_constitution = 0u;
	int base_focus = 0u;
	int base_strength = 0u;
	int base_intelligence = 0u;
	int base_dexterity = 0u;
	int base_agility = 0u;
	int base_physical_defense = 0u;
	int base_magical_defense = 0u;
	int base_luck = 0u;
	int base_judgement = 0u;
};

struct Altered_Stat {
	uint turn_left = 0u;

	bool stun = false;
	bool bleeding = false;
	bool poison = false;
	bool burn = false;

	int stat_effect_constitution = 0;
	int stat_effect_focus = 0;
	int stat_effect_strength = 0;
	int stat_effect_intelligence = 0;
	int stat_effect_dexterity = 0;
	int stat_effect_agility = 0;
	int stat_effect_physical_defense = 0;
	int stat_effect_magical_defense = 0;
	int stat_effect_luck = 0;
	int stat_effect_judgement = 0;
};

struct Item {
	uint constitution = 0u;
	uint focus = 0u;
	uint strength = 0u;
	uint intelligence = 0u;
	uint dexterity = 0u;
	uint agility = 0u;
	uint physical_defense = 0u;
	uint magical_defense = 0u;
	uint luck = 0u;
	uint judgement = 0u;
};

enum ActionType {
	DEFAULT_ATTACK,
	KICK,
	HIGH_AXE,
	HEAL,

	ACTION_NOT_DEFINED
};

enum ActionObjectiveType {
	HEROES,
	ENEMIES,
	OBJECTIVE_NOT_DEFINED
};

struct Action {

	std::string name;
	std::string description;
	ActionType type = ACTION_NOT_DEFINED;
	ActionObjectiveType objective = OBJECTIVE_NOT_DEFINED;

	uint mana_cost = 0u;

	int health_points_effect_to_himself = 0;
	int mana_points_effect_to_himself = 0;
	int health_points_effect = 0;
	int mana_points_effect = 0;

	uint stun_chance = 0u;
	uint bleeding_chance = 0u;
	uint position_chance = 0u;
	uint burn_chance = 0u;

	uint constitution_variation = 0u;
	uint focus_variation = 0u;
	uint strength_variation = 0u;
	uint intelligence_variation = 0u;
	uint dexterity_variation = 0u;
	uint agility_variation = 0u;
	uint physical_defense_variation = 0u;
	uint magical_defense_variation = 0u;
	uint luck_variation = 0u;
	uint judgement_variation = 0u;

};



class Entity
{
protected:
	
	bool key_entities_speed = false;

	int current_health_points, current_mana_points, current_agility_points, current_dexterity_points, current_physical_defense_points, current_judgement = 0;

	bool dead = false;

public:
	ctAnimation* animation = nullptr;
	ctAnimation* stun_animation = nullptr;

	ctAnimation idle = ctAnimation();
	ctAnimation run_forward = ctAnimation();
	ctAnimation run_backward = ctAnimation();
	ctAnimation attack = ctAnimation();
	ctAnimation hit = ctAnimation();
	ctAnimation death = ctAnimation();
	ctAnimation stun = ctAnimation();

	//custom animations
	ctAnimation kick = ctAnimation();
	ctAnimation high_axe = ctAnimation();
	ctAnimation heal = ctAnimation();

	Stats base_stats;
	iPoint position = iPoint(0, 0);
	iPoint initial_position = iPoint(0, 0);
	EntityType type = EntityType::NO_TYPE;
	bool to_destroy = false;
	bool flip_texture = false;
	
	SDL_Texture* texture = nullptr;

	//Priority Draw Order
	uint priority_draw_order = 0u;

	//Combat stuff
	Action default_attack;

	std::vector<Action> abilities;
	std::vector<Altered_Stat> altered_stats;

	//Sounds stuff
	uint attack_fx = 0u;
	uint death_fx = 0u;
	uint run_fx = 0u;
	uint damaged_fx = 0u;
	uint ability_1_fx = 0u;
	uint steps_timer = 0u;
	uint current_timer = 0u;
public:
	Entity(int x, int y, EntityType type);
	virtual ~Entity();

	virtual void Update(float dt) {};
	virtual void Draw();
	virtual void SetEntitiesSpeed(float dt) {};

	virtual void LoadAnimation(pugi::xml_node animation_node, ctAnimation* animation) {};
	bool LoadProperties(pugi::xml_node properties);

	//for sounds
	virtual void Attack() {};
	virtual void Run() {};
	virtual void Death() {};
	virtual void Damaged() {};
	virtual void Ability1() {};

	//Combat stuff

	virtual void NewTurn();

	virtual void PerformAction() {};

	void AddAlteredStat(Altered_Stat new_altered_stat);

	int GetCurrentHealthPoints();
	int GetCurrentManaPoints();
	int GetCurrentAgilityPoints();
	int GetCurrentDexterityPoints();
	int GetCurrentPhysicalDefensePoints();

	int GetCurrentJudgement();

	void SetCurrentHealthPoints(int new_health_points);
	void SetCurrentManaPoints(int new_mana_points);

	bool IsGoingToDoAnythingClever();

	void AddAction(Action new_action);
};

#endif // __ENTITY_H__