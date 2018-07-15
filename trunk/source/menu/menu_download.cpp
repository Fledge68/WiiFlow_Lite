#include <network.h>
#include <ogc/lwp_watchdog.h>
#include <time.h>
#include <fstream>

#include "menu.hpp"
#include "svnrev.h"
#include "types.h"
#include "lockMutex.hpp"
#include "channel/nand.hpp"
#include "devicemounter/usbstorage.h"
#include "gui/GameTDB.hpp"
#include "gui/pngu.h"
#include "loader/fs.h"
#include "loader/wbfs.h"
#include "loader/wdvd.h"
#include "network/http.h"
#include "unzip/ZipFile.h"

#define TAG_GAME_ID		"{gameid}"
#define TAG_LOC			"{loc}"
#define TAG_CONSOLE		"{console}"

#define GAMETDB_URL		"http://www.gametdb.com/wiitdb.zip?LANG=%s&FALLBACK=TRUE&WIIWARE=TRUE&GAMECUBE=TRUE"
#define CUSTOM_BANNER_URL	"http://banner.rc24.xyz/{gameid}.bnr"

static const char FMT_BPIC_URL[] = "http://art.gametdb.com/{console}/coverfullHQ/{loc}/{gameid}.png"\
"|http://art.gametdb.com/{console}/coverfull/{loc}/{gameid}.png";
static const char FMT_PIC_URL[] = "http://art.gametdb.com/{console}/cover/{loc}/{gameid}.png";
static const char FMT_CBPIC_URL[] = "http://art.gametdb.com/{console}/coverfullHQ2/{loc}/{gameid}.png";
static const char FMT_CPIC_URL[] = "http://art.gametdb.com/{console}/cover2/{loc}/{gameid}.png";

static block download = { 0, 0 };
static bool settingsmenu = false;

void CMenu::_hideSettings(bool instant)
{
	m_btnMgr.hide(m_downloadLblSetTitle, instant);
	m_btnMgr.hide(m_downloadLblCoverPrio, instant);
	m_btnMgr.hide(m_downloadLblPrio, instant);
	m_btnMgr.hide(m_downloadBtnPrioM, instant);
	m_btnMgr.hide(m_downloadBtnPrioP, instant);
	m_btnMgr.hide(m_downloadLblRegion, instant);
	m_btnMgr.hide(m_downloadBtnEN, instant);
	m_btnMgr.hide(m_downloadBtnJA, instant);
	m_btnMgr.hide(m_downloadBtnFR, instant);
	m_btnMgr.hide(m_downloadBtnDE, instant);
	m_btnMgr.hide(m_downloadBtnES, instant);
	m_btnMgr.hide(m_downloadBtnIT, instant);
	m_btnMgr.hide(m_downloadBtnNL, instant);
	m_btnMgr.hide(m_downloadBtnPT, instant);
	m_btnMgr.hide(m_downloadBtnRU, instant);
	m_btnMgr.hide(m_downloadBtnKO, instant);
	m_btnMgr.hide(m_downloadBtnZHCN, instant);
	m_btnMgr.hide(m_downloadBtnAU, instant);
	m_btnMgr.hide(m_downloadBtnENs, instant);
	m_btnMgr.hide(m_downloadBtnJAs, instant);
	m_btnMgr.hide(m_downloadBtnFRs, instant);
	m_btnMgr.hide(m_downloadBtnDEs, instant);
	m_btnMgr.hide(m_downloadBtnESs, instant);
	m_btnMgr.hide(m_downloadBtnITs, instant);
	m_btnMgr.hide(m_downloadBtnNLs, instant);
	m_btnMgr.hide(m_downloadBtnPTs, instant);
	m_btnMgr.hide(m_downloadBtnRUs, instant);
	m_btnMgr.hide(m_downloadBtnKOs, instant);
	m_btnMgr.hide(m_downloadBtnZHCNs, instant);
	m_btnMgr.hide(m_downloadBtnAUs, instant);
	m_btnMgr.hide(m_downloadBtnBack, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_downloadLblUser); ++i)
		if(m_downloadLblUser[i] != -1)
			m_btnMgr.hide(m_downloadLblUser[i], instant);
}

void CMenu::_showSettings()
{
	_hideDownload();
	for(u8 i = 0; i < ARRAY_SIZE(m_downloadLblUser); ++i)
		if(m_downloadLblUser[i] != -1)
			m_btnMgr.show(m_downloadLblUser[i]);
	m_btnMgr.show(m_downloadLblSetTitle);
	m_btnMgr.show(m_downloadLblCoverPrio);
	m_btnMgr.show(m_downloadLblPrio);
	m_btnMgr.show(m_downloadBtnPrioM);
	m_btnMgr.show(m_downloadBtnPrioP);
	m_btnMgr.show(m_downloadLblRegion);	
	if( m_downloadPrioVal&C_TYPE_EN )
		m_btnMgr.show(m_downloadBtnENs);
	else
		m_btnMgr.show(m_downloadBtnEN);
		
	if( m_downloadPrioVal&C_TYPE_JA )
		m_btnMgr.show(m_downloadBtnJAs);
	else
		m_btnMgr.show(m_downloadBtnJA);
		
	if( m_downloadPrioVal&C_TYPE_FR )
		m_btnMgr.show(m_downloadBtnFRs);
	else
		m_btnMgr.show(m_downloadBtnFR);
	
	if( m_downloadPrioVal&C_TYPE_DE )
		m_btnMgr.show(m_downloadBtnDEs);
	else
		m_btnMgr.show(m_downloadBtnDE);	
		
	if( m_downloadPrioVal&C_TYPE_ES )
		m_btnMgr.show(m_downloadBtnESs);
	else
		m_btnMgr.show(m_downloadBtnES);
	
	if( m_downloadPrioVal&C_TYPE_IT )
		m_btnMgr.show(m_downloadBtnITs);
	else
		m_btnMgr.show(m_downloadBtnIT);
		
	if( m_downloadPrioVal&C_TYPE_NL )
		m_btnMgr.show(m_downloadBtnNLs);
	else
		m_btnMgr.show(m_downloadBtnNL);
	
	if( m_downloadPrioVal&C_TYPE_PT )
		m_btnMgr.show(m_downloadBtnPTs);
	else
		m_btnMgr.show(m_downloadBtnPT);	
		
	if( m_downloadPrioVal&C_TYPE_RU )
		m_btnMgr.show(m_downloadBtnRUs);
	else
		m_btnMgr.show(m_downloadBtnRU);
	
	if( m_downloadPrioVal&C_TYPE_KO )
		m_btnMgr.show(m_downloadBtnKOs);
	else
		m_btnMgr.show(m_downloadBtnKO);

	if( m_downloadPrioVal&C_TYPE_ZHCN )
		m_btnMgr.show(m_downloadBtnZHCNs);
	else
		m_btnMgr.show(m_downloadBtnZHCN);
	
	if( m_downloadPrioVal&C_TYPE_AU )
		m_btnMgr.show(m_downloadBtnAUs);
	else
		m_btnMgr.show(m_downloadBtnAU);
		
	m_btnMgr.show(m_downloadBtnBack);
	
	if( m_downloadPrioVal&C_TYPE_ONOR )
	{
		m_btnMgr.setText(m_downloadLblPrio, _t("dl24", L"Custom only"));	
	}
	else
	{
		if( m_downloadPrioVal&C_TYPE_ONCU )
		{
			if( m_downloadPrioVal&C_TYPE_PRIOA )
			{
				if(m_downloadPrioVal&C_TYPE_PRIOB )
				{
					m_btnMgr.setText(m_downloadLblPrio, _t("dl23", L"Custom/Custom"));
				}
				else						
				{
					m_btnMgr.setText(m_downloadLblPrio, _t("dl22", L"Custom/Original"));
				}
			}
			else
			{
				if(m_downloadPrioVal&C_TYPE_PRIOB )
				{
					m_btnMgr.setText(m_downloadLblPrio, _t("dl21", L"Original/Custom"));
				}
				else
				{
					m_btnMgr.setText(m_downloadLblPrio, _t("dl20", L"Original/Original"));
				}
			}
		}
		else
		{
			m_btnMgr.setText(m_downloadLblPrio, _t("dl19", L"Original only"));
		}
	}
}

void CMenu::_hideDownload(bool instant)
{
	m_btnMgr.hide(m_downloadLblTitle, instant);
	m_btnMgr.hide(m_downloadBtnCancel, instant);
	m_btnMgr.hide(m_downloadBtnAll, instant);
	m_btnMgr.hide(m_downloadBtnMissing, instant);
	m_btnMgr.hide(m_downloadLblCoverSet, instant);
	m_btnMgr.hide(m_downloadBtnCoverSet, instant);
	m_btnMgr.hide(m_downloadBtnGameTDBDownload, instant);
	m_btnMgr.hide(m_downloadPBar, instant);
	m_btnMgr.hide(m_downloadLblMessage[0], 0, 0, -2.f, 0.f, instant);
	m_btnMgr.hide(m_downloadLblMessage[1], 0, 0, -2.f, 0.f, instant);
	m_btnMgr.hide(m_downloadLblCovers, instant);
	m_btnMgr.hide(m_downloadLblGameTDBDownload, instant);
	m_btnMgr.hide(m_downloadLblGameTDB, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_downloadLblUser); ++i)
		if(m_downloadLblUser[i] != -1)
			m_btnMgr.hide(m_downloadLblUser[i], instant);
}

void CMenu::_showDownload(void)
{
	_hideSettings();
	_setBg(m_downloadBg, m_downloadBg);
	m_btnMgr.show(m_downloadLblGameTDB);
	m_btnMgr.show(m_downloadLblTitle);
	m_btnMgr.show(m_downloadBtnBack);
	m_btnMgr.show(m_downloadLblCovers);
	m_btnMgr.show(m_downloadBtnAll);
	m_btnMgr.show(m_downloadBtnMissing);
	m_btnMgr.show(m_downloadLblCoverSet);
	m_btnMgr.show(m_downloadBtnCoverSet);
	m_btnMgr.show(m_downloadLblGameTDBDownload);
	m_btnMgr.show(m_downloadBtnGameTDBDownload);
	for(u8 i = 0; i < ARRAY_SIZE(m_downloadLblUser); ++i)
		if(m_downloadLblUser[i] != -1)
			m_btnMgr.show(m_downloadLblUser[i]);	
}

void CMenu::_download(string gameId)
{
	m_coverDLGameId = gameId;
	bool dl_finished = false;
	SetupInput();
	_showDownload();
	m_btnMgr.setText(m_downloadBtnCancel, _t("dl1", L"Cancel"));

	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
		{
			if(settingsmenu)
			{
				settingsmenu = false;
				_hideSettings();
				_showDownload();
			}
			else if(dl_finished)
			{
				dl_finished = false;
				m_btnMgr.hide(m_wbfsPBar);
				m_btnMgr.hide(m_wbfsLblMessage);
				m_btnMgr.hide(m_wbfsLblDialog);
				if(strlen(m_coverDLGameId.c_str()) > 0)
					break;
				_showDownload();
			}
			else
				break;
		}
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if(BTN_A_PRESSED || !gameId.empty())
		{
			if(m_btnMgr.selected(m_downloadBtnAll) || m_btnMgr.selected(m_downloadBtnMissing) || !gameId.empty())
			{
				m_refreshGameList = true;// not needed instead just initcf()
				bool dlAll = m_btnMgr.selected(m_downloadBtnAll);

				_hideSettings();
				m_btnMgr.hide(m_downloadLblCovers);
				m_btnMgr.hide(m_downloadBtnAll);
				m_btnMgr.hide(m_downloadBtnMissing);
				m_btnMgr.hide(m_downloadLblGameTDBDownload);
				m_btnMgr.hide(m_downloadBtnGameTDBDownload);
				m_btnMgr.hide(m_downloadLblCoverSet);
				m_btnMgr.hide(m_downloadBtnCoverSet);
				
				m_btnMgr.setProgress(m_wbfsPBar, 0.f, true);
				m_btnMgr.setText(m_wbfsLblMessage, L"0%");
				m_btnMgr.setText(m_wbfsLblDialog, L"");
				m_btnMgr.show(m_wbfsPBar);
				m_btnMgr.show(m_wbfsLblMessage);
				m_btnMgr.show(m_wbfsLblDialog);
				
				_start_pThread();
				int ret = _coverDownloader(dlAll);
				_stop_pThread();
				if(ret == -1)
					m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg27", L"Not enough memory!"));
				else if(ret == -2)
					m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg2", L"Network initialization failed!"));
				else if(ret == -3)
					m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg30", L"No covers missing."));
				else
					m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg14", L"Done."));
				dl_finished = true;
				gameId.clear();
				//maybe show back button
				//m_btnMgr.show(m_downloadBtnBack);
			}
			else if(m_btnMgr.selected(m_downloadBtnGameTDBDownload))
			{
				m_refreshGameList = true;// to refresh titles
				_hideSettings();
				m_btnMgr.hide(m_downloadLblCovers);
				m_btnMgr.hide(m_downloadBtnAll);
				m_btnMgr.hide(m_downloadBtnMissing);
				m_btnMgr.hide(m_downloadLblGameTDBDownload);
				m_btnMgr.hide(m_downloadBtnGameTDBDownload);
				m_btnMgr.hide(m_downloadLblCoverSet);
				m_btnMgr.hide(m_downloadBtnCoverSet);
				
				m_btnMgr.setProgress(m_wbfsPBar, 0.f, true);
				m_btnMgr.setText(m_wbfsLblMessage, L"0%");
				m_btnMgr.setText(m_wbfsLblDialog, L"");
				m_btnMgr.show(m_wbfsPBar);
				m_btnMgr.show(m_wbfsLblMessage);
				m_btnMgr.show(m_wbfsLblDialog);
				
				_start_pThread();
				int ret = _gametdbDownloaderAsync();
				_stop_pThread();
				if(ret == -1)
					m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg27", L"Not enough memory!"));
				else if(ret == -2)
					m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg2", L"Network initialization failed!"));
				else if(ret == -3)
					m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg12", L"Download failed!"));
				else if(ret == -4)
					m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg15", L"Couldn't save ZIP file"));
				else
					m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg14", L"Done."));
				dl_finished = true;
			}
			/*else if(m_btnMgr.selected(m_downloadBtnCancel))
			{
				LockMutex lock(m_mutex);
				m_thrdStop = true;
				m_thrdMessageAdded = true;
				m_thrdMessage = _t("dlmsg6", L"Canceling...");
			}*/
			else if(m_btnMgr.selected(m_downloadBtnCoverSet))
			{
				settingsmenu = true;
				_showSettings();				
			}
			else if(m_btnMgr.selected(m_downloadBtnBack))
			{
				if(settingsmenu)
				{
					settingsmenu = false;
					_hideSettings();
					_showDownload();
				}
				else
					break;
			}
			else if(m_btnMgr.selected(m_downloadBtnPrioM))
			{
				if(m_downloadPrioVal & C_TYPE_ONOR)
				{
					m_downloadPrioVal ^= C_TYPE_ONOR;
				}
				else
				{
					if(m_downloadPrioVal & C_TYPE_ONCU)
					{
						if(m_downloadPrioVal & C_TYPE_PRIOA)
						{
							if(m_downloadPrioVal & C_TYPE_PRIOB)
							{
								m_downloadPrioVal ^= C_TYPE_PRIOB;
							}
							else
							{
								m_downloadPrioVal ^= C_TYPE_PRIOA;
								m_downloadPrioVal ^= C_TYPE_PRIOB;
							}
						}
						else
						{
							if(m_downloadPrioVal & C_TYPE_PRIOB)
							{
								m_downloadPrioVal ^= C_TYPE_PRIOB;
							}
							else
							{
								m_downloadPrioVal ^= C_TYPE_ONCU;
							}
						}
					}
					else
					{
						m_downloadPrioVal ^= C_TYPE_ONOR;
						m_downloadPrioVal ^= C_TYPE_ONCU;
						m_downloadPrioVal ^= C_TYPE_PRIOA;
						m_downloadPrioVal ^= C_TYPE_PRIOB;
					}
				}
				_hideSettings();
				m_cfg.setUInt("GENERAL", "cover_prio", m_downloadPrioVal);
				_showSettings();
			}
			else if(m_btnMgr.selected(m_downloadBtnPrioP))
			{
				if(m_downloadPrioVal & C_TYPE_ONOR)
				{
					m_downloadPrioVal ^= C_TYPE_ONOR;
					m_downloadPrioVal ^= C_TYPE_ONCU;
					m_downloadPrioVal ^= C_TYPE_PRIOA;
					m_downloadPrioVal ^= C_TYPE_PRIOB;
				}
				else
				{
					if(m_downloadPrioVal & C_TYPE_ONCU)
					{
						if(m_downloadPrioVal & C_TYPE_PRIOA)
						{
							if(m_downloadPrioVal & C_TYPE_PRIOB)
							{
								m_downloadPrioVal ^= C_TYPE_ONOR;
							}
							else
							{
								m_downloadPrioVal ^= C_TYPE_PRIOB;
							}
						}
						else
						{
							if(m_downloadPrioVal & C_TYPE_PRIOB)
							{
								m_downloadPrioVal ^= C_TYPE_PRIOA;
								m_downloadPrioVal ^= C_TYPE_PRIOB;
							}
							else
							{
								m_downloadPrioVal ^= C_TYPE_PRIOB;
							}
						}
					}
					else
					{
						m_downloadPrioVal ^= C_TYPE_ONCU;
					}
				}
				_hideSettings();
				m_cfg.setUInt( "GENERAL", "cover_prio", m_downloadPrioVal );
				_showSettings();
			}
			else if(m_btnMgr.selected(m_downloadBtnEN) || m_btnMgr.selected(m_downloadBtnENs))
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_EN;
				m_cfg.setUInt("GENERAL", "cover_prio", m_downloadPrioVal);
				_showSettings();
			}
			else if(m_btnMgr.selected(m_downloadBtnJA) || m_btnMgr.selected(m_downloadBtnJAs))
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_JA;
				m_cfg.setUInt("GENERAL", "cover_prio", m_downloadPrioVal);
				_showSettings();
			}
			else if(m_btnMgr.selected(m_downloadBtnFR) || m_btnMgr.selected(m_downloadBtnFRs))
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_FR;
				m_cfg.setUInt("GENERAL", "cover_prio", m_downloadPrioVal);
				_showSettings();
			}
			else if(m_btnMgr.selected(m_downloadBtnDE) || m_btnMgr.selected(m_downloadBtnDEs))
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_DE;
				m_cfg.setUInt("GENERAL", "cover_prio", m_downloadPrioVal);
				_showSettings();
			}
			else if(m_btnMgr.selected(m_downloadBtnES) || m_btnMgr.selected(m_downloadBtnESs))
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_ES;
				m_cfg.setUInt("GENERAL", "cover_prio", m_downloadPrioVal);
				_showSettings();
			}
			else if(m_btnMgr.selected(m_downloadBtnIT) || m_btnMgr.selected(m_downloadBtnITs))
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_IT;
				m_cfg.setUInt("GENERAL", "cover_prio", m_downloadPrioVal);
				_showSettings();
			}
			else if(m_btnMgr.selected(m_downloadBtnNL) || m_btnMgr.selected(m_downloadBtnNLs))
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_NL;
				m_cfg.setUInt("GENERAL", "cover_prio", m_downloadPrioVal);
				_showSettings();
			}
			else if(m_btnMgr.selected(m_downloadBtnPT) || m_btnMgr.selected(m_downloadBtnPTs))
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_PT;
				m_cfg.setUInt("GENERAL", "cover_prio", m_downloadPrioVal);
				_showSettings();
			}
			else if(m_btnMgr.selected(m_downloadBtnRU) || m_btnMgr.selected(m_downloadBtnRUs))
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_RU;
				m_cfg.setUInt("GENERAL", "cover_prio", m_downloadPrioVal);
				_showSettings();
			}
			else if(m_btnMgr.selected(m_downloadBtnKO) || m_btnMgr.selected(m_downloadBtnKOs))
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_KO;
				m_cfg.setUInt("GENERAL", "cover_prio", m_downloadPrioVal);
				_showSettings();
			}
			else if(m_btnMgr.selected(m_downloadBtnZHCN) || m_btnMgr.selected(m_downloadBtnZHCNs))
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_ZHCN;
				m_cfg.setUInt("GENERAL", "cover_prio", m_downloadPrioVal);
				_showSettings();
			}
			else if(m_btnMgr.selected(m_downloadBtnAU) || m_btnMgr.selected(m_downloadBtnAUs))
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_AU;
				m_cfg.setUInt("GENERAL", "cover_prio", m_downloadPrioVal);
				_showSettings();
			}
		}
	}
	_hideDownload();
	_hideSettings();
}

void CMenu::_initDownloadMenu()
{
	// Download menu
	_addUserLabels(m_downloadLblUser, ARRAY_SIZE(m_downloadLblUser), "DOWNLOAD");
	m_downloadBg = _texture("DOWNLOAD/BG", "texture", theme.bg, false);
	m_downloadLblTitle = _addTitle("DOWNLOAD/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_downloadLblCovers = _addLabel("DOWNLOAD/COVERS", theme.btnFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadBtnAll = _addButton("DOWNLOAD/ALL_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);
	m_downloadBtnMissing = _addButton("DOWNLOAD/MISSING_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);
	m_downloadLblCoverSet = _addLabel("DOWNLOAD/COVERSSET", theme.btnFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadBtnCoverSet = _addButton("DOWNLOAD/COVERSET_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);
	m_downloadLblGameTDBDownload = _addLabel("DOWNLOAD/GAMETDB_DOWNLOAD", theme.btnFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadBtnGameTDBDownload = _addButton("DOWNLOAD/GAMETDB_DOWNLOAD_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);
	m_downloadLblGameTDB = _addLabel("DOWNLOAD/GAMETDB", theme.lblFont, L"", 20, 390, 370, 60, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadBtnCancel = _addButton("DOWNLOAD/CANCEL_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	m_downloadPBar = _addProgressBar("DOWNLOAD/PROGRESS_BAR", 40, 200, 560, 20);
	m_downloadLblMessage[0] = _addLabel("DOWNLOAD/MESSAGE1", theme.lblFont, L"", 40, 228, 560, 100, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_downloadLblMessage[1] = _addLabel("DOWNLOAD/MESSAGE2", theme.lblFont, L"", 40, 228, 560, 100, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);

	// Cover settings
	m_downloadLblSetTitle = _addTitle("DOWNLOAD/SETTITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_downloadLblCoverPrio = _addLabel("DOWNLOAD/COVERPRIO", theme.lblFont, L"", 20, 110, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadLblPrio = _addLabel("DOWNLOAD/PRIO_BTN", theme.btnFont, L"", 394, 110, 178, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_downloadBtnPrioM = _addPicButton("DOWNLOAD/PRIO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 346, 110, 48, 48);
	m_downloadBtnPrioP = _addPicButton("DOWNLOAD/PRIO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 110, 48, 48);
	m_downloadLblRegion = _addLabel("DOWNLOAD/REGION", theme.lblFont, L"", 20, 160, 600, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadBtnEN = _addPicButton("DOWNLOAD/EN", theme.btnENOff, theme.btnENOffs, 20, 215, 80, 80);
	m_downloadBtnFR = _addPicButton("DOWNLOAD/FR", theme.btnFROff, theme.btnFROffs, 130, 215, 80, 80);
	m_downloadBtnDE = _addPicButton("DOWNLOAD/DE", theme.btnDEOff, theme.btnDEOffs, 230, 215, 80, 80);
	m_downloadBtnAU = _addPicButton("DOWNLOAD/AU", theme.btnAUOff, theme.btnAUOffs, 330, 215, 80, 80);
	m_downloadBtnES = _addPicButton("DOWNLOAD/ES", theme.btnESOff, theme.btnESOffs, 430, 215, 80, 80);
	m_downloadBtnIT = _addPicButton("DOWNLOAD/IT", theme.btnITOff, theme.btnITOffs, 530, 215, 80, 80);
	m_downloadBtnNL = _addPicButton("DOWNLOAD/NL", theme.btnNLOff, theme.btnNLOffs, 30, 300, 80, 80);
	m_downloadBtnPT = _addPicButton("DOWNLOAD/PT", theme.btnPTOff, theme.btnPTOffs, 130, 300, 80, 80);
	m_downloadBtnKO = _addPicButton("DOWNLOAD/KO", theme.btnKOOff, theme.btnKOOffs, 230, 300, 80, 80);
	m_downloadBtnJA = _addPicButton("DOWNLOAD/JA", theme.btnJAOff, theme.btnJAOffs, 330, 300, 80, 80);
	m_downloadBtnRU = _addPicButton("DOWNLOAD/RU", theme.btnRUOff, theme.btnRUOffs, 430, 300, 80, 80);
	m_downloadBtnZHCN = _addPicButton("DOWNLOAD/ZHCN", theme.btnZHCNOff, theme.btnZHCNOffs, 530, 300, 80, 80);
	m_downloadBtnENs = _addPicButton("DOWNLOAD/ENS", theme.btnENOn, theme.btnENOns, 20, 215, 80, 80);
	m_downloadBtnFRs = _addPicButton("DOWNLOAD/FRS", theme.btnFROn, theme.btnFROns, 130, 215, 80, 80);
	m_downloadBtnDEs = _addPicButton("DOWNLOAD/DES", theme.btnDEOn, theme.btnDEOns, 230, 215, 80, 80);
	m_downloadBtnAUs = _addPicButton("DOWNLOAD/AUS", theme.btnAUOn, theme.btnAUOns, 330, 215, 80, 80);
	m_downloadBtnESs = _addPicButton("DOWNLOAD/ESS", theme.btnESOn, theme.btnESOns, 430, 215, 80, 80);
	m_downloadBtnITs = _addPicButton("DOWNLOAD/ITS", theme.btnITOn, theme.btnITOns, 530, 215, 80, 80);
	m_downloadBtnNLs = _addPicButton("DOWNLOAD/NLS", theme.btnNLOn, theme.btnNLOns, 30, 300, 80, 80);
	m_downloadBtnPTs = _addPicButton("DOWNLOAD/PTS", theme.btnPTOn, theme.btnPTOns, 130, 300, 80, 80);
	m_downloadBtnKOs = _addPicButton("DOWNLOAD/KOS", theme.btnKOOn, theme.btnKOOns, 230, 300, 80, 80);
	m_downloadBtnJAs = _addPicButton("DOWNLOAD/JAS", theme.btnJAOn, theme.btnJAOns, 330, 300, 80, 80);
	m_downloadBtnRUs = _addPicButton("DOWNLOAD/RUS", theme.btnRUOn, theme.btnRUOns, 430, 300, 80, 80);
	m_downloadBtnZHCNs = _addPicButton("DOWNLOAD/ZHCNS", theme.btnZHCNOn, theme.btnZHCNOns, 530, 300, 80, 80);
	m_downloadBtnBack = _addButton("DOWNLOAD/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);

	// Download menu
	_setHideAnim(m_downloadLblTitle, "DOWNLOAD/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadLblCovers, "DOWNLOAD/COVERS", 50, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnAll, "DOWNLOAD/ALL_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_downloadBtnMissing, "DOWNLOAD/MISSING_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_downloadLblCoverSet, "DOWNLOAD/COVERSSET", 50, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnCoverSet, "DOWNLOAD/COVERSET_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_downloadLblGameTDBDownload, "DOWNLOAD/GAMETDB_DOWNLOAD", 50, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnGameTDBDownload, "DOWNLOAD/GAMETDB_DOWNLOAD_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_downloadLblGameTDB, "DOWNLOAD/GAMETDB", 50, 0, -2.f, 0.f);
	_setHideAnim(m_downloadPBar, "DOWNLOAD/PROGRESS_BAR", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnCancel, "DOWNLOAD/CANCEL_BTN", 0, 0, 1.f, -1.f);

	// Cover settings	
	_setHideAnim(m_downloadLblSetTitle, "DOWNLOAD/SETTITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadLblCoverPrio, "DOWNLOAD/COVERPRIO", 50, 0, -2.f, 0.f);
	_setHideAnim(m_downloadLblPrio, "DOWNLOAD/PRIO_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_downloadBtnPrioM, "DOWNLOAD/PRIO_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_downloadBtnPrioP, "DOWNLOAD/PRIO_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_downloadLblRegion, "DOWNLOAD/REGION", 50, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnEN, "DOWNLOAD/EN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnFR, "DOWNLOAD/FR", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnDE, "DOWNLOAD/DE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnAU, "DOWNLOAD/AU", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnES, "DOWNLOAD/ES", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnIT, "DOWNLOAD/IT", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnNL, "DOWNLOAD/NL", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnPT, "DOWNLOAD/PT", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnKO, "DOWNLOAD/KO", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnJA, "DOWNLOAD/JA", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnRU, "DOWNLOAD/RU", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnZHCN, "DOWNLOAD/ZHCN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnENs, "DOWNLOAD/ENS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnFRs, "DOWNLOAD/FRS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnDEs, "DOWNLOAD/DES", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnAUs, "DOWNLOAD/AUS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnESs, "DOWNLOAD/ESS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnITs, "DOWNLOAD/ITS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnNLs, "DOWNLOAD/NLS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnPTs, "DOWNLOAD/PTS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnKOs, "DOWNLOAD/KOS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnJAs, "DOWNLOAD/JAS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnRUs, "DOWNLOAD/RUS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnZHCNs, "DOWNLOAD/ZHCNS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnBack, "DOWNLOAD/BACK_BTN", 0, 0, 1.f, -1.f);

	m_downloadPrioVal = m_cfg.getUInt("GENERAL", "cover_prio", 0);

	_hideDownload(true);
	_textDownload();
}

void CMenu::_textDownload(void)
{
	m_btnMgr.setText(m_downloadLblTitle, _t("dl5", L"Download"));
	m_btnMgr.setText(m_downloadLblCovers, _t("dl8", L"Covers"));
	m_btnMgr.setText(m_downloadBtnAll, _t("dl3", L"All"));
	m_btnMgr.setText(m_downloadBtnMissing, _t("dl4", L"Missing"));
	m_btnMgr.setText(m_downloadLblCoverSet, _t("dl15", L"Cover download settings"));
	m_btnMgr.setText(m_downloadBtnCoverSet, _t("dl16", L"Set"));	
	m_btnMgr.setText(m_downloadLblGameTDBDownload, _t("dl12", L"GameTDB"));
	m_btnMgr.setText(m_downloadBtnGameTDBDownload, _t("dl6", L"Download"));
	m_btnMgr.setText(m_downloadLblGameTDB, _t("dl10", L"Please donate\nto GameTDB.com"));
	m_btnMgr.setText(m_downloadBtnCancel, _t("dl1", L"Cancel"));
	
	m_btnMgr.setText(m_downloadLblSetTitle, _t("dl17", L"Cover Download Settings"));
	m_btnMgr.setText(m_downloadLblCoverPrio, _t("dl13", L"Download order"));
	m_btnMgr.setText(m_downloadLblRegion, _t("dl14", L"Select regions to check for covers:"));
	m_btnMgr.setText(m_downloadBtnBack, _t("dl18", L"Back"));
}

/************************************* Setup network connection *********************************************/

void CMenu::_initAsyncNetwork()
{
	if(!_isNetworkAvailable())
		return;
	m_thrdNetwork = true;
	net_init_async(_networkComplete, this);
}

s32 CMenu::_networkComplete(s32 ok, void *usrData)
{
	CMenu *m = (CMenu *) usrData;

	networkInit = ok == 0;
	m->m_thrdNetwork = false;

	gprintf("NET: Network init complete, enabled wifi_gecko: %s\n", m->m_use_wifi_gecko ? "yes" : "no");
	if(m->m_use_wifi_gecko)
	{
		const string &ip = m->m_cfg.getString("DEBUG", "wifi_gecko_ip");
		u16 port = m->m_cfg.getInt("DEBUG", "wifi_gecko_port", 4405);
		if(ip.size() > 0 && port != 0)
			WiFiDebugger.Init(ip.c_str(), port);
	}

	return 0;
}

bool CMenu::_isNetworkAvailable()
{
	bool retval = false;
	u32 size;
	char ISFS_Filepath[32] ATTRIBUTE_ALIGN(32);
	strcpy(ISFS_Filepath, "/shared2/sys/net/02/config.dat");
	u8 *buf = ISFS_GetFile(ISFS_Filepath, &size, -1);
	if(buf && size > 4)
	{
		retval = buf[4] > 0; // There is a valid connection defined.
		free(buf);
	}
	return retval;
}

int CMenu::_initNetwork()
{
	while(net_get_status() == -EBUSY && m_thrdNetwork == true)
	{
		usleep(100);// Async initialization may be busy, wait to see if it succeeds.
	}
	if(networkInit)
		return 0;
	if(!_isNetworkAvailable())
		return -2;

	char ip[16];
	int val = if_config(ip, NULL, NULL, true, 0);
	
	networkInit = !val;
	return val;
}

/************************************* Cover Downloading ******************************/

static string countryCode(const string &gameId)
{
	switch (gameId[3])
	{
		case 'E':
			return "US";
		case 'J':
			return "JA";
		case 'W':
			return "ZH";
		case 'K':
			return "KO";
		case 'R':
			return "RU";
		case 'P':
		case 'D':
		case 'F':
		case 'I':
		case 'S':
		case 'H':
		case 'X':
		case 'Y':
		case 'Z':
			switch (CONF_GetArea())
			{
				case CONF_AREA_BRA:
					return "PT";
				case CONF_AREA_AUS:
					return "AU";
			}
			switch (CONF_GetLanguage())
			{
				case CONF_LANG_ENGLISH:
					return "EN";
				case CONF_LANG_GERMAN:
					return "DE";
				case CONF_LANG_FRENCH:
					return "FR";
				case CONF_LANG_SPANISH:
					return "ES";
				case CONF_LANG_ITALIAN:
					return "IT";
				case CONF_LANG_DUTCH:
					return "NL";
			}
			return "other";
		case 'A':
			switch (CONF_GetArea())
			{
				case CONF_AREA_USA:
					return "US";
				case CONF_AREA_JPN:
					return "JA";
				case CONF_AREA_CHN:
				case CONF_AREA_HKG:
				case CONF_AREA_TWN:
					return "ZH";
				case CONF_AREA_KOR:
					return "KO";
				case CONF_AREA_BRA:
					return "PT";
				case CONF_AREA_AUS:
					return "AU";
			}
			switch (CONF_GetLanguage())
			{
				case CONF_LANG_ENGLISH:
					return "EN";
				case CONF_LANG_GERMAN:
					return "DE";
				case CONF_LANG_FRENCH:
					return "FR";
				case CONF_LANG_SPANISH:
					return "ES";
				case CONF_LANG_ITALIAN:
					return "IT";
				case CONF_LANG_DUTCH:
					return "NL";
			}
	}
	return "other";
}

static string makeURL(const string format, const string gameId, const string country)
{
	string url = format;
	if(url.find(TAG_LOC) != url.npos)
 		url.replace(url.find(TAG_LOC), strlen(TAG_LOC), country.c_str());

	if(url.find(TAG_CONSOLE) != url.npos)
		url.replace(url.find(TAG_CONSOLE), strlen(TAG_CONSOLE), "wii");

	url.replace(url.find(TAG_GAME_ID), strlen(TAG_GAME_ID), gameId.c_str());

	return url;
}

static bool checkPNGBuf(u8 *data)
{
	if(data == NULL)
		return false;

	PNGUPROP imgProp;
	IMGCTX ctx = PNGU_SelectImageFromBuffer(data);
	if (ctx == NULL)
		return false;
	int ret = PNGU_GetImageProperties(ctx, &imgProp);
	PNGU_ReleaseImageContext(ctx);
	return ret == PNGU_OK;
}

static bool checkPNGFile(const char *filename)
{
	u32 fileSize = 0;
	u8 *buffer = fsop_ReadFile(filename, &fileSize);
	bool ret = false;
	if(buffer != NULL)
	{
		ret = checkPNGBuf(buffer);
		MEM2_free(buffer);
	}
	return ret;
}

void CMenu::_setThrdMsg(const wstringEx &msg, float progress)
{
	if (m_thrdStop) return;
	if (msg != L"...") m_thrdMessage = msg;
	m_thrdMessageAdded = true;
	m_thrdProgress = progress;
}

void CMenu::_downloadProgress(void *obj, int size, int position)
{
	CMenu *m = (CMenu *)obj;
	m->m_progress = size == 0 ? 0.f : (float)position / (float)size;
	// Don't synchronize too often
	if(m->m_progress - m->m_thrdProgress >= 0.01f)
	{
		LWP_MutexLock(m->m_mutex);
		m->m_thrdProgress = m->m_progress;
		LWP_MutexUnlock(m->m_mutex);
	}
}

int CMenu::_coverDownloader(bool download_all)
{
	int count = 0, countFlat = 0;

	GameTDB c_gameTDB;
	if(m_settingsDir.size() > 0)
	{
		c_gameTDB.OpenFile(fmt("%s/wiitdb.xml", m_settingsDir.c_str()));
		c_gameTDB.SetLanguageCode(m_curLanguage.c_str());
	}
	
	//bool savePNG = m_cfg.getBool("GENERAL", "keep_png", true);

	vector<string> fmtURLBox = stringToVector(m_cfg.getString("GENERAL", "url_full_covers", FMT_BPIC_URL), '|');
	vector<string> fmtURLFlat = stringToVector(m_cfg.getString("GENERAL", "url_flat_covers", FMT_PIC_URL), '|');
	vector<string> fmtURLCBox = stringToVector(m_cfg.getString("GENERAL", "url_custom_full_covers", FMT_CBPIC_URL), '|');
	vector<string> fmtURLCFlat = stringToVector(m_cfg.getString("GENERAL", "url_custom_flat_covers", FMT_CPIC_URL), '|');

	char path[256];
	char id[7];
	vector<string> coverIDList;
		
	/* create list of cover ID's that need downloading */
	if(m_coverDLGameId.empty())
	{
		//coverIDList.reserve(m_gameList.size());
		for(u32 i = 0; i < m_gameList.size(); ++i)
		{
			m_thrdMessage = _t("dlmsg7", L"Listing covers to download...");
			m_thrdMessageAdded = true;
			
			memset(&path, 0, sizeof(path));
			memset(&id, 0, sizeof(id));
			
			if(m_gameList[i].type == TYPE_PLUGIN || m_gameList[i].type == TYPE_HOMEBREW)
				continue;
			else
			{
				strncpy(id, m_gameList[i].id, 6);
				strncpy(path, fmt("%s/%s.png", m_boxPicDir.c_str(), id), 255);
			}
			if(download_all || (strlen(path) > 0 && !fsop_FileExist(path)))
			{
				if(strlen(id) > 0)
					coverIDList.push_back(id);
			}
		}
	}
	else
		coverIDList.push_back(m_coverDLGameId);

	u32 n = coverIDList.size();
	m_thrdTotal = n * 3;// 3 = download cover, save png, and make wfc

	if(m_thrdTotal == 0)
	{
		if(c_gameTDB.IsLoaded())
			c_gameTDB.CloseFile();
		coverIDList.clear();
		return -3;
	}

	/* initialize network connection */
	if(m_thrdTotal > 0)
	{
		m_thrdMessage = _t("dlmsg1", L"Initializing network...");
		m_thrdMessageAdded = true;
		if(_initNetwork() < 0)
		{
			if(c_gameTDB.IsLoaded())
				c_gameTDB.CloseFile();
			coverIDList.clear();
			return -2;
		}

		/* download covers in the list */
		u32 CoverType = 0;
		string url;

		for(u32 i = 0; i < coverIDList.size(); ++i)
		{
			string coverID = coverIDList[i];
			bool success = false;
			bool original = true;
			bool custom = false;
			int c_altCase = 0;

			/* try downloading the cover 4 times but a different type each time.*/
			for(int p = 0; p < 4; ++p)
			{
				/* the cover type (BOX, CBOX, FLAT, CFLAT) is different each time based on m_downloadPrioVal */
				switch(p)
				{
					case 0:
						CoverType = m_downloadPrioVal & C_TYPE_PRIOA ? CBOX : BOX;
						break;
					case 1:
						CoverType = m_downloadPrioVal & C_TYPE_PRIOA ? (m_downloadPrioVal & C_TYPE_PRIOB ? CFLAT : BOX) :  (m_downloadPrioVal & C_TYPE_PRIOB ? CBOX : FLAT);
						break;
					case 2:
						CoverType = m_downloadPrioVal & C_TYPE_PRIOA ? (m_downloadPrioVal & C_TYPE_PRIOB ? BOX : CFLAT) :  (m_downloadPrioVal & C_TYPE_PRIOB ? FLAT : CBOX);
						break;
					case 3:
						CoverType = m_downloadPrioVal & C_TYPE_PRIOA ? FLAT : CFLAT;
						break;
				}

				switch(CoverType)
				{
					case BOX:
						if(m_downloadPrioVal & C_TYPE_ONOR)
							original = false;
						if(!success && original)
						{
							memset(&path, 0, sizeof(path));
							strncpy(path, fmt("%s/%s.png", m_boxPicDir.c_str(), coverID.c_str()), 255);
							
							/* if cover png doesn't already exist download it */
							if(strlen(path) > 0 && !checkPNGFile(path))
							{
								/* each fmtURL may have more than one URL */
								for(u8 j = 0; !success && j < fmtURLBox.size(); ++j)
								{
									url = makeURL(fmtURLBox[j], coverID, countryCode(coverID));
									
									m_thrdMessage = wfmt(_fmt("dlmsg3", L"Downloading %i/%i from %s"), i + 1, n, url.c_str());
									m_thrdMessageAdded = true;
									download = downloadfile(url.c_str());

									for(int o = 0; o < 12; ++o)
									{
										bool tdl = false;// tdl = try download
										if(download.data != NULL && download.size > 0)// && checkPNGBuf(download.data))
											break;
										switch( o )
										{
											case EN:
												if((coverID[3] == 'E' || coverID[3] == 'X' || coverID[3] == 'Y' || coverID[3] == 'P') && m_downloadPrioVal & C_TYPE_EN)
												{
													url = makeURL(fmtURLBox[j], coverID, "EN");
													tdl = true;
												}
												break;
											case JA:
												if(coverID[3] == 'J' && m_downloadPrioVal&C_TYPE_JA)
												{
													url = makeURL(fmtURLBox[j], coverID, "JA");
													tdl = true;
												}
												break;
											case FR:
												if((coverID[3] == 'F' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_FR)
												{
													url = makeURL(fmtURLBox[j], coverID, "FR");
													tdl = true;
												}
												break;
											case DE:
												if((coverID[3] == 'D' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_DE)
												{
													url = makeURL(fmtURLBox[j], coverID, "DE");
													tdl = true;
												}
												break;
											case ES:
												if((coverID[3] == 'S' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_ES)
												{
													url = makeURL(fmtURLBox[j], coverID, "ES");
													tdl = true;
												}
												break;
											case IT:
												if((coverID[3] == 'I' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_IT)
												{
													url = makeURL(fmtURLBox[j], coverID, "IT");
													tdl = true;
												}
												break;
											case NL:
												if(coverID[3] == 'P' && m_downloadPrioVal&C_TYPE_NL)
												{
													url = makeURL(fmtURLBox[j], coverID, "NL");
													tdl = true;
												}
												break;
											case PT:
												if(coverID[3] == 'P' && m_downloadPrioVal&C_TYPE_PT)
												{
													url = makeURL(fmtURLBox[j], coverID, "PT");
													tdl = true;
												}
												break;
											case RU:
												if((coverID[3] == 'R' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_RU)
												{
													url = makeURL(fmtURLBox[j], coverID, "RU");
													tdl = true;
												}
												break;
											case KO:
												if(coverID[3] == 'K' && m_downloadPrioVal&C_TYPE_KO)
												{
													url = makeURL(fmtURLBox[j], coverID, "KO");
													tdl = true;
												}
												break;
											case AU:
												if(coverID[3] == 'W' && m_downloadPrioVal&C_TYPE_ZHCN)
												{
													url = makeURL(fmtURLBox[j], coverID, "ZH");
													tdl = true;
												}
												break;
											case ZHCN:
												break;
										}
										if(tdl)// try another download
										{
											m_thrdMessage = wfmt(_fmt("dlmsg3", L"Downloading %i/%i from %s"), i + 1, n, url.c_str());
											m_thrdMessageAdded = true;
											download = downloadfile(url.c_str());
										}
									}

									/* if none of the downloads succeeded */
									if(download.data == NULL || download.size == 0)// || !checkPNGBuf(download.data))
										continue;
									
									/* a download succeeded */
									
									/* save cover png */
									update_pThread(1);
									m_thrdMessage = wfmt(_fmt("dlmsg4", L"Saving %s"), path);
									m_thrdMessageAdded = true;
									fsop_WriteFile(path, download.data, download.size);
									
									/* make cover cache file (wfc) */
									update_pThread(1);
									m_thrdMessage = wfmt(_fmt("dlmsg10", L"Making %s.wfc"), coverID.c_str());
									m_thrdMessageAdded = true;
									CoverFlow.preCacheCover(coverID.c_str(), download.data, true);//it may fail
									
									++count;
									update_pThread(1);
									success = true;
								}
							}
						}
						break;
					case CBOX:
						if(m_downloadPrioVal & C_TYPE_ONCU)
							custom = true;
						c_altCase = c_gameTDB.GetCaseVersions(coverID.c_str());
						if(!success && c_gameTDB.IsLoaded() && c_altCase > 1 && custom)
						{
							memset(&path, 0, sizeof(path));
							strncpy(path, fmt("%s/%s.png", m_boxPicDir.c_str(), coverID.c_str()), 255);
							
							/* if cover png doesn't already exist download it */
							if(strlen(path) > 0 && !checkPNGFile(path))
							{
								/* each fmtURL may have more than one URL */
								for(u8 j = 0; !success && j < fmtURLCBox.size(); ++j)
								{
									url = makeURL(fmtURLCBox[j], coverID, countryCode(coverID));

									m_thrdMessage = wfmt(_fmt("dlmsg3", L"Downloading %i/%i from %s"), i + 1, n, url.c_str());
									m_thrdMessageAdded = true;
									download = downloadfile(url.c_str());
									
									for(int o = 0; o < 12; ++o)
									{
										bool tdl = false;
										if(download.data != NULL && download.size > 0)// && checkPNGBuf(download.data))
											break;
										switch( o )
										{
											case EN:
												if(( coverID[3] == 'E' || coverID[3] == 'X' || coverID[3] == 'Y' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_EN )
												{
													url = makeURL(fmtURLCBox[j], coverID, "EN");
													tdl = true;
												}
												break;
											case JA:
												if(coverID[3] == 'J' && m_downloadPrioVal&C_TYPE_JA)
												{
													url = makeURL(fmtURLCBox[j], coverID, "JA");
													tdl = true;
												}
												break;
											case FR:
												if((coverID[3] == 'F' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_FR)
												{
													url = makeURL(fmtURLCBox[j], coverID, "FR");
													tdl = true;
												}
												break;
											case DE:
												if((coverID[3] == 'D' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_DE)
												{
													url = makeURL(fmtURLCBox[j], coverID, "DE");
													tdl = true;
												}
												break;
											case ES:
												if((coverID[3] == 'S' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_ES)
												{
													url = makeURL(fmtURLCBox[j], coverID, "ES");
													tdl = true;
												}
												break;
											case IT:
												if((coverID[3] == 'I' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_IT)
												{
													url = makeURL(fmtURLCBox[j], coverID, "IT");
													tdl = true;
												}
												break;
											case NL:
												if(coverID[3] == 'P' && m_downloadPrioVal&C_TYPE_NL)
												{
													url = makeURL(fmtURLCBox[j], coverID, "NL");
													tdl = true;
												}
												break;
											case PT:
												if(coverID[3] == 'P' && m_downloadPrioVal&C_TYPE_PT)
												{
													url = makeURL(fmtURLCBox[j], coverID, "PT");
													tdl = true;
												}
												break;
											case RU:
												if((coverID[3] == 'R' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_RU)
												{
													url = makeURL(fmtURLCBox[j], coverID, "RU");
													tdl = true;
												}
												break;
											case KO:
												if(coverID[3] == 'K' && m_downloadPrioVal&C_TYPE_KO)
												{
													url = makeURL(fmtURLCBox[j], coverID, "KO");
													tdl = true;
												}
												break;
											case AU:
												if(coverID[3] == 'W' && m_downloadPrioVal&C_TYPE_ZHCN)
												{	
													url = makeURL(fmtURLCBox[j], coverID, "ZH");
													tdl = true;
												}
												break;
											case ZHCN:
												break;
										}

										if(tdl)
										{
											m_thrdMessage = wfmt(_fmt("dlmsg3", L"Downloading %i/%i from %s"), i + 1, n, url.c_str());
											m_thrdMessageAdded = true;
											download = downloadfile(url.c_str());
										}
									}

									/* if none of the downloads succeeded */
									if(download.data == NULL || download.size == 0)// || !checkPNGBuf(download.data))
										continue;

									/* a download succeeded */
									
									/* save cover png */
									update_pThread(1);
									m_thrdMessage = wfmt(_fmt("dlmsg4", L"Saving %s"), path);
									m_thrdMessageAdded = true;
									fsop_WriteFile(path, download.data, download.size);
									
									/* make cover cache file (wfc) */
									update_pThread(1);
									m_thrdMessage = wfmt(_fmt("dlmsg10", L"Making %s.wfc"), coverID.c_str());
									m_thrdMessageAdded = true;
									CoverFlow.preCacheCover(coverID.c_str(), download.data, true);//it may fail
									
									update_pThread(1);
									++count;
									success = true;
								}
							}
						}
						break;
					case FLAT:
						if(m_downloadPrioVal & C_TYPE_ONOR)
							original = false;
						if(!success && original)
						{
							memset(&path, 0, sizeof(path));
							strncpy(path, fmt("%s/%s.png", m_picDir.c_str(), coverID.c_str()), 255);
							
							if(strlen(path) > 0 && !checkPNGFile(path))
							{
								for(u8 j = 0; !success && j < fmtURLFlat.size(); ++j)
								{
									url = makeURL(fmtURLFlat[j], coverID, countryCode(coverID));
									
									m_thrdMessage = wfmt(_fmt("dlmsg3", L"Downloading %i/%i from %s"), i + 1, n, url.c_str());
									m_thrdMessageAdded = true;
									download = downloadfile(url.c_str());

									for(int o = 0; o < 12; ++o)
									{
										bool tdl = false;
										if(download.data != NULL && download.size > 0)// && checkPNGBuf(download.data))
											break;
										switch( o )
										{
											case EN:
												if(( coverID[3] == 'E' || coverID[3] == 'X' || coverID[3] == 'Y' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_EN )
												{
													url = makeURL(fmtURLFlat[j], coverID, "EN");
													tdl = true;
												}
												break;
											case JA:
												if(coverID[3] == 'J' && m_downloadPrioVal&C_TYPE_JA)
												{
													url = makeURL(fmtURLFlat[j], coverID, "JA");
													tdl = true;
												}
												break;
											case FR:
												if((coverID[3] == 'F' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_FR)
												{
													url = makeURL(fmtURLFlat[j], coverID, "FR");
													tdl = true;
												}
												break;
											case DE:
												if((coverID[3] == 'D' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_DE)
												{
													url = makeURL(fmtURLFlat[j], coverID, "DE");
													tdl = true;
												}
												break;
											case ES:
												if((coverID[3] == 'S' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_ES)
												{
													url = makeURL(fmtURLFlat[j], coverID, "ES");
													tdl = true;
												}
												break;
											case IT:
												if((coverID[3] == 'I' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_IT)
												{
													url = makeURL(fmtURLFlat[j], coverID, "IT");
													tdl = true;
												}
												break;
											case NL:
												if(coverID[3] == 'P' && m_downloadPrioVal&C_TYPE_NL)
												{
													url = makeURL(fmtURLFlat[j], coverID, "NL");
													tdl = true;
												}
												break;
											case PT:
												if(coverID[3] == 'P' && m_downloadPrioVal&C_TYPE_PT)
												{
													url = makeURL(fmtURLFlat[j], coverID, "PT");
													tdl = true;
												}
												break;
											case RU:
												if((coverID[3] == 'R' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_RU)
												{
													url = makeURL(fmtURLFlat[j], coverID, "RU");
													tdl = true;
												}
												break;
											case KO:
												if(coverID[3] == 'K' && m_downloadPrioVal&C_TYPE_KO)
												{
													url = makeURL(fmtURLFlat[j], coverID, "KO");
													tdl = true;
												}
												break;
											case AU:
												if(coverID[3] == 'W' && m_downloadPrioVal&C_TYPE_ZHCN)
												{
													url = makeURL(fmtURLFlat[j], coverID, "ZH");
													tdl = true;
												}
												break;
											case ZHCN:
												break;
										}
										if(tdl)
										{
											m_thrdMessage = wfmt(_fmt("dlmsg3", L"Downloading %i/%i from %s"), i + 1, n, url.c_str());
											m_thrdMessageAdded = true;
											download = downloadfile(url.c_str());
										}
									}

									if(download.data == NULL || download.size == 0)// || !checkPNGBuf(download.data))
										continue;

									/*download succeeded - save png */
									update_pThread(1);
									m_thrdMessage = wfmt(_fmt("dlmsg4", L"Saving %s"), path);
									m_thrdMessageAdded = true;
									fsop_WriteFile(path, download.data, download.size);
									
									/* make cover cache file (wfc) */
									update_pThread(1);
									m_thrdMessage = wfmt(_fmt("dlmsg10", L"Making %s"), sfmt("%s.wfc", coverID.c_str()));
									m_thrdMessageAdded = true;
									CoverFlow.preCacheCover(coverID.c_str(), download.data, false);//it may fail
									
									++countFlat;
									update_pThread(1);
									success = true;
								}
							}
						}
						break;
					case CFLAT:
						if(m_downloadPrioVal & C_TYPE_ONCU)
							custom = true;
						if(!success && c_gameTDB.IsLoaded() && c_altCase > 1 && custom)
						{
							memset(&path, 0, sizeof(path));
							strncpy(path, fmt("%s/%s.png", m_picDir.c_str(), coverID.c_str()), 255);
							
							if(strlen(path) > 0 && !checkPNGFile(path))
							{
								for(u8 j = 0; !success && j < fmtURLCFlat.size(); ++j)
								{
									url = makeURL(fmtURLCFlat[j], coverID, countryCode(coverID));
									
									m_thrdMessage = wfmt(_fmt("dlmsg3", L"Downloading %i/%i from %s"), i + 1, n, url.c_str());
									m_thrdMessageAdded = true;
									download = downloadfile(url.c_str());
									
									for(int o = 0; o < 12; ++o)
									{
										bool tdl = false;
										if(download.data != NULL && download.size > 0)// && checkPNGBuf(download.data))
											break;

										switch( o )
										{
											case EN:
												if(( coverID[3] == 'E' || coverID[3] == 'X' || coverID[3] == 'Y' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_EN )
												{
													url = makeURL(fmtURLCFlat[j], coverID, "EN");
													tdl = true;
												}
												break;
											case JA:
												if(coverID[3] == 'J' && m_downloadPrioVal&C_TYPE_JA)
												{
													url = makeURL(fmtURLCFlat[j], coverID, "JA");
													tdl = true;
												}
												break;
											case FR:
												if((coverID[3] == 'F' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_FR)
												{
													url = makeURL(fmtURLCFlat[j], coverID, "FR");
													tdl = true;
												}
												break;
											case DE:
												if((coverID[3] == 'D' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_DE)
												{
													url = makeURL(fmtURLCFlat[j], coverID, "DE");
													tdl = true;
												}
												break;
											case ES:
												if((coverID[3] == 'S' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_ES)
												{
													url = makeURL(fmtURLCFlat[j], coverID, "ES");
													tdl = true;
												}
												break;
											case IT:
												if((coverID[3] == 'I' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_IT)
												{
													url = makeURL(fmtURLCFlat[j], coverID, "IT");
													tdl = true;
												}
												break;
											case NL:
												if(coverID[3] == 'P' && m_downloadPrioVal&C_TYPE_NL)
												{
													url = makeURL(fmtURLCFlat[j], coverID, "NL");
													tdl = true;
												}
												break;
											case PT:
												if(coverID[3] == 'P' && m_downloadPrioVal&C_TYPE_PT)
												{
													url = makeURL(fmtURLCFlat[j], coverID, "PT");
													tdl = true;
												}
												break;
											case RU:
												if((coverID[3] == 'R' || coverID[3] == 'P') && m_downloadPrioVal&C_TYPE_RU)
												{
													url = makeURL(fmtURLCFlat[j], coverID, "RU");
													tdl = true;
												}
												break;
											case KO:
												if(coverID[3] == 'K' && m_downloadPrioVal&C_TYPE_KO)
												{
													url = makeURL(fmtURLCFlat[j], coverID, "KO");
													tdl = true;
												}
												break;
											case AU:
												if((coverID[3] == 'P' || coverID[3] == 'Y' || coverID[3] == 'X') && m_downloadPrioVal&C_TYPE_ZHCN)
												{
													url = makeURL(fmtURLCFlat[j], coverID, "ZH");
													tdl = true;
												}
												break;
											case ZHCN:
												break;
										}
										if(tdl)
										{
											LWP_MutexLock(m_mutex);
											m_thrdMessage = wfmt(_fmt("dlmsg3", L"Downloading %i/%i from %s"), i + 1, n, url.c_str());
											m_thrdMessageAdded = true;
											LWP_MutexUnlock(m_mutex);
											
											download = downloadfile(url.c_str());
										}
									}

									if(download.data == NULL || download.size == 0)// || !checkPNGBuf(download.data))
										continue;

									/* save png */
									update_pThread(1);
									m_thrdMessage = wfmt(_fmt("dlmsg4", L"Saving %s"), path);
									m_thrdMessageAdded = true;
									fsop_WriteFile(path, download.data, download.size);
									
									/* make wfc */
									update_pThread(1);
									m_thrdMessage = wfmt(_fmt("dlmsg10", L"Making %s"), sfmt("%s.wfc", coverID.c_str()));
									m_thrdMessageAdded = true;
									CoverFlow.preCacheCover(coverID.c_str(), download.data, false);//it may fail
									
									++countFlat;
									update_pThread(1);
									success = true;
								}
							}
						}
						break;
				}
			}
		}
	}
	if(countFlat == 0)
		m_thrdMessage = wfmt(_fmt("dlmsg5", L"%i/%i files downloaded."), count, n);
	else
		m_thrdMessage = wfmt(_fmt("dlmsg9", L"%i/%i files downloaded. %i are front covers only."), count + countFlat, n, countFlat);
	m_thrdMessageAdded = true;
	
	if(c_gameTDB.IsLoaded())
		c_gameTDB.CloseFile();
	coverIDList.clear();
	return 0;
}

/*************************************************************************************************/
/*************************************************************************************************/

int CMenu::_gametdbDownloaderAsync()
{
	const string &langCode = m_loc.getString(m_curLanguage, "gametdb_code", "EN");
	m_thrdTotal = 3;// download, save, and unzip
	
	m_thrdMessage = _t("dlmsg1", L"Initializing network...");
	m_thrdMessageAdded = true;
	if(_initNetwork() < 0)
	{
		return -2;
	}
	else
	{
		m_thrdMessage = _t("dlmsg11", L"Downloading...");
		m_thrdMessageAdded = true;	
		download = downloadfile(fmt(GAMETDB_URL, langCode.c_str()));
		if(download.data == 0)
		{
			return -3;
		}
		else
		{
			update_pThread(1);//its downloaded
			bool res = false;
			char *zippath = fmt_malloc("%s/wiitdb.zip", m_settingsDir.c_str());
			if(zippath != NULL)
			{
				gprintf("Writing file to '%s'\n", zippath);
				
				fsop_deleteFile(zippath);
				
				m_thrdMessage = wfmt(_fmt("dlmsg4", L"Saving %s"), "wiitdb.zip");
				m_thrdMessageAdded = true;	
				res = fsop_WriteFile(zippath, download.data, download.size);
			}
			if(res == false)
			{
				gprintf("Can't save zip file\n");
				if(zippath != NULL)
					MEM2_free(zippath);
				return -4;
			}
			else
			{
				update_pThread(1);//its saved
				gprintf("Extracting zip file: ");

				m_thrdMessage = wfmt(_fmt("dlmsg24", L"Extracting %s"), "wiitdb.zip");
				m_thrdMessageAdded = true;	
				ZipFile zFile(zippath);
				bool zres = zFile.ExtractAll(m_settingsDir.c_str());
				gprintf(zres ? "success\n" : "failed\n");
				//may add if zres failed return -4 extraction failed

				// We don't need the zipfile anymore
				fsop_deleteFile(zippath);
				MEM2_free(zippath);

				// We should always remove the offsets file to make sure it's reloaded
				fsop_deleteFile(fmt("%s/gametdb_offsets.bin", m_settingsDir.c_str()));
				
				update_pThread(1);//its extracted

				// Update cache
				m_cfg.setBool(WII_DOMAIN, "update_cache", true);
				m_cfg.setBool(GC_DOMAIN, "update_cache", true);
				m_cfg.setBool(CHANNEL_DOMAIN, "update_cache", true);
				m_refreshGameList = true;
			}
		}
	}
	return 0;
}

/*********************************************************************************/
/*********************************************************************************/

const char *banner_url = NULL;
const char *banner_url_id3 = NULL;
char *banner_location = NULL;
int CMenu::_downloadBannerAsync()
{
	m_thrdTotal = 2;// download and save
	m_thrdMessage = _t("dlmsg1", L"Initializing network...");
	m_thrdMessageAdded = true;

	if(_initNetwork() < 0)
	{
		return -2;
	}

	m_thrdMessage = _t("cfgbnr7", L"Downloading banner...");
	m_thrdMessageAdded = true;
	
	download = downloadfile(banner_url);
	if(download.data == NULL || download.size < 0x5000)
		download = downloadfile(banner_url_id3);

	/* minimum 50kb */
	if(download.data != NULL && download.size > 51200 && download.data[0] != '<')
	{
		update_pThread(1);// its downloaded
		if(banner_location != NULL)
			fsop_WriteFile(banner_location, download.data, download.size);
		update_pThread(1);// its saved
		return 0;
	}
	return -3;// download failed
}

static const char *GAME_BNR_ID = "{gameid}";
void CMenu::_downloadBnr(const char *gameID)
{
	if(gameID == NULL || strlen(gameID) > 6)
		return;
	string base_url = m_cfg.getString("GENERAL", "custom_banner_url", CUSTOM_BANNER_URL);
	if(base_url.size() < 3 || base_url.find(GAME_BNR_ID) == string::npos)
		return;
	base_url.replace(base_url.find(GAME_BNR_ID), strlen(GAME_BNR_ID), gameID);
	banner_url = base_url.c_str();

	string base_url_id3 = m_cfg.getString("GENERAL", "custom_banner_url", CUSTOM_BANNER_URL);
	base_url_id3.replace(base_url_id3.find(GAME_BNR_ID), strlen(GAME_BNR_ID), gameID, 3);
	banner_url_id3 = base_url_id3.c_str();

	banner_location = fmt_malloc("%s/%s.bnr", m_customBnrDir.c_str(), gameID);
	if(banner_location == NULL)
		return;

	bool dl_finished = false;
	while(!m_exit)
	{
		_mainLoopCommon();
		if((BTN_HOME_PRESSED || BTN_B_PRESSED) && dl_finished)
		{
			m_btnMgr.hide(m_wbfsPBar);
			m_btnMgr.hide(m_wbfsLblMessage);
			m_btnMgr.hide(m_wbfsLblDialog);
			break;
		}
		if(!dl_finished)
		{
			m_btnMgr.setProgress(m_wbfsPBar, 0.f, true);
			m_btnMgr.setText(m_wbfsLblMessage, L"0%");
			m_btnMgr.setText(m_wbfsLblDialog, L"");
			m_btnMgr.show(m_wbfsPBar);
			m_btnMgr.show(m_wbfsLblMessage);
			m_btnMgr.show(m_wbfsLblDialog);
			
			_start_pThread();
			int ret = _downloadBannerAsync();
			_stop_pThread();
			if(ret == -1)
				m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg27", L"Not enough memory!"));
			else if(ret == -2)
				m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg2", L"Network initialization failed!"));
			else if(ret == -3)
				m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg12", L"Download failed!"));
			else
				m_btnMgr.setText(m_wbfsLblDialog, _t("dlmsg14", L"Done."));
			dl_finished = true;
		}
	}
	if(banner_location != NULL)
	{
		MEM2_free(banner_location);
		banner_location = NULL;
	}
}

/**************************************************************************************/
/**************************************************************************************/

const char *url_dl = NULL;
void CMenu::_downloadUrl(const char *url, u8 **dl_file, u32 *dl_size)// nothing uses this
{
	m_file = NULL;
	m_filesize = 0;
	url_dl = url;

	m_btnMgr.show(m_downloadPBar);
	m_btnMgr.setProgress(m_downloadPBar, 0.f);
	m_btnMgr.show(m_downloadBtnCancel);
	m_btnMgr.setText(m_downloadBtnCancel, _t("dl1", L"Cancel"));
	m_thrdStop = false;
	m_thrdMessageAdded = false;

	m_thrdWorking = true;
	lwp_t thread = LWP_THREAD_NULL;
	LWP_CreateThread(&thread, _downloadUrlAsync, this, downloadStack, downloadStackSize, 40);

	wstringEx prevMsg;
	while(m_thrdWorking)
	{
		_mainLoopCommon();
		if ((BTN_HOME_PRESSED || BTN_B_PRESSED) && !m_thrdWorking)
			break;
		if (BTN_A_PRESSED && !(m_thrdWorking && m_thrdStop))
		{
			if (m_btnMgr.selected(m_downloadBtnCancel))
			{
				LockMutex lock(m_mutex);
				m_thrdStop = true;
				m_thrdMessageAdded = true;
				m_thrdMessage = _t("dlmsg6", L"Canceling...");
			}
		}
		if (Sys_Exiting())
		{
			LockMutex lock(m_mutex);
			m_thrdStop = true;
			m_thrdMessageAdded = true;
			m_thrdMessage = _t("dlmsg6", L"Canceling...");
			m_thrdWorking = false;
		}
		if (m_thrdMessageAdded)
		{
			LockMutex lock(m_mutex);
			m_thrdMessageAdded = false;
			m_btnMgr.setProgress(m_downloadPBar, m_thrdProgress);
			if (prevMsg != m_thrdMessage)
			{
				prevMsg = m_thrdMessage;
				m_btnMgr.setText(m_downloadLblMessage[0], m_thrdMessage, false);
				m_btnMgr.hide(m_downloadLblMessage[0], 0, 0, -1.f, -1.f, true);
				m_btnMgr.show(m_downloadLblMessage[0]);
			}
		}
		if (m_thrdStop && !m_thrdWorking)
			break;
	}
	if (thread != LWP_THREAD_NULL)
	{
		LWP_JoinThread(thread, NULL);
		thread = LWP_THREAD_NULL;
	}
	m_btnMgr.hide(m_downloadLblMessage[0]);
	m_btnMgr.hide(m_downloadPBar);
	m_btnMgr.hide(m_downloadBtnCancel);

	*dl_file = m_file;
	*dl_size = m_filesize;

	m_file = NULL;
	m_filesize = 0;
	url_dl = url;
}

void * CMenu::_downloadUrlAsync(void *obj)
{
	CMenu *m = (CMenu *)obj;
	if (!m->m_thrdWorking)
		return 0;

	m->m_thrdStop = false;

	LWP_MutexLock(m->m_mutex);
	m->_setThrdMsg(m->_t("dlmsg11", L"Downloading..."), 0);
	LWP_MutexUnlock(m->m_mutex);

	if(m->_initNetwork() < 0 || url_dl == NULL)
	{
		m->m_thrdWorking = false;
		return 0;
	}

	u32 bufferSize = 0x400000; /* 4mb max */
	m->m_buffer = (u8*)MEM2_alloc(bufferSize);
	if(m->m_buffer == NULL)
	{
		m->m_thrdWorking = false;
		return 0;
	}
	//block file = downloadfile(m->m_buffer, bufferSize, url_dl, CMenu::_downloadProgress, m);
	DCFlushRange(m->m_buffer, bufferSize);
	//m->m_file = file.data;
	//m->m_filesize = file.size;
	m->m_thrdWorking = false;
	return 0;
}
