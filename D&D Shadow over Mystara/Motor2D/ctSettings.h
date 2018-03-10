#ifndef __ctSETTINGS_H__
#define __ctSETTINGS_H__

#include "ctModule.h"
#include "ctGui.h"


class ctSettings : public ctModule
{
public:

	ctSettings();

	// Destructor
	virtual ~ctSettings();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	bool Load(pugi::xml_node&);

	bool Save(pugi::xml_node&) const;

	void OnUITrigger(UIElement* elementTriggered, UI_State ui_state);

private:
	bool quit_pressed = false;
	bool first_update = true;

	UIElement* music_volume;
	UIElement* fx_volume;
	UIElement* back;
	

	

};


#endif // __ctSETTINGS_H__