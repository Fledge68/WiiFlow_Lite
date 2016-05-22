/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_optionbrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

/**
 * Constructor for the GuiOptionBrowser class.
 */
GuiOptionBrowser::GuiOptionBrowser(int w, int h, OptionList * l)
{
	width = w;
	height = h;
	options = l;
	selectable = true;
	listOffset = this->FindMenuItem(-1, 1);
	listChanged = true; // trigger an initial list update
	selectedItem = 0;
	focus = 0; // allow focus

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trig2 = new GuiTrigger;
	trig2->SetSimpleTrigger(-1, WPAD_BUTTON_2, 0);

	btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

	bgOptions = new GuiImageData(bg_options_png);
	bgOptionsImg = new GuiImage(bgOptions);
	bgOptionsImg->SetParent(this);
	bgOptionsImg->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);

	bgOptionsEntry = new GuiImageData(bg_options_entry_png);

	scrollbar = new GuiImageData(scrollbar_png);
	scrollbarImg = new GuiImage(scrollbar);
	scrollbarImg->SetParent(this);
	scrollbarImg->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbarImg->SetPosition(0, 30);

	arrowDown = new GuiImageData(scrollbar_arrowdown_png);
	arrowDownImg = new GuiImage(arrowDown);
	arrowDownOver = new GuiImageData(scrollbar_arrowdown_over_png);
	arrowDownOverImg = new GuiImage(arrowDownOver);
	arrowUp = new GuiImageData(scrollbar_arrowup_png);
	arrowUpImg = new GuiImage(arrowUp);
	arrowUpOver = new GuiImageData(scrollbar_arrowup_over_png);
	arrowUpOverImg = new GuiImage(arrowUpOver);

	arrowUpBtn = new GuiButton(arrowUpImg->GetWidth(), arrowUpImg->GetHeight());
	arrowUpBtn->SetParent(this);
	arrowUpBtn->SetImage(arrowUpImg);
	arrowUpBtn->SetImageOver(arrowUpOverImg);
	arrowUpBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	arrowUpBtn->SetSelectable(false);
	arrowUpBtn->SetTrigger(trigA);
	arrowUpBtn->SetSoundOver(btnSoundOver);
	arrowUpBtn->SetSoundClick(btnSoundClick);

	arrowDownBtn = new GuiButton(arrowDownImg->GetWidth(), arrowDownImg->GetHeight());
	arrowDownBtn->SetParent(this);
	arrowDownBtn->SetImage(arrowDownImg);
	arrowDownBtn->SetImageOver(arrowDownOverImg);
	arrowDownBtn->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	arrowDownBtn->SetSelectable(false);
	arrowDownBtn->SetTrigger(trigA);
	arrowDownBtn->SetSoundOver(btnSoundOver);
	arrowDownBtn->SetSoundClick(btnSoundClick);

	for(int i=0; i<PAGESIZE; i++)
	{
		optionTxt[i] = new GuiText(NULL, 20, (GXColor){0, 0, 0, 0xff});
		optionTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		optionTxt[i]->SetPosition(8,0);

		optionVal[i] = new GuiText(NULL, 20, (GXColor){0, 0, 0, 0xff});
		optionVal[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		optionVal[i]->SetPosition(250,0);

		optionBg[i] = new GuiImage(bgOptionsEntry);

		optionBtn[i] = new GuiButton(512,30);
		optionBtn[i]->SetParent(this);
		optionBtn[i]->SetLabel(optionTxt[i], 0);
		optionBtn[i]->SetLabel(optionVal[i], 1);
		optionBtn[i]->SetImageOver(optionBg[i]);
		optionBtn[i]->SetPosition(0,30*i+3);
		optionBtn[i]->SetTrigger(trigA);
		optionBtn[i]->SetTrigger(trig2);
		optionBtn[i]->SetSoundClick(btnSoundClick);
	}
}

/**
 * Destructor for the GuiOptionBrowser class.
 */
GuiOptionBrowser::~GuiOptionBrowser()
{
	delete arrowUpBtn;
	delete arrowDownBtn;

	delete bgOptionsImg;
	delete scrollbarImg;
	delete arrowDownImg;
	delete arrowDownOverImg;
	delete arrowUpImg;
	delete arrowUpOverImg;

	delete bgOptions;
	delete bgOptionsEntry;
	delete scrollbar;
	delete arrowDown;
	delete arrowDownOver;
	delete arrowUp;
	delete arrowUpOver;

	delete trigA;
	delete trig2;
	delete btnSoundOver;
	delete btnSoundClick;

	for(int i=0; i<PAGESIZE; i++)
	{
		delete optionTxt[i];
		delete optionVal[i];
		delete optionBg[i];
		delete optionBtn[i];
	}
}

void GuiOptionBrowser::SetCol1Position(int x)
{
	for(int i=0; i<PAGESIZE; i++)
		optionTxt[i]->SetPosition(x,0);
}

void GuiOptionBrowser::SetCol2Position(int x)
{
	for(int i=0; i<PAGESIZE; i++)
		optionVal[i]->SetPosition(x,0);
}

void GuiOptionBrowser::SetFocus(int f)
{
	focus = f;

	for(int i=0; i<PAGESIZE; i++)
		optionBtn[i]->ResetState();

	if(f == 1)
		optionBtn[selectedItem]->SetState(STATE_SELECTED);
}

void GuiOptionBrowser::ResetState()
{
	if(state != STATE_DISABLED)
	{
		state = STATE_DEFAULT;
		stateChan = -1;
	}

	for(int i=0; i<PAGESIZE; i++)
	{
		optionBtn[i]->ResetState();
	}
}

int GuiOptionBrowser::GetClickedOption()
{
	int found = -1;
	for(int i=0; i<PAGESIZE; i++)
	{
		if(optionBtn[i]->GetState() == STATE_CLICKED)
		{
			optionBtn[i]->SetState(STATE_SELECTED);
			found = optionIndex[i];
			break;
		}
	}
	return found;
}

/****************************************************************************
 * FindMenuItem
 *
 * Help function to find the next visible menu item on the list
 ***************************************************************************/

int GuiOptionBrowser::FindMenuItem(int currentItem, int direction)
{
	int nextItem = currentItem + direction;

	if(nextItem < 0 || nextItem >= options->length)
		return -1;

	if(strlen(options->name[nextItem]) > 0)
		return nextItem;
	else
		return FindMenuItem(nextItem, direction);
}

/**
 * Draw the button on screen
 */
void GuiOptionBrowser::Draw()
{
	if(!this->IsVisible())
		return;

	bgOptionsImg->Draw();

	int next = listOffset;

	for(int i=0; i<PAGESIZE; ++i)
	{
		if(next >= 0)
		{
			optionBtn[i]->Draw();
			next = this->FindMenuItem(next, 1);
		}
		else
			break;
	}

	scrollbarImg->Draw();
	arrowUpBtn->Draw();
	arrowDownBtn->Draw();

	this->UpdateEffects();
}

void GuiOptionBrowser::TriggerUpdate()
{
	listChanged = true;
}

void GuiOptionBrowser::ResetText()
{
	int next = listOffset;

	for(int i=0; i<PAGESIZE; i++)
	{
		if(next >= 0)
		{
			optionBtn[i]->ResetText();
			next = this->FindMenuItem(next, 1);
		}
		else
			break;
	}
}

void GuiOptionBrowser::Update(GuiTrigger * t)
{
	if(state == STATE_DISABLED || !t)
		return;

	int next, prev;

	arrowUpBtn->Update(t);
	arrowDownBtn->Update(t);

	next = listOffset;

	if(listChanged)
	{
		listChanged = false;
		for(int i=0; i<PAGESIZE; ++i)
		{
			if(next >= 0)
			{
				if(optionBtn[i]->GetState() == STATE_DISABLED)
				{
					optionBtn[i]->SetVisible(true);
					optionBtn[i]->SetState(STATE_DEFAULT);
				}

				optionTxt[i]->SetText(options->name[next]);
				optionVal[i]->SetText(options->value[next]);
				optionIndex[i] = next;
				next = this->FindMenuItem(next, 1);
			}
			else
			{
				optionBtn[i]->SetVisible(false);
				optionBtn[i]->SetState(STATE_DISABLED);
			}
		}
	}

	for(int i=0; i<PAGESIZE; ++i)
	{
		if(i != selectedItem && optionBtn[i]->GetState() == STATE_SELECTED)
			optionBtn[i]->ResetState();
		else if(focus && i == selectedItem && optionBtn[i]->GetState() == STATE_DEFAULT)
			optionBtn[selectedItem]->SetState(STATE_SELECTED, t->chan);

		int currChan = t->chan;

		if(t->wpad->ir.valid && !optionBtn[i]->IsInside(t->wpad->ir.x, t->wpad->ir.y))
			t->chan = -1;

		optionBtn[i]->Update(t);
		t->chan = currChan;

		if(optionBtn[i]->GetState() == STATE_SELECTED)
			selectedItem = i;
	}

	// pad/joystick navigation
	if(!focus)
		return; // skip navigation

	if(t->Down() || arrowDownBtn->GetState() == STATE_CLICKED)
	{
		next = this->FindMenuItem(optionIndex[selectedItem], 1);

		if(next >= 0)
		{
			if(selectedItem == PAGESIZE-1)
			{
				// move list down by 1
				listOffset = this->FindMenuItem(listOffset, 1);
				listChanged = true;
			}
			else if(optionBtn[selectedItem+1]->IsVisible())
			{
				optionBtn[selectedItem]->ResetState();
				optionBtn[selectedItem+1]->SetState(STATE_SELECTED, t->chan);
				++selectedItem;
			}
		}
		arrowDownBtn->ResetState();
	}
	else if(t->Up() || arrowUpBtn->GetState() == STATE_CLICKED)
	{
		prev = this->FindMenuItem(optionIndex[selectedItem], -1);

		if(prev >= 0)
		{
			if(selectedItem == 0)
			{
				// move list up by 1
				listOffset = prev;
				listChanged = true;
			}
			else
			{
				optionBtn[selectedItem]->ResetState();
				optionBtn[selectedItem-1]->SetState(STATE_SELECTED, t->chan);
				--selectedItem;
			}
		}
		arrowUpBtn->ResetState();
	}

	if(updateCB)
		updateCB(this);
}
