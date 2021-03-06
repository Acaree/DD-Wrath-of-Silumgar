#include "ctApp.h"
#include "UIFloatingValue.h"
#include "ctLog.h"
#include "ctGui.h"
#include "ctTextures.h"


UIFloatingValue::UIFloatingValue(int x, int y, UI_Type type, std::string text, SDL_Color color, int size, ctModule* callback, const char* path, UIElement* parent) : UIElement(x, y, type, parent)
{
	this->callback = callback;
	this->text = text;

	App->fonts->size = size;
	_TTF_Font* newFont = App->fonts->Load(path, size);

	texture = App->fonts->Print(text.c_str(), color, newFont);

	int width = 0, height = 0;
	App->fonts->CalcSize(this->text.c_str(), width, height, newFont);
	current_rect.w = width;
	current_rect.h = height;
}

void UIFloatingValue::Update()
{
	this->screen_position.y--;

	alpha -= 2;
	if (alpha <= 0)
		alpha = 0;
		
}