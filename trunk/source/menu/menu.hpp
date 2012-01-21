#ifndef __MENU_HPP
#define __MENU_HPP
//#define SHOWMEM 1
#include <wiiuse/wpad.h>
#include <ogc/pad.h>

#include "safe_vector.hpp"
#include "cachedlist.hpp"

#include <map>
#include "gui_sound.h"
#include "cursor.hpp"
#include "gui.hpp"
#include "coverflow.hpp"
#include "fanart.hpp"
#include "loader/disc.h"
#include "btnmap.h"
#include "banner.h"
#include "channels.h"
#include "gct.h"
#include "DeviceHandler.hpp"
#include "musicplayer.h"

//Also in wbfs.h
#define PART_FS_WBFS 0
#define PART_FS_FAT  1
#define PART_FS_NTFS 2
#define PART_FS_EXT  3

extern "C" {extern u8 currentPartition;}
extern bool bootHB;

class CMenu
{
public:
	CMenu(CVideo &vid);
	~CMenu(void) {cleanup();}
	void init(void);
	void error(const wstringEx &msg);
	int main(void);
	void cleanup(bool ios_reload = false);
	u32 m_current_view;
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
	CachedList<dir_discHdr> m_gameList;
	Config m_cfg;
	Config m_loc;
	Config m_cat;
	Config m_gcfg1;
	Config m_gcfg2;
	Config m_theme;
	Config m_titles;
	Config m_version;
	Channels m_channels;
	safe_vector<std::string> m_homebrewArgs;
	SmartBuf m_base_font;
	u32 m_base_font_size;
	u8 m_aa;
	bool m_directLaunch;
	bool m_gamelistdump;
	bool m_locked;
	bool m_favorites;
	s16 m_showtimer;
	std::string m_curLanguage;
	std::string m_curGameId;
	std::string m_curChanId;

	u8 m_numCFVersions;
	// 
	std::string m_themeDataDir;
	std::string m_appDir;
	std::string m_dataDir;
	std::string m_picDir;
	std::string m_boxPicDir;
	std::string m_cacheDir;
	std::string m_themeDir;
	std::string m_musicDir;
	std::string m_txtCheatDir;
	std::string m_cheatDir;
	std::string m_wipDir;
	std::string m_videoDir;
	std::string m_fanartDir;
	std::string m_screenshotDir;
	std::string m_settingsDir;
	std::string m_languagesDir;
	std::string m_listCacheDir;
	/* Updates */
	const char* m_app_update_url;
	const char* m_data_update_url;
	std::string m_dol;
	std::string m_app_update_zip;
	u32 m_app_update_size;
	std::string m_data_update_zip;
	u32 m_data_update_size;
	std::string m_ver;
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
	STexture m_categoryBg;
	// 
	u32 m_errorLblMessage;
	u32 m_errorLblIcon;
	u32 m_errorLblUser[4];
//Main Coverflow
	u32 m_mainBtnConfig;
	u32 m_mainBtnInfo;
	u32 m_mainBtnFavoritesOn;
	u32 m_mainBtnFavoritesOff;
	u32 m_mainLblLetter;
#ifdef SHOWMEM
	u32 m_mem2FreeSize;
#endif
	u32 m_mainLblNotice;
	u32 m_mainBtnNext;
	u32 m_mainBtnPrev;
	u32 m_mainBtnQuit;
	u32 m_mainBtnDVD;
	u32 m_mainBtnUsb;
	u32 m_mainBtnChannel;
	u32 m_mainBtnHomebrew;
	u32 m_mainBtnInit;
	u32 m_mainBtnInit2;
	u32 m_mainLblInit;
	u32 m_mainLblUser[6];
//Main Config menus
	u32 m_configLblPage;
	u32 m_configBtnPageM;
	u32 m_configBtnPageP;
	u32 m_configBtnBack;
	u32 m_configLblTitle;
	u32 m_configLblDownload;
	u32 m_configBtnDownload; 
	u32 m_configLblParental;
	u32 m_configBtnUnlock;
	u32 m_configBtnSetCode;
	u32 m_configLblPartitionName;
	u32 m_configLblPartition;
	u32 m_configBtnPartitionP;
	u32 m_configBtnPartitionM;
	u32 m_configBtnEmulation;
	u32 m_configLblEmulation;
	u32 m_configLblUser[4];
	u32 m_configAdvLblTheme;
	u32 m_configAdvLblCurTheme;
	u32 m_configAdvBtnCurThemeM;
	u32 m_configAdvBtnCurThemeP;
	u32 m_configAdvLblLanguage;
	u32 m_configAdvLblCurLanguage;
	u32 m_configAdvBtnCurLanguageM;
	u32 m_configAdvBtnCurLanguageP;
	u32 m_configAdvLblCFTheme;
	u32 m_configAdvBtnCFTheme;
	u32 m_configAdvLblInstall;
	u32 m_configAdvBtnInstall;
	u32 m_configAdvLblUser[4];
	u32 m_config3LblGameLanguage;
	u32 m_config3LblLanguage;
	u32 m_config3BtnLanguageP;
	u32 m_config3BtnLanguageM;
	u32 m_config3LblGameVideo;
	u32 m_config3LblVideo;
	u32 m_config3BtnVideoP;
	u32 m_config3BtnVideoM;
	u32 m_config3LblOcarina;
	u32 m_config3BtnOcarina;
	u32 m_config3LblAsyncNet;
	u32 m_config3BtnAsyncNet;
	u32 m_config3LblUser[4];
	u32 m_config4LblReturnTo;
	u32 m_config4LblReturnToVal;
	u32 m_config4BtnReturnToM;
	u32 m_config4BtnReturnToP;
	u32 m_config4LblHome;
	u32 m_config4BtnHome;
	u32 m_config4LblSaveFavMode;
	u32 m_config4BtnSaveFavMode;
	u32 m_config4LblCategoryOnBoot;
	u32 m_config4BtnCategoryOnBoot;
	u32 m_config4LblUser[4];
	u32 m_configSndLblBnrVol;
	u32 m_configSndLblBnrVolVal;
	u32 m_configSndBtnBnrVolP;
	u32 m_configSndBtnBnrVolM;
	u32 m_configSndLblMusicVol;
	u32 m_configSndLblMusicVolVal;
	u32 m_configSndBtnMusicVolP;
	u32 m_configSndBtnMusicVolM;
	u32 m_configSndLblGuiVol;
	u32 m_configSndLblGuiVolVal;
	u32 m_configSndBtnGuiVolP;
	u32 m_configSndBtnGuiVolM;
	u32 m_configSndLblCFVol;
	u32 m_configSndLblCFVolVal;
	u32 m_configSndBtnCFVolP;
	u32 m_configSndBtnCFVolM;
	u32 m_configSndLblUser[4];
	u32 m_configScreenLblTVHeight;
	u32 m_configScreenLblTVHeightVal;
	u32 m_configScreenBtnTVHeightP;
	u32 m_configScreenBtnTVHeightM;
	u32 m_configScreenLblTVWidth;
	u32 m_configScreenLblTVWidthVal;
	u32 m_configScreenBtnTVWidthP;
	u32 m_configScreenBtnTVWidthM;
	u32 m_configScreenLblTVX;
	u32 m_configScreenLblTVXVal;
	u32 m_configScreenBtnTVXM;
	u32 m_configScreenBtnTVXP;
	u32 m_configScreenLblTVY;
	u32 m_configScreenLblTVYVal;
	u32 m_configScreenBtnTVYM;
	u32 m_configScreenBtnTVYP;
	u32 m_configScreenLblUser[4];
//Download menu
	u32 m_downloadLblTitle;
	u32 m_downloadPBar;
	u32 m_downloadBtnCancel;
	u32 m_downloadBtnAll;
	u32 m_downloadBtnMissing;
	u32 m_downloadBtnGameTDBDownload;
	u32 m_downloadLblGameTDBDownload;
	u32 m_downloadLblMessage[2];
	u32 m_downloadLblCovers;
	u32 m_downloadLblGameTDB;
	u32 m_downloadLblUser[4];
	u32 m_downloadBtnVersion;
	static s8 _versionDownloaderInit(CMenu *m);
	static s8 _versionTxtDownloaderInit(CMenu *m);
	s8 _versionDownloader();
	s8 _versionTxtDownloader();
//Game menu
	u32 m_gameLblInfo;
	u32 m_gameBtnFavoriteOn;
	u32 m_gameBtnFavoriteOff;
	u32 m_gameBtnAdultOn;
	u32 m_gameBtnAdultOff;
	u32 m_gameBtnPlay;
	u32 m_gameBtnDelete;
	u32 m_gameBtnSettings;
	u32 m_gameBtnBack;
	u32 m_gameLblUser[4];
// Parental code menu	
	u32 m_codeLblTitle;
	u32 m_codeBtnKey[10];
	u32 m_codeBtnBack;
	u32 m_codeBtnErase;
	u32 m_codeLblUser[4];
//About menu
	u32 m_aboutLblTitle;
	u32 m_aboutLblInfo;
	u32 m_aboutLblUser[4];
	u32 m_aboutLblIOS;
	u32 m_aboutBtnSystem;
//menu_wbfs
	u32 m_wbfsLblTitle;
	u32 m_wbfsPBar;
	u32 m_wbfsBtnBack;
	u32 m_wbfsBtnGo;
	u32 m_wbfsLblDialog;
	u32 m_wbfsLblMessage;
	u32 m_wbfsLblUser[4];
//Theme Adjust menus
	u32 m_cfThemeBtnAlt;
	u32 m_cfThemeBtnSelect;
	u32 m_cfThemeBtnWide;
	u32 m_cfThemeLblParam;
	u32 m_cfThemeBtnParamM;
	u32 m_cfThemeBtnParamP;
	u32 m_cfThemeBtnCopy;
	u32 m_cfThemeBtnPaste;
	u32 m_cfThemeBtnSave;
	u32 m_cfThemeBtnCancel;
	u32 m_cfThemeLblVal[4 * 4];
	u32 m_cfThemeBtnValM[4 * 4];
	u32 m_cfThemeBtnValP[4 * 4];
	u32 m_cfThemeLblValTxt[4];
//Game Settings menus
	u32 m_gameSettingsLblPage;
	u32 m_gameSettingsBtnPageM;
	u32 m_gameSettingsBtnPageP;
	u32 m_gameSettingsBtnBack;
	u32 m_gameSettingsLblTitle;
	u32 m_gameSettingsLblGameLanguage;
	u32 m_gameSettingsLblLanguage;
	u32 m_gameSettingsBtnLanguageP;
	u32 m_gameSettingsBtnLanguageM;
	u32 m_gameSettingsLblGameVideo;
	u32 m_gameSettingsLblVideo;
	u32 m_gameSettingsBtnVideoP;
	u32 m_gameSettingsBtnVideoM;
	u32 m_gameSettingsLblOcarina;
	u32 m_gameSettingsBtnOcarina;
	u32 m_gameSettingsLblVipatch;
	u32 m_gameSettingsBtnVipatch;
	u32 m_gameSettingsLblCountryPatch;
	u32 m_gameSettingsBtnCountryPatch;
	u32 m_gameSettingsLblCover;
	u32 m_gameSettingsBtnCover;
	u32 m_gameSettingsLblPatchVidModes;
	u32 m_gameSettingsLblPatchVidModesVal;
	u32 m_gameSettingsBtnPatchVidModesM;
	u32 m_gameSettingsBtnPatchVidModesP;
	u32 m_gameSettingsLblUser[3 * 2];
	u32 m_gameSettingsLblHooktype;
	u32 m_gameSettingsLblHooktypeVal;
	u32 m_gameSettingsBtnHooktypeM;
	u32 m_gameSettingsBtnHooktypeP;
	u32 m_gameSettingsBtnEmulation;
	u32 m_gameSettingsLblEmulation;
	u32 m_gameSettingsLblDebugger;
	u32 m_gameSettingsLblDebuggerV;
	u32 m_gameSettingsBtnDebuggerP;
	u32 m_gameSettingsBtnDebuggerM;
	u32 m_gameSettingsLblCheat;
	u32 m_gameSettingsBtnCheat;
	u32 m_gameSettingsLblCategoryMain;
	u32 m_gameSettingsBtnCategoryMain;
	u32 m_gameSettingsLblCategory[12];
	u32 m_gameSettingsBtnCategory[12];
	u32 m_gameCategoryPage;
	u32 m_gameSettingsPage;
// System Menu
	u32 m_systemBtnBack;
	u32 m_systemLblTitle;
	u32 m_systemLblVersionTxt;
	u32 m_systemLblVersion;
	u32 m_systemLblVersionRev;
	u32 m_systemLblUser[4];
	u32 m_systemBtnDownload;
	u32 m_systemLblInfo;
	u32 m_systemLblVerSelectVal;	
	u32 m_systemBtnVerSelectM;	
	u32 m_systemBtnVerSelectP;	
//Cheat menu
	u32 m_cheatBtnBack;
	u32 m_cheatBtnApply;
	u32 m_cheatBtnDownload;
	u32 m_cheatLblTitle;
	u32 m_cheatLblPage;
	u32 m_cheatBtnPageM;
	u32 m_cheatBtnPageP;
	u32 m_cheatLblItem[4];
	u32 m_cheatBtnItem[4];
	u32 m_cheatSettingsPage;
	u32 m_cheatLblUser[4];
	STexture m_cheatBg;
	GCTCheats m_cheatfile;
// Gameinfo menu
	u32 m_gameinfoLblTitle;
	u32 m_gameinfoLblID;
	u32 m_gameinfoLblSynopsis;
	u32 m_gameinfoLblDev;
	u32 m_gameinfoLblRegion;
	u32 m_gameinfoLblPublisher;
	u32 m_gameinfoLblRlsdate;
	u32 m_gameinfoLblGenre;
	u32 m_gameinfoLblRating;
	u32 m_gameinfoLblWifiplayers;
	u32 m_gameinfoLblUser[5];
	u32 m_gameinfoLblControlsReq[4];
	u32 m_gameinfoLblControls[4];
	STexture m_gameinfoBg;
	STexture m_rating;
	STexture m_wifi;
	STexture m_controlsreq[4];
	STexture m_controls[4];
// Category menu
	u32 m_categoryBtn[12];
	u32 m_categoryBtnBack;
	u32 m_categoryLblUser[4];
	u8 m_max_categories;
	u8 m_category;
// Zones
	SZone m_mainPrevZone;
	SZone m_mainNextZone;
	SZone m_mainButtonsZone;
	SZone m_mainButtonsZone2;
	SZone m_mainButtonsZone3;
	SZone m_gameButtonsZone;
	bool m_reload;
	bool m_initialCoverStatusComplete;
	
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

	void SetupInput(void);
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
	std::string m_coverDLGameId;
	mutex_t m_mutex;
	wstringEx m_thrdMessage;
	volatile float m_thrdProgress;
	volatile bool m_thrdMessageAdded;
	volatile bool m_gameSelected;
	GuiSound m_gameSound;
	SmartGuiSound m_cameraSound;
	dir_discHdr *m_gameSoundHdr;
	lwp_t m_gameSoundThread;
	bool m_gamesound_changed;
	u8 m_bnrSndVol;
	
	bool m_video_playing;

private:
	enum WBFS_OP { WO_ADD_GAME, WO_REMOVE_GAME, WO_FORMAT };
	typedef std::pair<std::string, u32> FontDesc;
	typedef std::map<FontDesc, SFont> FontSet;
	typedef std::map<std::string, STexture> TexSet;
	typedef std::map<std::string, SmartGuiSound > SoundSet;
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
		STexture bg;
		STexture btnTexL;
		STexture btnTexR;
		STexture btnTexC;
		STexture btnTexLS;
		STexture btnTexRS;
		STexture btnTexCS;
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
	struct SCFParamDesc
	{
		enum { PDT_EMPTY, PDT_FLOAT, PDT_V3D, PDT_COLOR, PDT_BOOL, PDT_INT, PDT_TXTSTYLE } paramType[4];
		enum { PDD_BOTH, PDD_NORMAL, PDD_SELECTED } domain;
		bool scrnFmt;
		const char name[32];
		const char valName[4][64];
		const char key[4][48];
		float step[4];
		float minMaxVal[4][2];
	};
	// 
	bool _loadChannelList(void);
	bool _loadList(void);
	bool _loadHomebrewList(void);
	bool _loadGameList(void);
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
	void _initCategorySettingsMenu(SThemeData &theme);
	void _initSystemMenu(SThemeData &theme);
	void _initGameInfoMenu(SThemeData &theme);
	//
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
	//
	void _hideCheatSettings(bool instant = false);
	void _hideError(bool instant = false);
	void _hideMain(bool instant = false);
	void _hideConfig(bool instant = false);
	void _hideConfig3(bool instant = false);
	void _hideConfigScreen(bool instant = false);
	void _hideConfig4(bool instant = false);
	void _hideConfigAdv(bool instant = false);
	void _hideConfigSnd(bool instant = false);
	void _hideGame(bool instant = false);
	void _hideDownload(bool instant = false);
	void _hideCode(bool instant = false);
	void _hideAbout(bool instant = false);
	void _hideWBFS(bool instant = false);
	void _hideCFTheme(bool instant = false);
	void _hideGameSettings(bool instant = false);
	void _hideCategorySettings(bool instant = false);
	void _hideSystem(bool instant = false);
	void _hideGameInfo(bool instant = false);
	void _hideCheatDownload(bool instant = false);
	//
	void _showError(void);
	void _showMain(void);
	void _showConfig(void);
	void _showConfig3(void);
	void _showConfigScreen(void);
	void _showConfig4(void);
	void _showConfigAdv(void);
	void _showConfigSnd(void);
	void _showGame(void);
	void _showDownload(void);
	void _showCode(void);
	void _showAbout(void);
	void _showCategorySettings(void);
	void _showCheatSettings(void);
	void _showSystem(void);
	void _showGameInfo(void);
	void _showWBFS(WBFS_OP op);
	void _showCFTheme(u32 curParam, int version, bool wide);
	void _showGameSettings(void);
	void _showCheatDownload(void);
	void _setBg(const STexture &tex, const STexture &lqTex);
	void _updateBg(void);
	void _drawBg(void);
	void _updateText(void);
	// 
	void _config(int page);
	int _config1(void);
	int _config3(void);
	int _configScreen(void);
	int _config4(void);
	int _configAdv(void);
	int _configSnd(void);
	void _game(bool launch = false);
	void _download(std::string gameId = std::string());
	bool _code(char code[4], bool erase = false);
	void _about(void);
	bool _wbfsOp(WBFS_OP op);
	void _cfTheme(void);
	void _system(void);
	void _gameinfo(void);
	void _gameSettings(void);
	void _CheatSettings();
	void _CategorySettings();
	//
	void _mainLoopCommon(bool withCF = false, bool blockReboot = false, bool adjusting = false);
	// 
	safe_vector<dir_discHdr> _searchGamesByID(const char *gameId);
/* 	safe_vector<dir_discHdr> _searchGamesByTitle(wchar_t letter);
	safe_vector<dir_discHdr> _searchGamesByType(const char type);
	safe_vector<dir_discHdr> _searchGamesByRegion(const char region); */
public:
	void _directlaunch(const std::string &id);
private:
	bool _loadFile(SmartBuf &buffer, u32 &size, const char *path, const char *file);
	void _launch(dir_discHdr *hdr);
	void _launchGame(dir_discHdr *hdr, bool dvd);
	void _launchChannel(dir_discHdr *hdr);
	void _launchHomebrew(const char *filepath, safe_vector<std::string> arguments);
	void _setAA(int aa);
	void _loadCFCfg(SThemeData &theme);
	void _loadCFLayout(int version, bool forceAA = false, bool otherScrnFmt = false);
	Vector3D _getCFV3D(const std::string &domain, const std::string &key, const Vector3D &def, bool otherScrnFmt = false);
	int _getCFInt(const std::string &domain, const std::string &key, int def, bool otherScrnFmt = false);
	float _getCFFloat(const std::string &domain, const std::string &key, float def, bool otherScrnFmt = false);
	void _cfParam(bool inc, int i, const SCFParamDesc &p, int cfVersion, bool wide);
	void _buildMenus(void);
	void _loadDefaultFont(bool korean);
	void _cleanupDefaultFont();
	const char *_domainFromView(void);
	void UpdateCache(u32 view = COVERFLOW_MAX);
	SFont _font(CMenu::FontSet &fontSet, const char *domain, const char *key, u32 fontSize, u32 lineSpacing, u32 weight, u32 index, const char *genKey);
	STexture _texture(TexSet &texSet, const char *domain, const char *key, STexture def);
	safe_vector<STexture> _textures(TexSet &texSet, const char *domain, const char *key);
	void _showWaitMessage();
public:
	void _hideWaitMessage(bool force = false);
private:
	SmartGuiSound _sound(CMenu::SoundSet &soundSet, const char *domain, const char *key, const u8 * snd, u32 len, string name, bool isAllocated);
	SmartGuiSound _sound(CMenu::SoundSet &soundSet, const char *domain, const char *key, string name);
	u16 _textStyle(const char *domain, const char *key, u16 def);
	u32 _addButton(SThemeData &theme, const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color);
	u32 _addPicButton(SThemeData &theme, const char *domain, STexture &texNormal, STexture &texSelected, int x, int y, u32 width, u32 height);
	u32 _addLabel(SThemeData &theme, const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, u16 style);
	u32 _addLabel(SThemeData &theme, const char *domain, SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, u16 style, STexture &bg);
	u32 _addProgressBar(SThemeData &theme, const char *domain, int x, int y, u32 width, u32 height);
	void _setHideAnim(u32 id, const char *domain, int dx, int dy, float scaleX, float scaleY);
	void _addUserLabels(CMenu::SThemeData &theme, u32 *ids, u32 size, const char *domain);
	void _addUserLabels(CMenu::SThemeData &theme, u32 *ids, u32 start, u32 size, const char *domain);
	// 
	const wstringEx _t(const char *key, const wchar_t *def = L"") { return m_loc.getWString(m_curLanguage, key, def); }
	const wstringEx _fmt(const char *key, const wchar_t *def);
	wstringEx _getNoticeTranslation(int sorting, wstringEx curLetter);
	// 
	void _setThrdMsg(const wstringEx &msg, float progress);
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
	static int GetCoverStatusAsync(CMenu *m);
	void LoadView(void);
	void _getGrabStatus(void);
	static void _addDiscProgress(int status, int total, void *user_data);
	static int _gameInstaller(void *obj);
	wstringEx _optBoolToString(int b);
	void _stopSounds(void);
	//
	static u32 _downloadCheatFileAsync(void *obj);
	// 
	void _playGameSound(void);
	void CheckGameSoundThread(bool force = false);
	void CheckThreads(bool force = false);
	static void _gameSoundThread(CMenu *m);
	//
	static void _load_installed_cioses();
	//
	struct SOption { const char id[10]; const wchar_t text[16]; };
	static const string _translations[23];
	static const SOption _languages[11];
	static const SOption _videoModes[7];
	static const SOption _vidModePatch[4];
	static const SOption _hooktype[8];
	static const SOption _exitTo[5];
	static std::map<u8, u8> _installed_cios;
	typedef std::map<u8, u8>::iterator CIOSItr;
	static int _version[9];
	static const SCFParamDesc _cfParams[];
	static const int _nbCfgPages;
};

#define ARRAY_SIZE(a)		(sizeof a / sizeof a[0])

#endif // !defined(__MENU_HPP)
