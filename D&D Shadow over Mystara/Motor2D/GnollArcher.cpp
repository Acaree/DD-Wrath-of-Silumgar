#include "GnollArcher.h"



GnollArcher::GnollArcher() :Entity(GNOLLARCHER)
{

	pugi::xml_document data;

	if (pugi::xml_parse_result result = data.load_file("Entities.xml"))
	{
		pugi::xml_node anim = data.child("Entities").child("GnollArcher").child("Animations").child("Idle").child("frame");
		pugi::xml_node path = data.child("Entities").child("GnollArcher").child("spritesheetSource");

		anim_Path.assign(path.attribute("name").as_string());

		while (anim != nullptr)
		{
			anim_attack.PushBack({ anim.attribute("x").as_int(), anim.attribute("y").as_int(), anim.attribute("w").as_int(), anim.attribute("h").as_int() });
			anim = anim.next_sibling();
		}

		LoadProperties(data.child("Entities").child("GnollArcher").child("Statistics"));

		//anim_attack.speed = anim.child("speed").attribute("speed").as_int();
		animation = &anim_attack;
	}


}

GnollArcher::~GnollArcher()
{

}