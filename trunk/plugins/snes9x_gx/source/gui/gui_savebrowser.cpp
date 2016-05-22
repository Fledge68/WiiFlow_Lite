/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_savebrowser.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"
#include "filebrowser.h"

/**
 * Constructor for the GuiSaveBrowser class.
 */
GuiSaveBrowser::GuiSaveBrowser(int w, int h, SaveList * s, int a)
{
	width = w;
	height = h;
	saves = s;
	action = a;
	selectable = true;

	if(action == 0) // load
		listOffset = 0;
	else
		listOffset = -2; // save - reserve -2 & -1 for new slots

	selectedItem = 0;
	focus = 0; // allow focus

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	trig2 = new GuiTrigger;
	trig2->SetSimpleTrigger(-1, WPAD_BUTTON_2, 0);

	btnSoundOver = new GuiSound(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	btnSoundClick = new GuiSound(button_click_pcm, button_click_pcm_size, SOUND_PCM);

	gameSave = new GuiImageData(button_gamesave_png);
	gameSaveOver = new GuiImageData(button_gamesave_over_png);
	gameSaveBlank = new GuiImageData(button_gamesave_blank_png);

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

	for(int i=0; i<SAVELISTSIZE; i++)
	{
		saveDate[i] = new GuiText(NULL, 18, (GXColor){0, 0, 0, 0xff});
		saveDate[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
		saveDate[i]->SetPosition(80,5);
		saveTime[i] = new GuiText(NULL, 18, (GXColor){0, 0, 0, 0xff});
		saveTime[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
		saveTime[i]->SetPosition(80,27);

		saveType[i] = new GuiText(NULL, 18, (GXColor){0, 0, 0, 0xff});
		saveType[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
		saveType[i]->SetPosition(80,50);

		saveBgImg[i] = new GuiImage(gameSave);
		saveBgOverImg[i] = new GuiImage(gameSaveOver);
		savePreviewImg[i] = new GuiImage(gameSaveBlank);
		savePreviewImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		savePreviewImg[i]->SetPosition(5,0);

		saveBtn[i] = new GuiButton(saveBgImg[i]->GetWidth(),saveBgImg[i]->GetHeight());
		saveBtn[i]->SetParent(this);
		saveBtn[i]->SetLabel(saveDate[i], 0);
		saveBtn[i]->SetLabel(saveTime[i], 1);
		saveBtn[i]->SetLabel(saveType[i], 2);
		saveBtn[i]->SetImage(saveBgImg[i]);
		saveBtn[i]->SetImageOver(saveBgOverImg[i]);
		saveBtn[i]->SetIcon(savePreviewImg[i]);
		saveBtn[i]->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
		saveBtn[i]->SetPosition(257*(i % 2),87*(i>>1));
		saveBtn[i]->SetTrigger(trigA);
		saveBtn[i]->SetTrigger(trig2);
		saveBtn[i]->SetState(STATE_DISABLED);
		saveBtn[i]->SetEffectGrow();
		saveBtn[i]->SetVisible(false);
		saveBtn[i]->SetSoundOver(btnSoundOver);
		saveBtn[i]->SetSoundClick(btnSoundClick);
		saveBtnLastOver[i] = false;
	}
	saveBtn[0]->SetState(STATE_SELECTED, -1);
	saveBtn[0]->SetVisible(true);
}

/**
 * Destructor for the GuiSaveBrowser class.
 */
GuiSaveBrowser::~GuiSaveBrowser()
{
	delete arrowUpBtn;
	delete arrowDownBtn;

	delete scrollbarImg;
	delete arrowDownImg;
	delete arrowDownOverImg;
	delete arrowUpImg;
	delete arrowUpOverImg;

	delete gameSave;
	delete gameSaveOver;
	delete gameSaveBlank;
	delete scrollbar;
	delete arrowDown;
	delete arrowDownOver;
	delete arrowUp;
	delete arrowUpOver;

	delete btnSoundOver;
	delete btnSoundClick;
	delete trigA;
	delete trig2;

	for(int i=0; i<SAVELISTSIZE; i++)
	{
		delete saveBtn[i];
		delete saveDate[i];
		delete saveTime[i];
		delete saveType[i];
		delete saveBgImg[i];
		delete saveBgOverImg[i];
		delete savePreviewImg[i];
	}
}

void GuiSaveBrowser::SetFocus(int f)
{
	focus = f;

	for(int i=0; i<SAVELISTSIZE; i++)
		saveBtn[i]->ResetState();

	if(f == 1 && selectedItem >= 0)
		saveBtn[selectedItem]->SetState(STATE_SELECTED);
}

void GuiSaveBrowser::ResetState()
{
	if(state != STATE_DISABLED)
	{
		state = STATE_DEFAULT;
		stateChan = -1;
	}

	for(int i=0; i<SAVELISTSIZE; i++)
	{
		saveBtn[i]->ResetState();
	}
}

int GuiSaveBrowser::GetClickedSave()
{
	int found = -3;
	for(int i=0; i<SAVELISTSIZE; i++)
	{
		if(saveBtn[i]->GetState() == STATE_CLICKED)
		{
			saveBtn[i]->SetState(STATE_SELECTED);
			found = listOffset+i;
			break;
		}
	}
	return found;
}

/**
 * Draw the button on screen
 */
void GuiSaveBrowser::Draw()
{
	if(!this->IsVisible())
		return;

	for(int i=0; i<SAVELISTSIZE; i++)
		saveBtn[i]->Draw();

	scrollbarImg->Draw();
	arrowUpBtn->Draw();
	arrowDownBtn->Draw();

	this->UpdateEffects();
}

void GuiSaveBrowser::Update(GuiTrigger * t)
{
	if(state == STATE_DISABLED || !t)
		return;

	int i, len;
	char savetext[50];

	arrowUpBtn->Update(t);
	arrowDownBtn->Update(t);

	// pad/joystick navigation
	if(!focus)
		goto endNavigation; // skip navigation

	if(selectedItem < 0) selectedItem = 0;

	if(t->Right())
	{
		if(selectedItem == SAVELISTSIZE-1)
		{
			if(listOffset + SAVELISTSIZE < saves->length)
			{
				// move list down by 1 row
				listOffset += 2;
				selectedItem -= 1;
			}
		}
		else if(saveBtn[selectedItem+1]->IsVisible())
		{
			saveBtn[selectedItem]->ResetState();
			saveBtn[selectedItem+1]->SetState(STATE_SELECTED, t->chan);
			selectedItem += 1;
		}
	}
	else if(t->Left())
	{
		if(selectedItem == 0)
		{
			if((listOffset - 2 >= 0 && action == 0) ||
				(listOffset >= 0 && action == 1))
			{
				// move list up by 1
				listOffset -= 2;
				selectedItem = SAVELISTSIZE-1;
			}
		}
		else
		{
			selectedItem -= 1;
		}
	}
	else if(t->Down() || arrowDownBtn->GetState() == STATE_CLICKED)
	{
		if(selectedItem >= SAVELISTSIZE-2)
		{
			if(listOffset + SAVELISTSIZE + 1 < saves->length)
			{
				listOffset += 2;
			}
			else if(listOffset + SAVELISTSIZE < saves->length)
			{
				listOffset += 2;

				if(selectedItem == SAVELISTSIZE-1)
					selectedItem -= 1;
			}
		}
		else if(saveBtn[selectedItem+2]->IsVisible())
		{
			selectedItem += 2;
		}
	}
	else if(t->Up() || arrowUpBtn->GetState() == STATE_CLICKED)
	{
		if(selectedItem < 2)
		{
			if((listOffset - 2 >= 0 && action == 0) ||
				(listOffset >= 0 && action == 1))
			{
				// move list up by 1
				listOffset -= 2;
			}
		}
		else
		{
			selectedItem -= 2;
		}
	}

	endNavigation:

	if(arrowDownBtn->GetState() == STATE_CLICKED)
		arrowDownBtn->ResetState();

	if(arrowUpBtn->GetState() == STATE_CLICKED)
		arrowUpBtn->ResetState();

	for(i=0; i<SAVELISTSIZE; i++)
	{
		if(listOffset+i < 0 && action == 1)
		{
			saveDate[0]->SetText(NULL);
			saveDate[1]->SetText(NULL);
			saveTime[0]->SetText("New");
			saveTime[1]->SetText("New");
			saveType[0]->SetText("SRAM");
			saveType[1]->SetText("Snapshot");
			savePreviewImg[0]->SetImage(gameSaveBlank);
			savePreviewImg[1]->SetImage(gameSaveBlank);
			saveBtn[0]->SetVisible(true);
			saveBtn[1]->SetVisible(true);

			if(saveBtn[0]->GetState() == STATE_DISABLED)
				saveBtn[0]->SetState(STATE_DEFAULT);
			if(saveBtn[1]->GetState() == STATE_DISABLED)
				saveBtn[1]->SetState(STATE_DEFAULT);
		}
		else if(listOffset+i < saves->length)
		{
			if(saveBtn[i]->GetState() == STATE_DISABLED || !saveBtn[i]->IsVisible())
			{
				saveBtn[i]->SetVisible(true);
				saveBtn[i]->SetState(STATE_DEFAULT);
			}

			saveDate[i]->SetText(saves->date[listOffset+i]);
			saveTime[i]->SetText(saves->time[listOffset+i]);

			if(saves->type[listOffset+i] == FILE_SRAM)
				sprintf(savetext, "SRAM");
			else
				sprintf(savetext, "Snapshot");

			len = strlen(saves->filename[listOffset+i]);
			if(len > 10 &&
				((saves->filename[listOffset+i][len-8] == 'A' &&
				saves->filename[listOffset+i][len-7] == 'u' &&
				saves->filename[listOffset+i][len-6] == 't' &&
				saves->filename[listOffset+i][len-5] == 'o') ||
				saves->filename[listOffset+i][len-5] == '0')
				)
			{
				strcat(savetext, " (Auto)");
			}
			saveType[i]->SetText(savetext);

			if(saves->previewImg[listOffset+i] != NULL)
				savePreviewImg[i]->SetImage(saves->previewImg[listOffset+i]);
			else
				savePreviewImg[i]->SetImage(gameSaveBlank);
		}
		else
		{
			saveBtn[i]->SetVisible(false);
			saveBtn[i]->SetState(STATE_DISABLED);
		}

		if(i != selectedItem && saveBtn[i]->GetState() == STATE_SELECTED)
			saveBtn[i]->ResetState();
		else if(focus && i == selectedItem && saveBtn[i]->GetState() == STATE_DEFAULT)
			saveBtn[selectedItem]->SetState(STATE_SELECTED, t->chan);

		if(t->wpad->ir.valid)
		{
			if(!saveBtnLastOver[i] && saveBtn[i]->IsInside(t->wpad->ir.x, t->wpad->ir.y))
				saveBtn[i]->ResetState();
			saveBtnLastOver[i] = saveBtn[i]->IsInside(t->wpad->ir.x, t->wpad->ir.y);
		}

		saveBtn[i]->Update(t);

		if(saveBtn[i]->GetState() == STATE_SELECTED)
			selectedItem = i;
	}

	if(updateCB)
		updateCB(this);
}
