#include "ctApp.h"
#include "UIVerticalSliceInfo.h"
#include "ctLog.h"
#include "ctInput.h"
#include "ctAudio.h"

UIVerticalSliceInfo:: UIVerticalSliceInfo(int x, int y,bool overcome,ctModule* callback, UIElement* parent) : UIElement(x, y, type, parent)
{
	this->callback = callback;
	
	background = App->gui->AddUIImage(x, y, { 1141,484,340,178 },callback);
	background->alpha = 0;
	if (overcome == true)
	{
		congratulations = App->gui->AddUITextBox(x+20, y+20, 15, 200, "CONGRATULATIONS!!", {255,255,255,255},this, Second_Font);
		congratulations->alpha = 0;
		text = App->gui->AddUITextBox(x+20,y+50, 12,320,"You have managed to overcome the first part of this adventure. We hope you've enjoyed this gameplay. Thanks for playing and we hope to see you soon.", { 255,255,255,255 }, this, Second_Font);
		text->alpha = 0;
	}
	else if (overcome == false)
	{
		congratulations = App->gui->AddUITextBox(x + 20, y + 20, 15, 200, "TRY AGAIN!!", { 255,255,255,255 }, this, Second_Font);
		congratulations->alpha = 0;
		text = App->gui->AddUITextBox(x + 20, y + 50, 12, 320, "Your efforts have not been enough to defeat Lich's troops, try again!", { 255,255,255,255 }, this, Second_Font);
		text->alpha = 0;
	}

	
}

UIVerticalSliceInfo::~UIVerticalSliceInfo() {
	background->to_destroy = true;
	congratulations->to_destroy = true;
	text->to_destroy = true;
	callback = nullptr;
	
}

void UIVerticalSliceInfo::Update()
{
	background->alpha += 3;
	if (background->alpha >= 255)
		background->alpha = 255;
	congratulations->alpha += 3;
	if (congratulations->alpha >= 255)
		congratulations->alpha = 255;
	text->alpha += 3;
	if (text->alpha >= 255)
		text->alpha = 255;

	/*background->alpha -= 4;
	if (background->alpha <= 0)
		background->alpha = 0;*/

}