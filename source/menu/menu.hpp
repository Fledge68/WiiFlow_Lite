#ifndef __MENU_HPP
#define __MENU_HPP
//#define SHOWMEM
//#define SHOWMEMGECKO

#include <ogc/pad.h>
#include <vector>
#include <map>

#include "btnmap.h"
#include "channel/banner.h"
#include "channel/channels.h"
#include "cheats/gct.h"
#include "devicemounter/DeviceHandler.hpp"
#include "gecko/gecko.h"
#include "gui/coverflow.hpp"
#include "gui/cursor.hpp"
#include "gui/fanart.hpp"
#include "gui/gui.hpp"
#include "list/cachedlist.hpp"
#include "loader/disc.h"
#include "loader/gc_disc_dump.hpp"
#include "loader/wbfs.h"
#include "music/gui_sound.h"
#include "music/musicplayer.h"
#include "plugin/plugin.hpp"
#include "wiiuse/wpad.h"

using namespace std;

extern "C" { extern u8 currentPartition; }

class CMenu
{
public:
	CMenu(CVideo &vid);
	~CMenu(void) {cleanup();}
	void init(void);
	void error(const wstringEx &msg);
	void terror(const char *key, const wchar_t *msg) { error(_fmt(key, msg)); }
	void exitHandler(int ExitTo);
	int main(void);
	void cleanup(void);
	u8 m_current_view;
private:
	struct SZone
	{
		int x;
		int y;
		int w;
		int h;
		bool hide;
	};
	CVideo &m_vid;
	CCursor m_cursor[WPAD_MAX_WIIMOTES];
	CButtonsMgr m_btnMgr;
	CCoverFlow m_cf;
	CFanart m_fa;
	CachedList m_gameList;
	Config m_cfg;
	Config m_loc;
	Config m_cat;
	Config m_gcfg1;
	Config m_gcfg2;
	Config m_theme;
	Config m_titles;
	Config m_version;
	Plugin m_plugin;
	vector<string> m_homebrewArgs;
	SmartBuf m_base_font;
	u32 m_base_font_size;
	u8 m_aa;
	bool m_bnr_settings;
	bool m_directLaunch;
	bool m_locked;
	bool m_favorites;
	s16 m_showtimer;
	string m_curLanguage;
	string m_curGameId;

	u8 m_numCFVersions;

	string m_themeDataDir;
	string m_appDir;
	string m_dataDir;
	string m_pluginsDir;
	string m_customBnrDir;
	string m_picDir;
	string m_boxPicDir;
	string m_boxcPicDir;
	string m_cacheDir;
	string m_listCacheDir;
	string m_bnrCacheDir;
	string m_themeDir;
	string m_musicDir;
	string m_txtCheatDir;
	string m_cheatDir;
	string m_wipDir;
	string m_videoDir;
	string m_fanartDir;
	string m_screenshotDir;
	string m_settingsDir;
	string m_languagesDir;
	string m_DMLgameDir;
	string m_helpDir;
	
	/* Updates */
	char m_app_update_drive[6];
	const char* m_app_update_url;
	const char* m_data_update_url;
	string m_dol;
	string m_app_update_zip;
	u32 m_app_update_size;
	string m_data_update_zip;
	u32 m_data_update_size;
	string m_ver;
	/* End Updates */
	// 
	STexture m_prevBg;
	STexture m_nextBg;
	STexture m_curBg;
	STexture m_lqBg;
	u8 m_bgCrossFade;
	//
	STexture m_errorBg;
	STexture m_mainBg;
	STexture m_configBg;
	STexture m_config3Bg;
	STexture m_configScreenBg;
	STexture m_config4Bg;
	STexture m_configAdvBg;
	STexture m_configSndBg;
	STexture m_downloadBg;
	STexture m_gameBg;
	STexture m_codeBg;
	STexture m_aboutBg;
	STexture m_systemBg;
	STexture m_wbfsBg;
	STexture m_gameSettingsBg;
	STexture m_gameBgLQ;
	STexture m_mainBgLQ;
//Main Coverflow
	u16 m_mainBtnConfig;
	u16 m_mainBtnInfo;
	u16 m_mainBtnFavoritesOn;
	u16 m_mainBtnFavoritesOff;
	u16 m_mainLblLetter;
#ifdef SHOWMEM
	u32 m_mem2FreeSize;
#endif
#ifdef SHOWMEMGECKO
	unsigned int mem1old;
	unsigned int mem1;
	unsigned int mem2old;
	unsigned int mem2;
#endif
	u16 m_mainLblNotice;
	u16 m_mainBtnNext;
	u16 m_mainBtnPrev;
	u16 m_mainBtnQuit;
	u16 m_mainBtnDVD;
	u16 m_mainBtnDML;
	u16 m_mainBtnEmu;
	u16 m_mainBtnUsb;
	u16 m_mainBtnChannel;
	u16 m_mainBtnHomebrew;
	u16 m_mainBtnInit;
	u16 m_mainBtnInit2;
	u16 m_mainLblInit;
	u16 m_mainLblUser[6];
	u8 m_show_dml;
	bool m_devo_installed;
	bool m_new_dml;
	bool m_new_dm_cfg;
	bool m_GameTDBLoaded;
//Main Config menus
	u16 m_configLblPage;
	u16 m_configBtnPageM;
	u16 m_configBtnPageP;
	u16 m_configBtnBack;
	u16 m_configLblTitle;
	u16 m_configLblDownload;
	u16 m_configBtnDownload; 
	u16 m_configLblParental;
	u16 m_configBtnUnlock;
	u16 m_configBtnSetCode;
	u16 m_configLblPartitionName;
	u16 m_configLblPartition;
	u16 m_configBtnPartitionP;
	u16 m_configBtnPartitionM;
	u16 m_configLblCfg4;
	u16 m_configBtnCfg4;
	u16 m_configLblUser[4];
	u16 m_configAdvLblTheme;
	u16 m_configAdvLblCurTheme;
	u16 m_configAdvBtnCurThemeM;
	u16 m_configAdvBtnCurThemeP;
	u16 m_configAdvLblLanguage;
	u16 m_configAdvLblCurLanguage;
	u16 m_configAdvBtnCurLanguageM;
	u16 m_configAdvBtnCurLanguageP;
	u16 m_configAdvLblCFTheme;
	u16 m_configAdvBtnCFTheme;
	u16 m_configAdvLblInstall;
	u16 m_configAdvBtnInstall;
	u16 m_configAdvLblUser[4];
	u16 m_config3LblGameLanguage;
	u16 m_config3LblLanguage;
	u16 m_config3BtnLanguageP;
	u16 m_config3BtnLanguageM;
	u16 m_config3LblGameVideo;
	u16 m_config3LblVideo;
	u16 m_config3BtnVideoP;
	u16 m_config3BtnVideoM;

	u16 m_config3LblDMLGameLanguage;
	u16 m_config3LblDMLLanguage;
	u16 m_config3BtnDMLLanguageP;
	u16 m_config3BtnDMLLanguageM;
	u16 m_config3LblDMLGameVideo;
	u16 m_config3LblDMLVideo;
	u16 m_config3BtnDMLVideoP;
	u16 m_config3BtnDMLVideoM;

	u16 m_config3LblOcarina;
	u16 m_config3BtnOcarina;
	u16 m_config3LblAsyncNet;
	u16 m_config3BtnAsyncNet;
	u16 m_config3LblUser[4];
	u16 m_config4LblReturnTo;
	u16 m_config4LblReturnToVal;
	u16 m_config4BtnReturnToM;
	u16 m_config4BtnReturnToP;
	u16 m_config4LblHome;
	u16 m_config4BtnHome;
	u16 m_config4LblSaveFavMode;
	u16 m_config4BtnSaveFavMode;
	u16 m_config4LblCategoryOnBoot;
	u16 m_config4BtnCategoryOnBoot;
	u16 m_config4LblUser[4];
	u16 m_configSndLblBnrVol;
	u16 m_configSndLblBnrVolVal;
	u16 m_configSndBtnBnrVolP;
	u16 m_configSndBtnBnrVolM;
	u16 m_configSndLblMusicVol;
	u16 m_configSndLblMusicVolVal;
	u16 m_configSndBtnMusicVolP;
	u16 m_configSndBtnMusicVolM;
	u16 m_configSndLblGuiVol;
	u16 m_configSndLblGuiVolVal;
	u16 m_configSndBtnGuiVolP;
	u16 m_configSndBtnGuiVolM;
	u16 m_configSndLblCFVol;
	u16 m_configSndLblCFVolVal;
	u16 m_configSndBtnCFVolP;
	u16 m_configSndBtnCFVolM;
	u16 m_configSndLblUser[4];
	u16 m_configScreenLblTVHeight;
	u16 m_configScreenLblTVHeightVal;
	u16 m_configScreenBtnTVHeightP;
	u16 m_configScreenBtnTVHeightM;
	u16 m_configScreenLblTVWidth;
	u16 m_configScreenLblTVWidthVal;
	u16 m_configScreenBtnTVWidthP;
	u16 m_configScreenBtnTVWidthM;
	u16 m_configScreenLblTVX;
	u16 m_configScreenLblTVXVal;
	u16 m_configScreenBtnTVXM;
	u16 m_configScreenBtnTVXP;
	u16 m_configScreenLblTVY;
	u16 m_configScreenLblTVYVal;
	u16 m_configScreenBtnTVYM;
	u16 m_configScreenBtnTVYP;
	u16 m_configScreenLblUser[4];
//Download menu
	enum CoverPrio
	{
		C_TYPE_PRIOA = (1<<0),
		C_TYPE_PRIOB = (1<<1),
		C_TYPE_EN =    (1<<2),
		C_TYPE_JA =    (1<<3),
		C_TYPE_FR =    (1<<4),
		C_TYPE_DE =    (1<<5),
		C_TYPE_ES =    (1<<6),
		C_TYPE_IT =    (1<<7),
		C_TYPE_NL =    (1<<8),
		C_TYPE_PT =    (1<<9),
		C_TYPE_RU =    (1<<10),
		C_TYPE_KO =    (1<<11),
		C_TYPE_ZHCN =  (1<<12),
		C_TYPE_AU =    (1<<13),
		C_TYPE_ONOR =  (1<<14),
		C_TYPE_ONCU =  (1<<15),
		
	};
	enum CoverType
	{
		BOX = 1,
		CBOX,
		FLAT,
		CFLAT,
	};
	u16 m_downloadPrioVal;
	u16 m_downloadLblTitle;
	u16 m_downloadPBar;
	u16 m_downloadBtnCancel;
	u16 m_downloadBtnAll;
	u16 m_downloadBtnMissing;
	u16 m_downloadBtnGameTDBDownload;
	u16 m_downloadLblGameTDBDownload;
	u16 m_downloadLblMessage[2];
	u16 m_downloadLblCovers;
	u16 m_downloadLblGameTDB;
	u16 m_downloadLblUser[4];
	u16 m_downloadLblCoverPrio;
	u16 m_downloadLblPrio;
	u16 m_downloadBtnPrioM;
	u16 m_downloadBtnPrioP;
	u16 m_downloadBtnVersion;
	u16 m_downloadLblCoverSet;
	u16 m_downloadBtnCoverSet;
	u16 m_downloadLblSetTitle;
	u16 m_downloadLblRegion;
	enum Regions
	{
		EN = 1,
		JA,
		FR,
		DE,		
		ES,
		IT,
		NL,
		PT,
		RU,
		KO,
		ZHCN,
		AU,
	};
	u16 m_downloadBtnEN;
	u16 m_downloadBtnJA;
	u16 m_downloadBtnFR;
	u16 m_downloadBtnDE;
	u16 m_downloadBtnES;
	u16 m_downloadBtnIT;
	u16 m_downloadBtnNL;
	u16 m_downloadBtnPT;
	u16 m_downloadBtnRU;
	u16 m_downloadBtnKO;
	u16 m_downloadBtnZHCN;
	u16 m_downloadBtnAU;
	u16 m_downloadBtnENs;
	u16 m_downloadBtnJAs;
	u16 m_downloadBtnFRs;
	u16 m_downloadBtnDEs;
	u16 m_downloadBtnESs;
	u16 m_downloadBtnITs;
	u16 m_downloadBtnNLs;
	u16 m_downloadBtnPTs;
	u16 m_downloadBtnRUs;
	u16 m_downloadBtnKOs;
	u16 m_downloadBtnZHCNs;
	u16 m_downloadBtnAUs;
	u16 m_downloadBtnBack;
	static s8 _versionDownloaderInit(CMenu *m);
	static s8 _versionTxtDownloaderInit(CMenu *m);
	s8 _versionDownloader();
	s8 _versionTxtDownloader();
//Game menu
	enum
	{
		LOAD_IOS_FAILED = 0,
		LOAD_IOS_SUCCEEDED,
		LOAD_IOS_NOT_NEEDED
	};
	u16 m_gameLblInfo;
	u16 m_gameBtnFavoriteOn;
	u16 m_gameBtnFavoriteOff;
	u16 m_gameBtnAdultOn;
	u16 m_gameBtnAdultOff;
	u16 m_gameBtnPlay;
	u16 m_gameBtnDelete;
	u16 m_gameBtnSettings;
	u16 m_gameBtnBack;
	u16 m_gameLblUser[4];
// Parental code menu	
	u16 m_codeLblTitle;
	u16 m_codeBtnKey[10];
	u16 m_codeBtnBack;
	u16 m_codeBtnErase;
	u16 m_codeBtnAge;
	u16 m_codeLblAge;
	u16 m_codeLblUser[4];
//menu_wbfs
	u16 m_wbfsLblTitle;
	u16 m_wbfsPBar;
	u16 m_wbfsBtnBack;
	u16 m_wbfsBtnGo;
	u16 m_wbfsLblDialog;
	u16 m_wbfsLblMessage;
	u16 m_wbfsLblUser[4];
//Theme Adjust menus
	u16 m_cfThemeBtnAlt;
	u16 m_cfThemeBtnSelect;
	u16 m_cfThemeBtnWide;
	u16 m_cfThemeLblParam;
	u16 m_cfThemeBtnParamM;
	u16 m_cfThemeBtnParamP;
	u16 m_cfThemeBtnCopy;
	u16 m_cfThemeBtnPaste;
	u16 m_cfThemeBtnSave;
	u16 m_cfThemeBtnCancel;
	u16 m_cfThemeLblVal[4 * 4];
	u16 m_cfThemeBtnValM[4 * 4];
	u16 m_cfThemeBtnValP[4 * 4];
	u16 m_cfThemeLblValTxt[4];
//Game Settings menus
	u16 m_gameSettingsLblPage;
	u16 m_gameSettingsBtnPageM;
	u16 m_gameSettingsBtnPageP;
	u16 m_gameSettingsBtnBack;
	u16 m_gameSettingsLblTitle;
	u16 m_gameSettingsLblGameLanguage;
	u16 m_gameSettingsLblLanguage;
	u16 m_gameSettingsBtnLanguageP;
	u16 m_gameSettingsBtnLanguageM;
	u16 m_gameSettingsLblGameVideo;
	u16 m_gameSettingsLblVideo;
	u16 m_gameSettingsBtnVideoP;
	u16 m_gameSettingsBtnVideoM;
	
	u16 m_gameSettingsLblDMLGameVideo;
	u16 m_gameSettingsLblDMLVideo;
	u16 m_gameSettingsBtnDMLVideoP;
	u16 m_gameSettingsBtnDMLVideoM;

	u16 m_gameSettingsLblGClanguageVal;
	u16 m_gameSettingsLblGClanguage;
	u16 m_gameSettingsBtnGClanguageP;
	u16 m_gameSettingsBtnGClanguageM;
	
	u16 m_gameSettingsLblIOSreloadBlock;
	u16 m_gameSettingsBtnIOSreloadBlock;
	
	u16 m_gameSettingsLblAspectRatio;
	u16 m_gameSettingsLblAspectRatioVal;
	u16 m_gameSettingsBtnAspectRatioP;
	u16 m_gameSettingsBtnAspectRatioM;

	u16 m_gameSettingsLblNMM;
	u16 m_gameSettingsLblNMM_Val;
	u16 m_gameSettingsBtnNMM_P;
	u16 m_gameSettingsBtnNMM_M;

	u16 m_gameSettingsLblNoDVD;
	u16 m_gameSettingsLblNoDVD_Val;
	u16 m_gameSettingsBtnNoDVD_P;
	u16 m_gameSettingsBtnNoDVD_M;

	u16 m_gameSettingsLblDevoMemcardEmu;
	u16 m_gameSettingsBtnDevoMemcardEmu;

	u16 m_gameSettingsLblDM_Widescreen;
	u16 m_gameSettingsBtnDM_Widescreen;

	u16 m_gameSettingsLblGCLoader;
	u16 m_gameSettingsLblGCLoader_Val;
	u16 m_gameSettingsBtnGCLoader_P;
	u16 m_gameSettingsBtnGCLoader_M;

	u16 m_gameSettingsLblCustom;
	u16 m_gameSettingsBtnCustom;
	u16 m_gameSettingsLblLaunchNK;
	u16 m_gameSettingsBtnLaunchNK;

	u16 m_gameSettingsLblOcarina;
	u16 m_gameSettingsBtnOcarina;
	u16 m_gameSettingsLblVipatch;
	u16 m_gameSettingsBtnVipatch;
	u16 m_gameSettingsLblCountryPatch;
	u16 m_gameSettingsBtnCountryPatch;
	u16 m_gameSettingsLblCover;
	u16 m_gameSettingsBtnCover;
	u16 m_gameSettingsLblPatchVidModes;
	u16 m_gameSettingsLblPatchVidModesVal;
	u16 m_gameSettingsBtnPatchVidModesM;
	u16 m_gameSettingsBtnPatchVidModesP;
	u16 m_gameSettingsLblUser[3 * 2];
	u16 m_gameSettingsLblHooktype;
	u16 m_gameSettingsLblHooktypeVal;
	u16 m_gameSettingsBtnHooktypeM;
	u16 m_gameSettingsBtnHooktypeP;
	u16 m_gameSettingsLblEmulationVal;
	u16 m_gameSettingsBtnEmulationP;
	u16 m_gameSettingsBtnEmulationM;
	u16 m_gameSettingsLblEmulation;
	u16 m_gameSettingsLblDebugger;
	u16 m_gameSettingsLblDebuggerV;
	u16 m_gameSettingsBtnDebuggerP;
	u16 m_gameSettingsBtnDebuggerM;
	u16 m_gameSettingsLblCheat;
	u16 m_gameSettingsBtnCheat;
	u16 m_gameSettingsLblCategoryMain;
	u16 m_gameSettingsBtnCategoryMain;
	u16 m_gameSettingsPage;
 	u16 m_gameSettingsLblGameIOS;
 	u16 m_gameSettingsLblIOS;
 	u16 m_gameSettingsBtnIOSP;
 	u16 m_gameSettingsBtnIOSM;
	u16 m_gameSettingsLblExtractSave;
	u16 m_gameSettingsBtnExtractSave;
	u16 m_gameSettingsLblFlashSave;
	u16 m_gameSettingsBtnFlashSave;
// System Menu
	u16 m_systemBtnBack;
	u16 m_systemLblTitle;
	u16 m_systemLblVersionTxt;
	u16 m_systemLblVersion;
	u16 m_systemLblVersionRev;
	u16 m_systemLblUser[4];
	u16 m_systemBtnDownload;
	u16 m_systemLblInfo;
	u16 m_systemLblVerSelectVal;	
	u16 m_systemBtnVerSelectM;	
	u16 m_systemBtnVerSelectP;	
//Cheat menu
	u16 m_cheatBtnBack;
	u16 m_cheatBtnApply;
	u16 m_cheatBtnDownload;
	u16 m_cheatLblTitle;
	u16 m_cheatLblPage;
	u16 m_cheatBtnPageM;
	u16 m_cheatBtnPageP;
	u16 m_cheatLblItem[4];
	u16 m_cheatBtnItem[4];
	u16 m_cheatSettingsPage;
	u16 m_cheatLblUser[4];
	STexture m_cheatBg;
	GCTCheats m_cheatfile;
// Gameinfo menu
	u16 m_gameinfoLblTitle;
	u16 m_gameinfoLblID;
	u16 m_gameinfoLblSynopsis;
	u16 m_gameinfoLblDev;
	u16 m_gameinfoLblRegion;
	u16 m_gameinfoLblPublisher;
	u16 m_gameinfoLblRlsdate;
	u16 m_gameinfoLblGenre;
	u16 m_gameinfoLblRating;
	u16 m_gameinfoLblWifiplayers;
	u16 m_gameinfoLblUser[5];
	u16 m_gameinfoLblControlsReq[4];
	u16 m_gameinfoLblControls[4];
	STexture m_gameinfoBg;
	STexture m_rating;
	STexture m_wifi;
	STexture m_controlsreq[4];
	STexture m_controls[4];
// NandEmulation
	string m_saveExtGameId;
	bool m_forceext;
	bool m_tempView;
	s32 m_partRequest;
// Zones
	SZone m_mainPrevZone;
	SZone m_mainNextZone;
	SZone m_mainButtonsZone;
	SZone m_mainButtonsZone2;
	SZone m_mainButtonsZone3;
	SZone m_gameButtonsZone;
	bool m_reload;

	WPADData *wd[WPAD_MAX_WIIMOTES];
	void LeftStick();
	u8 pointerhidedelay[WPAD_MAX_WIIMOTES];
	u16 stickPointer_x[WPAD_MAX_WIIMOTES];
	u16 stickPointer_y[WPAD_MAX_WIIMOTES];
	
	u8 m_wpadLeftDelay;
	u8 m_wpadDownDelay;
	u8 m_wpadRightDelay;
	u8 m_wpadUpDelay;
	u8 m_wpadADelay;
	//u8 m_wpadBDelay;

	u8 m_padLeftDelay;
	u8 m_padDownDelay;
	u8 m_padRightDelay;
	u8 m_padUpDelay;
	u8 m_padADelay;
	//u8 m_padBDelay;
	
	u32 wii_btnsPressed;
	u32 wii_btnsHeld;
	u32 gc_btnsPressed;
	u32 gc_btnsHeld;

	bool m_show_pointer[WPAD_MAX_WIIMOTES];
	float left_stick_angle[WPAD_MAX_WIIMOTES];
	float left_stick_mag[WPAD_MAX_WIIMOTES];
	float right_stick_angle[WPAD_MAX_WIIMOTES];
	float right_stick_mag[WPAD_MAX_WIIMOTES];
	float wmote_roll[WPAD_MAX_WIIMOTES];
	s32   right_stick_skip[WPAD_MAX_WIIMOTES];
	s32	  wmote_roll_skip[WPAD_MAX_WIIMOTES];
	bool  enable_wmote_roll;

	bool m_cfNeedsUpdate;

	void SetupInput(bool reset_pos = false);
	void ScanInput(void);

	void ButtonsPressed(void);
	void ButtonsHeld(void);

	bool lStick_Up(void);
	bool lStick_Right(void);
	bool lStick_Down(void);
	bool lStick_Left(void);

	bool rStick_Up(void);
	bool rStick_Right(void);
	bool rStick_Down(void);
	bool rStick_Left(void);

	bool wRoll_Left(void);
	bool wRoll_Right(void);

	bool wii_btnRepeat(s64 btn);
	bool gc_btnRepeat(s64 btn);

	bool WPadIR_Valid(int chan);
	bool WPadIR_ANY(void);
	
	void ShowZone(SZone zone, bool &showZone);
	void ShowMainZone(void);
	void ShowMainZone2(void);
	void ShowMainZone3(void);
	void ShowPrevZone(void);
	void ShowNextZone(void);
	void ShowGameZone(void);
	bool m_show_zone_main;
	bool m_show_zone_main2;
	bool m_show_zone_main3;
	bool m_show_zone_prev;
	bool m_show_zone_next;
	bool m_show_zone_game;

	volatile bool m_exit;
	volatile bool m_disable_exit;
	
	volatile bool m_networkInit;
	volatile bool m_thrdStop;
	volatile bool m_thrdWorking;
	volatile bool m_thrdNetwork;
	float m_thrdStep;
	float m_thrdStepLen;
	string m_coverDLGameId;
	mutex_t m_mutex;
	wstringEx m_thrdMessage;
	volatile float m_thrdProgress;
	volatile float m_fileProgress;
	volatile bool m_thrdMessageAdded;
	volatile bool m_gameSelected;
	GuiSound m_gameSound;
	SmartGuiSound m_cameraSound;
	dir_discHdr *m_gameSoundHdr;
	lwp_t m_gameSoundThread;
	bool m_gamesound_changed;
	u8 m_bnrSndVol;
	u8 m_max_categories;
	bool m_video_playing;

private:
	enum WBFS_OP
	{
		WO_ADD_GAME,
		WO_REMOVE_GAME,
		WO_FORMAT,
		WO_COPY_GAME,
	};
	typedef pair<string, u32> FontDesc;
	typedef map<FontDesc, SFont> FontSet;
	typedef map<string, STexture> TexSet;
	typedef map<string, SmartGuiSound> SoundSet;
	struct SThemeData
	{
		TexSet texSet;
		FontSet fontSet;
		SoundSet soundSet;
		SFont btnFont;
		SFont lblFont;
		SFont titleFont;
		SFont txtFont;
		CColor btnFontColor;
		CColor lblFontColor;
		CColor txtFontColor;
		CColor titleFontColor;
		CColor selubtnFontColor;
		CColor selsbtnFontColor;
		STexture bg;
		STexture btnTexL;
		STexture btnTexR;
		STexture btnTexC;
		STexture btnTexLS;
		STexture btnTexRS;
		STexture btnTexCS;
		STexture btnTexLH;
		STexture btnTexRH;
		STexture btnTexCH;
		STexture btnTexLSH;
		STexture btnTexRSH;
		STexture btnTexCSH;
		STexture btnAUOn;
		STexture btnAUOns;
		STexture btnAUOff;
		STexture btnAUOffs;
		STexture btnENOn;
		STexture btnENOns;
		STexture btnENOff;
		STexture btnENOffs;
		STexture btnJAOn;
		STexture btnJAOns;
		STexture btnJAOff;
		STexture btnJAOffs;
		STexture btnFROn;
		STexture btnFROns;
		STexture btnFROff;
		STexture btnFROffs;
		STexture btnDEOn;
		STexture btnDEOns;
		STexture btnDEOff;
		STexture btnDEOffs;
		STexture btnESOn;
		STexture btnESOns;
		STexture btnESOff;
		STexture btnESOffs;
		STexture btnITOn;
		STexture btnITOns;
		STexture btnITOff;
		STexture btnITOffs;
		STexture btnNLOn;
		STexture btnNLOns;
		STexture btnNLOff;
		STexture btnNLOffs;
		STexture btnPTOn;
		STexture btnPTOns;
		STexture btnPTOff;
		STexture btnPTOffs;
		STexture btnRUOn;
		STexture btnRUOns;
		STexture btnRUOff;
		STexture btnRUOffs;
		STexture btnKOOn;
		STexture btnKOOns;
		STexture btnKOOff;
		STexture btnKOOffs;
		STexture btnZHCNOn;
		STexture btnZHCNOns;
		STexture btnZHCNOff;
		STexture btnZHCNOffs;
		STexture checkboxoff;
		STexture checkboxoffs;
		STexture checkboxon;
		STexture checkboxons;
		STexture checkboxHid;
		STexture checkboxHids;
		STexture checkboxReq;
		STexture checkboxReqs;
		STexture pbarTexL;
		STexture pbarTexR;
		STexture pbarTexC;
		STexture pbarTexLS;
		STexture pbarTexRS;
		STexture pbarTexCS;
		STexture btnTexPlus;
		STexture btnTexPlusS;
		STexture btnTexMinus;
		STexture btnTexMinusS;
		SmartGuiSound clickSound;
		SmartGuiSound hoverSound;
		SmartGuiSound cameraSound;
	};
	SThemeData theme;
	struct SCFParamDesc
	{
		enum
		{
			PDT_EMPTY,
			PDT_FLOAT,
			PDT_V3D,
			PDT_COLOR,
			PDT_BOOL,
			PDT_INT, 
			PDT_TXTSTYLE,
		} paramType[4];
		enum
		{
			PDD_BOTH,
			PDD_NORMAL, 
			PDD_SELECTED,
		} domain;
		bool scrnFmt;
		const char name[32];
		const char valName[4][64];
		const char key[4][48];
		float step[4];
		float minMaxVal[4][2];
	};
	// 
	bool _loadList(void);
	bool _loadGameList(void);
	bool _loadDmlList(void);
	bool _loadChannelList(void);
	bool _loadEmuList(void);
	bool _loadHomebrewList(void);
	void _initCF(void);
	// 
	void _initMainMenu(SThemeData &theme);
	void _initErrorMenu(SThemeData &theme);
	void _initConfigMenu(SThemeData &theme);
	void _initConfigAdvMenu(SThemeData &theme);
	void _initConfig3Menu(SThemeData &theme);
	void _initConfig4Menu(SThemeData &theme);
	void _initConfigSndMenu(SThemeData &theme);
	void _initConfigScreenMenu(SThemeData &theme);
	void _initGameMenu(SThemeData &theme);
	void _initDownloadMenu(SThemeData &theme);
	void _initCodeMenu(SThemeData &theme);
	void _initAboutMenu(SThemeData &theme);
	void _initWBFSMenu(SThemeData &theme);
	void _initCFThemeMenu(SThemeData &theme);
	void _initGameSettingsMenu(SThemeData &theme);
	void _initCheatSettingsMenu(SThemeData &theme);
	void _initCheatButtons();
	void _initSourceMenu(SThemeData &theme);
	void _initPluginSettingsMenu(SThemeData &theme);
	void _initCategorySettingsMenu(SThemeData &theme);
	void _initSystemMenu(SThemeData &theme);
	void _initGameInfoMenu(SThemeData &theme);
	void _initNandEmuMenu(CMenu::SThemeData &theme);
	void _initHomeAndExitToMenu(CMenu::SThemeData &theme);
	//
	void _textSource(void);
	void _textPluginSettings(void);
	void _textCategorySettings(void);
	void _textCheatSettings(void);
	void _textSystem(void);
	void _textMain(void);
	void _textError(void);
	void _textYesNo(void);
	void _textConfig(void);
	void _textConfig3(void);
	void _textConfigScreen(void);
	void _textConfig4(void);
	void _textConfigAdv(void);
	void _textConfigSnd(void);
	void _textGame(void);
	void _textDownload(void);
	void _textCode(void);
	void _textAbout(void);
	void _textWBFS(void);
	void _textGameSettings(void);
	void _textGameInfo(void);
	void _textNandEmu(void);
	void _textHome(void);
	void _textExitTo(void);
	//
	void _hideCheatSettings(bool instant = false);
	void _hideError(bool instant = false);
	void _hideMain(bool instant = false);
	void _hideConfigCommon(bool instant = false);
	void _hideConfig(bool instant = false);
	void _hideConfig3(bool instant = false);
	void _hideConfigScreen(bool instant = false);
	void _hideConfig4(bool instant = false);
	void _hideConfigAdv(bool instant = false);
	void _hideConfigSnd(bool instant = false);
	void _hideGame(bool instant = false);
	void _hideDownload(bool instant = false);
	void _hideSettings(bool instant = false);
	void _hideCode(bool instant = false);
	void _hideAbout(bool instant = false);
	void _hideWBFS(bool instant = false);
	void _hideCFTheme(bool instant = false);
	void _hideGameSettings(bool instant = false);
	void _hideSource(bool instant = false);
	void _hidePluginSettings(bool instant = false);
	void _hideCategorySettings(bool instant = false);
	void _hideSystem(bool instant = false);
	void _hideGameInfo(bool instant = false);
	void _hideCheatDownload(bool instant = false);
	void _hideNandEmu(bool instant = false);
	void _hideHome(bool instant = false);
	void _hideExitTo(bool instant = false);
	//
	void _showError(void);
	void _showMain(void);
	void _showConfigCommon(const STexture & bg, int page);
	void _showConfig(void);
	void _showConfig3(void);
	void _showConfigScreen(void);
	void _showConfig4(void);
	void _showConfigAdv(void);
	void _showConfigSnd(void);
	void _enableNandEmu(bool fromconfig);
	void _showGame(void);
	void _showDownload(void);
	void _showSettings();
	void _showCode(void);
	void _showAbout(void);
	void _showSource(void);
	void _showSourceNotice(void);
	void _showPluginSettings(void);
	void _showCategorySettings(void);
	void _showCheatSettings(void);
	void _showSystem(void);
	void _showGameInfo(void);
	void _showWBFS(WBFS_OP op);
	void _showCFTheme(u32 curParam, int version, bool wide);
	void _showGameSettings(void);
	void _showCheatDownload(void);
	void _showHome(void);
	void _showExitTo(void);
	void _updateSourceBtns(void);
	void _updatePluginCheckboxes(void);
	void _updateCheckboxes(void);
	void _setBg(const STexture &tex, const STexture &lqTex);
	void _updateBg(void);
	void _drawBg(void);
	void _updateText(void);
	void _showNandEmu(void);
	//
	void _config(int page);
	int _configCommon(void);
	int _config1(void);
	int _config3(void);
	int _configScreen(void);
	int _config4(void);
	int _configAdv(void);
	int _configSnd(void);
	int _NandEmuCfg(void);
	int _AutoCreateNand(void);
	int _AutoExtractSave(string gameId);
	int _FlashSave(string gameId);
	enum configPageChanges
	{
		CONFIG_PAGE_DEC = -1,
		CONFIG_PAGE_NO_CHANGE = 0,
		CONFIG_PAGE_INC = 1,
		CONFIG_PAGE_BACK,
	};
	void _cfNeedsUpdate(void);
	void _game(bool launch = false);
	void _download(string gameId = string());
	void _code(void);
	void _about(bool help = false);
	bool _wbfsOp(WBFS_OP op);
	void _cfTheme(void);
	void _system(void);
	void _gameinfo(void);
	void _gameSettings(void);
	void _CheatSettings();
	bool _Source();
	void _PluginSettings();
	void _CategorySettings(bool fromGameSet = false);
	bool _Home();
	bool _ExitTo();
	//
	void _mainLoopCommon(bool withCF = false, bool blockReboot = false, bool adjusting = false);
	// 
	vector<dir_discHdr> _searchGamesByID(const char *gameId);
/* 	vector<dir_discHdr> _searchGamesByTitle(wchar_t letter);
	vector<dir_discHdr> _searchGamesByType(const char type);
	vector<dir_discHdr> _searchGamesByRegion(const char region); */
public:
	void directlaunch(const string &id);
private:
	bool m_use_wifi_gecko;
	void _reload_wifi_gecko();
	bool _loadFile(SmartBuf &buffer, u32 &size, const char *path, const char *file);
	int _loadIOS(u8 ios, int userIOS, string id);
	void _launch(dir_discHdr *hdr);
	void _launchGame(dir_discHdr *hdr, bool dvd);
	void _launchChannel(dir_discHdr *hdr);
	void _launchHomebrew(const char *filepath, vector<string> arguments);
	void _launchGC(dir_discHdr *hdr, bool disc);
	void _setAA(int aa);
	void _loadCFCfg(SThemeData &theme);
	void _loadCFLayout(int version, bool forceAA = false, bool otherScrnFmt = false);
	Vector3D _getCFV3D(const string &domain, const string &key, const Vector3D &def, bool otherScrnFmt = false);
	int _getCFInt(const string &domain, const string &key, int def, bool otherScrnFmt = false);
	float _getCFFloat(const string &domain, const string &key, float def, bool otherScrnFmt = false);
	void _cfParam(bool inc, int i, const SCFParamDesc &p, int cfVersion, bool wide);
	void _buildMenus(void);
	void _loadDefaultFont(bool korean);
	void _cleanupDefaultFont();
	const char *_domainFromView(void);
	const char *_cfDomain(bool selected = false);	
	void UpdateCache(u32 view = COVERFLOW_MAX);
	int MIOSisDML();
	void RemoveCover( char * id );
	SFont _font(CMenu::FontSet &fontSet, const char *domain, const char *key, u32 fontSize, u32 lineSpacing, u32 weight, u32 index, const char *genKey);
	STexture _texture(TexSet &texSet, const char *domain, const char *key, STexture def);
	vector<STexture> _textures(TexSet &texSet, const char *domain, const char *key);
	void _showWaitMessage();
public:
	void _hideWaitMessage();
	bool m_Emulator_boot;
private:
	SmartGuiSound _sound(CMenu::SoundSet &soundSet, const char *domain, const char *key, const u8 * snd, u32 len, string name, bool isAllocated);
	SmartGuiSound _sound(CMenu::SoundSet &soundSet, const char *domain, const char *key, string name);
	u16 _textStyle(const char *domain, const char *key, u16 def);
	u16 _addButton(SThemeData &theme, const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color);
	u16 _addSelButton(SThemeData &theme, const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color);
	u16 _addPicButton(SThemeData &theme, const char *domain, STexture &texNormal, STexture &texSelected, int x, int y, u32 width, u32 height);
	u16 _addTitle(SThemeData &theme, const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, u16 style);
	u16 _addText(SThemeData &theme, const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, u16 style);
	u16 _addLabel(SThemeData &theme, const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, u16 style);
	u16 _addLabel(SThemeData &theme, const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, u16 style, STexture &bg);
	u16 _addProgressBar(SThemeData &theme, const char *domain, int x, int y, u32 width, u32 height);
	void _setHideAnim(u16 id, const char *domain, int dx, int dy, float scaleX, float scaleY);
	void _addUserLabels(CMenu::SThemeData &theme, u16 *ids, u32 size, const char *domain);
	void _addUserLabels(CMenu::SThemeData &theme, u16 *ids, u32 start, u32 size, const char *domain);
	// 
	const wstringEx _t(const char *key, const wchar_t *def = L"") { return m_loc.getWString(m_curLanguage, key, def); }
	const wstringEx _fmt(const char *key, const wchar_t *def);
	wstringEx _getNoticeTranslation(int sorting, wstringEx curLetter);
	// 
	void _setThrdMsg(const wstringEx &msg, float progress);
	void _setDumpMsg(const wstringEx &msg, float progress, float fileprog);
	int _coverDownloader(bool missingOnly);
	static int _coverDownloaderAll(CMenu *m);
	static int _coverDownloaderMissing(CMenu *m);
	static bool _downloadProgress(void *obj, int size, int position);
	static int _gametdbDownloader(CMenu *m);
	int _gametdbDownloaderAsync();

	static s32 _networkComplete(s32 result, void *usrData);
	void _initAsyncNetwork();
	bool _isNetworkAvailable();
	int _initNetwork();
	void _deinitNetwork();
	void LoadView(void);
	void _getGrabStatus(void);
	static void _addDiscProgress(int status, int total, void *user_data);
	static void _Messenger(int message, int info, char *cinfo, void *user_data);
	static void _ShowProgress(int dumpstat, int dumpprog, int filestat, int fileprog, int files, int folders, char *tmess, void *user_data);
	static int _gameInstaller(void *obj);	
	static int _GCgameInstaller(void *obj);
	static int _GCcopyGame(void *obj);
	float m_progress;
	float m_fprogress;
	int m_fileprog;
	int m_filesize;
	int m_dumpsize;
	int m_filesdone;
	int m_foldersdone;
	int m_nandexentry;
	wstringEx _optBoolToString(int b);
	void _stopSounds(void);
	static int _NandDumper(void *obj);
	static int _NandFlasher(void *obj);
	int _FindEmuPart(string *emuPath, int part, bool searchvalid);
	bool _checkSave(string id, bool nand);
	bool _TestEmuNand(int epart, const char *path, bool indept);	

	static u32 _downloadCheatFileAsync(void *obj);

	void _playGameSound(void);
	void CheckGameSoundThread(void);
	void ClearGameSoundThreadStack(void);
	static void _gameSoundThread(CMenu *m);

	static void _load_installed_cioses();
	//
	struct SOption { const char id[10]; const wchar_t text[16]; };
	static const string _translations[23];
	static const SOption _languages[11];

	static const SOption _GlobalVideoModes[6];
	static const SOption _VideoModes[7];
	
	static const SOption _GlobalDMLvideoModes[6];
	static const SOption _GlobalGClanguages[7];
	static const SOption _DMLvideoModes[7];
	static const SOption _GClanguages[8];

	static const SOption _NandEmu[2];
	static const SOption _SaveEmu[5];
	static const SOption _GlobalSaveEmu[4];
	static const SOption _AspectRatio[3];
	static const SOption _NMM[4];
	static const SOption _NoDVD[3];
	static const SOption _GCLoader[3];
	static const SOption _vidModePatch[4];
	static const SOption _hooktype[8];
	static const SOption _exitTo[6];
	static map<u8, u8> _installed_cios;
	typedef map<u8, u8>::iterator CIOSItr;
	static int _version[9];
	static const SCFParamDesc _cfParams[];
	static const int _nbCfgPages;
};

#define ARRAY_SIZE(a)		(sizeof a / sizeof a[0])

#endif // !defined(__MENU_HPP)
