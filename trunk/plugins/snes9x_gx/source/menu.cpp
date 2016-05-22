/****************************************************************************
 * Snes9x Nintendo Wii/Gamecube Port
 *
 * Tantric 2008-2010
 *
 * menu.cpp
 *
 * Menu flow routines - handles all menu logic
 ***************************************************************************/

#include <gccore.h>
#include <ogcsys.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiiuse/wpad.h>
#include <wupc/wupc.h>
#include <sys/stat.h>

#ifdef HW_RVL
#include <di/di.h>
#endif

#include "snes9xgx.h"
#include "video.h"
#include "filebrowser.h"
#include "gcunzip.h"
#include "networkop.h"
#include "fileop.h"
#include "sram.h"
#include "freeze.h"
#include "preferences.h"
#include "button_mapping.h"
#include "input.h"
#include "filter.h"
#include "filelist.h"
#include "gui/gui.h"
#include "menu.h"
#include "utils/gettext.h"

#include "snes9x/snes9x.h"
#include "snes9x/memmap.h"
#include "snes9x/cheats.h"

extern SCheatData Cheat;

#define THREAD_SLEEP 100

#ifdef HW_RVL
static GuiImageData * pointer[4];
#endif

#ifdef HW_RVL
	#include "mem2.h"

	#define MEM_ALLOC(A) (u8*)mem2_malloc(A)
	#define MEM_DEALLOC(A) mem2_free(A)
#else
	#define MEM_ALLOC(A) (u8*)memalign(32, A)
	#define MEM_DEALLOC(A) free(A)
#endif

static GuiTrigger * trigA = NULL;
static GuiTrigger * trig2 = NULL;

static GuiButton * btnLogo = NULL;
static GuiImageData * gameScreen = NULL;
static GuiImage * gameScreenImg = NULL;
static GuiImage * bgTopImg = NULL;
static GuiImage * bgBottomImg = NULL;
static GuiSound * bgMusic = NULL;
static GuiSound * enterSound = NULL;
static GuiSound * exitSound = NULL;
static GuiWindow * mainWindow = NULL;
static GuiText * settingText = NULL;
static GuiText * settingText2 = NULL;
static int lastMenu = MENU_NONE;
static int wiiuproCtrl = 0;
static int mapMenuCtrl = 0;
static int mapMenuCtrlSNES = 0;

static lwp_t guithread = LWP_THREAD_NULL;
static lwp_t progressthread = LWP_THREAD_NULL;
#ifdef HW_RVL
static lwp_t updatethread = LWP_THREAD_NULL;
#endif
static bool guiHalt = true;
static int showProgress = 0;

static char progressTitle[101];
static char progressMsg[201];
static int progressDone = 0;
static int progressTotal = 0;

/****************************************************************************
 * ResumeGui
 *
 * Signals the GUI thread to start, and resumes the thread. This is called
 * after finishing the removal/insertion of new elements, and after initial
 * GUI setup.
 ***************************************************************************/
static void
ResumeGui()
{
	guiHalt = false;
	LWP_ResumeThread (guithread);
}

/****************************************************************************
 * HaltGui
 *
 * Signals the GUI thread to stop, and waits for GUI thread to stop
 * This is necessary whenever removing/inserting new elements into the GUI.
 * This eliminates the possibility that the GUI is in the middle of accessing
 * an element that is being changed.
 ***************************************************************************/
static void
HaltGui()
{
	guiHalt = true;

	// wait for thread to finish
	while(!LWP_ThreadIsSuspended(guithread))
		usleep(THREAD_SLEEP);
}

void ResetText()
{
	LoadLanguage();

	if(mainWindow)
		mainWindow->ResetText();
}

/****************************************************************************
 * WindowPrompt
 *
 * Displays a prompt window to user, with information, an error message, or
 * presenting a user with a choice
 ***************************************************************************/
int
WindowPrompt(const char *title, const char *msg, const char *btn1Label, const char *btn2Label)
{
	if(!mainWindow || ExitRequested || ShutdownRequested)
		return 0;

	int choice = -1;

	GuiWindow promptWindow(448,288);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);
	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_prompt_png);
	GuiImageData btnOutlineOver(button_prompt_over_png);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);

	GuiText titleTxt(title, 26, (GXColor){70, 70, 10, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,14);
	GuiText msgTxt(msg, 26, (GXColor){0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	msgTxt.SetPosition(0,-20);
	msgTxt.SetWrap(true, 430);

	GuiText btn1Txt(btn1Label, 22, (GXColor){0, 0, 0, 255});
	GuiImage btn1Img(&btnOutline);
	GuiImage btn1ImgOver(&btnOutlineOver);
	GuiButton btn1(btnOutline.GetWidth(), btnOutline.GetHeight());

	if(btn2Label)
	{
		btn1.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
		btn1.SetPosition(20, -25);
	}
	else
	{
		btn1.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
		btn1.SetPosition(0, -25);
	}

	btn1.SetLabel(&btn1Txt);
	btn1.SetImage(&btn1Img);
	btn1.SetImageOver(&btn1ImgOver);
	btn1.SetSoundOver(&btnSoundOver);
	btn1.SetSoundClick(&btnSoundClick);
	btn1.SetTrigger(trigA);
	btn1.SetTrigger(trig2);
	btn1.SetState(STATE_SELECTED);
	btn1.SetEffectGrow();

	GuiText btn2Txt(btn2Label, 22, (GXColor){0, 0, 0, 255});
	GuiImage btn2Img(&btnOutline);
	GuiImage btn2ImgOver(&btnOutlineOver);
	GuiButton btn2(btnOutline.GetWidth(), btnOutline.GetHeight());
	btn2.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	btn2.SetPosition(-20, -25);
	btn2.SetLabel(&btn2Txt);
	btn2.SetImage(&btn2Img);
	btn2.SetImageOver(&btn2ImgOver);
	btn2.SetSoundOver(&btnSoundOver);
	btn2.SetSoundClick(&btnSoundClick);
	btn2.SetTrigger(trigA);
	btn2.SetTrigger(trig2);
	btn2.SetEffectGrow();

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);
	promptWindow.Append(&btn1);

	if(btn2Label)
		promptWindow.Append(&btn2);

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
	CancelAction();
	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	if(btn2Label)
	{
		btn1.ResetState();
		btn2.SetState(STATE_SELECTED);
	}
	ResumeGui();

	while(choice == -1)
	{
		usleep(THREAD_SLEEP);

		if(btn1.GetState() == STATE_CLICKED)
			choice = 1;
		else if(btn2.GetState() == STATE_CLICKED)
			choice = 0;
	}

	promptWindow.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	while(promptWindow.GetEffect() > 0) usleep(THREAD_SLEEP);
	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	return choice;
}

#ifdef HW_RVL
/****************************************************************************
 * EmulatorUpdate
 *
 * Prompts for confirmation, and downloads/installs updates
 ***************************************************************************/
static void *
EmulatorUpdate (void *arg)
{
	bool installUpdate = WindowPrompt(
		"Update Available",
		"An update is available!",
		"Update now",
		"Update later");
	if(installUpdate)
		if(DownloadUpdate())
			ExitRequested = 1;
	return NULL;
}
#endif

/****************************************************************************
 * UpdateGUI
 *
 * Primary thread to allow GUI to respond to state changes, and draws GUI
 ***************************************************************************/

static void *
UpdateGUI (void *arg)
{
	int i;

	while(1)
	{
		if(guiHalt)
			LWP_SuspendThread(guithread);

		UpdatePads();
		mainWindow->Draw();

		if (mainWindow->GetState() != STATE_DISABLED)
			mainWindow->DrawTooltip();

		#ifdef HW_RVL
		i = 3;
		do
		{
			if(userInput[i].wpad->ir.valid)
				Menu_DrawImg(userInput[i].wpad->ir.x-48, userInput[i].wpad->ir.y-48,
					96, 96, pointer[i]->GetImage(), userInput[i].wpad->ir.angle, 1, 1, 255);
			DoRumble(i);
			--i;
		} while(i>=0);
		#endif

		Menu_Render();

		mainWindow->Update(&userInput[3]);
		mainWindow->Update(&userInput[2]);
		mainWindow->Update(&userInput[1]);
		mainWindow->Update(&userInput[0]);

		#ifdef HW_RVL
		if(updateFound)
		{
			updateFound = false;

			if(!loadingFile)
				LWP_CreateThread (&updatethread, EmulatorUpdate, NULL, NULL, 0, 70);
		}
		#endif

		if(ExitRequested || ShutdownRequested)
		{
			for(i = 0; i <= 255; i += 15)
			{
				mainWindow->Draw();
				Menu_DrawRectangle(0,0,screenwidth,screenheight,(GXColor){0, 0, 0, (u8)i},1);
				Menu_Render();
			}
			ExitApp();
		}
		usleep(THREAD_SLEEP);
	}
	return NULL;
}

/****************************************************************************
 * ProgressWindow
 *
 * Opens a window, which displays progress to the user. Can either display a
 * progress bar showing % completion, or a throbber that only shows that an
 * action is in progress.
 ***************************************************************************/
static int progsleep = 0;

static void
ProgressWindow(char *title, char *msg)
{
	GuiWindow promptWindow(448,288);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);
	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);

	GuiImageData progressbarOutline(progressbar_outline_png);
	GuiImage progressbarOutlineImg(&progressbarOutline);
	progressbarOutlineImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarOutlineImg.SetPosition(25, 40);

	GuiImageData progressbarEmpty(progressbar_empty_png);
	GuiImage progressbarEmptyImg(&progressbarEmpty);
	progressbarEmptyImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarEmptyImg.SetPosition(25, 40);
	progressbarEmptyImg.SetTile(100);

	GuiImageData progressbar(progressbar_png);
	GuiImage progressbarImg(&progressbar);
	progressbarImg.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	progressbarImg.SetPosition(25, 40);

	GuiImageData throbber(throbber_png);
	GuiImage throbberImg(&throbber);
	throbberImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	throbberImg.SetPosition(0, 40);

	GuiText titleTxt(title, 26, (GXColor){70, 70, 10, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,14);
	GuiText msgTxt(msg, 26, (GXColor){0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	msgTxt.SetPosition(0,80);

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);

	if(showProgress == 1)
	{
		promptWindow.Append(&progressbarEmptyImg);
		promptWindow.Append(&progressbarImg);
		promptWindow.Append(&progressbarOutlineImg);
	}
	else
	{
		promptWindow.Append(&throbberImg);
	}

	// wait to see if progress flag changes soon
	progsleep = 400000;

	while(progsleep > 0)
	{
		if(!showProgress)
			break;
		usleep(THREAD_SLEEP);
		progsleep -= THREAD_SLEEP;
	}

	if(!showProgress)
		return;

	HaltGui();
	int oldState = mainWindow->GetState();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	ResumeGui();

	float angle = 0;
	u32 count = 0;

	while(showProgress)
	{
		progsleep = 20000;

		while(progsleep > 0)
		{
			if(!showProgress)
				break;
			usleep(THREAD_SLEEP);
			progsleep -= THREAD_SLEEP;
		}

		if(showProgress == 1)
		{
			progressbarImg.SetTile(100*progressDone/progressTotal);
		}
		else if(showProgress == 2)
		{
			if(count % 5 == 0)
			{
				angle+=45.0f;
				if(angle >= 360.0f)
					angle = 0;
				throbberImg.SetAngle(angle);
			}
			++count;
		}
	}

	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(oldState);
	ResumeGui();
}

static void * ProgressThread (void *arg)
{
	while(1)
	{
		if(!showProgress)
			LWP_SuspendThread (progressthread);

		ProgressWindow(progressTitle, progressMsg);
		usleep(THREAD_SLEEP);
	}
	return NULL;
}

/****************************************************************************
 * InitGUIThread
 *
 * Startup GUI threads
 ***************************************************************************/
void
InitGUIThreads()
{
	LWP_CreateThread (&guithread, UpdateGUI, NULL, NULL, 0, 70);
	LWP_CreateThread (&progressthread, ProgressThread, NULL, NULL, 0, 40);
}

/****************************************************************************
 * CancelAction
 *
 * Signals the GUI progress window thread to halt, and waits for it to
 * finish. Prevents multiple progress window events from interfering /
 * overriding each other.
 ***************************************************************************/
void
CancelAction()
{
	showProgress = 0;

	// wait for thread to finish
	while(!LWP_ThreadIsSuspended(progressthread))
		usleep(THREAD_SLEEP);
}

/****************************************************************************
 * ShowProgress
 *
 * Updates the variables used by the progress window for drawing a progress
 * bar. Also resumes the progress window thread if it is suspended.
 ***************************************************************************/
void
ShowProgress (const char *msg, int done, int total)
{
	if(!mainWindow || ExitRequested || ShutdownRequested)
		return;

	if(total < (256*1024))
		return;
	else if(done > total) // this shouldn't happen
		done = total;

	if(done/total > 0.99)
		done = total;

	if(showProgress != 1)
		CancelAction(); // wait for previous progress window to finish

	snprintf(progressMsg, 200, "%s", msg);
	sprintf(progressTitle, "Please Wait");
	showProgress = 1;
	progressTotal = total;
	progressDone = done;
	LWP_ResumeThread (progressthread);
}

/****************************************************************************
 * ShowAction
 *
 * Shows that an action is underway. Also resumes the progress window thread
 * if it is suspended.
 ***************************************************************************/
void
ShowAction (const char *msg)
{
	if(!mainWindow || ExitRequested || ShutdownRequested)
		return;

	if(showProgress != 0)
		CancelAction(); // wait for previous progress window to finish

	snprintf(progressMsg, 200, "%s", msg);
	sprintf(progressTitle, "Please Wait");
	showProgress = 2;
	progressDone = 0;
	progressTotal = 0;
	LWP_ResumeThread (progressthread);
}

void ErrorPrompt(const char *msg)
{
	WindowPrompt("Error", msg, "OK", NULL);
}

int ErrorPromptRetry(const char *msg)
{
	return WindowPrompt("Error", msg, "Retry", "Cancel");
}

void InfoPrompt(const char *msg)
{
	WindowPrompt("Information", msg, "OK", NULL);
}

/****************************************************************************
 * AutoSave
 *
 * Automatically saves SRAM/snapshot when returning from in-game to the menu
 ***************************************************************************/
void AutoSave()
{
	if (GCSettings.AutoSave == 1)
	{
		SaveSRAMAuto(SILENT);
	}
	else if (GCSettings.AutoSave == 2)
	{
		if (WindowPrompt("Save", "Save Snapshot?", "Save", "Don't Save") )
			SaveSnapshotAuto(NOTSILENT);
	}
	else if (GCSettings.AutoSave == 3)
	{
		if (WindowPrompt("Save", "Save SRAM and Snapshot?", "Save", "Don't Save") )
		{
			SaveSRAMAuto(NOTSILENT);
			SaveSnapshotAuto(NOTSILENT);
		}
	}
}

/****************************************************************************
 * OnScreenKeyboard
 *
 * Opens an on-screen keyboard window, with the data entered being stored
 * into the specified variable.
 ***************************************************************************/
static void OnScreenKeyboard(char * var, u32 maxlen)
{
	int save = -1;

	GuiKeyboard keyboard(var, maxlen);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiText okBtnTxt("OK", 22, (GXColor){0, 0, 0, 255});
	GuiImage okBtnImg(&btnOutline);
	GuiImage okBtnImgOver(&btnOutlineOver);
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

	okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	okBtn.SetPosition(25, -25);

	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetSoundOver(&btnSoundOver);
	okBtn.SetSoundClick(&btnSoundClick);
	okBtn.SetTrigger(trigA);
	okBtn.SetTrigger(trig2);
	okBtn.SetEffectGrow();

	GuiText cancelBtnTxt("Cancel", 22, (GXColor){0, 0, 0, 255});
	GuiImage cancelBtnImg(&btnOutline);
	GuiImage cancelBtnImgOver(&btnOutlineOver);
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	cancelBtn.SetPosition(-25, -25);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetImageOver(&cancelBtnImgOver);
	cancelBtn.SetSoundOver(&btnSoundOver);
	cancelBtn.SetSoundClick(&btnSoundClick);
	cancelBtn.SetTrigger(trigA);
	cancelBtn.SetTrigger(trig2);
	cancelBtn.SetEffectGrow();

	keyboard.Append(&okBtn);
	keyboard.Append(&cancelBtn);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&keyboard);
	mainWindow->ChangeFocus(&keyboard);
	ResumeGui();

	while(save == -1)
	{
		usleep(THREAD_SLEEP);

		if(okBtn.GetState() == STATE_CLICKED)
			save = 1;
		else if(cancelBtn.GetState() == STATE_CLICKED)
			save = 0;
	}

	if(save)
	{
		snprintf(var, maxlen, "%s", keyboard.kbtextstr);
	}

	HaltGui();
	mainWindow->Remove(&keyboard);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
}

/****************************************************************************
 * SettingWindow
 *
 * Opens a new window, with the specified window element appended. Allows
 * for a customizable prompted setting.
 ***************************************************************************/
static int
SettingWindow(const char * title, GuiWindow * w)
{
	int save = -1;

	GuiWindow promptWindow(448,288);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);

	GuiText titleTxt(title, 26, (GXColor){70, 70, 10, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,14);

	GuiText okBtnTxt("OK", 22, (GXColor){0, 0, 0, 255});
	GuiImage okBtnImg(&btnOutline);
	GuiImage okBtnImgOver(&btnOutlineOver);
	GuiButton okBtn(btnOutline.GetWidth(), btnOutline.GetHeight());

	okBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	okBtn.SetPosition(20, -25);

	okBtn.SetLabel(&okBtnTxt);
	okBtn.SetImage(&okBtnImg);
	okBtn.SetImageOver(&okBtnImgOver);
	okBtn.SetSoundOver(&btnSoundOver);
	okBtn.SetSoundClick(&btnSoundClick);
	okBtn.SetTrigger(trigA);
	okBtn.SetTrigger(trig2);
	okBtn.SetEffectGrow();

	GuiText cancelBtnTxt("Cancel", 22, (GXColor){0, 0, 0, 255});
	GuiImage cancelBtnImg(&btnOutline);
	GuiImage cancelBtnImgOver(&btnOutlineOver);
	GuiButton cancelBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	cancelBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	cancelBtn.SetPosition(-20, -25);
	cancelBtn.SetLabel(&cancelBtnTxt);
	cancelBtn.SetImage(&cancelBtnImg);
	cancelBtn.SetImageOver(&cancelBtnImgOver);
	cancelBtn.SetSoundOver(&btnSoundOver);
	cancelBtn.SetSoundClick(&btnSoundClick);
	cancelBtn.SetTrigger(trigA);
	cancelBtn.SetTrigger(trig2);
	cancelBtn.SetEffectGrow();

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&okBtn);
	promptWindow.Append(&cancelBtn);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->Append(w);
	mainWindow->ChangeFocus(w);
	ResumeGui();

	while(save == -1)
	{
		usleep(THREAD_SLEEP);

		if(okBtn.GetState() == STATE_CLICKED)
			save = 1;
		else if(cancelBtn.GetState() == STATE_CLICKED)
			save = 0;
	}
	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->Remove(w);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();
	return save;
}

/****************************************************************************
 * WindowCredits
 * Display credits, legal copyright and licence
 *
 * THIS MUST NOT BE REMOVED OR DISABLED IN ANY DERIVATIVE WORK
 ***************************************************************************/
static void WindowCredits(void * ptr)
{
	if(btnLogo->GetState() != STATE_CLICKED)
		return;

	btnLogo->ResetState();

	bool exit = false;
	int i = 0;
	int y = 20;

	GuiWindow creditsWindow(screenwidth,screenheight);
	GuiWindow creditsWindowBox(580,448);
	creditsWindowBox.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	GuiImageData creditsBox(credits_box_png);
	GuiImage creditsBoxImg(&creditsBox);
	creditsBoxImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	creditsWindowBox.Append(&creditsBoxImg);

	int numEntries = 24;
	GuiText * txt[numEntries];

	txt[i] = new GuiText("Credits", 30, (GXColor){0, 0, 0, 255});
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(0,y); i++; y+=32;

	txt[i] = new GuiText("Official Site: http://code.google.com/p/snes9x-gx/", 20, (GXColor){0, 0, 0, 255});
	txt[i]->SetAlignment(ALIGN_CENTRE, ALIGN_TOP); txt[i]->SetPosition(0,y); i++; y+=40;

	txt[i]->SetPresets(20, (GXColor){0, 0, 0, 255}, 0,
			FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP, ALIGN_LEFT, ALIGN_TOP);

	txt[i] = new GuiText("Coding & menu design");
	txt[i]->SetPosition(60,y); i++;
	txt[i] = new GuiText("Tantric");
	txt[i]->SetPosition(350,y); i++; y+=24;
	txt[i] = new GuiText("Coding");
	txt[i]->SetPosition(60,y); i++;
	txt[i] = new GuiText("michniewski");
	txt[i]->SetPosition(350,y); i++; y+=24;
	txt[i] = new GuiText("Menu artwork");
	txt[i]->SetPosition(60,y); i++;
	txt[i] = new GuiText("the3seashells");
	txt[i]->SetPosition(350,y); i++; y+=24;
	txt[i] = new GuiText("Menu sound");
	txt[i]->SetPosition(60,y); i++;
	txt[i] = new GuiText("Peter de Man");
	txt[i]->SetPosition(350,y); i++; y+=48;

	txt[i] = new GuiText("Snes9x GX GameCube");
	txt[i]->SetPosition(60,y); i++;
	txt[i] = new GuiText("SoftDev, crunchy2,");
	txt[i]->SetPosition(350,y); i++; y+=24;
	txt[i] = new GuiText("eke-eke, others");
	txt[i]->SetPosition(350,y); i++; y+=24;
	txt[i] = new GuiText("Snes9x");
	txt[i]->SetPosition(60,y); i++;
	txt[i] = new GuiText("Snes9x Team");
	txt[i]->SetPosition(350,y); i++; y+=24;

	txt[i] = new GuiText("libogc / devkitPPC");
	txt[i]->SetPosition(60,y); i++;
	txt[i] = new GuiText("shagkur & wintermute");
	txt[i]->SetPosition(350,y); i++; y+=24;
	txt[i] = new GuiText("FreeTypeGX");
	txt[i]->SetPosition(60,y); i++;
	txt[i] = new GuiText("Armin Tamzarian");
	txt[i]->SetPosition(350,y); i++; y+=48;

	txt[i]->SetPresets(18, (GXColor){0, 0, 0, 255}, 0,
		FTGX_JUSTIFY_CENTER | FTGX_ALIGN_TOP, ALIGN_CENTRE, ALIGN_TOP);

	txt[i] = new GuiText("Snes9x - Copyright (c) Snes9x Team 1996 - 2006");
	txt[i]->SetPosition(0,y); i++; y+=20;
	txt[i] = new GuiText("This software is open source and may be copied,");
	txt[i]->SetPosition(0,y); i++; y+=20;
	txt[i] = new GuiText("distributed, or modified under the terms of the");
	txt[i]->SetPosition(0,y); i++; y+=20;
	txt[i] = new GuiText("GNU General Public License (GPL) Version 2.");
	txt[i]->SetPosition(0,y); i++; y+=20;

	char iosVersion[20];

#ifdef HW_RVL
	sprintf(iosVersion, "IOS: %d", IOS_GetVersion());
#endif

	txt[i] = new GuiText(iosVersion, 18, (GXColor){0, 0, 0, 255});
	txt[i]->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	txt[i]->SetPosition(20,-20);

	for(i=0; i < numEntries; i++)
		creditsWindowBox.Append(txt[i]);

	creditsWindow.Append(&creditsWindowBox);

	while(!exit)
	{
		UpdatePads();

		gameScreenImg->Draw();
		bgBottomImg->Draw();
		bgTopImg->Draw();
		creditsWindow.Draw();

		#ifdef HW_RVL
		i = 3;
		do {	
		if(userInput[i].wpad->ir.valid)
			Menu_DrawImg(userInput[i].wpad->ir.x-48, userInput[i].wpad->ir.y-48,
				96, 96, pointer[i]->GetImage(), userInput[i].wpad->ir.angle, 1, 1, 255);
			DoRumble(i);
			--i;
		} while(i >= 0);
		#endif

		Menu_Render();

		if((userInput[0].wpad->btns_d || userInput[0].pad.btns_d || userInput[0].wupcdata.btns_d) ||
		   (userInput[1].wpad->btns_d || userInput[1].pad.btns_d || userInput[1].wupcdata.btns_d) ||
		   (userInput[2].wpad->btns_d || userInput[2].pad.btns_d || userInput[2].wupcdata.btns_d) ||
		   (userInput[3].wpad->btns_d || userInput[3].pad.btns_d || userInput[3].wupcdata.btns_d))
		{
			exit = true;
		}
		usleep(THREAD_SLEEP);
	}

	// clear buttons pressed
	for(i=0; i < 4; i++)
	{
		userInput[i].wupcdata.btns_d = 0;
		userInput[i].wpad->btns_d = 0;
		userInput[i].pad.btns_d = 0;
	}

	for(i=0; i < numEntries; i++)
		delete txt[i];
}

/****************************************************************************
 * MenuGameSelection
 *
 * Displays a list of games on the specified load device, and allows the user
 * to browse and select from this list.
 ***************************************************************************/
static int MenuGameSelection()
{
	int menu = MENU_NONE;
	bool res;
	int i;

	GuiText titleTxt("Choose Game", 26, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData iconHome(icon_home_png);
	GuiImageData iconSettings(icon_settings_png);
	GuiImageData btnOutline(button_long_png);
	GuiImageData btnOutlineOver(button_long_over_png);
	GuiImageData bgPreviewImg(bg_preview_png);

	GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

	GuiText settingsBtnTxt("Settings", 22, (GXColor){0, 0, 0, 255});
	GuiImage settingsBtnIcon(&iconSettings);
	settingsBtnIcon.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	settingsBtnIcon.SetPosition(14,0);
	GuiImage settingsBtnImg(&btnOutline);
	GuiImage settingsBtnImgOver(&btnOutlineOver);
	GuiButton settingsBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	settingsBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	settingsBtn.SetPosition(90, -35);
	settingsBtn.SetLabel(&settingsBtnTxt);
	settingsBtn.SetIcon(&settingsBtnIcon);
	settingsBtn.SetImage(&settingsBtnImg);
	settingsBtn.SetImageOver(&settingsBtnImgOver);
	settingsBtn.SetSoundOver(&btnSoundOver);
	settingsBtn.SetSoundClick(&btnSoundClick);
	settingsBtn.SetTrigger(trigA);
	settingsBtn.SetTrigger(trig2);
	settingsBtn.SetEffectGrow();

	GuiText exitBtnTxt("Exit", 22, (GXColor){0, 0, 0, 255});
	GuiImage exitBtnIcon(&iconHome);
	exitBtnIcon.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	exitBtnIcon.SetPosition(14,0);
	GuiImage exitBtnImg(&btnOutline);
	GuiImage exitBtnImgOver(&btnOutlineOver);
	GuiButton exitBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	exitBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	exitBtn.SetPosition(-90, -35);
	exitBtn.SetLabel(&exitBtnTxt);
	exitBtn.SetIcon(&exitBtnIcon);
	exitBtn.SetImage(&exitBtnImg);
	exitBtn.SetImageOver(&exitBtnImgOver);
	exitBtn.SetSoundOver(&btnSoundOver);
	exitBtn.SetSoundClick(&btnSoundClick);
	exitBtn.SetTrigger(trigA);
	exitBtn.SetTrigger(trig2);
	exitBtn.SetTrigger(&trigHome);
	exitBtn.SetEffectGrow();

	GuiWindow buttonWindow(screenwidth, screenheight);
	buttonWindow.Append(&settingsBtn);
	buttonWindow.Append(&exitBtn);

	GuiFileBrowser gameBrowser(330, 268);
	gameBrowser.SetPosition(20, 98);
	ResetBrowser();
		
	GuiImage bgPreview(&bgPreviewImg);
	bgPreview.SetPosition(365, 98);
	
	GuiImage preview;
	preview.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	preview.SetPosition(174, -8);
	u8* imgBuffer = MEM_ALLOC(512 * 512 * 4);
	int  previousBrowserIndex = -1;
	char screenshotPath[MAXJOLIET + 1];
		
	HaltGui();
	btnLogo->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	btnLogo->SetPosition(-50, 24);
	mainWindow->Append(&titleTxt);
	mainWindow->Append(&gameBrowser);
	mainWindow->Append(&buttonWindow);
	mainWindow->Append(&bgPreview);
	mainWindow->Append(&preview);
	ResumeGui();

	#ifdef HW_RVL
	ShutoffRumble();
	#endif

	// populate initial directory listing
	selectLoadedFile = 1;
	OpenGameList();

	gameBrowser.ResetState();
	gameBrowser.fileList[0]->SetState(STATE_SELECTED);
	gameBrowser.TriggerUpdate();
	titleTxt.SetText(inSz ? szname : "Choose Game");

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		if(selectLoadedFile == 2)
		{
			selectLoadedFile = 0;
			mainWindow->ChangeFocus(&gameBrowser);
			gameBrowser.TriggerUpdate();
		}

		// update gameWindow based on arrow buttons
		// set MENU_EXIT if A button pressed on a game
		for(i=0; i < FILE_PAGESIZE; i++)
		{
			if(gameBrowser.fileList[i]->GetState() == STATE_CLICKED)
			{
				gameBrowser.fileList[i]->ResetState();
				
				// check corresponding browser entry
				if(browserList[browser.selIndex].isdir || IsSz())
				{	
					HaltGui();
					res = BrowserChangeFolder();
					if(res)
					{
						gameBrowser.ResetState();
						gameBrowser.fileList[0]->SetState(STATE_SELECTED);
						gameBrowser.TriggerUpdate();
						previousBrowserIndex = -1;			
					}
					else
					{
						menu = MENU_GAMESELECTION;
						break;
					}
					
					
					titleTxt.SetText(inSz ? szname : "Choose Game");
					
					ResumeGui();
				}
				else
				{
					#ifdef HW_RVL
					ShutoffRumble();
					#endif
					mainWindow->SetState(STATE_DISABLED);
					SavePrefs(SILENT);
					if(BrowserLoadFile())
						menu = MENU_EXIT;
					else
						mainWindow->SetState(STATE_DEFAULT);
				}
			}
		}
		
		//update game screenshot
		if(previousBrowserIndex != browser.selIndex)
		{			
			previousBrowserIndex = browser.selIndex;
			snprintf(screenshotPath, MAXJOLIET, "%s%s/%s.png", pathPrefix[GCSettings.LoadMethod], GCSettings.ScreenshotsFolder, browserList[browser.selIndex].displayname);
			
			AllocSaveBuffer();
			int width, height;
			if(LoadFile(screenshotPath, SILENT))
			{
				if(DecodePNG(savebuffer, &width, &height, imgBuffer, 512, 512))
				{
					preview.SetImage(imgBuffer, width, height);
					//preview.SetScale(225.0f / width);
					preview.SetScale( MIN(225.0f / width, 235.0f / height) );
				}
				else
				{
					preview.SetImage(NULL, 0, 0);
				}
			}
			else 
			{
				preview.SetImage(NULL, 0, 0);
			}
			FreeSaveBuffer();
		}

		if(settingsBtn.GetState() == STATE_CLICKED)
			menu = MENU_SETTINGS;
		else if(exitBtn.GetState() == STATE_CLICKED)
			ExitRequested = 1;
	}

	HaltParseThread(); // halt parsing
	HaltGui();
	ResetBrowser();
	mainWindow->Remove(&titleTxt);
	mainWindow->Remove(&buttonWindow);
	mainWindow->Remove(&gameBrowser);
	mainWindow->Remove(&bgPreview);
	mainWindow->Remove(&preview);
	MEM_DEALLOC(imgBuffer);
	return menu;
}

/****************************************************************************
 * ControllerWindowUpdate
 *
 * Callback for controller window. Responds to clicks on window elements.
 ***************************************************************************/
static void ControllerWindowUpdate(void * ptr, int dir)
{
	GuiButton * b = (GuiButton *)ptr;
	if(b->GetState() == STATE_CLICKED)
	{
		GCSettings.Controller += dir;

		if(GCSettings.Controller > CTRL_PAD4)
			GCSettings.Controller = CTRL_MOUSE;
		if(GCSettings.Controller < CTRL_MOUSE)
			GCSettings.Controller = CTRL_PAD4;

		settingText->SetText(ctrlName[GCSettings.Controller]);
		b->ResetState();
	}
}

/****************************************************************************
 * ControllerWindowLeftClick / ControllerWindowRightsClick
 *
 * Callbacks for controller window arrows. Responds arrow clicks.
 ***************************************************************************/
static void ControllerWindowLeftClick(void * ptr) { ControllerWindowUpdate(ptr, -1); }
static void ControllerWindowRightClick(void * ptr) { ControllerWindowUpdate(ptr, +1); }

/****************************************************************************
 * ControllerWindow
 *
 * Opens a window to allow the user to select the controller to be used.
 ***************************************************************************/
static void ControllerWindow()
{
	GuiWindow * w = new GuiWindow(300,250);
	w->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	GuiTrigger trigLeft;
	trigLeft.SetButtonOnlyInFocusTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);

	GuiTrigger trigRight;
	trigRight.SetButtonOnlyInFocusTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);

	GuiImageData arrowLeft(button_arrow_left_png);
	GuiImage arrowLeftImg(&arrowLeft);
	GuiImageData arrowLeftOver(button_arrow_left_over_png);
	GuiImage arrowLeftOverImg(&arrowLeftOver);
	GuiButton arrowLeftBtn(arrowLeft.GetWidth(), arrowLeft.GetHeight());
	arrowLeftBtn.SetImage(&arrowLeftImg);
	arrowLeftBtn.SetImageOver(&arrowLeftOverImg);
	arrowLeftBtn.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	arrowLeftBtn.SetTrigger(trigA);
	arrowLeftBtn.SetTrigger(trig2);
	arrowLeftBtn.SetTrigger(&trigLeft);
	arrowLeftBtn.SetSelectable(false);
	arrowLeftBtn.SetUpdateCallback(ControllerWindowLeftClick);

	GuiImageData arrowRight(button_arrow_right_png);
	GuiImage arrowRightImg(&arrowRight);
	GuiImageData arrowRightOver(button_arrow_right_over_png);
	GuiImage arrowRightOverImg(&arrowRightOver);
	GuiButton arrowRightBtn(arrowRight.GetWidth(), arrowRight.GetHeight());
	arrowRightBtn.SetImage(&arrowRightImg);
	arrowRightBtn.SetImageOver(&arrowRightOverImg);
	arrowRightBtn.SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
	arrowRightBtn.SetTrigger(trigA);
	arrowRightBtn.SetTrigger(trig2);
	arrowRightBtn.SetTrigger(&trigRight);
	arrowRightBtn.SetSelectable(false);
	arrowRightBtn.SetUpdateCallback(ControllerWindowRightClick);

	settingText = new GuiText(ctrlName[GCSettings.Controller], 22, (GXColor){0, 0, 0, 255});

	int currentController = GCSettings.Controller;

	w->Append(&arrowLeftBtn);
	w->Append(&arrowRightBtn);
	w->Append(settingText);

	if(!SettingWindow("Controller",w))
		GCSettings.Controller = currentController; // undo changes

	delete(w);
	delete(settingText);
}

/****************************************************************************
 * MenuGame
 *
 * Menu displayed when returning to the menu from in-game.
 ***************************************************************************/
static int MenuGame()
{
	int menu = MENU_NONE;
	
	GuiText titleTxt((char *)Memory.ROMFilename, 22, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData btnCloseOutline(button_small_png);
	GuiImageData btnCloseOutlineOver(button_small_over_png);
	GuiImageData btnLargeOutline(button_large_png);
	GuiImageData btnLargeOutlineOver(button_large_over_png);
	GuiImageData iconGameSettings(icon_game_settings_png);
	GuiImageData iconLoad(icon_game_load_png);
	GuiImageData iconSave(icon_game_save_png);
	GuiImageData iconReset(icon_game_reset_png);

	GuiImageData battery(battery_png);
	GuiImageData batteryRed(battery_red_png);
	GuiImageData batteryBar(battery_bar_png);

	GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

	GuiText saveBtnTxt("Save", 22, (GXColor){0, 0, 0, 255});
	GuiImage saveBtnImg(&btnLargeOutline);
	GuiImage saveBtnImgOver(&btnLargeOutlineOver);
	GuiImage saveBtnIcon(&iconSave);
	GuiButton saveBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	saveBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	saveBtn.SetPosition(-125, 120);
	saveBtn.SetLabel(&saveBtnTxt);
	saveBtn.SetImage(&saveBtnImg);
	saveBtn.SetImageOver(&saveBtnImgOver);
	saveBtn.SetIcon(&saveBtnIcon);
	saveBtn.SetSoundOver(&btnSoundOver);
	saveBtn.SetSoundClick(&btnSoundClick);
	saveBtn.SetTrigger(trigA);
	saveBtn.SetTrigger(trig2);
	saveBtn.SetEffectGrow();

	GuiText loadBtnTxt("Load", 22, (GXColor){0, 0, 0, 255});
	GuiImage loadBtnImg(&btnLargeOutline);
	GuiImage loadBtnImgOver(&btnLargeOutlineOver);
	GuiImage loadBtnIcon(&iconLoad);
	GuiButton loadBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	loadBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	loadBtn.SetPosition(125, 120);
	loadBtn.SetLabel(&loadBtnTxt);
	loadBtn.SetImage(&loadBtnImg);
	loadBtn.SetImageOver(&loadBtnImgOver);
	loadBtn.SetIcon(&loadBtnIcon);
	loadBtn.SetSoundOver(&btnSoundOver);
	loadBtn.SetSoundClick(&btnSoundClick);
	loadBtn.SetTrigger(trigA);
	loadBtn.SetTrigger(trig2);
	loadBtn.SetEffectGrow();

	GuiText resetBtnTxt("Reset", 22, (GXColor){0, 0, 0, 255});
	GuiImage resetBtnImg(&btnLargeOutline);
	GuiImage resetBtnImgOver(&btnLargeOutlineOver);
	GuiImage resetBtnIcon(&iconReset);
	GuiButton resetBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	resetBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	resetBtn.SetPosition(125, 250);
	resetBtn.SetLabel(&resetBtnTxt);
	resetBtn.SetImage(&resetBtnImg);
	resetBtn.SetImageOver(&resetBtnImgOver);
	resetBtn.SetIcon(&resetBtnIcon);
	resetBtn.SetSoundOver(&btnSoundOver);
	resetBtn.SetSoundClick(&btnSoundClick);
	resetBtn.SetTrigger(trigA);
	resetBtn.SetTrigger(trig2);
	resetBtn.SetEffectGrow();

	GuiText gameSettingsBtnTxt("Game Settings", 22, (GXColor){0, 0, 0, 255});
	gameSettingsBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-20);
	GuiImage gameSettingsBtnImg(&btnLargeOutline);
	GuiImage gameSettingsBtnImgOver(&btnLargeOutlineOver);
	GuiImage gameSettingsBtnIcon(&iconGameSettings);
	GuiButton gameSettingsBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	gameSettingsBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	gameSettingsBtn.SetPosition(-125, 250);
	gameSettingsBtn.SetLabel(&gameSettingsBtnTxt);
	gameSettingsBtn.SetImage(&gameSettingsBtnImg);
	gameSettingsBtn.SetImageOver(&gameSettingsBtnImgOver);
	gameSettingsBtn.SetIcon(&gameSettingsBtnIcon);
	gameSettingsBtn.SetSoundOver(&btnSoundOver);
	gameSettingsBtn.SetSoundClick(&btnSoundClick);
	gameSettingsBtn.SetTrigger(trigA);
	gameSettingsBtn.SetTrigger(trig2);
	gameSettingsBtn.SetEffectGrow();

	char Lable[28];
	snprintf(Lable, sizeof(Lable), "Back to %s", GCSettings.LoaderName);
	GuiText mainmenuBtnTxt(Lable, 22, (GXColor){0, 0, 0, 255});
	GuiImage mainmenuBtnImg(&btnOutline);
	GuiImage mainmenuBtnImgOver(&btnOutlineOver);
	GuiButton mainmenuBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	mainmenuBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	mainmenuBtn.SetPosition(0, -35);
	mainmenuBtn.SetLabel(&mainmenuBtnTxt);
	mainmenuBtn.SetImage(&mainmenuBtnImg);
	mainmenuBtn.SetImageOver(&mainmenuBtnImgOver);
	mainmenuBtn.SetSoundOver(&btnSoundOver);
	mainmenuBtn.SetSoundClick(&btnSoundClick);
	mainmenuBtn.SetTrigger(trigA);
	mainmenuBtn.SetTrigger(trig2);
	mainmenuBtn.SetEffectGrow();

	GuiText closeBtnTxt("Close", 20, (GXColor){0, 0, 0, 255});
	GuiImage closeBtnImg(&btnCloseOutline);
	GuiImage closeBtnImgOver(&btnCloseOutlineOver);
	GuiButton closeBtn(btnCloseOutline.GetWidth(), btnCloseOutline.GetHeight());
	closeBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	closeBtn.SetPosition(-50, 35);
	closeBtn.SetLabel(&closeBtnTxt);
	closeBtn.SetImage(&closeBtnImg);
	closeBtn.SetImageOver(&closeBtnImgOver);
	closeBtn.SetSoundOver(&btnSoundOver);
	closeBtn.SetSoundClick(&btnSoundClick);
	closeBtn.SetTrigger(trigA);
	closeBtn.SetTrigger(trig2);
	closeBtn.SetTrigger(&trigHome);
	closeBtn.SetEffectGrow();

	#ifdef HW_RVL
	int i;
	char txt[3];
	bool status[4] = { false, false, false, false };
	int level[4] = { 0, 0, 0, 0 };
	bool newStatus;
	int newLevel;
	GuiText * batteryTxt[4];
	GuiImage * batteryImg[4];
	GuiImage * batteryBarImg[4];
	GuiButton * batteryBtn[4];

	for(i=0; i < 4; i++)
	{
		sprintf(txt, "P%d", i+1);

		batteryTxt[i] = new GuiText(txt, 20, (GXColor){255, 255, 255, 255});
		batteryTxt[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryImg[i] = new GuiImage(&battery);
		batteryImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryImg[i]->SetPosition(30, 0);
		batteryBarImg[i] = new GuiImage(&batteryBar);
		batteryBarImg[i]->SetTile(0);
		batteryBarImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
		batteryBarImg[i]->SetPosition(34, 0);

		batteryBtn[i] = new GuiButton(70, 20);
		batteryBtn[i]->SetLabel(batteryTxt[i]);
		batteryBtn[i]->SetImage(batteryImg[i]);
		batteryBtn[i]->SetIcon(batteryBarImg[i]);
		batteryBtn[i]->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
		batteryBtn[i]->SetRumble(false);
		batteryBtn[i]->SetSelectable(false);
		batteryBtn[i]->SetAlpha(150);
	}

	batteryBtn[0]->SetPosition(45, -65);
	batteryBtn[1]->SetPosition(135, -65);
	batteryBtn[2]->SetPosition(45, -40);
	batteryBtn[3]->SetPosition(135, -40);
	#endif

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&titleTxt);
	w.Append(&saveBtn);
	w.Append(&loadBtn);
	w.Append(&resetBtn);
	w.Append(&gameSettingsBtn);

	#ifdef HW_RVL
	w.Append(batteryBtn[0]);
	w.Append(batteryBtn[1]);
	w.Append(batteryBtn[2]);
	w.Append(batteryBtn[3]);
	#endif

	w.Append(&mainmenuBtn);
	w.Append(&closeBtn);

	btnLogo->SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	btnLogo->SetPosition(-50, -40);
	mainWindow->Append(&w);

	if(lastMenu == MENU_NONE)
	{
		enterSound->Play();
		bgTopImg->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 35);
		closeBtn.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 35);
		titleTxt.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 35);
		mainmenuBtn.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 35);
		bgBottomImg->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 35);
		btnLogo->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 35);
		#ifdef HW_RVL
		batteryBtn[0]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 35);
		batteryBtn[1]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 35);
		batteryBtn[2]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 35);
		batteryBtn[3]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 35);
		#endif

		w.SetEffect(EFFECT_FADE, 15);
	}

	ResumeGui();
	
	if(lastMenu == MENU_NONE)
		AutoSave();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		#ifdef HW_RVL
		for(i=0; i < 4; i++)
		{
			if(WPAD_Probe(i, NULL) == WPAD_ERR_NONE)
			{
				newStatus = true;
				newLevel = (userInput[i].wpad->battery_level / 100.0) * 4;
				if(newLevel > 4) newLevel = 4;
			}
			else
			{
				newStatus = false;
				newLevel = 0;
			}
			
			if(status[i] != newStatus || level[i] != newLevel)
			{
				if(newStatus == true) // controller connected
				{
					batteryBtn[i]->SetAlpha(255);
					batteryBarImg[i]->SetTile(newLevel);

					if(newLevel == 0)
						batteryImg[i]->SetImage(&batteryRed);
					else
						batteryImg[i]->SetImage(&battery);
				}
				else // controller not connected
				{
					batteryBtn[i]->SetAlpha(150);
					batteryBarImg[i]->SetTile(0);
					batteryImg[i]->SetImage(&battery);
				}
				status[i] = newStatus;
				level[i] = newLevel;
			}
		}
		#endif

		if(saveBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAME_SAVE;
		}
		else if(loadBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAME_LOAD;
		}
		else if(resetBtn.GetState() == STATE_CLICKED)
		{
			if (WindowPrompt("Reset Game", "Are you sure that you want to reset this game? Any unsaved progress will be lost.", "OK", "Cancel"))
			{
				S9xSoftReset ();
				menu = MENU_EXIT;
			}
		}
		else if(gameSettingsBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAMESETTINGS;
		}
		else if(mainmenuBtn.GetState() == STATE_CLICKED)
		{
			if (WindowPrompt("Quit Game", "Quit this game? Any unsaved progress will be lost.", "OK", "Cancel"))
			{
				HaltGui();
				mainWindow->Remove(gameScreenImg);
				delete gameScreenImg;
				delete gameScreen;
				gameScreen = NULL;
				free(gameScreenPng);
				gameScreenPng = NULL;
				ExitToWiiflow();
			}
		}
		else if(closeBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_EXIT;

			exitSound->Play();
			bgTopImg->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 15);
			closeBtn.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 15);
			titleTxt.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 15);
			mainmenuBtn.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 15);
			bgBottomImg->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 15);
			btnLogo->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 15);
			#ifdef HW_RVL
			batteryBtn[0]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 15);
			batteryBtn[1]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 15);
			batteryBtn[2]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 15);
			batteryBtn[3]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 15);
			#endif

			w.SetEffect(EFFECT_FADE, -15);
			usleep(350000); // wait for effects to finish
		}
	}

	HaltGui();

	#ifdef HW_RVL
	for(i=0; i < 4; i++)
	{
		delete batteryTxt[i];
		delete batteryImg[i];
		delete batteryBarImg[i];
		delete batteryBtn[i];
	}
	#endif

	mainWindow->Remove(&w);
	return menu;
}

/****************************************************************************
 * FindGameSaveNum
 *
 * Determines the save file number of the given file name
 * Returns -1 if none is found
 ***************************************************************************/
static int FindGameSaveNum(char * savefile, int device)
{
	int n = -1;
	int romlen = strlen(Memory.ROMFilename);
	int savelen = strlen(savefile);

	int diff = savelen-romlen;

	if(strncmp(savefile, Memory.ROMFilename, romlen) != 0)
		return -1;

	if(savefile[romlen] == ' ')
	{
		if(diff == 5 && strncmp(&savefile[romlen+1], "Auto", 4) == 0)
			n = 0; // found Auto save
		else if(diff == 2 || diff == 3)
			n = atoi(&savefile[romlen+1]);
	}

	if(n >= 0 && n < MAX_SAVES)
		return n;
	else
		return -1;
}

/****************************************************************************
 * MenuGameSaves
 *
 * Allows the user to load or save progress.
 ***************************************************************************/
static int MenuGameSaves(int action)
{
	int menu = MENU_NONE;
	int ret;
	int i, n, type, len, len2;
	int j = 0;
	SaveList saves;
	char filepath[1024];
	char scrfile[1024];
	char tmp[MAXJOLIET+1];
	struct stat filestat;
	struct tm * timeinfo;
	int device = GCSettings.SaveMethod;

	if(device == DEVICE_AUTO)
		autoSaveMethod(NOTSILENT);

	if(!ChangeInterface(device, NOTSILENT))
		return MENU_GAME;

	GuiText titleTxt(NULL, 26, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	if(action == 0)
		titleTxt.SetText("Load Game");
	else
		titleTxt.SetText("Save Game");

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData btnCloseOutline(button_small_png);
	GuiImageData btnCloseOutlineOver(button_small_over_png);

	GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

	GuiText backBtnTxt("Go Back", 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetSoundClick(&btnSoundClick);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trig2);
	backBtn.SetEffectGrow();

	GuiText closeBtnTxt("Close", 20, (GXColor){0, 0, 0, 255});
	GuiImage closeBtnImg(&btnCloseOutline);
	GuiImage closeBtnImgOver(&btnCloseOutlineOver);
	GuiButton closeBtn(btnCloseOutline.GetWidth(), btnCloseOutline.GetHeight());
	closeBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	closeBtn.SetPosition(-50, 35);
	closeBtn.SetLabel(&closeBtnTxt);
	closeBtn.SetImage(&closeBtnImg);
	closeBtn.SetImageOver(&closeBtnImgOver);
	closeBtn.SetSoundOver(&btnSoundOver);
	closeBtn.SetSoundClick(&btnSoundClick);
	closeBtn.SetTrigger(trigA);
	closeBtn.SetTrigger(trig2);
	closeBtn.SetTrigger(&trigHome);
	closeBtn.SetEffectGrow();

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	w.Append(&closeBtn);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	memset(&saves, 0, sizeof(saves));

	sprintf(browser.dir, "%s%s", pathPrefix[GCSettings.SaveMethod], GCSettings.SaveFolder);
	ParseDirectory(true, false);

	len = strlen(Memory.ROMFilename);

	// find matching files
	AllocSaveBuffer();

	for(i=0; i < browser.numEntries; i++)
	{
		len2 = strlen(browserList[i].filename);

		if(len2 < 6 || len2-len < 5)
			continue;

		if(strncmp(&browserList[i].filename[len2-4], ".srm", 4) == 0)
			type = FILE_SRAM;
		else if(strncmp(&browserList[i].filename[len2-4], ".frz", 4) == 0)
			type = FILE_SNAPSHOT;
		else
			continue;

		strcpy(tmp, browserList[i].filename);
		tmp[len2-4] = 0;
		n = FindGameSaveNum(tmp, device);

		if(n >= 0)
		{
			saves.type[j] = type;
			saves.files[saves.type[j]][n] = 1;
			strcpy(saves.filename[j], browserList[i].filename);

			if(saves.type[j] == FILE_SNAPSHOT)
			{
				sprintf(scrfile, "%s%s/%s.png", pathPrefix[GCSettings.SaveMethod], GCSettings.SaveFolder, tmp);

				memset(savebuffer, 0, SAVEBUFFERSIZE);
				if(LoadFile(scrfile, SILENT))
					saves.previewImg[j] = new GuiImageData(savebuffer, 64, 48);
			}
			snprintf(filepath, 1024, "%s%s/%s", pathPrefix[GCSettings.SaveMethod], GCSettings.SaveFolder, saves.filename[j]);
			if (stat(filepath, &filestat) == 0)
			{
				timeinfo = localtime(&filestat.st_mtime);
				strftime(saves.date[j], 20, "%a %b %d", timeinfo);
				strftime(saves.time[j], 10, "%I:%M %p", timeinfo);
			}
			j++;
		}
	}

	FreeSaveBuffer();
	saves.length = j;

	if(saves.length == 0 && action == 0)
	{
		InfoPrompt("No game saves found.");
		menu = MENU_GAME;
	}

	GuiSaveBrowser saveBrowser(552, 248, &saves, action);
	saveBrowser.SetPosition(0, 108);
	saveBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

	HaltGui();
	mainWindow->Append(&saveBrowser);
	mainWindow->ChangeFocus(&saveBrowser);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		ret = saveBrowser.GetClickedSave();

		// load or save game
		if(ret > -3)
		{
			int result = 0;

			if(action == 0) // load
			{
				MakeFilePath(filepath, saves.type[ret], saves.filename[ret]);
				switch(saves.type[ret])
				{
					case FILE_SRAM:
						result = LoadSRAM(filepath, NOTSILENT);
						break;
					case FILE_SNAPSHOT:
						result = LoadSnapshot (filepath, NOTSILENT);
						break;
				}
				if(result)
					menu = MENU_EXIT;
			}
			else // save
			{
				if(ret == -2) // new SRAM
				{
					for(i=1; i < 100; i++)
						if(saves.files[FILE_SRAM][i] == 0)
							break;

					if(i < 100)
					{
						MakeFilePath(filepath, FILE_SRAM, Memory.ROMFilename, i);
						SaveSRAM(filepath, NOTSILENT);
						menu = MENU_GAME_SAVE;
					}
				}
				else if(ret == -1) // new Snapshot
				{
					for(i=1; i < 100; i++)
						if(saves.files[FILE_SNAPSHOT][i] == 0)
							break;

					if(i < 100)
					{
						MakeFilePath(filepath, FILE_SNAPSHOT, Memory.ROMFilename, i);
						SaveSnapshot (filepath, NOTSILENT);
						menu = MENU_GAME_SAVE;
					}
				}
				else // overwrite SRAM/Snapshot
				{
					MakeFilePath(filepath, saves.type[ret], saves.filename[ret]);
					switch(saves.type[ret])
					{
						case FILE_SRAM:
							SaveSRAM(filepath, NOTSILENT);
							break;
						case FILE_SNAPSHOT:
							SaveSnapshot (filepath, NOTSILENT);
							break;
					}
					menu = MENU_GAME_SAVE;
				}
			}
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAME;
		}
		else if(closeBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_EXIT;

			exitSound->Play();
			bgTopImg->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 15);
			closeBtn.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 15);
			titleTxt.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 15);
			backBtn.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 15);
			bgBottomImg->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 15);
			btnLogo->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 15);

			w.SetEffect(EFFECT_FADE, -15);

			usleep(350000); // wait for effects to finish
		}
	}

	HaltGui();

	for(i=0; i < saves.length; i++)
		if(saves.previewImg[i])
			delete saves.previewImg[i];

	mainWindow->Remove(&saveBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	ResetBrowser();
	return menu;
}

/****************************************************************************
 * MenuGameSettings
 ***************************************************************************/
static int MenuGameSettings()
{
	int menu = MENU_NONE;
	char filepath[1024];

	GuiText titleTxt("Game Settings", 26, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData btnLargeOutline(button_large_png);
	GuiImageData btnLargeOutlineOver(button_large_over_png);
	GuiImageData iconMappings(icon_settings_mappings_png);
	GuiImageData iconVideo(icon_settings_video_png);
	GuiImageData iconController(icon_game_controllers_png);
	GuiImageData iconCheats(icon_game_cheats_png);
	GuiImageData iconScreenshot(icon_settings_screenshot_png);
	GuiImageData btnCloseOutline(button_small_png);
	GuiImageData btnCloseOutlineOver(button_small_over_png);

	GuiTrigger trigHome;
	trigHome.SetButtonOnlyTrigger(-1, WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME, 0);

	GuiText mappingBtnTxt("Button Mappings", 22, (GXColor){0, 0, 0, 255});
	mappingBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-30);
	GuiImage mappingBtnImg(&btnLargeOutline);
	GuiImage mappingBtnImgOver(&btnLargeOutlineOver);
	GuiImage mappingBtnIcon(&iconMappings);
	GuiButton mappingBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	mappingBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	mappingBtn.SetPosition(-125, 120);
	mappingBtn.SetLabel(&mappingBtnTxt);
	mappingBtn.SetImage(&mappingBtnImg);
	mappingBtn.SetImageOver(&mappingBtnImgOver);
	mappingBtn.SetIcon(&mappingBtnIcon);
	mappingBtn.SetSoundOver(&btnSoundOver);
	mappingBtn.SetSoundClick(&btnSoundClick);
	mappingBtn.SetTrigger(trigA);
	mappingBtn.SetTrigger(trig2);
	mappingBtn.SetEffectGrow();

	GuiText videoBtnTxt("Video", 22, (GXColor){0, 0, 0, 255});
	videoBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-20);
	GuiImage videoBtnImg(&btnLargeOutline);
	GuiImage videoBtnImgOver(&btnLargeOutlineOver);
	GuiImage videoBtnIcon(&iconVideo);
	GuiButton videoBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	videoBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	videoBtn.SetPosition(125, 120);
	videoBtn.SetLabel(&videoBtnTxt);
	videoBtn.SetImage(&videoBtnImg);
	videoBtn.SetImageOver(&videoBtnImgOver);
	videoBtn.SetIcon(&videoBtnIcon);
	videoBtn.SetSoundOver(&btnSoundOver);
	videoBtn.SetSoundClick(&btnSoundClick);
	videoBtn.SetTrigger(trigA);
	videoBtn.SetTrigger(trig2);
	videoBtn.SetEffectGrow();

	GuiText controllerBtnTxt("Controller", 22, (GXColor){0, 0, 0, 255});
	GuiImage controllerBtnImg(&btnLargeOutline);
	GuiImage controllerBtnImgOver(&btnLargeOutlineOver);
	GuiImage controllerBtnIcon(&iconController);
	GuiButton controllerBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	controllerBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	controllerBtn.SetPosition(-200, 250);
	controllerBtn.SetLabel(&controllerBtnTxt);
	controllerBtn.SetImage(&controllerBtnImg);
	controllerBtn.SetImageOver(&controllerBtnImgOver);
	controllerBtn.SetIcon(&controllerBtnIcon);
	controllerBtn.SetSoundOver(&btnSoundOver);
	controllerBtn.SetSoundClick(&btnSoundClick);
	controllerBtn.SetTrigger(trigA);
	controllerBtn.SetTrigger(trig2);
	controllerBtn.SetEffectGrow();

	GuiText screenshotBtnTxt("ScreenShot", 22, (GXColor){0, 0, 0, 255});
	GuiImage screenshotBtnImg(&btnLargeOutline);
	GuiImage screenshotBtnImgOver(&btnLargeOutlineOver);
	GuiImage screenshotBtnIcon(&iconScreenshot);
	GuiButton screenshotBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	screenshotBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	screenshotBtn.SetPosition(0, 250);
	screenshotBtn.SetLabel(&screenshotBtnTxt);
	screenshotBtn.SetImage(&screenshotBtnImg);
	screenshotBtn.SetImageOver(&screenshotBtnImgOver);
	screenshotBtn.SetIcon(&screenshotBtnIcon);
	screenshotBtn.SetSoundOver(&btnSoundOver);
	screenshotBtn.SetSoundClick(&btnSoundClick);
	screenshotBtn.SetTrigger(trigA);
	screenshotBtn.SetTrigger(trig2);
	screenshotBtn.SetEffectGrow();
	
	GuiText cheatsBtnTxt("Cheats", 22, (GXColor){0, 0, 0, 255});
	GuiImage cheatsBtnImg(&btnLargeOutline);
	GuiImage cheatsBtnImgOver(&btnLargeOutlineOver);
	GuiImage cheatsBtnIcon(&iconCheats);
	GuiButton cheatsBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	cheatsBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	cheatsBtn.SetPosition(200, 250);
	cheatsBtn.SetLabel(&cheatsBtnTxt);
	cheatsBtn.SetImage(&cheatsBtnImg);
	cheatsBtn.SetImageOver(&cheatsBtnImgOver);
	cheatsBtn.SetIcon(&cheatsBtnIcon);
	cheatsBtn.SetSoundOver(&btnSoundOver);
	cheatsBtn.SetSoundClick(&btnSoundClick);
	cheatsBtn.SetTrigger(trigA);
	cheatsBtn.SetTrigger(trig2);
	cheatsBtn.SetEffectGrow();

	GuiText closeBtnTxt("Close", 20, (GXColor){0, 0, 0, 255});
	GuiImage closeBtnImg(&btnCloseOutline);
	GuiImage closeBtnImgOver(&btnCloseOutlineOver);
	GuiButton closeBtn(btnCloseOutline.GetWidth(), btnCloseOutline.GetHeight());
	closeBtn.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	closeBtn.SetPosition(-50, 35);
	closeBtn.SetLabel(&closeBtnTxt);
	closeBtn.SetImage(&closeBtnImg);
	closeBtn.SetImageOver(&closeBtnImgOver);
	closeBtn.SetSoundOver(&btnSoundOver);
	closeBtn.SetSoundClick(&btnSoundClick);
	closeBtn.SetTrigger(trigA);
	closeBtn.SetTrigger(trig2);
	closeBtn.SetTrigger(&trigHome);
	closeBtn.SetEffectGrow();

	GuiText backBtnTxt("Go Back", 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetSoundClick(&btnSoundClick);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trig2);
	backBtn.SetEffectGrow();

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&titleTxt);
	w.Append(&mappingBtn);
	w.Append(&videoBtn);
	w.Append(&controllerBtn);
	w.Append(&screenshotBtn);
	w.Append(&cheatsBtn);
	w.Append(&closeBtn);
	w.Append(&backBtn);
	
	mainWindow->Append(&w);

	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		if(mappingBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAMESETTINGS_MAPPINGS;
		}
		else if(videoBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAMESETTINGS_VIDEO;
		}
		else if(controllerBtn.GetState() == STATE_CLICKED)
		{
			ControllerWindow();
		}
		else if(cheatsBtn.GetState() == STATE_CLICKED)
		{
			cheatsBtn.ResetState();
			if(Cheat.num_cheats > 0)
				menu = MENU_GAMESETTINGS_CHEATS;
			else
				InfoPrompt("Cheats file not found!");
		}
		else if(screenshotBtn.GetState() == STATE_CLICKED)
		{
			if (WindowPrompt("Preview Screenshot", "Save a new Preview Screenshot? Current Screenshot image will be overwritten.", "OK", "Cancel"))
			{
				snprintf(filepath, 1024, "%s%s/%s", pathPrefix[GCSettings.SaveMethod], GCSettings.ScreenshotsFolder, Memory.ROMFilename);
				SavePreviewImg(filepath, NOTSILENT); 
			}
		}
		else if(closeBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_EXIT;

			exitSound->Play();
			bgTopImg->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 15);
			closeBtn.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 15);
			titleTxt.SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 15);
			backBtn.SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 15);
			bgBottomImg->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 15);
			btnLogo->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 15);

			w.SetEffect(EFFECT_FADE, -15);

			usleep(350000); // wait for effects to finish
		}
		else if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAME;
		}
	}

	HaltGui();
	mainWindow->Remove(&w);
	return menu;
}

/****************************************************************************
 * MenuGameCheats
 *
 * Displays a list of cheats available, and allows the user to enable/disable
 * them.
 ***************************************************************************/
static int MenuGameCheats()
{
	int menu = MENU_NONE;
	int ret;
	u16 i = 0;
	OptionList options;

	for(i=0; i < Cheat.num_cheats; i++)
	{
		sprintf (options.name[i], "%s", Cheat.c[i].name);
		sprintf (options.value[i], "%s", Cheat.c[i].enabled == true ? "On" : "Off");
	}

	options.length = i;

	GuiText titleTxt("Game Settings - Cheats", 26, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiText backBtnTxt("Go Back", 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetSoundClick(&btnSoundClick);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trig2);
	backBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(552, 248, &options);
	optionBrowser.SetPosition(0, 108);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		if(ret >= 0)
		{
			if(Cheat.c[ret].enabled)
				S9xDisableCheat(ret);
			else
				S9xEnableCheat(ret);
			sprintf (options.value[ret], "%s", Cheat.c[ret].enabled == true ? "On" : "Off");
			optionBrowser.TriggerUpdate();
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAMESETTINGS;
		}
	}
	HaltGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	return menu;
}

/****************************************************************************
 * MenuSettingsMappings
 ***************************************************************************/
static int MenuSettingsMappings()
{
	int menu = MENU_NONE;

	GuiText titleTxt("Game Settings - Button Mappings", 26, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData btnLargeOutline(button_large_png);
	GuiImageData btnLargeOutlineOver(button_large_over_png);
	GuiImageData iconSNESController(icon_settings_snescontroller_png);
	GuiImageData iconSuperscope(icon_settings_superscope_png);
	GuiImageData iconJustifier(icon_settings_justifier_png);
	GuiImageData iconMouse(icon_settings_mouse_png);

	GuiText snesBtnTxt("SNES Controller", 22, (GXColor){0, 0, 0, 255});
	snesBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-40);
	GuiImage snesBtnImg(&btnLargeOutline);
	GuiImage snesBtnImgOver(&btnLargeOutlineOver);
	GuiImage snesBtnIcon(&iconSNESController);
	GuiButton snesBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	snesBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	snesBtn.SetPosition(-125, 120);
	snesBtn.SetLabel(&snesBtnTxt);
	snesBtn.SetImage(&snesBtnImg);
	snesBtn.SetImageOver(&snesBtnImgOver);
	snesBtn.SetIcon(&snesBtnIcon);
	snesBtn.SetSoundOver(&btnSoundOver);
	snesBtn.SetSoundClick(&btnSoundClick);
	snesBtn.SetTrigger(trigA);
	snesBtn.SetTrigger(trig2);
	snesBtn.SetEffectGrow();

	GuiText superscopeBtnTxt("Super Scope", 22, (GXColor){0, 0, 0, 255});
	superscopeBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-20);
	GuiImage superscopeBtnImg(&btnLargeOutline);
	GuiImage superscopeBtnImgOver(&btnLargeOutlineOver);
	GuiImage superscopeBtnIcon(&iconSuperscope);
	GuiButton superscopeBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	superscopeBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	superscopeBtn.SetPosition(125, 120);
	superscopeBtn.SetLabel(&superscopeBtnTxt);
	superscopeBtn.SetImage(&superscopeBtnImg);
	superscopeBtn.SetImageOver(&superscopeBtnImgOver);
	superscopeBtn.SetIcon(&superscopeBtnIcon);
	superscopeBtn.SetSoundOver(&btnSoundOver);
	superscopeBtn.SetSoundClick(&btnSoundClick);
	superscopeBtn.SetTrigger(trigA);
	superscopeBtn.SetTrigger(trig2);
	superscopeBtn.SetEffectGrow();

	GuiText mouseBtnTxt("SNES Mouse", 22, (GXColor){0, 0, 0, 255});
	mouseBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-55);
	GuiImage mouseBtnImg(&btnLargeOutline);
	GuiImage mouseBtnImgOver(&btnLargeOutlineOver);
	GuiImage mouseBtnIcon(&iconMouse);
	GuiButton mouseBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	mouseBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	mouseBtn.SetPosition(-125, 250);
	mouseBtn.SetLabel(&mouseBtnTxt);
	mouseBtn.SetImage(&mouseBtnImg);
	mouseBtn.SetImageOver(&mouseBtnImgOver);
	mouseBtn.SetIcon(&mouseBtnIcon);
	mouseBtn.SetSoundOver(&btnSoundOver);
	mouseBtn.SetSoundClick(&btnSoundClick);
	mouseBtn.SetTrigger(trigA);
	mouseBtn.SetTrigger(trig2);
	mouseBtn.SetEffectGrow();

	GuiText justifierBtnTxt("Justifier", 22, (GXColor){0, 0, 0, 255});
	GuiImage justifierBtnImg(&btnLargeOutline);
	GuiImage justifierBtnImgOver(&btnLargeOutlineOver);
	GuiImage justifierBtnIcon(&iconJustifier);
	GuiButton justifierBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	justifierBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	justifierBtn.SetPosition(125, 250);
	justifierBtn.SetLabel(&justifierBtnTxt);
	justifierBtn.SetImage(&justifierBtnImg);
	justifierBtn.SetImageOver(&justifierBtnImgOver);
	justifierBtn.SetIcon(&justifierBtnIcon);
	justifierBtn.SetSoundOver(&btnSoundOver);
	justifierBtn.SetSoundClick(&btnSoundClick);
	justifierBtn.SetTrigger(trigA);
	justifierBtn.SetTrigger(trig2);
	justifierBtn.SetEffectGrow();

	GuiText backBtnTxt("Go Back", 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetSoundClick(&btnSoundClick);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trig2);
	backBtn.SetEffectGrow();

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&titleTxt);
	w.Append(&snesBtn);
	w.Append(&superscopeBtn);
	w.Append(&mouseBtn);
	w.Append(&justifierBtn);

	w.Append(&backBtn);

	mainWindow->Append(&w);

	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		if(snesBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAMESETTINGS_MAPPINGS_CTRL;
			mapMenuCtrlSNES = CTRL_PAD;
		}
		else if(superscopeBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAMESETTINGS_MAPPINGS_CTRL;
			mapMenuCtrlSNES = CTRL_SCOPE;
		}
		else if(mouseBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAMESETTINGS_MAPPINGS_CTRL;
			mapMenuCtrlSNES = CTRL_MOUSE;
		}
		else if(justifierBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAMESETTINGS_MAPPINGS_CTRL;
			mapMenuCtrlSNES = CTRL_JUST;
		}
		else if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAMESETTINGS;
		}
	}
	HaltGui();
	mainWindow->Remove(&w);
	return menu;
}

static int MenuSettingsMappingsController()
{
	int menu = MENU_NONE;
	char menuTitle[100];
	char menuSubtitle[100];

	sprintf(menuTitle, "Game Settings - Button Mappings");
	GuiText titleTxt(menuTitle, 26, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,30);

	sprintf(menuSubtitle, "%s", ctrlName[mapMenuCtrlSNES]);
	GuiText subtitleTxt(menuSubtitle, 20, (GXColor){255, 255, 255, 255});
	subtitleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	subtitleTxt.SetPosition(50,60);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData btnLargeOutline(button_large_png);
	GuiImageData btnLargeOutlineOver(button_large_over_png);
	GuiImageData iconWiimote(icon_settings_wiimote_png);
	GuiImageData iconClassic(icon_settings_classic_png);
	GuiImageData iconGamecube(icon_settings_gamecube_png);
	GuiImageData iconNunchuk(icon_settings_nunchuk_png);
	GuiImageData iconWiiupro(icon_settings_wiiupro_png);
	
	GuiText gamecubeBtnTxt("GameCube Controller", 22, (GXColor){0, 0, 0, 255});
	gamecubeBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-20);
	GuiImage gamecubeBtnImg(&btnLargeOutline);
	GuiImage gamecubeBtnImgOver(&btnLargeOutlineOver);
	GuiImage gamecubeBtnIcon(&iconGamecube);
	GuiButton gamecubeBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	gamecubeBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	gamecubeBtn.SetPosition(-125, 120);
	gamecubeBtn.SetLabel(&gamecubeBtnTxt);
	gamecubeBtn.SetImage(&gamecubeBtnImg);
	gamecubeBtn.SetImageOver(&gamecubeBtnImgOver);
	gamecubeBtn.SetIcon(&gamecubeBtnIcon);
	gamecubeBtn.SetSoundOver(&btnSoundOver);
	gamecubeBtn.SetSoundClick(&btnSoundClick);
	gamecubeBtn.SetTrigger(trigA);
	gamecubeBtn.SetTrigger(trig2);
	gamecubeBtn.SetEffectGrow();

	GuiText wiimoteBtnTxt("Wiimote", 22, (GXColor){0, 0, 0, 255});
	GuiImage wiimoteBtnImg(&btnLargeOutline);
	GuiImage wiimoteBtnImgOver(&btnLargeOutlineOver);
	GuiImage wiimoteBtnIcon(&iconWiimote);
	GuiButton wiimoteBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	wiimoteBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	wiimoteBtn.SetPosition(125, 120);
	wiimoteBtn.SetLabel(&wiimoteBtnTxt);
	wiimoteBtn.SetImage(&wiimoteBtnImg);
	wiimoteBtn.SetImageOver(&wiimoteBtnImgOver);
	wiimoteBtn.SetIcon(&wiimoteBtnIcon);
	wiimoteBtn.SetSoundOver(&btnSoundOver);
	wiimoteBtn.SetSoundClick(&btnSoundClick);
	wiimoteBtn.SetTrigger(trigA);
	wiimoteBtn.SetTrigger(trig2);
	wiimoteBtn.SetEffectGrow();

	GuiText classicBtnTxt("Classic Controller", 22, (GXColor){0, 0, 0, 255});
	classicBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-20);
	GuiImage classicBtnImg(&btnLargeOutline);
	GuiImage classicBtnImgOver(&btnLargeOutlineOver);
	GuiImage classicBtnIcon(&iconClassic);
	GuiButton classicBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	classicBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	classicBtn.SetPosition(-200, 250);
	classicBtn.SetLabel(&classicBtnTxt);
	classicBtn.SetImage(&classicBtnImg);
	classicBtn.SetImageOver(&classicBtnImgOver);
	classicBtn.SetIcon(&classicBtnIcon);
	classicBtn.SetSoundOver(&btnSoundOver);
	classicBtn.SetSoundClick(&btnSoundClick);
	classicBtn.SetTrigger(trigA);
	classicBtn.SetTrigger(trig2);
	classicBtn.SetEffectGrow();
	
	GuiText wiiuproBtnTxt("Wii U Pro Controller", 22, (GXColor){0, 0, 0, 255});
	wiiuproBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-20);
	GuiImage wiiuproBtnImg(&btnLargeOutline);
	GuiImage wiiuproBtnImgOver(&btnLargeOutlineOver);
	GuiImage wiiuproBtnIcon(&iconWiiupro);
	GuiButton wiiuproBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	wiiuproBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	wiiuproBtn.SetPosition(0, 250);
	wiiuproBtn.SetLabel(&wiiuproBtnTxt);
	wiiuproBtn.SetImage(&wiiuproBtnImg);
	wiiuproBtn.SetImageOver(&wiiuproBtnImgOver);
	wiiuproBtn.SetIcon(&wiiuproBtnIcon);
	wiiuproBtn.SetSoundOver(&btnSoundOver);
	wiiuproBtn.SetSoundClick(&btnSoundClick);
	wiiuproBtn.SetTrigger(trigA);
	wiiuproBtn.SetTrigger(trig2);
	wiiuproBtn.SetEffectGrow();

	GuiText nunchukBtnTxt1("Wiimote", 22, (GXColor){0, 0, 0, 255});
	GuiText nunchukBtnTxt2("&", 18, (GXColor){0, 0, 0, 255});
	GuiText nunchukBtnTxt3("Nunchuk", 22, (GXColor){0, 0, 0, 255});
	nunchukBtnTxt1.SetPosition(0, -20);
	nunchukBtnTxt3.SetPosition(0, +20);
	GuiImage nunchukBtnImg(&btnLargeOutline);
	GuiImage nunchukBtnImgOver(&btnLargeOutlineOver);
	GuiImage nunchukBtnIcon(&iconNunchuk);
	GuiButton nunchukBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	nunchukBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	nunchukBtn.SetPosition(200, 250);
	nunchukBtn.SetLabel(&nunchukBtnTxt1, 0);
	nunchukBtn.SetLabel(&nunchukBtnTxt2, 1);
	nunchukBtn.SetLabel(&nunchukBtnTxt3, 2);
	nunchukBtn.SetImage(&nunchukBtnImg);
	nunchukBtn.SetImageOver(&nunchukBtnImgOver);
	nunchukBtn.SetIcon(&nunchukBtnIcon);
	nunchukBtn.SetSoundOver(&btnSoundOver);
	nunchukBtn.SetSoundClick(&btnSoundClick);
	nunchukBtn.SetTrigger(trigA);
	nunchukBtn.SetTrigger(trig2);
	nunchukBtn.SetEffectGrow();

	GuiText backBtnTxt("Go Back", 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetSoundClick(&btnSoundClick);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trig2);
	backBtn.SetEffectGrow();

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&titleTxt);
	w.Append(&subtitleTxt);

	w.Append(&gamecubeBtn);
#ifdef HW_RVL
	w.Append(&wiimoteBtn);

	if(mapMenuCtrlSNES == CTRL_PAD)
	{
		w.Append(&nunchukBtn);
		w.Append(&classicBtn);
		w.Append(&wiiuproBtn);
	}
#endif
	w.Append(&backBtn);

	mainWindow->Append(&w);

	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		if(wiimoteBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAMESETTINGS_MAPPINGS_MAP;
			mapMenuCtrl = CTRLR_WIIMOTE;
		}
		else if(nunchukBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAMESETTINGS_MAPPINGS_MAP;
			mapMenuCtrl = CTRLR_NUNCHUK;
		}
		else if(classicBtn.GetState() == STATE_CLICKED)
		{
			wiiuproCtrl = 0;
			menu = MENU_GAMESETTINGS_MAPPINGS_MAP;
			mapMenuCtrl = CTRLR_CLASSIC;
		}
		else if(wiiuproBtn.GetState() == STATE_CLICKED)
		{
			wiiuproCtrl = 1;
			menu = MENU_GAMESETTINGS_MAPPINGS_MAP;
			mapMenuCtrl = CTRLR_CLASSIC;
		}
		else if(gamecubeBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAMESETTINGS_MAPPINGS_MAP;
			mapMenuCtrl = CTRLR_GCPAD;
		}
		else if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAMESETTINGS_MAPPINGS;
		}
	}
	HaltGui();
	mainWindow->Remove(&w);
	return menu;
}

/****************************************************************************
 * ButtonMappingWindow
 ***************************************************************************/

static u32
ButtonMappingWindow()
{
	GuiWindow promptWindow(448,288);
	promptWindow.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	promptWindow.SetPosition(0, -10);
	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiImageData dialogBox(dialogue_box_png);
	GuiImage dialogBoxImg(&dialogBox);

	GuiText titleTxt("Button Mapping", 26, (GXColor){70, 70, 10, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetPosition(0,14);

	char msg[200];

	switch(mapMenuCtrl)
	{
		case CTRLR_GCPAD:
			#ifdef HW_RVL
			sprintf(msg, "Press any button on the GameCube Controller now. Press Home or the C-Stick in any direction to clear the existing mapping.");
			#else
			sprintf(msg, "Press any button on the GameCube Controller now. Press the C-Stick in any direction to clear the existing mapping.");
			#endif
			break;
		case CTRLR_WIIMOTE:
			sprintf(msg, "Press any button on the Wiimote now. Press Home to clear the existing mapping.");
			break;
		case CTRLR_CLASSIC:
			if(wiiuproCtrl == 1)
			{
				sprintf(msg, "Press any button on the Wii U Pro Controller now. Press Home to clear the existing mapping.");
			}
			else
			{
				sprintf(msg, "Press any button on the Classic Controller now. Press Home to clear the existing mapping.");
			}
			break;
		case CTRLR_NUNCHUK:
			sprintf(msg, "Press any button on the Wiimote or Nunchuk now. Press Home to clear the existing mapping.");
			break;
	}

	GuiText msgTxt(msg, 26, (GXColor){0, 0, 0, 255});
	msgTxt.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	msgTxt.SetPosition(0,-20);
	msgTxt.SetWrap(true, 430);

	promptWindow.Append(&dialogBoxImg);
	promptWindow.Append(&titleTxt);
	promptWindow.Append(&msgTxt);

	HaltGui();
	mainWindow->SetState(STATE_DISABLED);
	mainWindow->Append(&promptWindow);
	mainWindow->ChangeFocus(&promptWindow);
	ResumeGui();

	u32 pressed = 0;

	while(pressed == 0)
	{
		usleep(THREAD_SLEEP);

		if(mapMenuCtrl == CTRLR_GCPAD)
		{
			pressed = userInput[0].pad.btns_d;

			if(userInput[0].pad.substickX < -70 ||
					userInput[0].pad.substickX > 70 ||
					userInput[0].pad.substickY < -70 ||
					userInput[0].pad.substickY > 70)
				pressed = WPAD_BUTTON_HOME;

			if(userInput[0].wpad->btns_d == WPAD_BUTTON_HOME)
				pressed = WPAD_BUTTON_HOME;
		}
		else
		{
			pressed = userInput[0].wpad->btns_d;

			// always allow Home button to be pressed to clear the existing mapping
			if(pressed != WPAD_BUTTON_HOME)
			{
				switch(mapMenuCtrl)
				{
					case CTRLR_WIIMOTE:
						if(pressed > 0x1000)
							pressed = 0; // not a valid input
						break;

					case CTRLR_CLASSIC:
						if(userInput[0].wpad->exp.type != WPAD_EXP_CLASSIC)
							pressed = 0; // not a valid input
						else if(pressed <= 0x1000)
							pressed = 0; // not a valid input
						break;

					case CTRLR_NUNCHUK:
						if(userInput[0].wpad->exp.type != WPAD_EXP_NUNCHUK)
							pressed = 0; // not a valid input
						break;
				}
			}
			if(pressed == 0)
			pressed = userInput[0].wupcdata.btns_d;
		}
	}

	if(pressed == WPAD_BUTTON_HOME
		|| pressed == WPAD_CLASSIC_BUTTON_HOME)
		pressed = 0;

	HaltGui();
	mainWindow->Remove(&promptWindow);
	mainWindow->SetState(STATE_DEFAULT);
	ResumeGui();

	return pressed;
}

static int MenuSettingsMappingsMap()
{
	int menu = MENU_NONE;
	int ret,i,j;
	bool firstRun = true;
	OptionList options;

	char menuTitle[100];
	char menuSubtitle[100];
	sprintf(menuTitle, "Game Settings - Button Mappings");

	GuiText titleTxt(menuTitle, 26, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,30);

	if(wiiuproCtrl == 1)
	{
		sprintf(menuSubtitle, "%s - %s", gettext(ctrlName[mapMenuCtrlSNES]),"Wii U Pro Controller");
	}
	else
	{
		sprintf(menuSubtitle, "%s - %s", gettext(ctrlName[mapMenuCtrlSNES]), gettext(ctrlrName[mapMenuCtrl]));
	}
		
	GuiText subtitleTxt(menuSubtitle, 20, (GXColor){255, 255, 255, 255});
	subtitleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	subtitleTxt.SetPosition(50,60);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);
	GuiImageData btnShortOutline(button_short_png);
	GuiImageData btnShortOutlineOver(button_short_over_png);

	GuiText backBtnTxt("Go Back", 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetSoundClick(&btnSoundClick);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trig2);
	backBtn.SetEffectGrow();

	GuiText resetBtnTxt("Reset", 22, (GXColor){0, 0, 0, 255});
	GuiImage resetBtnImg(&btnShortOutline);
	GuiImage resetBtnImgOver(&btnShortOutlineOver);
	GuiButton resetBtn(btnShortOutline.GetWidth(), btnShortOutline.GetHeight());
	resetBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	resetBtn.SetPosition(260, -35);
	resetBtn.SetLabel(&resetBtnTxt);
	resetBtn.SetImage(&resetBtnImg);
	resetBtn.SetImageOver(&resetBtnImgOver);
	resetBtn.SetSoundOver(&btnSoundOver);
	resetBtn.SetSoundClick(&btnSoundClick);
	resetBtn.SetTrigger(trigA);
	resetBtn.SetTrigger(trig2);
	resetBtn.SetEffectGrow();

	i=0;

	switch(mapMenuCtrlSNES)
	{
		case CTRL_PAD:
			sprintf(options.name[i++], "A");
			sprintf(options.name[i++], "B");
			sprintf(options.name[i++], "X");
			sprintf(options.name[i++], "Y");
			sprintf(options.name[i++], "L");
			sprintf(options.name[i++], "R");
			sprintf(options.name[i++], "Start");
			sprintf(options.name[i++], "Select");
			sprintf(options.name[i++], "Up");
			sprintf(options.name[i++], "Down");
			sprintf(options.name[i++], "Left");
			sprintf(options.name[i++], "Right");
			options.length = i;
			break;
		case CTRL_SCOPE:
			sprintf(options.name[i++], "Fire");
			sprintf(options.name[i++], "Aim Offscreen");
			sprintf(options.name[i++], "Cursor");
			sprintf(options.name[i++], "Turbo On");
			sprintf(options.name[i++], "Turbo Off");
			sprintf(options.name[i++], "Pause");
			options.length = i;
			break;
		case CTRL_MOUSE:
			sprintf(options.name[i++], "Left Button");
			sprintf(options.name[i++], "Right Button");
			options.length = i;
			break;
		case CTRL_JUST:
			sprintf(options.name[i++], "Fire");
			sprintf(options.name[i++], "Aim Offscreen");
			sprintf(options.name[i++], "Start");
			options.length = i;
			break;
	};

	for(i=0; i < options.length; i++)
		options.value[i][0] = 0;

	GuiOptionBrowser optionBrowser(552, 248, &options);
	optionBrowser.SetPosition(0, 108);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	optionBrowser.SetCol2Position(215);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	w.Append(&resetBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	mainWindow->Append(&subtitleTxt);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAMESETTINGS_MAPPINGS_CTRL;
		}
		else if(resetBtn.GetState() == STATE_CLICKED)
		{
			resetBtn.ResetState();

			int choice = WindowPrompt(
				"Reset Mappings",
				"Are you sure that you want to reset your mappings?",
				"Yes",
				"No");

			if(choice == 1)
			{
				ResetControls(mapMenuCtrlSNES, mapMenuCtrl);
				firstRun = true;
			}
		}

		ret = optionBrowser.GetClickedOption();

		if(ret >= 0)
		{
			// get a button selection from user
			btnmap[mapMenuCtrlSNES][mapMenuCtrl][ret] = ButtonMappingWindow();
		}

		if(ret >= 0 || firstRun)
		{
			firstRun = false;

			for(i=0; i < options.length; i++)
			{
				for(j=0; j < ctrlr_def[mapMenuCtrl].num_btns; j++)
				{
					if(btnmap[mapMenuCtrlSNES][mapMenuCtrl][i] == 0)
					{
						options.value[i][0] = 0;
					}
					else if(btnmap[mapMenuCtrlSNES][mapMenuCtrl][i] ==
						ctrlr_def[mapMenuCtrl].map[j].btn)
					{
						if(strcmp(options.value[i], ctrlr_def[mapMenuCtrl].map[j].name) != 0)
							sprintf(options.value[i], ctrlr_def[mapMenuCtrl].map[j].name);
						break;
					}
				}
			}
			optionBrowser.TriggerUpdate();
		}
	}
	wiiuproCtrl = 0;
	HaltGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	mainWindow->Remove(&subtitleTxt);
	return menu;
}

/****************************************************************************
 * MenuSettingsVideo
 ***************************************************************************/

static void ScreenZoomWindowUpdate(void * ptr, float h, float v)
{
	GuiButton * b = (GuiButton *)ptr;
	if(b->GetState() == STATE_CLICKED)
	{
		GCSettings.zoomHor += h;
		GCSettings.zoomVert += v;

		char zoom[10];
		sprintf(zoom, "%.2f%%", GCSettings.zoomHor*100);
		settingText->SetText(zoom);
		sprintf(zoom, "%.2f%%", GCSettings.zoomVert*100);
		settingText2->SetText(zoom);
		b->ResetState();
	}
}

static void ScreenZoomWindowLeftClick(void * ptr) { ScreenZoomWindowUpdate(ptr, -0.01, 0); }
static void ScreenZoomWindowRightClick(void * ptr) { ScreenZoomWindowUpdate(ptr, +0.01, 0); }
static void ScreenZoomWindowUpClick(void * ptr) { ScreenZoomWindowUpdate(ptr, 0, +0.01); }
static void ScreenZoomWindowDownClick(void * ptr) { ScreenZoomWindowUpdate(ptr, 0, -0.01); }

static void ScreenZoomWindow()
{
	GuiWindow * w = new GuiWindow(200,200);
	w->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	GuiTrigger trigLeft;
	trigLeft.SetButtonOnlyInFocusTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);

	GuiTrigger trigRight;
	trigRight.SetButtonOnlyInFocusTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);

	GuiTrigger trigUp;
	trigUp.SetButtonOnlyInFocusTrigger(-1, WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_UP, PAD_BUTTON_UP);

	GuiTrigger trigDown;
	trigDown.SetButtonOnlyInFocusTrigger(-1, WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_DOWN, PAD_BUTTON_DOWN);

	GuiImageData arrowLeft(button_arrow_left_png);
	GuiImage arrowLeftImg(&arrowLeft);
	GuiImageData arrowLeftOver(button_arrow_left_over_png);
	GuiImage arrowLeftOverImg(&arrowLeftOver);
	GuiButton arrowLeftBtn(arrowLeft.GetWidth(), arrowLeft.GetHeight());
	arrowLeftBtn.SetImage(&arrowLeftImg);
	arrowLeftBtn.SetImageOver(&arrowLeftOverImg);
	arrowLeftBtn.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	arrowLeftBtn.SetPosition(50, 0);
	arrowLeftBtn.SetTrigger(trigA);
	arrowLeftBtn.SetTrigger(trig2);
	arrowLeftBtn.SetTrigger(&trigLeft);
	arrowLeftBtn.SetSelectable(false);
	arrowLeftBtn.SetUpdateCallback(ScreenZoomWindowLeftClick);

	GuiImageData arrowRight(button_arrow_right_png);
	GuiImage arrowRightImg(&arrowRight);
	GuiImageData arrowRightOver(button_arrow_right_over_png);
	GuiImage arrowRightOverImg(&arrowRightOver);
	GuiButton arrowRightBtn(arrowRight.GetWidth(), arrowRight.GetHeight());
	arrowRightBtn.SetImage(&arrowRightImg);
	arrowRightBtn.SetImageOver(&arrowRightOverImg);
	arrowRightBtn.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	arrowRightBtn.SetPosition(164, 0);
	arrowRightBtn.SetTrigger(trigA);
	arrowRightBtn.SetTrigger(trig2);
	arrowRightBtn.SetTrigger(&trigRight);
	arrowRightBtn.SetSelectable(false);
	arrowRightBtn.SetUpdateCallback(ScreenZoomWindowRightClick);

	GuiImageData arrowUp(button_arrow_up_png);
	GuiImage arrowUpImg(&arrowUp);
	GuiImageData arrowUpOver(button_arrow_up_over_png);
	GuiImage arrowUpOverImg(&arrowUpOver);
	GuiButton arrowUpBtn(arrowUp.GetWidth(), arrowUp.GetHeight());
	arrowUpBtn.SetImage(&arrowUpImg);
	arrowUpBtn.SetImageOver(&arrowUpOverImg);
	arrowUpBtn.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	arrowUpBtn.SetPosition(-76, -27);
	arrowUpBtn.SetTrigger(trigA);
	arrowUpBtn.SetTrigger(trig2);
	arrowUpBtn.SetTrigger(&trigUp);
	arrowUpBtn.SetSelectable(false);
	arrowUpBtn.SetUpdateCallback(ScreenZoomWindowUpClick);

	GuiImageData arrowDown(button_arrow_down_png);
	GuiImage arrowDownImg(&arrowDown);
	GuiImageData arrowDownOver(button_arrow_down_over_png);
	GuiImage arrowDownOverImg(&arrowDownOver);
	GuiButton arrowDownBtn(arrowDown.GetWidth(), arrowDown.GetHeight());
	arrowDownBtn.SetImage(&arrowDownImg);
	arrowDownBtn.SetImageOver(&arrowDownOverImg);
	arrowDownBtn.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	arrowDownBtn.SetPosition(-76, 27);
	arrowDownBtn.SetTrigger(trigA);
	arrowDownBtn.SetTrigger(trig2);
	arrowDownBtn.SetTrigger(&trigDown);
	arrowDownBtn.SetSelectable(false);
	arrowDownBtn.SetUpdateCallback(ScreenZoomWindowDownClick);

	GuiImageData screenPosition(screen_position_png);
	GuiImage screenPositionImg(&screenPosition);
	screenPositionImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	screenPositionImg.SetPosition(0, 0);

	settingText = new GuiText(NULL, 20, (GXColor){0, 0, 0, 255});
	settingText2 = new GuiText(NULL, 20, (GXColor){0, 0, 0, 255});
	char zoom[10];
	sprintf(zoom, "%.2f%%", GCSettings.zoomHor*100);
	settingText->SetText(zoom);
	settingText->SetPosition(108, 0);
	sprintf(zoom, "%.2f%%", GCSettings.zoomVert*100);
	settingText2->SetText(zoom);
	settingText2->SetPosition(-76, 0);

	float currentZoomHor = GCSettings.zoomHor;
	float currentZoomVert = GCSettings.zoomVert;

	w->Append(&arrowLeftBtn);
	w->Append(&arrowRightBtn);
	w->Append(&arrowUpBtn);
	w->Append(&arrowDownBtn);
	w->Append(&screenPositionImg);
	w->Append(settingText);
	w->Append(settingText2);

	if(!SettingWindow("Screen Zoom",w))
	{
		// undo changes
		GCSettings.zoomHor = currentZoomHor;
		GCSettings.zoomVert = currentZoomVert;
	}

	delete(w);
	delete(settingText);
	delete(settingText2);
}

static void ScreenPositionWindowUpdate(void * ptr, int x, int y)
{
	GuiButton * b = (GuiButton *)ptr;
	if(b->GetState() == STATE_CLICKED)
	{
		GCSettings.xshift += x;
		GCSettings.yshift += y;

		char shift[10];
		sprintf(shift, "%i, %i", GCSettings.xshift, GCSettings.yshift);
		settingText->SetText(shift);
		b->ResetState();
	}
}

static void ScreenPositionWindowLeftClick(void * ptr) { ScreenPositionWindowUpdate(ptr, -1, 0); }
static void ScreenPositionWindowRightClick(void * ptr) { ScreenPositionWindowUpdate(ptr, +1, 0); }
static void ScreenPositionWindowUpClick(void * ptr) { ScreenPositionWindowUpdate(ptr, 0, -1); }
static void ScreenPositionWindowDownClick(void * ptr) { ScreenPositionWindowUpdate(ptr, 0, +1); }

static void ScreenPositionWindow()
{
	GuiWindow * w = new GuiWindow(150,150);
	w->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	w->SetPosition(0, -10);

	GuiTrigger trigLeft;
	trigLeft.SetButtonOnlyInFocusTrigger(-1, WPAD_BUTTON_LEFT | WPAD_CLASSIC_BUTTON_LEFT, PAD_BUTTON_LEFT);

	GuiTrigger trigRight;
	trigRight.SetButtonOnlyInFocusTrigger(-1, WPAD_BUTTON_RIGHT | WPAD_CLASSIC_BUTTON_RIGHT, PAD_BUTTON_RIGHT);

	GuiTrigger trigUp;
	trigUp.SetButtonOnlyInFocusTrigger(-1, WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_UP, PAD_BUTTON_UP);

	GuiTrigger trigDown;
	trigDown.SetButtonOnlyInFocusTrigger(-1, WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_DOWN, PAD_BUTTON_DOWN);

	GuiImageData arrowLeft(button_arrow_left_png);
	GuiImage arrowLeftImg(&arrowLeft);
	GuiImageData arrowLeftOver(button_arrow_left_over_png);
	GuiImage arrowLeftOverImg(&arrowLeftOver);
	GuiButton arrowLeftBtn(arrowLeft.GetWidth(), arrowLeft.GetHeight());
	arrowLeftBtn.SetImage(&arrowLeftImg);
	arrowLeftBtn.SetImageOver(&arrowLeftOverImg);
	arrowLeftBtn.SetAlignment(ALIGN_LEFT, ALIGN_MIDDLE);
	arrowLeftBtn.SetTrigger(trigA);
	arrowLeftBtn.SetTrigger(trig2);
	arrowLeftBtn.SetTrigger(&trigLeft);
	arrowLeftBtn.SetSelectable(false);
	arrowLeftBtn.SetUpdateCallback(ScreenPositionWindowLeftClick);

	GuiImageData arrowRight(button_arrow_right_png);
	GuiImage arrowRightImg(&arrowRight);
	GuiImageData arrowRightOver(button_arrow_right_over_png);
	GuiImage arrowRightOverImg(&arrowRightOver);
	GuiButton arrowRightBtn(arrowRight.GetWidth(), arrowRight.GetHeight());
	arrowRightBtn.SetImage(&arrowRightImg);
	arrowRightBtn.SetImageOver(&arrowRightOverImg);
	arrowRightBtn.SetAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
	arrowRightBtn.SetTrigger(trigA);
	arrowRightBtn.SetTrigger(trig2);
	arrowRightBtn.SetTrigger(&trigRight);
	arrowRightBtn.SetSelectable(false);
	arrowRightBtn.SetUpdateCallback(ScreenPositionWindowRightClick);

	GuiImageData arrowUp(button_arrow_up_png);
	GuiImage arrowUpImg(&arrowUp);
	GuiImageData arrowUpOver(button_arrow_up_over_png);
	GuiImage arrowUpOverImg(&arrowUpOver);
	GuiButton arrowUpBtn(arrowUp.GetWidth(), arrowUp.GetHeight());
	arrowUpBtn.SetImage(&arrowUpImg);
	arrowUpBtn.SetImageOver(&arrowUpOverImg);
	arrowUpBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	arrowUpBtn.SetTrigger(trigA);
	arrowUpBtn.SetTrigger(trig2);
	arrowUpBtn.SetTrigger(&trigUp);
	arrowUpBtn.SetSelectable(false);
	arrowUpBtn.SetUpdateCallback(ScreenPositionWindowUpClick);

	GuiImageData arrowDown(button_arrow_down_png);
	GuiImage arrowDownImg(&arrowDown);
	GuiImageData arrowDownOver(button_arrow_down_over_png);
	GuiImage arrowDownOverImg(&arrowDownOver);
	GuiButton arrowDownBtn(arrowDown.GetWidth(), arrowDown.GetHeight());
	arrowDownBtn.SetImage(&arrowDownImg);
	arrowDownBtn.SetImageOver(&arrowDownOverImg);
	arrowDownBtn.SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	arrowDownBtn.SetTrigger(trigA);
	arrowDownBtn.SetTrigger(trig2);
	arrowDownBtn.SetTrigger(&trigDown);
	arrowDownBtn.SetSelectable(false);
	arrowDownBtn.SetUpdateCallback(ScreenPositionWindowDownClick);

	GuiImageData screenPosition(screen_position_png);
	GuiImage screenPositionImg(&screenPosition);
	screenPositionImg.SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);

	settingText = new GuiText(NULL, 20, (GXColor){0, 0, 0, 255});
	char shift[10];
	sprintf(shift, "%i, %i", GCSettings.xshift, GCSettings.yshift);
	settingText->SetText(shift);

	int currentX = GCSettings.xshift;
	int currentY = GCSettings.yshift;

	w->Append(&arrowLeftBtn);
	w->Append(&arrowRightBtn);
	w->Append(&arrowUpBtn);
	w->Append(&arrowDownBtn);
	w->Append(&screenPositionImg);
	w->Append(settingText);

	if(!SettingWindow("Screen Position",w))
	{
		// undo changes
		GCSettings.xshift = currentX;
		GCSettings.yshift = currentY;
	}

	delete(w);
	delete(settingText);
}

static int MenuSettingsVideo()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;

	sprintf(options.name[i++], "Rendering");
	sprintf(options.name[i++], "Scaling");
	sprintf(options.name[i++], "Filtering");
	sprintf(options.name[i++], "Screen Zoom");
	sprintf(options.name[i++], "Screen Position");
	sprintf(options.name[i++], "Crosshair");
	sprintf(options.name[i++], "Video Mode");
	options.length = i;
	
#ifdef HW_DOL
	options.name[2][0] = 0; // disable hq2x on GameCube
#endif

	for(i=0; i < options.length; i++)
		options.value[i][0] = 0;

	GuiText titleTxt("Game Settings - Video", 26, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_png);
	GuiImageData btnOutlineOver(button_over_png);

	GuiText backBtnTxt("Go Back", 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(50, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetSoundClick(&btnSoundClick);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trig2);
	backBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(552, 248, &options);
	optionBrowser.SetPosition(0, 108);
	optionBrowser.SetCol2Position(200);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				GCSettings.render++;
				if (GCSettings.render > 2)
					GCSettings.render = 0;
				break;

			case 1:
				GCSettings.widescreen ^= 1;
				break;

			case 2:
				GCSettings.FilterMethod++;
				if (GCSettings.FilterMethod >= NUM_FILTERS)
					GCSettings.FilterMethod = 0;
				break;

			case 3:
				ScreenZoomWindow();
				break;

			case 4:
				ScreenPositionWindow();
				break;

			case 5:
				GCSettings.crosshair ^= 1;
				break;

			case 6:
				GCSettings.videomode++;
				if(GCSettings.videomode > 4)
					GCSettings.videomode = 0;
				break;
		}

		if(ret >= 0 || firstRun)
		{
			firstRun = false;

			// don't allow original render mode if progressive video mode detected
			if (GCSettings.render==0 && progressive)
				GCSettings.render++;

			if (GCSettings.render == 0)
				sprintf (options.value[0], "Original");
			else if (GCSettings.render == 1)
				sprintf (options.value[0], "Filtered");
			else if (GCSettings.render == 2)
				sprintf (options.value[0], "Unfiltered");

			if(GCSettings.widescreen)
				sprintf (options.value[1], "16:9 Correction");
			else
				sprintf (options.value[1], "Default");
#ifdef HW_RVL
			sprintf (options.value[2], "%s", GetFilterName((RenderFilter)GCSettings.FilterMethod));
#endif
			sprintf (options.value[3], "%.2f%%, %.2f%%", GCSettings.zoomHor*100, GCSettings.zoomVert*100);
			sprintf (options.value[4], "%d, %d", GCSettings.xshift, GCSettings.yshift);
			sprintf (options.value[5], "%s", GCSettings.crosshair == 1 ? "On" : "Off");

			switch(GCSettings.videomode)
			{
				case 0:
					sprintf (options.value[6], "Automatic (Recommended)"); break;
				case 1:
					sprintf (options.value[6], "NTSC (480i)"); break;
				case 2:
					sprintf (options.value[6], "Progressive (480p)"); break;
				case 3:
					sprintf (options.value[6], "PAL (50Hz)"); break;
				case 4:
					sprintf (options.value[6], "PAL (60Hz)"); break;
			}
			optionBrowser.TriggerUpdate();
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAMESETTINGS;
		}
	}
	HaltGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	return menu;
}

/****************************************************************************
 * MenuSettings
 ***************************************************************************/
static int MenuSettings()
{
	int menu = MENU_NONE;

	GuiText titleTxt("Settings", 26, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_long_png);
	GuiImageData btnOutlineOver(button_long_over_png);
	GuiImageData btnLargeOutline(button_large_png);
	GuiImageData btnLargeOutlineOver(button_large_over_png);
	GuiImageData iconFile(icon_settings_file_png);
	GuiImageData iconMenu(icon_settings_menu_png);
	GuiImageData iconNetwork(icon_settings_network_png);

	GuiText savingBtnTxt1("Saving", 22, (GXColor){0, 0, 0, 255});
	GuiText savingBtnTxt2("&", 18, (GXColor){0, 0, 0, 255});
	GuiText savingBtnTxt3("Loading", 22, (GXColor){0, 0, 0, 255});
	savingBtnTxt1.SetPosition(0, -20);
	savingBtnTxt3.SetPosition(0, +20);
	GuiImage savingBtnImg(&btnLargeOutline);
	GuiImage savingBtnImgOver(&btnLargeOutlineOver);
	GuiImage fileBtnIcon(&iconFile);
	GuiButton savingBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	savingBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	savingBtn.SetPosition(-125, 120);
	savingBtn.SetLabel(&savingBtnTxt1, 0);
	savingBtn.SetLabel(&savingBtnTxt2, 1);
	savingBtn.SetLabel(&savingBtnTxt3, 2);
	savingBtn.SetImage(&savingBtnImg);
	savingBtn.SetImageOver(&savingBtnImgOver);
	savingBtn.SetIcon(&fileBtnIcon);
	savingBtn.SetSoundOver(&btnSoundOver);
	savingBtn.SetSoundClick(&btnSoundClick);
	savingBtn.SetTrigger(trigA);
	savingBtn.SetTrigger(trig2);
	savingBtn.SetEffectGrow();

	GuiText menuBtnTxt("Menu", 22, (GXColor){0, 0, 0, 255});
	menuBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-20);
	GuiImage menuBtnImg(&btnLargeOutline);
	GuiImage menuBtnImgOver(&btnLargeOutlineOver);
	GuiImage menuBtnIcon(&iconMenu);
	GuiButton menuBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	menuBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	menuBtn.SetPosition(125, 120);
	menuBtn.SetLabel(&menuBtnTxt);
	menuBtn.SetImage(&menuBtnImg);
	menuBtn.SetImageOver(&menuBtnImgOver);
	menuBtn.SetIcon(&menuBtnIcon);
	menuBtn.SetSoundOver(&btnSoundOver);
	menuBtn.SetSoundClick(&btnSoundClick);
	menuBtn.SetTrigger(trigA);
	menuBtn.SetTrigger(trig2);
	menuBtn.SetEffectGrow();

	GuiText networkBtnTxt("Network", 22, (GXColor){0, 0, 0, 255});
	networkBtnTxt.SetWrap(true, btnLargeOutline.GetWidth()-20);
	GuiImage networkBtnImg(&btnLargeOutline);
	GuiImage networkBtnImgOver(&btnLargeOutlineOver);
	GuiImage networkBtnIcon(&iconNetwork);
	GuiButton networkBtn(btnLargeOutline.GetWidth(), btnLargeOutline.GetHeight());
	networkBtn.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	networkBtn.SetPosition(0, 250);
	networkBtn.SetLabel(&networkBtnTxt);
	networkBtn.SetImage(&networkBtnImg);
	networkBtn.SetImageOver(&networkBtnImgOver);
	networkBtn.SetIcon(&networkBtnIcon);
	networkBtn.SetSoundOver(&btnSoundOver);
	networkBtn.SetSoundClick(&btnSoundClick);
	networkBtn.SetTrigger(trigA);
	networkBtn.SetTrigger(trig2);
	networkBtn.SetEffectGrow();

	GuiText backBtnTxt("Go Back", 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(90, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetSoundClick(&btnSoundClick);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trig2);
	backBtn.SetEffectGrow();

	GuiText resetBtnTxt("Reset Settings", 22, (GXColor){0, 0, 0, 255});
	GuiImage resetBtnImg(&btnOutline);
	GuiImage resetBtnImgOver(&btnOutlineOver);
	GuiButton resetBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	resetBtn.SetAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
	resetBtn.SetPosition(-90, -35);
	resetBtn.SetLabel(&resetBtnTxt);
	resetBtn.SetImage(&resetBtnImg);
	resetBtn.SetImageOver(&resetBtnImgOver);
	resetBtn.SetSoundOver(&btnSoundOver);
	resetBtn.SetSoundClick(&btnSoundClick);
	resetBtn.SetTrigger(trigA);
	resetBtn.SetTrigger(trig2);
	resetBtn.SetEffectGrow();

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&titleTxt);
	w.Append(&savingBtn);
	w.Append(&menuBtn);
	w.Append(&networkBtn);
	w.Append(&backBtn);
	w.Append(&resetBtn);

	mainWindow->Append(&w);

	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		if(savingBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS_FILE;
		}
		else if(menuBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS_MENU;
		}
		else if(networkBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS_NETWORK;
		}
		else if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_GAMESELECTION;
		}
		else if(resetBtn.GetState() == STATE_CLICKED)
		{
			resetBtn.ResetState();

			int choice = WindowPrompt(
				"Reset Settings",
				"Are you sure that you want to reset your settings?",
				"Yes",
				"No");

			if(choice == 1)
				DefaultSettings();
		}
	}

	HaltGui();
	mainWindow->Remove(&w);
	return menu;
}

/****************************************************************************
 * MenuSettingsFile
 ***************************************************************************/

static int MenuSettingsFile()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;
	sprintf(options.name[i++], "Load Device");
	sprintf(options.name[i++], "Save Device");
	sprintf(options.name[i++], "Load Folder");
	sprintf(options.name[i++], "Save Folder");
	sprintf(options.name[i++], "Cheats Folder");
	sprintf(options.name[i++], "Screenshots Folder");
	sprintf(options.name[i++], "Auto Load");
	sprintf(options.name[i++], "Auto Save");
	options.length = i;

	for(i=0; i < options.length; i++)
		options.value[i][0] = 0;

	GuiText titleTxt("Settings - Saving & Loading", 26, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_long_png);
	GuiImageData btnOutlineOver(button_long_over_png);

	GuiText backBtnTxt("Go Back", 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(90, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetSoundClick(&btnSoundClick);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trig2);
	backBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(552, 248, &options);
	optionBrowser.SetPosition(0, 108);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	optionBrowser.SetCol2Position(215);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				GCSettings.LoadMethod++;
				break;

			case 1:
				GCSettings.SaveMethod++;
				break;

			case 2:
				OnScreenKeyboard(GCSettings.LoadFolder, MAXPATHLEN);
				break;

			case 3:
				OnScreenKeyboard(GCSettings.SaveFolder, MAXPATHLEN);
				break;

			case 4:
				OnScreenKeyboard(GCSettings.CheatFolder, MAXPATHLEN);
				break;
				
			case 5:
				OnScreenKeyboard(GCSettings.ScreenshotsFolder, MAXPATHLEN);
				break;

			case 6:
				GCSettings.AutoLoad++;
				if (GCSettings.AutoLoad > 2)
					GCSettings.AutoLoad = 0;
				break;

			case 7:
				GCSettings.AutoSave++;
				if (GCSettings.AutoSave > 3)
					GCSettings.AutoSave = 0;
				break;
		}

		if(ret >= 0 || firstRun)
		{
			firstRun = false;

			// some load/save methods are not implemented - here's where we skip them
			// they need to be skipped in the order they were enumerated

			// no SD/USB ports on GameCube
			#ifdef HW_DOL
			if(GCSettings.LoadMethod == DEVICE_SD)
				GCSettings.LoadMethod++;
			if(GCSettings.SaveMethod == DEVICE_SD)
				GCSettings.SaveMethod++;
			if(GCSettings.LoadMethod == DEVICE_USB)
				GCSettings.LoadMethod++;
			if(GCSettings.SaveMethod == DEVICE_USB)
				GCSettings.SaveMethod++;
			#endif

			// saving to DVD is impossible
			if(GCSettings.SaveMethod == DEVICE_DVD)
				GCSettings.SaveMethod++;

			// don't allow SD Gecko on Wii
			#ifdef HW_RVL
			if(GCSettings.LoadMethod == DEVICE_SD_SLOTA)
				GCSettings.LoadMethod++;
			if(GCSettings.SaveMethod == DEVICE_SD_SLOTA)
				GCSettings.SaveMethod++;
			if(GCSettings.LoadMethod == DEVICE_SD_SLOTB)
				GCSettings.LoadMethod++;
			if(GCSettings.SaveMethod == DEVICE_SD_SLOTB)
				GCSettings.SaveMethod++;
			#endif

			// correct load/save methods out of bounds
			if(GCSettings.LoadMethod > 6)
				GCSettings.LoadMethod = 0;
			if(GCSettings.SaveMethod > 6)
				GCSettings.SaveMethod = 0;

			if (GCSettings.LoadMethod == DEVICE_AUTO) sprintf (options.value[0],"Auto Detect");
			else if (GCSettings.LoadMethod == DEVICE_SD) sprintf (options.value[0],"SD");
			else if (GCSettings.LoadMethod == DEVICE_USB) sprintf (options.value[0],"USB");
			else if (GCSettings.LoadMethod == DEVICE_DVD) sprintf (options.value[0],"DVD");
			else if (GCSettings.LoadMethod == DEVICE_SMB) sprintf (options.value[0],"Network");
			else if (GCSettings.LoadMethod == DEVICE_SD_SLOTA) sprintf (options.value[0],"SD Gecko Slot A");
			else if (GCSettings.LoadMethod == DEVICE_SD_SLOTB) sprintf (options.value[0],"SD Gecko Slot B");

			if (GCSettings.SaveMethod == DEVICE_AUTO) sprintf (options.value[1],"Auto Detect");
			else if (GCSettings.SaveMethod == DEVICE_SD) sprintf (options.value[1],"SD");
			else if (GCSettings.SaveMethod == DEVICE_USB) sprintf (options.value[1],"USB");
			else if (GCSettings.SaveMethod == DEVICE_SMB) sprintf (options.value[1],"Network");
			else if (GCSettings.SaveMethod == DEVICE_SD_SLOTA) sprintf (options.value[1],"SD Gecko Slot A");
			else if (GCSettings.SaveMethod == DEVICE_SD_SLOTB) sprintf (options.value[1],"SD Gecko Slot B");

			snprintf (options.value[2], 35, "%s", GCSettings.LoadFolder);
			snprintf (options.value[3], 35, "%s", GCSettings.SaveFolder);
			snprintf (options.value[4], 35, "%s", GCSettings.CheatFolder);
			snprintf (options.value[5], 35, "%s", GCSettings.ScreenshotsFolder);

			if (GCSettings.AutoLoad == 0) sprintf (options.value[6],"Off");
			else if (GCSettings.AutoLoad == 1) sprintf (options.value[6],"SRAM");
			else if (GCSettings.AutoLoad == 2) sprintf (options.value[6],"Snapshot");

			if (GCSettings.AutoSave == 0) sprintf (options.value[7],"Off");
			else if (GCSettings.AutoSave == 1) sprintf (options.value[7],"SRAM");
			else if (GCSettings.AutoSave == 2) sprintf (options.value[7],"Snapshot");
			else if (GCSettings.AutoSave == 3) sprintf (options.value[7],"Both");

			optionBrowser.TriggerUpdate();
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS;
		}
	}
	HaltGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	return menu;
}

/****************************************************************************
 * MenuSettingsMenu
 ***************************************************************************/

static int MenuSettingsMenu()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;

	sprintf(options.name[i++], "Exit Action");
	sprintf(options.name[i++], "Wiimote Orientation");
	sprintf(options.name[i++], "Music Volume");
	sprintf(options.name[i++], "Sound Effects Volume");
	sprintf(options.name[i++], "Rumble");
	sprintf(options.name[i++], "Language");
	options.length = i;

	for(i=0; i < options.length; i++)
		options.value[i][0] = 0;

	GuiText titleTxt("Settings - Menu", 26, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_long_png);
	GuiImageData btnOutlineOver(button_long_over_png);

	GuiText backBtnTxt("Go Back", 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(90, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetSoundClick(&btnSoundClick);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trig2);
	backBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(552, 248, &options);
	optionBrowser.SetPosition(0, 108);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	optionBrowser.SetCol2Position(275);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				GCSettings.ExitAction++;
				if(GCSettings.ExitAction > 3)
					GCSettings.ExitAction = 0;
				break;
			case 1:
				GCSettings.WiimoteOrientation ^= 1;
				break;
			case 2:
				GCSettings.MusicVolume += 10;
				if(GCSettings.MusicVolume > 100)
					GCSettings.MusicVolume = 0;
				bgMusic->SetVolume(GCSettings.MusicVolume);
				break;
			case 3:
				GCSettings.SFXVolume += 10;
				if(GCSettings.SFXVolume > 100)
					GCSettings.SFXVolume = 0;
				break;
			case 4:
				GCSettings.Rumble ^= 1;
				break;
			case 5:
				GCSettings.language++;
				
				if(GCSettings.language >= LANG_LENGTH)
					GCSettings.language = LANG_JAPANESE;

				if(GCSettings.language == LANG_SIMP_CHINESE)
					GCSettings.language = LANG_PORTUGUESE;
				else if(GCSettings.language == LANG_JAPANESE)
					GCSettings.language = LANG_ENGLISH;
	
				break;
		}

		if(ret >= 0 || firstRun)
		{
			firstRun = false;

			#ifdef HW_RVL
			if (GCSettings.ExitAction == 1)
				sprintf (options.value[0], "Return to Wii Menu");
			else if (GCSettings.ExitAction == 2)
				sprintf (options.value[0], "Power off Wii");
			else if (GCSettings.ExitAction == 3)
				sprintf (options.value[0], "Return to Loader");
			else
				sprintf (options.value[0], "Auto");
			#else // GameCube
			if(GCSettings.ExitAction > 1)
				GCSettings.ExitAction = 0;
			if (GCSettings.ExitAction == 0)
				sprintf (options.value[0], "Return to Loader");
			else
				sprintf (options.value[0], "Reboot");

			options.name[1][0] = 0; // Wiimote
			options.name[2][0] = 0; // Music
			options.name[3][0] = 0; // Sound Effects
			options.name[4][0] = 0; // Rumble
			#endif

			if (GCSettings.WiimoteOrientation == 0)
				sprintf (options.value[1], "Vertical");
			else if (GCSettings.WiimoteOrientation == 1)
				sprintf (options.value[1], "Horizontal");

			if(GCSettings.MusicVolume > 0)
				sprintf(options.value[2], "%d%%", GCSettings.MusicVolume);
			else
				sprintf(options.value[2], "Mute");

			if(GCSettings.SFXVolume > 0)
				sprintf(options.value[3], "%d%%", GCSettings.SFXVolume);
			else
				sprintf(options.value[3], "Mute");

			if (GCSettings.Rumble == 1)
				sprintf (options.value[4], "Enabled");
			else
				sprintf (options.value[4], "Disabled");

			switch(GCSettings.language)
			{
				case LANG_JAPANESE:		sprintf(options.value[5], "Japanese"); break;
				case LANG_ENGLISH:		sprintf(options.value[5], "English"); break;
				case LANG_GERMAN:		sprintf(options.value[5], "German"); break;
				case LANG_FRENCH:		sprintf(options.value[5], "French"); break;
				case LANG_SPANISH:		sprintf(options.value[5], "Spanish"); break;
				case LANG_ITALIAN:		sprintf(options.value[5], "Italian"); break;
				case LANG_DUTCH:		sprintf(options.value[5], "Dutch"); break;
				case LANG_SIMP_CHINESE:	sprintf(options.value[5], "Chinese (Simplified)"); break;
				case LANG_TRAD_CHINESE:	sprintf(options.value[5], "Chinese (Traditional)"); break;
				case LANG_KOREAN:		sprintf(options.value[5], "Korean"); break;
				case LANG_PORTUGUESE:	sprintf(options.value[5], "Portuguese"); break;
				case LANG_BRAZILIAN_PORTUGUESE: sprintf(options.value[5], "Brazilian Portuguese"); break;
				case LANG_CATALAN:		sprintf(options.value[5], "Catalan"); break;
				case LANG_TURKISH:		sprintf(options.value[5], "Turkish"); break;
			}
			
			optionBrowser.TriggerUpdate();
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS;
		}
	}
	HaltGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	ResetText();
	return menu;
}

/****************************************************************************
 * MenuSettingsNetwork
 ***************************************************************************/

static int MenuSettingsNetwork()
{
	int menu = MENU_NONE;
	int ret;
	int i = 0;
	bool firstRun = true;
	OptionList options;
	sprintf(options.name[i++], "SMB Share IP");
	sprintf(options.name[i++], "SMB Share Name");
	sprintf(options.name[i++], "SMB Share Username");
	sprintf(options.name[i++], "SMB Share Password");
	options.length = i;

	for(i=0; i < options.length; i++)
		options.value[i][0] = 0;

	GuiText titleTxt("Settings - Network", 26, (GXColor){255, 255, 255, 255});
	titleTxt.SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	titleTxt.SetPosition(50,50);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData btnOutline(button_long_png);
	GuiImageData btnOutlineOver(button_long_over_png);

	GuiText backBtnTxt("Go Back", 22, (GXColor){0, 0, 0, 255});
	GuiImage backBtnImg(&btnOutline);
	GuiImage backBtnImgOver(&btnOutlineOver);
	GuiButton backBtn(btnOutline.GetWidth(), btnOutline.GetHeight());
	backBtn.SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	backBtn.SetPosition(90, -35);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetImage(&backBtnImg);
	backBtn.SetImageOver(&backBtnImgOver);
	backBtn.SetSoundOver(&btnSoundOver);
	backBtn.SetSoundClick(&btnSoundClick);
	backBtn.SetTrigger(trigA);
	backBtn.SetTrigger(trig2);
	backBtn.SetEffectGrow();

	GuiOptionBrowser optionBrowser(552, 248, &options);
	optionBrowser.SetPosition(0, 108);
	optionBrowser.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	optionBrowser.SetCol2Position(290);

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&backBtn);
	mainWindow->Append(&optionBrowser);
	mainWindow->Append(&w);
	mainWindow->Append(&titleTxt);
	ResumeGui();

	while(menu == MENU_NONE)
	{
		usleep(THREAD_SLEEP);

		ret = optionBrowser.GetClickedOption();

		switch (ret)
		{
			case 0:
				OnScreenKeyboard(GCSettings.smbip, 80);
				break;

			case 1:
				OnScreenKeyboard(GCSettings.smbshare, 20);
				break;

			case 2:
				OnScreenKeyboard(GCSettings.smbuser, 20);
				break;

			case 3:
				OnScreenKeyboard(GCSettings.smbpwd, 20);
				break;
		}

		if(ret >= 0 || firstRun)
		{
			firstRun = false;
			snprintf (options.value[0], 25, "%s", GCSettings.smbip);
			snprintf (options.value[1], 19, "%s", GCSettings.smbshare);
			snprintf (options.value[2], 19, "%s", GCSettings.smbuser);
			snprintf (options.value[3], 19, "%s", GCSettings.smbpwd);
			optionBrowser.TriggerUpdate();
		}

		if(backBtn.GetState() == STATE_CLICKED)
		{
			menu = MENU_SETTINGS;
		}
	}
	HaltGui();
	mainWindow->Remove(&optionBrowser);
	mainWindow->Remove(&w);
	mainWindow->Remove(&titleTxt);
	CloseShare();
	return menu;
}

/****************************************************************************
 * MainMenu
 ***************************************************************************/

void
MainMenu (int menu)
{
	static bool init = false;
	int currentMenu = menu;
	lastMenu = MENU_NONE;
	
	if(!init)
	{
		init = true;
		#ifdef HW_RVL
		pointer[0] = new GuiImageData(player1_point_png);
		pointer[1] = new GuiImageData(player2_point_png);
		pointer[2] = new GuiImageData(player3_point_png);
		pointer[3] = new GuiImageData(player4_point_png);
		#endif

		trigA = new GuiTrigger;
		trigA->SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
		trig2 = new GuiTrigger;
		trig2->SetSimpleTrigger(-1, WPAD_BUTTON_2, 0);
	}

	mainWindow = new GuiWindow(screenwidth, screenheight);

	if(menu == MENU_GAME)
	{
		gameScreen = new GuiImageData(gameScreenPng);
		gameScreenImg = new GuiImage(gameScreen);
		gameScreenImg->SetAlpha(192);
		gameScreenImg->ColorStripe(30);
		gameScreenImg->SetScaleX(screenwidth/(float)vmode->fbWidth);
		gameScreenImg->SetScaleY(screenheight/(float)vmode->efbHeight);
	}
	else
	{
		gameScreenImg = new GuiImage(screenwidth, screenheight, (GXColor){175, 200, 215, 255});
		gameScreenImg->ColorStripe(10);
	}

	mainWindow->Append(gameScreenImg);

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM);
	GuiSound btnSoundClick(button_click_pcm, button_click_pcm_size, SOUND_PCM);
	GuiImageData bgTop(bg_top_png);
	bgTopImg = new GuiImage(&bgTop);
	GuiImageData bgBottom(bg_bottom_png);
	bgBottomImg = new GuiImage(&bgBottom);
	bgBottomImg->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	GuiImageData logo(logo_png);
	GuiImage logoImg(&logo);
	GuiImageData logoOver(logo_over_png);
	GuiImage logoImgOver(&logoOver);
	GuiText logoTxt(APPVERSION, 18, (GXColor){255, 255, 255, 255});
	logoTxt.SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	logoTxt.SetPosition(0, 4);
	btnLogo = new GuiButton(logoImg.GetWidth(), logoImg.GetHeight());
	btnLogo->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	btnLogo->SetPosition(-50, 24);
	btnLogo->SetImage(&logoImg);
	btnLogo->SetImageOver(&logoImgOver);
	btnLogo->SetLabel(&logoTxt);
	btnLogo->SetSoundOver(&btnSoundOver);
	btnLogo->SetSoundClick(&btnSoundClick);
	btnLogo->SetTrigger(trigA);
	btnLogo->SetTrigger(trig2);
	btnLogo->SetUpdateCallback(WindowCredits);

	mainWindow->Append(bgTopImg);
	mainWindow->Append(bgBottomImg);
	mainWindow->Append(btnLogo);

	if(currentMenu == MENU_GAMESELECTION)
		ResumeGui();

	// Load preferences
	if(!LoadPrefs())
		SavePrefs(SILENT);

#ifdef HW_RVL
	static bool checkIOS = true;

	if(checkIOS)
	{
		u32 ios = IOS_GetVersion();

		if(!SupportedIOS(ios))
			ErrorPrompt("The current IOS is unsupported. Functionality and/or stability may be adversely affected.");
		else if(!SaneIOS(ios))
			ErrorPrompt("The current IOS has been altered (fake-signed). Functionality and/or stability may be adversely affected.");
	}

	checkIOS = false;
#endif

	#ifndef NO_SOUND
	bgMusic = new GuiSound(bg_music_ogg, bg_music_ogg_size, SOUND_OGG);
	bgMusic->SetVolume(GCSettings.MusicVolume);
	bgMusic->SetLoop(true);
	enterSound = new GuiSound(enter_ogg, enter_ogg_size, SOUND_OGG);
	enterSound->SetVolume(GCSettings.SFXVolume);
	exitSound = new GuiSound(exit_ogg, exit_ogg_size, SOUND_OGG);
	exitSound->SetVolume(GCSettings.SFXVolume);
	if(currentMenu == MENU_GAMESELECTION) bgMusic->Play(); // startup music
	#endif

	while(currentMenu != MENU_EXIT || SNESROMSize <= 0)
	{
		switch (currentMenu)
		{
			case MENU_GAMESELECTION:
				currentMenu = MenuGameSelection();
				break;
			case MENU_GAME:
				currentMenu = MenuGame();
				break;
			case MENU_GAME_LOAD:
				currentMenu = MenuGameSaves(0);
				break;
			case MENU_GAME_SAVE:
				currentMenu = MenuGameSaves(1);
				break;
			case MENU_GAMESETTINGS:
				currentMenu = MenuGameSettings();
				break;
			case MENU_GAMESETTINGS_MAPPINGS:
				currentMenu = MenuSettingsMappings();
				break;
			case MENU_GAMESETTINGS_MAPPINGS_CTRL:
				currentMenu = MenuSettingsMappingsController();
				break;
			case MENU_GAMESETTINGS_MAPPINGS_MAP:
				currentMenu = MenuSettingsMappingsMap();
				break;
			case MENU_GAMESETTINGS_VIDEO:
				currentMenu = MenuSettingsVideo();
				break;
			case MENU_GAMESETTINGS_CHEATS:
				currentMenu = MenuGameCheats();
				break;
			case MENU_SETTINGS:
				currentMenu = MenuSettings();
				break;
			case MENU_SETTINGS_FILE:
				currentMenu = MenuSettingsFile();
				break;
			case MENU_SETTINGS_MENU:
				currentMenu = MenuSettingsMenu();
				break;
			case MENU_SETTINGS_NETWORK:
				currentMenu = MenuSettingsNetwork();
				break;
			default: // unrecognized menu
				currentMenu = MenuGameSelection();
				break;
		}
		lastMenu = currentMenu;
		usleep(THREAD_SLEEP);
	}

	#ifdef HW_RVL
	ShutoffRumble();
	#endif

	CancelAction();
	HaltGui();

	#ifdef HW_RVL
	if(updatethread != LWP_THREAD_NULL)
		LWP_JoinThread(updatethread, NULL);
	#endif

	#ifndef NO_SOUND
	delete bgMusic;
	delete enterSound;
	delete exitSound;
	#endif

	delete btnLogo;
	delete gameScreenImg;
	delete bgTopImg;
	delete bgBottomImg;
	delete mainWindow;

	mainWindow = NULL;

	if(gameScreen)
		delete gameScreen;

	if(gameScreenPng)
	{
		free(gameScreenPng);
		gameScreenPng = NULL;
	}

	// wait for keys to be depressed
	while(MenuRequested())
	{
		UpdatePads();
		usleep(THREAD_SLEEP);
	}
}
