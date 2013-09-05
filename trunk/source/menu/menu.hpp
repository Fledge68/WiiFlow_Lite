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
#include "fileOps/fileOps.h"
#include "gecko/gecko.hpp"
#include "gecko/wifi_gecko.hpp"
#include "gui/coverflow.hpp"
#include "gui/cursor.hpp"
#include "gui/fanart.hpp"
#include "gui/gui.hpp"
#include "list/ListGenerator.hpp"
#include "loader/disc.h"
#include "loader/sys.h"
#include "loader/gc_disc_dump.hpp"
#include "loader/wbfs.h"
#include "music/gui_sound.h"
#include "music/MusicPlayer.hpp"
#include "plugin/plugin.hpp"
#include "wiiuse/wpad.h"

using namespace std;

class CMenu
{
public:
	CMenu();
	void init();
	void error(const wstringEx &msg);
	void terror(const char *key, const wchar_t *msg) { error(_fmt(key, msg)); }
	void exitHandler(int ExitTo);
	int main(void);
	void cleanup(void);
	void loadDefaultFont(void);
	void TempLoadIOS(int IOS = 0);

	const char *getBoxPath(const dir_discHdr *element);
	const char *getFrontPath(const dir_discHdr *element);
	const char *getBlankCoverPath(const dir_discHdr *element);

	u8 m_current_view;
	u8 m_last_view;
	bool m_combined_view;
	u8 enabledPluginsCount;
	u8 m_catStartPage;
	bool m_clearCats;
private:
	struct SZone
	{
		int x;
		int y;
		int w;
		int h;
		bool hide;
	};
	CCursor m_cursor[WPAD_MAX_WIIMOTES];
	CFanart m_fa;
	Config m_cfg;
	Config m_loc;
	Config m_cat;
	Config m_source;
	Config m_gcfg1;
	Config m_gcfg2;
	Config m_theme;
	Config m_titles;
	Config m_version;
	vector<string> m_homebrewArgs;
	u8 *m_base_font;
	u32 m_base_font_size;
	u8 *m_wbf1_font;
	u8 *m_wbf2_font;
	u8 *m_file;
	u8 *m_buffer;
	u8 m_aa;
	bool m_bnr_settings;
	bool m_directLaunch;
	bool m_locked;
	bool m_favorites;
	bool m_music_info;
	bool m_use_source;
	bool m_multisource;
	bool m_load_view;
	s16 m_showtimer;
	string m_curLanguage;

	u8 m_numCFVersions;

	string m_miosDir;
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

	/* Explorer stuff */
	bool m_txt_view;
	const char *m_txt_path;

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
	TexData m_curBg;
	const TexData *m_prevBg;
	const TexData *m_nextBg;
	const TexData *m_lqBg;
	u8 m_bgCrossFade;
	//
	TexData m_errorBg;
	TexData m_mainBg;
	TexData m_configBg;
	TexData m_config3Bg;
	TexData m_configScreenBg;
	TexData m_config4Bg;
	TexData m_configAdvBg;
	TexData m_configSndBg;
	TexData m_downloadBg;
	TexData m_gameBg;
	TexData m_codeBg;
	TexData m_aboutBg;
	TexData m_systemBg;
	TexData m_wbfsBg;
	TexData m_gameSettingsBg;
	TexData m_gameBgLQ;
	TexData m_mainBgLQ;
//Main Coverflow
	s16 m_mainBtnConfig;
	s16 m_mainBtnInfo;
	s16 m_mainBtnFavoritesOn;
	s16 m_mainBtnFavoritesOff;
	s16 m_mainLblLetter;
	s16 m_mainLblCurMusic;
#ifdef SHOWMEM
	s16 m_mem1FreeSize;
	s16 m_mem2FreeSize;
#endif
#ifdef SHOWMEMGECKO
	unsigned int mem1old;
	unsigned int mem1;
	unsigned int mem2old;
	unsigned int mem2;
#endif
	s16 m_mainLblNotice;
	s16 m_mainBtnNext;
	s16 m_mainBtnPrev;
	s16 m_mainBtnQuit;
	s16 m_mainBtnDVD;
	s16 m_mainBtnDML;
	s16 m_mainBtnEmu;
	s16 m_mainBtnUsb;
	s16 m_mainBtnChannel;
	s16 m_mainBtnHomebrew;
	s16 m_mainBtnInit;
	s16 m_mainBtnInit2;
	s16 m_mainLblInit;
	s16 m_mainLblUser[6];
	u8 m_mios_ver;
	bool m_show_dml;
	bool m_sd_dm;
	bool m_devo_installed;
	bool m_new_dml;
	bool m_new_dm_cfg;
	bool m_GameTDBLoaded;
//Main Config menus
	s16 m_configLblPage;
	s16 m_configBtnPageM;
	s16 m_configBtnPageP;
	s16 m_configBtnBack;
	s16 m_configLblTitle;
	s16 m_configLblDownload;
	s16 m_configBtnDownload; 
	s16 m_configLblParental;
	s16 m_configBtnUnlock;
	s16 m_configBtnSetCode;
	s16 m_configLblPartitionName;
	s16 m_configLblPartition;
	s16 m_configBtnPartitionP;
	s16 m_configBtnPartitionM;
	s16 m_configLblCfg4;
	s16 m_configBtnCfg4;
	s16 m_configLblUser[4];
	s16 m_configAdvLblTheme;
	s16 m_configAdvLblCurTheme;
	s16 m_configAdvBtnCurThemeM;
	s16 m_configAdvBtnCurThemeP;
	s16 m_configAdvLblLanguage;
	s16 m_configAdvBtnManageLanguage;
	s16 m_configAdvLblCFTheme;
	s16 m_configAdvBtnCFTheme;
	s16 m_configAdvLblBootChange;
	s16 m_configAdvBtnBootChange;
	s16 m_configAdvLblUser[4];
	s16 m_config3LblGameLanguage;
	s16 m_config3LblLanguage;
	s16 m_config3BtnLanguageP;
	s16 m_config3BtnLanguageM;
	s16 m_config3LblGameVideo;
	s16 m_config3LblVideo;
	s16 m_config3BtnVideoP;
	s16 m_config3BtnVideoM;

	s16 m_config3LblDMLGameLanguage;
	s16 m_config3LblDMLLanguage;
	s16 m_config3BtnDMLLanguageP;
	s16 m_config3BtnDMLLanguageM;
	s16 m_config3LblDMLGameVideo;
	s16 m_config3LblDMLVideo;
	s16 m_config3BtnDMLVideoP;
	s16 m_config3BtnDMLVideoM;
	s16 m_config3LblDMLGameLoader;
	s16 m_config3LblDMLLoader;
	s16 m_config3BtnDMLLoaderP;
	s16 m_config3BtnDMLLoaderM;
	
	s16 m_config3LblOcarina;
	s16 m_config3BtnOcarina;
	s16 m_config3LblUser[4];
	s16 m_config4LblReturnTo;
	s16 m_config4LblReturnToVal;
	s16 m_config4BtnReturnToM;
	s16 m_config4BtnReturnToP;
	s16 m_config4LblHome;
	s16 m_config4BtnHome;
	s16 m_config4LblSaveFavMode;
	s16 m_config4BtnSaveFavMode;
	s16 m_config4LblPathManager;
	s16 m_config4BtnPathManager;
	s16 m_config4LblUser[4];
	s16 m_configSndLblBnrVol;
	s16 m_configSndLblBnrVolVal;
	s16 m_configSndBtnBnrVolP;
	s16 m_configSndBtnBnrVolM;
	s16 m_configSndLblMusicVol;
	s16 m_configSndLblMusicVolVal;
	s16 m_configSndBtnMusicVolP;
	s16 m_configSndBtnMusicVolM;
	s16 m_configSndLblGuiVol;
	s16 m_configSndLblGuiVolVal;
	s16 m_configSndBtnGuiVolP;
	s16 m_configSndBtnGuiVolM;
	s16 m_configSndLblCFVol;
	s16 m_configSndLblCFVolVal;
	s16 m_configSndBtnCFVolP;
	s16 m_configSndBtnCFVolM;
	s16 m_configSndLblUser[4];
	s16 m_configScreenLblTVHeight;
	s16 m_configScreenLblTVHeightVal;
	s16 m_configScreenBtnTVHeightP;
	s16 m_configScreenBtnTVHeightM;
	s16 m_configScreenLblTVWidth;
	s16 m_configScreenLblTVWidthVal;
	s16 m_configScreenBtnTVWidthP;
	s16 m_configScreenBtnTVWidthM;
	s16 m_configScreenLblTVX;
	s16 m_configScreenLblTVXVal;
	s16 m_configScreenBtnTVXM;
	s16 m_configScreenBtnTVXP;
	s16 m_configScreenLblTVY;
	s16 m_configScreenLblTVYVal;
	s16 m_configScreenBtnTVYM;
	s16 m_configScreenBtnTVYP;
	s16 m_configScreenLblUser[4];
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
	s16 m_downloadPrioVal;
	s16 m_downloadLblTitle;
	s16 m_downloadPBar;
	s16 m_downloadBtnCancel;
	s16 m_downloadBtnAll;
	s16 m_downloadBtnMissing;
	s16 m_downloadBtnGameTDBDownload;
	s16 m_downloadLblGameTDBDownload;
	s16 m_downloadLblMessage[2];
	s16 m_downloadLblCovers;
	s16 m_downloadLblGameTDB;
	s16 m_downloadLblUser[4];
	s16 m_downloadLblCoverPrio;
	s16 m_downloadLblPrio;
	s16 m_downloadBtnPrioM;
	s16 m_downloadBtnPrioP;
	s16 m_downloadBtnVersion;
	s16 m_downloadLblCoverSet;
	s16 m_downloadBtnCoverSet;
	s16 m_downloadLblSetTitle;
	s16 m_downloadLblRegion;
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
	s16 m_downloadBtnEN;
	s16 m_downloadBtnJA;
	s16 m_downloadBtnFR;
	s16 m_downloadBtnDE;
	s16 m_downloadBtnES;
	s16 m_downloadBtnIT;
	s16 m_downloadBtnNL;
	s16 m_downloadBtnPT;
	s16 m_downloadBtnRU;
	s16 m_downloadBtnKO;
	s16 m_downloadBtnZHCN;
	s16 m_downloadBtnAU;
	s16 m_downloadBtnENs;
	s16 m_downloadBtnJAs;
	s16 m_downloadBtnFRs;
	s16 m_downloadBtnDEs;
	s16 m_downloadBtnESs;
	s16 m_downloadBtnITs;
	s16 m_downloadBtnNLs;
	s16 m_downloadBtnPTs;
	s16 m_downloadBtnRUs;
	s16 m_downloadBtnKOs;
	s16 m_downloadBtnZHCNs;
	s16 m_downloadBtnAUs;
	s16 m_downloadBtnBack;
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
	s16 m_gameLblInfo;
	s16 m_gameBtnFavoriteOn;
	s16 m_gameBtnFavoriteOff;
	s16 m_gameBtnAdultOn;
	s16 m_gameBtnAdultOff;
	s16 m_gameBtnPlay;
	s16 m_gameBtnDelete;
	s16 m_gameBtnSettings;
	s16 m_gameBtnBack;
	s16 m_gameLblUser[5];
// Parental code menu
	s16 m_codeLblTitle;
	s16 m_codeBtnKey[10];
	s16 m_codeBtnBack;
	s16 m_codeBtnErase;
	s16 m_codeBtnAge;
	s16 m_codeLblAge;
	s16 m_codeLblUser[4];
//menu_wbfs
	s16 m_wbfsLblTitle;
	s16 m_wbfsPBar;
	s16 m_wbfsBtnGo;
	s16 m_wbfsLblDialog;
	s16 m_wbfsLblMessage;
	s16 m_wbfsLblUser[4];
//Theme Adjust menus
	s16 m_cfThemeBtnAlt;
	s16 m_cfThemeBtnSelect;
	s16 m_cfThemeBtnWide;
	s16 m_cfThemeLblParam;
	s16 m_cfThemeBtnParamM;
	s16 m_cfThemeBtnParamP;
	s16 m_cfThemeBtnCopy;
	s16 m_cfThemeBtnPaste;
	s16 m_cfThemeBtnSave;
	s16 m_cfThemeBtnCancel;
	s16 m_cfThemeLblVal[4 * 4];
	s16 m_cfThemeBtnValM[4 * 4];
	s16 m_cfThemeBtnValP[4 * 4];
	s16 m_cfThemeLblValTxt[4];
//Game Settings menus
	s16 m_gameSettingsLblPage;
	s16 m_gameSettingsBtnPageM;
	s16 m_gameSettingsBtnPageP;
	s16 m_gameSettingsBtnBack;
	s16 m_gameSettingsLblTitle;
	s16 m_gameSettingsLblGameLanguage;
	s16 m_gameSettingsLblLanguage;
	s16 m_gameSettingsBtnLanguageP;
	s16 m_gameSettingsBtnLanguageM;
	s16 m_gameSettingsLblGameVideo;
	s16 m_gameSettingsLblVideo;
	s16 m_gameSettingsBtnVideoP;
	s16 m_gameSettingsBtnVideoM;
	
	s16 m_gameSettingsLblDMLGameVideo;
	s16 m_gameSettingsLblDMLVideo;
	s16 m_gameSettingsBtnDMLVideoP;
	s16 m_gameSettingsBtnDMLVideoM;

	s16 m_gameSettingsLblGClanguageVal;
	s16 m_gameSettingsLblGClanguage;
	s16 m_gameSettingsBtnGClanguageP;
	s16 m_gameSettingsBtnGClanguageM;
	
	s16 m_gameSettingsLblIOSreloadBlock;
	s16 m_gameSettingsBtnIOSreloadBlock;
	
	s16 m_gameSettingsLblAspectRatio;
	s16 m_gameSettingsLblAspectRatioVal;
	s16 m_gameSettingsBtnAspectRatioP;
	s16 m_gameSettingsBtnAspectRatioM;

	s16 m_gameSettingsLblNMM;
	s16 m_gameSettingsLblNMM_Val;
	s16 m_gameSettingsBtnNMM_P;
	s16 m_gameSettingsBtnNMM_M;

	s16 m_gameSettingsLblNoDVD;
	s16 m_gameSettingsLblNoDVD_Val;
	s16 m_gameSettingsBtnNoDVD_P;
	s16 m_gameSettingsBtnNoDVD_M;

	s16 m_gameSettingsLblDevoMemcardEmu;
	s16 m_gameSettingsBtnDevoMemcardEmu;

	s16 m_gameSettingsLblDM_Widescreen;
	s16 m_gameSettingsBtnDM_Widescreen;

	s16 m_gameSettingsLblGCLoader;
	s16 m_gameSettingsLblGCLoader_Val;
	s16 m_gameSettingsBtnGCLoader_P;
	s16 m_gameSettingsBtnGCLoader_M;

	s16 m_gameSettingsLblCustom;
	s16 m_gameSettingsBtnCustom;
	s16 m_gameSettingsLblLaunchNK;
	s16 m_gameSettingsBtnLaunchNK;

	s16 m_gameSettingsLblOcarina;
	s16 m_gameSettingsBtnOcarina;
	s16 m_gameSettingsLblVipatch;
	s16 m_gameSettingsBtnVipatch;
	s16 m_gameSettingsLblCountryPatch;
	s16 m_gameSettingsBtnCountryPatch;
	s16 m_gameSettingsLblManage;
	s16 m_gameSettingsBtnManage;
	s16 m_gameSettingsLblPatchVidModes;
	s16 m_gameSettingsLblPatchVidModesVal;
	s16 m_gameSettingsBtnPatchVidModesM;
	s16 m_gameSettingsBtnPatchVidModesP;
	s16 m_gameSettingsLblUser[3 * 2];
	s16 m_gameSettingsLblHooktype;
	s16 m_gameSettingsLblHooktypeVal;
	s16 m_gameSettingsBtnHooktypeM;
	s16 m_gameSettingsBtnHooktypeP;
	s16 m_gameSettingsLblEmulationVal;
	s16 m_gameSettingsBtnEmulationP;
	s16 m_gameSettingsBtnEmulationM;
	s16 m_gameSettingsLblEmulation;
	s16 m_gameSettingsLblDebugger;
	s16 m_gameSettingsLblDebuggerV;
	s16 m_gameSettingsBtnDebuggerP;
	s16 m_gameSettingsBtnDebuggerM;
	s16 m_gameSettingsLblCheat;
	s16 m_gameSettingsBtnCheat;
	s16 m_gameSettingsLblCategoryMain;
	s16 m_gameSettingsBtnCategoryMain;
 	s16 m_gameSettingsLblGameIOS;
 	s16 m_gameSettingsLblIOS;
 	s16 m_gameSettingsBtnIOSP;
 	s16 m_gameSettingsBtnIOSM;
	s16 m_gameSettingsLblExtractSave;
	s16 m_gameSettingsBtnExtractSave;
	s16 m_gameSettingsLblFlashSave;
	s16 m_gameSettingsBtnFlashSave;
// System Menu
	s16 m_systemBtnBack;
	s16 m_systemLblTitle;
	s16 m_systemLblVersionTxt;
	s16 m_systemLblVersion;
	s16 m_systemLblVersionRev;
	s16 m_systemLblUser[4];
	s16 m_systemBtnDownload;
	s16 m_systemLblInfo;
	s16 m_systemLblVerSelectVal;	
	s16 m_systemBtnVerSelectM;	
	s16 m_systemBtnVerSelectP;	
//Cheat menu
	s16 m_cheatBtnBack;
	s16 m_cheatBtnApply;
	s16 m_cheatBtnDownload;
	s16 m_cheatLblTitle;
	s16 m_cheatLblPage;
	s16 m_cheatBtnPageM;
	s16 m_cheatBtnPageP;
	s16 m_cheatLblItem[4];
	s16 m_cheatBtnItem[4];
	s16 m_cheatLblUser[4];
	TexData m_cheatBg;
	GCTCheats m_cheatfile;
// Gameinfo menu
	s16 m_gameinfoLblTitle;
	s16 m_gameinfoLblID;
	s16 m_gameinfoLblSynopsis;
	s16 m_gameinfoLblDev;
	s16 m_gameinfoLblRegion;
	s16 m_gameinfoLblPublisher;
	s16 m_gameinfoLblRlsdate;
	s16 m_gameinfoLblGenre;
	s16 m_gameinfoLblRating;
	s16 m_gameinfoLblWifiplayers;
	s16 m_gameinfoLblUser[5];
	s16 m_gameinfoLblControlsReq[4];
	s16 m_gameinfoLblControls[4];
	TexData m_gameinfoBg;
	TexData m_rating;
	TexData m_wifi;
	TexData m_controlsreq[4];
	TexData m_controls[4];
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

	u32 wii_btnsPressed[WPAD_MAX_WIIMOTES];
	u32 wii_btnsHeld[WPAD_MAX_WIIMOTES];
	bool wBtn_Pressed(int btn, u8 ext);
	bool wBtn_PressedChan(int btn, u8 ext, int &chan);
	bool wBtn_Held(int btn, u8 ext);
	bool wBtn_HeldChan(int btn, u8 ext, int &chan);
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

	bool wii_btnRepeat(u8 btn);
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
	bool ShowPointer(void);
	bool m_show_zone_main;
	bool m_show_zone_main2;
	bool m_show_zone_main3;
	bool m_show_zone_prev;
	bool m_show_zone_next;
	bool m_show_zone_game;

	volatile bool m_exit;
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
	volatile bool m_soundThrdBusy;
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
	typedef map<string, TexData> TexSet;
	typedef map<string, GuiSound*> SoundSet;
	struct SThemeData
	{
		TexSet texSet;
		vector<SFont> fontSet;
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
		TexData bg;
		TexData btnTexL;
		TexData btnTexR;
		TexData btnTexC;
		TexData btnTexLS;
		TexData btnTexRS;
		TexData btnTexCS;
		TexData btnAUOn;
		TexData btnAUOns;
		TexData btnAUOff;
		TexData btnAUOffs;
		TexData btnENOn;
		TexData btnENOns;
		TexData btnENOff;
		TexData btnENOffs;
		TexData btnJAOn;
		TexData btnJAOns;
		TexData btnJAOff;
		TexData btnJAOffs;
		TexData btnFROn;
		TexData btnFROns;
		TexData btnFROff;
		TexData btnFROffs;
		TexData btnDEOn;
		TexData btnDEOns;
		TexData btnDEOff;
		TexData btnDEOffs;
		TexData btnESOn;
		TexData btnESOns;
		TexData btnESOff;
		TexData btnESOffs;
		TexData btnITOn;
		TexData btnITOns;
		TexData btnITOff;
		TexData btnITOffs;
		TexData btnNLOn;
		TexData btnNLOns;
		TexData btnNLOff;
		TexData btnNLOffs;
		TexData btnPTOn;
		TexData btnPTOns;
		TexData btnPTOff;
		TexData btnPTOffs;
		TexData btnRUOn;
		TexData btnRUOns;
		TexData btnRUOff;
		TexData btnRUOffs;
		TexData btnKOOn;
		TexData btnKOOns;
		TexData btnKOOff;
		TexData btnKOOffs;
		TexData btnZHCNOn;
		TexData btnZHCNOns;
		TexData btnZHCNOff;
		TexData btnZHCNOffs;
		TexData checkboxoff;
		TexData checkboxoffs;
		TexData checkboxon;
		TexData checkboxons;
		TexData checkboxHid;
		TexData checkboxHids;
		TexData checkboxReq;
		TexData checkboxReqs;
		TexData pbarTexL;
		TexData pbarTexR;
		TexData pbarTexC;
		TexData pbarTexLS;
		TexData pbarTexRS;
		TexData pbarTexCS;
		TexData btnTexPlus;
		TexData btnTexPlusS;
		TexData btnTexMinus;
		TexData btnTexMinusS;
		GuiSound *clickSound;
		GuiSound *hoverSound;
		GuiSound *cameraSound;
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
	void _initBoot(void);
	// 
	void _initMainMenu();
	void _initErrorMenu();
	void _initConfigMenu();
	void _initConfigAdvMenu();
	void _initConfig3Menu();
	void _initConfig4Menu();
	void _initConfigSndMenu();
	void _initConfigScreenMenu();
	void _initLangSettingsMenu();
	void _initGameMenu();
	void _initDownloadMenu();
	void _initCodeMenu();
	void _initAboutMenu();
	void _initWBFSMenu();
	void _initCFThemeMenu();
	void _initGameSettingsMenu();
	void _initCheatSettingsMenu();
	void _initCheatButtons();
	void _initSourceMenu();
	void _initPluginSettingsMenu();
	void _initCategorySettingsMenu();
	void _initSystemMenu();
	void _initGameInfoMenu();
	void _initNandEmuMenu();
	void _initHomeAndExitToMenu();
	void _initCoverBanner();
	void _initExplorer();
	void _initWad();
	void _initPathsMenu();
	void _initFTP();
	//
	void _textSource(void);
	void _textPluginSettings(void);
	void _textCategorySettings(void);
	void _textCheatSettings(void);
	void _textSystem(void);
	void _textMain(void);
	void _textError(void);
	void _textConfig(void);
	void _textConfig3(void);
	void _textConfigScreen(void);
	void _textConfig4(void);
	void _textConfigAdv(void);
	void _textConfigSnd(void);
	void _textLangSettings(void);
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
	void _textBoot(void);
	void _textCoverBanner(void);
	void _textExplorer(void);
	void _textWad(void);
	void _textPaths(void);
	void _textFTP(void);
	//
	void _refreshBoot();
	void _refreshExplorer(s8 direction = 0);
	void _refreshLangSettings();
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
	void _hideLangSettings(bool instant = false);
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
	void _hideCoverBanner(bool instant = false);
	void _hideExplorer(bool instant = false);
	void _hideWad(bool instant = false);
	void _hidePaths(bool instant = false);
	void _hideFTP(bool instant = false);
	//
	void _showError(void);
	void _showMain(void);
	void _showConfigCommon(const TexData & bg, int page);
	void _showConfig(void);
	void _showConfig3(void);
	void _showConfigScreen(void);
	void _showConfig4(void);
	void _showConfigAdv(void);
	void _showConfigSnd(void);
	void _setPartition(s8 direction = 0);
	void _showLangSettings(void);
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
	void _showCoverBanner(void);
	void _showExplorer(void);
	void _showWad(void);
	void _showPaths(void);
	void _showFTP(void);

	void _clearSources(void);
	void _updateSourceBtns(void);
	void _updatePluginText(void);
	void _updatePluginCheckboxes(void);
	void _updateCheckboxes(void);
	void _updateFTP(void);
	void _getIDCats(void);
	void _setIDCats(void);
	void _setBg(const TexData &tex, const TexData &lqTex);
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
	void _downloadUrl(const char *url, u8 **dl_file, u32 *dl_size);
	void _download(string gameId = string());
	void _downloadBnr(const char *gameID);
	bool _LangSettings(void);
	void _code(void);
	void _about(bool help = false);
	bool _wbfsOp(WBFS_OP op);
	void _cfTheme(void);
	void _system(void);
	void _gameinfo(void);
	void _gameSettings(void);
	void _CoverBanner(void);
	void _Explorer(void);
	const char *_FolderExplorer(const char *startPath);
	void _Wad(const char *wad_path = NULL, bool autoInstall = false);
	void _CheatSettings();
	bool _Source();
	void _PluginSettings();
	void _CategorySettings(bool fromGameSet = false);
	bool _Home();
	void _FTP();
	bool _FTP_Loop();
	bool _ExitTo();
	bool _Boot();
	void _Paths();
	void _mainLoopCommon(bool withCF = false, bool adjusting = false);
public:
	void directlaunch(const char *GameID);
private:
	bool m_use_wifi_gecko;
	bool m_use_sd_logging;
	bool init_network;
	bool m_init_ftp;
	bool m_ftp_inited;
	void _netInit();
	bool _loadFile(u8 * &buffer, u32 &size, const char *path, const char *file);
	int _loadIOS(u8 ios, int userIOS, string id, bool RealNAND_Channels = false);
	void _launch(const dir_discHdr *hdr);
	void _launchGame(dir_discHdr *hdr, bool dvd);
	void _launchChannel(dir_discHdr *hdr);
	void _launchHomebrew(const char *filepath, vector<string> arguments);
	void _launchGC(dir_discHdr *hdr, bool disc);
	void _launchShutdown();
	void _setAA(int aa);
	void _loadCFCfg();
	void _loadCFLayout(int version, bool forceAA = false, bool otherScrnFmt = false);
	Vector3D _getCFV3D(const string &domain, const string &key, const Vector3D &def, bool otherScrnFmt = false);
	int _getCFInt(const string &domain, const string &key, int def, bool otherScrnFmt = false);
	float _getCFFloat(const string &domain, const string &key, float def, bool otherScrnFmt = false);
	void _cfParam(bool inc, int i, const SCFParamDesc &p, int cfVersion, bool wide);
	void _buildMenus(void);
	void _cleanupDefaultFont();
	void _Theme_Cleanup();
	const char *_getId(void);
	const char *_domainFromView(void);
	const char *_cfDomain(bool selected = false);
	void UpdateCache(u32 view = COVERFLOW_MAX);
	void MIOSisDML();
	void RemoveCover(const char *id);
	SFont _font(const char *domain, const char *key, u32 fontSize, u32 lineSpacing, u32 weight, u32 index, const char *genKey);
	TexData _texture(const char *domain, const char *key, TexData &def, bool freeDef = true);
	vector<TexData> _textures(const char *domain, const char *key);
	void _showWaitMessage();
public:
	void _hideWaitMessage();
	bool m_Emulator_boot;
	void GC_Messenger(int message, int info, char *cinfo);

	/* general thread updating stuff */
	u64 m_thrdTotal;
	void update_pThread(u64 added);
private:
	static int _pThread(void *obj);
	void _start_pThread(void);
	void _stop_pThread(void);
	lwp_t m_thrdPtr;
	volatile bool m_thrdInstalling;
	volatile bool m_thrdUpdated;
	volatile bool m_thrdDone;
	vu64 m_thrdWritten;

	GuiSound *_sound(CMenu::SoundSet &soundSet, const char *domain, const char *key, const u8 * snd, u32 len, const char *name, bool isAllocated);
	GuiSound *_sound(CMenu::SoundSet &soundSet, const char *domain, const char *key, const char *name);
	u16 _textStyle(const char *domain, const char *key, u16 def);
	s16 _addButton(const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color);
	s16 _addPicButton(const char *domain, TexData &texNormal, TexData &texSelected, int x, int y, u32 width, u32 height);
	s16 _addTitle(const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, s16 style);
	s16 _addText(const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, s16 style);
	s16 _addLabel(const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, s16 style);
	s16 _addLabel(const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, s16 style, TexData &bg);
	s16 _addProgressBar(const char *domain, int x, int y, u32 width, u32 height);
	void _setHideAnim(s16 id, const char *domain, int dx, int dy, float scaleX, float scaleY);
	void _addUserLabels(s16 *ids, u32 size, const char *domain);
	void _addUserLabels(s16 *ids, u32 start, u32 size, const char *domain);
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
	void LoadView(void);
	void _getGrabStatus(void);
	static void _addDiscProgress(int status, int total, void *user_data);
	static void _ShowProgress(int dumpstat, int dumpprog, int filestat, int fileprog, int files, int folders, const char *tmess, void *user_data);
	static int _gameInstaller(void *obj);
	static int _GCcopyGame(void *obj);
	int _GCgameInstaller();
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
	int _FindEmuPart(string &emuPath, bool searchvalid);
	bool _checkSave(string id, bool nand);
	bool _TestEmuNand(int epart, const char *path, bool indept);

	static u32 _downloadCheatFileAsync(void *obj);
	static u32 _downloadBannerAsync(void *obj);
	static u32 _downloadUrlAsync(void *obj);

	void _playGameSound(void);
	void CheckGameSoundThread(void);
	static void _gameSoundThread(CMenu *m);

	static void _load_installed_cioses();

	struct SOption { const char id[10]; const wchar_t text[16]; };
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
	static const SOption _GlobalGCLoaders[2];
	static const SOption _GCLoader[3];
	static const SOption _vidModePatch[4];
	static const SOption _debugger[3];
	static const SOption _hooktype[8];
	static const SOption _exitTo[5];
	static map<u8, u8> _installed_cios;
	typedef map<u8, u8>::iterator CIOSItr;
	static int _version[9];
	static const SCFParamDesc _cfParams[];
	static const int _nbCfgPages;
	static const u32 SVN_REV_NUM;
};

extern CMenu mainMenu;

#define ARRAY_SIZE(a)		(sizeof a / sizeof a[0])

#endif // !defined(__MENU_HPP)
