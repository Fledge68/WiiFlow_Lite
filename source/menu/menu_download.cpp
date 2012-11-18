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
#include "gecko/gecko.h"
#include "gecko/wifi_gecko.h"
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

#define TITLES_URL		"http://www.gametdb.com/titles.txt?LANG=%s"
#define GAMETDB_URL		"http://www.gametdb.com/wiitdb.zip?LANG=%s&FALLBACK=TRUE&WIIWARE=TRUE&GAMECUBE=TRUE"
#define UPDATE_URL_VERSION	"http://dl.dropbox.com/u/25620767/WiiflowMod/versions.txt"

static const char FMT_BPIC_URL[] = "http://art.gametdb.com/{console}/coverfullHQ/{loc}/{gameid}.png"\
"|http://art.gametdb.com/{console}/coverfull/{loc}/{gameid}.png";
static const char FMT_PIC_URL[] = "http://art.gametdb.com/{console}/cover/{loc}/{gameid}.png";
static const char FMT_CBPIC_URL[] = "http://art.gametdb.com/{console}/coverfullHQ2/{loc}/{gameid}.png";
static const char FMT_CPIC_URL[] = "http://art.gametdb.com/{console}/cover2/{loc}/{gameid}.png";

static block download = { 0, 0 };
static bool settingsmenu = false;
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
}
void CMenu::_showSettings()
{
	_hideDownload();
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
	m_btnMgr.show(m_downloadBtnCancel);
	m_btnMgr.show(m_downloadBtnAll);
	m_btnMgr.show(m_downloadBtnMissing);
	m_btnMgr.show(m_downloadLblCoverSet);
	m_btnMgr.show(m_downloadBtnCoverSet);
	m_btnMgr.show(m_downloadLblCovers);
	if (!m_locked)
	{
		m_btnMgr.show(m_downloadLblGameTDBDownload);
		m_btnMgr.show(m_downloadBtnGameTDBDownload);
	}
	for(u8 i = 0; i < ARRAY_SIZE(m_downloadLblUser); ++i)
		if(m_downloadLblUser[i] != -1)
			m_btnMgr.show(m_downloadLblUser[i]);	
}

void CMenu::_setThrdMsg(const wstringEx &msg, float progress)
{
	if (m_thrdStop) return;
	if (msg != L"...") m_thrdMessage = msg;
	m_thrdMessageAdded = true;
	m_thrdProgress = progress;
}

bool CMenu::_downloadProgress(void *obj, int size, int position)
{
	CMenu *m = (CMenu *)obj;
	LWP_MutexLock(m->m_mutex);
	m->_setThrdMsg(L"...", m->m_thrdStep + m->m_thrdStepLen * ((float)position / (float)size));
	LWP_MutexUnlock(m->m_mutex);
	return !m->m_thrdStop;
}

int CMenu::_coverDownloaderAll(CMenu *m)
{
	if (!m->m_thrdWorking) return 0;
	m->_coverDownloader(false);
	m->m_thrdWorking = false;
	return 0;
}

int CMenu::_coverDownloaderMissing(CMenu *m)
{
	if (!m->m_thrdWorking) return 0;
	m->_coverDownloader(true);
	m->m_thrdWorking = false;
	return 0;
}

static bool checkPNGBuf(u8 *data)
{
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
	u8 *buffer = NULL;
	FILE *file = fopen(filename, "rb");
	if (file == NULL) return false;
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	if (fileSize > 0)
	{
		buffer = (u8*)MEM2_alloc(fileSize);
		if(buffer != NULL)
			fread(buffer, 1, fileSize, file);
	}
	fclose(file);
	return buffer == NULL ? false : checkPNGBuf(buffer);
}

void CMenu::_initAsyncNetwork()
{
	if (!_isNetworkAvailable()) return;
	m_thrdNetwork = true;
	net_init_async(_networkComplete, this);
}

bool CMenu::_isNetworkAvailable()
{
	bool retval = false;
	u32 size;
	char ISFS_Filepath[32] ATTRIBUTE_ALIGN(32);
	strcpy(ISFS_Filepath, "/shared2/sys/net/02/config.dat");
	u8 *buf = ISFS_GetFile(ISFS_Filepath, &size, -1);
	if (buf && size > 4)
	{
		retval = buf[4] > 0; // There is a valid connection defined.
		free(buf);
	}
	return retval;
}

s32 CMenu::_networkComplete(s32 ok, void *usrData)
{
	CMenu *m = (CMenu *) usrData;

	m->m_networkInit = ok == 0;
	m->m_thrdNetwork = false;

	bool wifigecko = m->m_cfg.getBool("DEBUG", "wifi_gecko", false);
	gprintf("NET: Network init complete, enabled wifi_gecko: %s\n", wifigecko ? "yes" : "no");

	if (wifigecko)
	{
		// Get ip
		std::string ip = m->m_cfg.getString("DEBUG", "wifi_gecko_ip");
		u16 port = m->m_cfg.getInt("DEBUG", "wifi_gecko_port", 4405);

		if (ip.size() > 0 && port != 0)
		{
			gprintf("NET: WIFI Gecko to %s:%d\n", ip.c_str(), port);
			WifiGecko_Init(ip.c_str(), port);
		}
	}

	return 0;
}

int CMenu::_initNetwork()
{
	NandHandle.Disable_Emu();
	while (net_get_status() == -EBUSY || m_thrdNetwork) {}; // Async initialization may be busy, wait to see if it succeeds.
	if (m_networkInit) return 0;
	if (!_isNetworkAvailable()) return -2;

	char ip[16];
	int val = if_config(ip, NULL, NULL, true);
	
	m_networkInit = !val;
	return val;
}

void CMenu::_deinitNetwork()
{
	while(net_get_status() == -EBUSY)
		usleep(100);
	net_wc24cleanup();
	net_deinit();
	m_networkInit = false;
}

int CMenu::_coverDownloader(bool missingOnly)
{
	string path;
	vector<string> coverList;
	vector<dir_discHdr> pluginCoverList;

	int count = 0, countFlat = 0;
	float listWeight = missingOnly ? 0.125f : 0.f;	// 1/8 of the progress bar for testing the PNGs we already have
	float dlWeight = 1.f - listWeight;

	u32 bufferSize = 0x280000;	// Maximum download size 2 MB
	u8 *buffer = (u8*)MEM2_alloc(bufferSize);
	if(buffer == NULL)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(L"Not enough memory!", 1.f);
		LWP_MutexUnlock(m_mutex);
		m_thrdWorking = false;
		return 0;
	}
	bool savePNG = m_cfg.getBool("GENERAL", "keep_png", true);

	vector<string> fmtURLBox = stringToVector(m_cfg.getString("GENERAL", "url_full_covers", FMT_BPIC_URL), '|');
	vector<string> fmtURLFlat = stringToVector(m_cfg.getString("GENERAL", "url_flat_covers", FMT_PIC_URL), '|');
	vector<string> fmtURLCBox = stringToVector(m_cfg.getString("GENERAL", "url_custom_full_covers", FMT_CBPIC_URL), '|');
	vector<string> fmtURLCFlat = stringToVector(m_cfg.getString("GENERAL", "url_custom_flat_covers", FMT_CPIC_URL), '|');

	u32 nbSteps = m_gameList.size();
	u32 step = 0;

	GameTDB c_gameTDB;
	if (m_settingsDir.size() > 0)
	{
		c_gameTDB.OpenFile(fmt("%s/wiitdb.xml", m_settingsDir.c_str()));
		c_gameTDB.SetLanguageCode(m_curLanguage.c_str());
	}

	Config m_checksums;
	m_checksums.load(fmt("%s/%s", m_settingsDir.c_str(), PLUGIN_CRCS_FILENAME));

	if (m_coverDLGameId.empty())
	{
		coverList.reserve(m_gameList.size());
		for (u32 i = 0; i < m_gameList.size() && !m_thrdStop; ++i)
		{
			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg7", L"Listing covers to download..."), listWeight * (float)step / (float)nbSteps);
			LWP_MutexUnlock(m_mutex);
			++step;
			string id;
			if(m_gameList[i].type == TYPE_PLUGIN)
			{
				char gamePath[256];
				if(string(m_gameList[i].path).find_last_of("/") != string::npos)
					strncpy(gamePath, &m_gameList[i].path[string(m_gameList[i].path).find_last_of("/")+1], sizeof(gamePath));
				else
					strncpy(gamePath, m_gameList[i].path, sizeof(gamePath));
				path = fmt("%s/%s.png", m_boxPicDir.c_str(), gamePath);
				id = path;
			}
			else
			{
				id = (const char *)m_gameList[i].id;
				path = fmt("%s/%s.png", m_boxPicDir.c_str(), id.c_str());
			}
			if(!missingOnly || (!CoverFlow.fullCoverCached(id.c_str()) && !checkPNGFile(path.c_str())))
			{
				if(m_gameList[i].type == TYPE_PLUGIN)
					pluginCoverList.push_back(m_gameList[i]);
				coverList.push_back(id);
			}
		}
	}
	else
		coverList.push_back(m_coverDLGameId);

	u32 n = coverList.size();
	if (n > 0 && !m_thrdStop)
	{
		step = 0;
		nbSteps = 1 + n * 2;
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg1", L"Initializing network..."), listWeight + dlWeight * (float)step / (float)nbSteps);
		LWP_MutexUnlock(m_mutex);
		if (_initNetwork() < 0)
		{
			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg2", L"Network initialization failed!"), 1.f);
			LWP_MutexUnlock(m_mutex);
			m_thrdWorking = false;
			free(buffer);
			return 0;
		}
		m_thrdStepLen = dlWeight / (float)nbSteps;

		Config m_newID;
		m_newID.load(fmt("%s/newid.ini", m_settingsDir.c_str()));
		m_newID.setString("CHANNELS", "WFSF", "DWFA");		

		u32 CoverType = 0;		

		for(u32 i = 0; i < coverList.size() && !m_thrdStop; ++i)
		{
			string url;
			const char *domain = _domainFromView();
			bool success = false;
			bool original = true;
			bool custom = false;
			FILE *file = NULL;

			int c_altCase = 0;

			string newID = m_newID.getString(domain, coverList[i], coverList[i]);

			if(!newID.empty() && strncasecmp(newID.c_str(), coverList[i].c_str(), m_current_view != COVERFLOW_USB ? 4 : 6) == 0)
				m_newID.remove(domain, coverList[i]);
			else if(!newID.empty())
			{
				gprintf("old id = %s\nnew id = %s\n", coverList[i].c_str(), newID.c_str());
			}

			for( int p = 0; p < 4; ++p )
			{
				switch(p)
				{
					case 0:
						CoverType = m_downloadPrioVal&C_TYPE_PRIOA ? CBOX : BOX;
						break;
					case 1:
						CoverType = m_downloadPrioVal&C_TYPE_PRIOA ? ( m_downloadPrioVal&C_TYPE_PRIOB ? CFLAT : BOX ) :  ( m_downloadPrioVal&C_TYPE_PRIOB ? CBOX : FLAT );
						break;
					case 2:
						CoverType = m_downloadPrioVal&C_TYPE_PRIOA ? ( m_downloadPrioVal&C_TYPE_PRIOB ? BOX : CFLAT ) :  ( m_downloadPrioVal&C_TYPE_PRIOB ? FLAT : CBOX );
						break;
					case 3:
						CoverType = m_downloadPrioVal&C_TYPE_PRIOA ? FLAT : CFLAT;
						break;
				}				

				switch( CoverType )
				{
					case BOX:
						if( m_downloadPrioVal&C_TYPE_ONOR )
							original = false;
						if (!success && !m_thrdStop && original)
						{
							path = fmt("%s/%s.png", m_boxPicDir.c_str(), coverList[i].c_str());
							if (!checkPNGFile(path.c_str()))
							{
								for (u32 j = 0; !success && j < fmtURLBox.size() && !m_thrdStop; ++j)
								{
									if(pluginCoverList.size() && pluginCoverList[i].type == TYPE_PLUGIN)
										url = m_plugin.GenerateCoverLink(pluginCoverList[i], fmtURLBox[j], m_checksums);
									else
										url = makeURL(fmtURLBox[j], newID, countryCode(newID));
									if (j == 0) ++step;
									m_thrdStep = listWeight + dlWeight * (float)step / (float)nbSteps;
									LWP_MutexLock(m_mutex);
									_setThrdMsg(wfmt(_fmt("dlmsg3", L"Downloading from %s"), url.c_str()), m_thrdStep);
									LWP_MutexUnlock(m_mutex);
									download = downloadfile(buffer, bufferSize, url.c_str(), CMenu::_downloadProgress, this);

									for( int o = 0; o < 12; ++o )
									{
										bool tdl = false;
										if(download.data != NULL && download.size > 0 && checkPNGBuf(download.data))
											break;
										if(pluginCoverList.size() && pluginCoverList[i].type == TYPE_PLUGIN)
											break;
										switch( o )
										{
											case EN:
												if(( newID[3] == 'E' || newID[3] == 'X' || newID[3] == 'Y' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_EN )
													url = makeURL(fmtURLBox[j], newID, "EN");
													tdl = true;
												break;
											case JA:
												if(newID[3] == 'J' && m_downloadPrioVal&C_TYPE_JA)
												{
													url = makeURL(fmtURLBox[j], newID, "JA");
													tdl = true;
												}
												break;
											case FR:
												if((newID[3] == 'F' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_FR)
												{
													url = makeURL(fmtURLBox[j], newID, "FR");
													tdl = true;
												}
												break;
											case DE:
												if((newID[3] == 'D' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_DE)
												{
													url = makeURL(fmtURLBox[j], newID, "DE");
													tdl = true;
												}
												break;
											case ES:
												if((newID[3] == 'S' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_ES)
												{
													url = makeURL(fmtURLBox[j], newID, "ES");
													tdl = true;
												}
												break;
											case IT:
												if((newID[3] == 'I' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_IT)
												{
													url = makeURL(fmtURLBox[j], newID, "IT");
													tdl = true;
												}
												break;
											case NL:											
												if(newID[3] == 'P' && m_downloadPrioVal&C_TYPE_NL)
												{
													url = makeURL(fmtURLBox[j], newID, "NL");
													tdl = true;
												}
												break;
											case PT:											
												if(newID[3] == 'P' && m_downloadPrioVal&C_TYPE_PT)
												{
													url = makeURL(fmtURLBox[j], newID, "PT");
													tdl = true;
												}
												break;
											case RU:											
												if((newID[3] == 'R' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_RU)
												{
													url = makeURL(fmtURLBox[j], newID, "RU");
													tdl = true;
												}
												break;
											case KO:											
												if(newID[3] == 'K' && m_downloadPrioVal&C_TYPE_KO)
												{
													url = makeURL(fmtURLBox[j], newID, "KO");
													tdl = true;
												}
												break;
											case AU:											
												if(newID[3] == 'W' && m_downloadPrioVal&C_TYPE_ZHCN)
												{
													url = makeURL(fmtURLBox[j], newID, "ZH");
													tdl = true;
												}
												break;
											case ZHCN:											
												break;
										}
										if ( tdl )										
										{
											LWP_MutexLock(m_mutex);
											_setThrdMsg(wfmt(_fmt("dlmsg3", L"Downloading from %s"), url.c_str()), m_thrdStep);
											LWP_MutexUnlock(m_mutex);
											download = downloadfile(buffer, bufferSize, url.c_str(), CMenu::_downloadProgress, this);
										}
									}

									if(download.data == NULL || download.size == 0 || !checkPNGBuf(download.data))
										continue;

									if (savePNG)
									{
										LWP_MutexLock(m_mutex);
										_setThrdMsg(wfmt(_fmt("dlmsg4", L"Saving %s"), path.c_str()), listWeight + dlWeight * (float)(step + 1) / (float)nbSteps);
										LWP_MutexUnlock(m_mutex);
										file = fopen(path.c_str(), "wb");
										if(file != NULL)
										{
											fwrite(download.data, download.size, 1, file);
											fclose(file);
										}
									}

									LWP_MutexLock(m_mutex);
									_setThrdMsg(wfmt(_fmt("dlmsg10", L"Making %s"), sfmt("%s.wfc", coverList[i].c_str()).c_str()), listWeight + dlWeight * (float)(step + 1) / (float)nbSteps);
									LWP_MutexUnlock(m_mutex);
									if (CoverFlow.preCacheCover(coverList[i].c_str(), download.data, true))
									{
										++count;
										success = true;
									}
								}
							}
						}
						break;
					case CBOX:
						if( m_downloadPrioVal&C_TYPE_ONCU )
							custom = true;
						c_altCase = c_gameTDB.GetCaseVersions( coverList[i].c_str() );
						if (!success && !m_thrdStop && c_gameTDB.IsLoaded() && c_altCase > 1 && custom)
						{
							path = fmt("%s/%s.png", m_boxPicDir.c_str(), coverList[i].c_str());
							if (!checkPNGFile(path.c_str()))
							{
								for (u32 j = 0; !success && j < fmtURLCBox.size() && !m_thrdStop; ++j)
								{
									if(pluginCoverList.size() && pluginCoverList[i].type == TYPE_PLUGIN)
										url = m_plugin.GenerateCoverLink(pluginCoverList[i], fmtURLCBox[j], m_checksums);
									else
										url = makeURL(fmtURLCBox[j], newID, countryCode(newID));
									if (j == 0) ++step;
									m_thrdStep = listWeight + dlWeight * (float)step / (float)nbSteps;
									LWP_MutexLock(m_mutex);
									_setThrdMsg(wfmt(_fmt("dlmsg3", L"Downloading from %s"), url.c_str()), m_thrdStep);
									LWP_MutexUnlock(m_mutex);
									download = downloadfile(buffer, bufferSize, url.c_str(), CMenu::_downloadProgress, this);
									for( int o = 0; o < 12; ++o )
									{
										bool tdl = false;
										if(download.data != NULL && download.size > 0 && checkPNGBuf(download.data))
											break;
										if(pluginCoverList.size() && pluginCoverList[i].type == TYPE_PLUGIN)
											break;
										switch( o )
										{
											case EN:
												if(( newID[3] == 'E' || newID[3] == 'X' || newID[3] == 'Y' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_EN )
												{
													url = makeURL(fmtURLCBox[j], newID, "EN");
													tdl = true;
												}
												break;
											case JA:
												if(newID[3] == 'J' && m_downloadPrioVal&C_TYPE_JA)
												{
													url = makeURL(fmtURLCBox[j], newID, "JA");
													tdl = true;
												}
												break;
											case FR:
												if((newID[3] == 'F' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_FR)
												{
													url = makeURL(fmtURLCBox[j], newID, "FR");
													tdl = true;
												}
												break;
											case DE:
												if((newID[3] == 'D' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_DE)
												{
													url = makeURL(fmtURLCBox[j], newID, "DE");
													tdl = true;
												}
												break;
											case ES:
												if((newID[3] == 'S' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_ES)
												{
													url = makeURL(fmtURLCBox[j], newID, "ES");
													tdl = true;
												}
												break;
											case IT:
												if((newID[3] == 'I' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_IT)
												{
													url = makeURL(fmtURLCBox[j], newID, "IT");
													tdl = true;
												}
												break;
											case NL:											
												if(newID[3] == 'P' && m_downloadPrioVal&C_TYPE_NL)
												{
													url = makeURL(fmtURLCBox[j], newID, "NL");
													tdl = true;
												}
												break;
											case PT:											
												if(newID[3] == 'P' && m_downloadPrioVal&C_TYPE_PT)
												{
													url = makeURL(fmtURLCBox[j], newID, "PT");
													tdl = true;
												}
												break;
											case RU:											
												if((newID[3] == 'R' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_RU)
												{
													url = makeURL(fmtURLCBox[j], newID, "RU");
													tdl = true;
												}
												break;
											case KO:											
												if(newID[3] == 'K' && m_downloadPrioVal&C_TYPE_KO)
												{
													url = makeURL(fmtURLCBox[j], newID, "KO");
													tdl = true;
												}
												break;
											case AU:											
												if(newID[3] == 'W' && m_downloadPrioVal&C_TYPE_ZHCN)
												{	
													url = makeURL(fmtURLCBox[j], newID, "ZH");
													tdl = true;
												}
												break;
											case ZHCN:											
												break;
										}

										if ( tdl )										
										{
											LWP_MutexLock(m_mutex);
											_setThrdMsg(wfmt(_fmt("dlmsg3", L"Downloading from %s"), url.c_str()), m_thrdStep);
											LWP_MutexUnlock(m_mutex);
											download = downloadfile(buffer, bufferSize, url.c_str(), CMenu::_downloadProgress, this);
										}
									}

									if(download.data == NULL || download.size == 0 || !checkPNGBuf(download.data))
										continue;

									if (savePNG)
									{	
										LWP_MutexLock(m_mutex);
										_setThrdMsg(wfmt(_fmt("dlmsg4", L"Saving %s"), path.c_str()), listWeight + dlWeight * (float)(step + 1) / (float)nbSteps);
										LWP_MutexUnlock(m_mutex);
										file = fopen(path.c_str(), "wb");
										if (file != NULL)
										{
											fwrite(download.data, download.size, 1, file);
											fclose(file);
										}
									}

									LWP_MutexLock(m_mutex);
									_setThrdMsg(wfmt(_fmt("dlmsg10", L"Making %s"), sfmt("%s.wfc", coverList[i].c_str()).c_str()), listWeight + dlWeight * (float)(step + 1) / (float)nbSteps);
									LWP_MutexUnlock(m_mutex);
									if (CoverFlow.preCacheCover(coverList[i].c_str(), download.data, true))
									{
										++count;
										success = true;
									}
								}
							}
						}
						break;
					case FLAT:
						if( m_downloadPrioVal&C_TYPE_ONOR )
							original = false;
						if (!success && !m_thrdStop && original)
						{
							path = fmt("%s/%s.png", m_picDir.c_str(), coverList[i].c_str());
							if (!checkPNGFile(path.c_str()))
							{
								// Try to get the front cover
								if (m_thrdStop) break;
								for (u32 j = 0; !success && j < fmtURLFlat.size() && !m_thrdStop; ++j)
								{
									if(pluginCoverList.size() && pluginCoverList[i].type == TYPE_PLUGIN)
										url = m_plugin.GenerateCoverLink(pluginCoverList[i], fmtURLFlat[j], m_checksums);
									else
										url = makeURL(fmtURLFlat[j], newID, countryCode(newID));
									LWP_MutexLock(m_mutex);
									_setThrdMsg(wfmt(_fmt("dlmsg8", L"Full cover not found. Downloading from %s"), url.c_str()), listWeight + dlWeight * (float)step / (float)nbSteps);
									LWP_MutexUnlock(m_mutex);
									download = downloadfile(buffer, bufferSize, url.c_str(), CMenu::_downloadProgress, this);

									for( int o = 0; o < 12; ++o )
									{
										bool tdl = false;
										if(download.data != NULL && download.size > 0 && checkPNGBuf(download.data))
											break;
										if(pluginCoverList.size() && pluginCoverList[i].type == TYPE_PLUGIN)
											break;
										switch( o )
										{
											case EN:
												if(( newID[3] == 'E' || newID[3] == 'X' || newID[3] == 'Y' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_EN )
												{
													url = makeURL(fmtURLFlat[j], newID, "EN");
													tdl = true;
												}
												break;
											case JA:
												if(newID[3] == 'J' && m_downloadPrioVal&C_TYPE_JA)
												{
													url = makeURL(fmtURLFlat[j], newID, "JA");
													tdl = true;
												}
												break;
											case FR:
												if((newID[3] == 'F' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_FR)
												{
													url = makeURL(fmtURLFlat[j], newID, "FR");
													tdl = true;
												}
												break;
											case DE:
												if((newID[3] == 'D' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_DE)
												{
													url = makeURL(fmtURLFlat[j], newID, "DE");
													tdl = true;
												}
												break;
											case ES:
												if((newID[3] == 'S' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_ES)
												{
													url = makeURL(fmtURLFlat[j], newID, "ES");
													tdl = true;
												}
												break;
											case IT:
												if((newID[3] == 'I' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_IT)
												{
													url = makeURL(fmtURLFlat[j], newID, "IT");
													tdl = true;
												}
												break;
											case NL:											
												if(newID[3] == 'P' && m_downloadPrioVal&C_TYPE_NL)
												{
													url = makeURL(fmtURLFlat[j], newID, "NL");
													tdl = true;
												}
												break;
											case PT:											
												if(newID[3] == 'P' && m_downloadPrioVal&C_TYPE_PT)
												{
													url = makeURL(fmtURLFlat[j], newID, "PT");
													tdl = true;
												}
												break;
											case RU:											
												if((newID[3] == 'R' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_RU)
												{
													url = makeURL(fmtURLFlat[j], newID, "RU");
													tdl = true;
												}
												break;
											case KO:											
												if(newID[3] == 'K' && m_downloadPrioVal&C_TYPE_KO)
												{
													url = makeURL(fmtURLFlat[j], newID, "KO");
													tdl = true;
												}
												break;
											case AU:											
												if(newID[3] == 'W' && m_downloadPrioVal&C_TYPE_ZHCN)
												{
													url = makeURL(fmtURLFlat[j], newID, "ZH");
													tdl = true;
												}
												break;
											case ZHCN:											
												break;
										}
										if ( tdl )										
										{
											LWP_MutexLock(m_mutex);
											_setThrdMsg(wfmt(_fmt("dlmsg3", L"Downloading from %s"), url.c_str()), m_thrdStep);
											LWP_MutexUnlock(m_mutex);
											download = downloadfile(buffer, bufferSize, url.c_str(), CMenu::_downloadProgress, this);
										}
									}

									if(download.data == NULL || download.size == 0 || !checkPNGBuf(download.data))
										continue;

									if (savePNG)
									{
										LWP_MutexLock(m_mutex);
										_setThrdMsg(wfmt(_fmt("dlmsg4", L"Saving %s"), path.c_str()), listWeight + dlWeight * (float)(step + 1) / (float)nbSteps);
										LWP_MutexUnlock(m_mutex);
										file = fopen(path.c_str(), "wb");
										if (file != NULL)
										{
											fwrite(download.data, download.size, 1, file);
											fclose(file);
										}
									}

									LWP_MutexLock(m_mutex);
									_setThrdMsg(wfmt(_fmt("dlmsg10", L"Making %s"), sfmt("%s.wfc", coverList[i].c_str()).c_str()), listWeight + dlWeight * (float)(step + 1) / (float)nbSteps);
									LWP_MutexUnlock(m_mutex);
									if (CoverFlow.preCacheCover(coverList[i].c_str(), download.data, false))
									{
										++countFlat;
										success = true;
									}
								}
							}
						}
						break;
					case CFLAT:						
						if( m_downloadPrioVal&C_TYPE_ONCU )
							custom = true;
						if (!success && !m_thrdStop && c_gameTDB.IsLoaded() && c_altCase > 1 && custom)
						{	
							path = fmt("%s/%s.png", m_picDir.c_str(), coverList[i].c_str());
							if (!checkPNGFile(path.c_str()))
							{
								// Try to get the front cover
								if (m_thrdStop) break;
								for (u32 j = 0; !success && j < fmtURLCFlat.size() && !m_thrdStop; ++j)
								{
									url = makeURL(fmtURLCFlat[j], newID, countryCode(newID));
									LWP_MutexLock(m_mutex);
									_setThrdMsg(wfmt(_fmt("dlmsg8", L"Full cover not found. Downloading from %s"), url.c_str()), listWeight + dlWeight * (float)step / (float)nbSteps);
									LWP_MutexUnlock(m_mutex);
									download = downloadfile(buffer, bufferSize, url.c_str(), CMenu::_downloadProgress, this);

									for( int o = 0; o < 12; ++o )
									{
										bool tdl = false;
										if(download.data != NULL && download.size > 0 && checkPNGBuf(download.data))
											break;										

										switch( o )
										{
											case EN:										
												if(( newID[3] == 'E' || newID[3] == 'X' || newID[3] == 'Y' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_EN )
												{
													url = makeURL(fmtURLCFlat[j], newID, "EN");
													tdl = true;
												}
												break;
											case JA:
												if(newID[3] == 'J' && m_downloadPrioVal&C_TYPE_JA)
												{
													url = makeURL(fmtURLCFlat[j], newID, "JA");
													tdl = true;
												}
												break;
											case FR:
												if((newID[3] == 'F' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_FR)
												{
													url = makeURL(fmtURLCFlat[j], newID, "FR");
													tdl = true;
												}
												break;
											case DE:
												if((newID[3] == 'D' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_DE)
												{
													url = makeURL(fmtURLCFlat[j], newID, "DE");
													tdl = true;
												}
												break;
											case ES:
												if((newID[3] == 'S' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_ES)
												{
													url = makeURL(fmtURLCFlat[j], newID, "ES");
													tdl = true;
												}
												break;
											case IT:
												if((newID[3] == 'I' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_IT)
												{
													url = makeURL(fmtURLCFlat[j], newID, "IT");
													tdl = true;
												}
												break;
											case NL:											
												if(newID[3] == 'P' && m_downloadPrioVal&C_TYPE_NL)
												{
													url = makeURL(fmtURLCFlat[j], newID, "NL");
													tdl = true;
												}
												break;
											case PT:											
												if(newID[3] == 'P' && m_downloadPrioVal&C_TYPE_PT)
												{
													url = makeURL(fmtURLCFlat[j], newID, "PT");
													tdl = true;
												}
												break;
											case RU:											
												if((newID[3] == 'R' || newID[3] == 'P') && m_downloadPrioVal&C_TYPE_RU)
												{
													url = makeURL(fmtURLCFlat[j], newID, "RU");
													tdl = true;
												}
												break;
											case KO:											
												if(newID[3] == 'K' && m_downloadPrioVal&C_TYPE_KO)
												{
													url = makeURL(fmtURLCFlat[j], newID, "KO");
													tdl = true;
												}
												break;
											case AU:											
												if((newID[3] == 'P' || newID[3] == 'Y' || newID[3] == 'X') && m_downloadPrioVal&C_TYPE_ZHCN)
												{
													url = makeURL(fmtURLCFlat[j], newID, "ZH");
													tdl = true;
												}
												break;
											case ZHCN:											
												break;
										}
										if ( tdl )										
										{
											LWP_MutexLock(m_mutex);
											_setThrdMsg(wfmt(_fmt("dlmsg3", L"Downloading from %s"), url.c_str()), m_thrdStep);
											LWP_MutexUnlock(m_mutex);
											download = downloadfile(buffer, bufferSize, url.c_str(), CMenu::_downloadProgress, this);
										}
									}

									if(download.data == NULL || download.size == 0 || !checkPNGBuf(download.data))
										continue;

									if (savePNG)
									{
										LWP_MutexLock(m_mutex);
										_setThrdMsg(wfmt(_fmt("dlmsg4", L"Saving %s"), path.c_str()), listWeight + dlWeight * (float)(step + 1) / (float)nbSteps);
										LWP_MutexUnlock(m_mutex);
										file = fopen(path.c_str(), "wb");
										if (file != NULL)
										{
											fwrite(download.data, download.size, 1, file);
											fclose(file);
										}
									}

									LWP_MutexLock(m_mutex);
									_setThrdMsg(wfmt(_fmt("dlmsg10", L"Making %s"), sfmt("%s.wfc", coverList[i].c_str()).c_str()), listWeight + dlWeight * (float)(step + 1) / (float)nbSteps);
									LWP_MutexUnlock(m_mutex);
									if (CoverFlow.preCacheCover(coverList[i].c_str(), download.data, false))
									{
										++countFlat;
										success = true;
									}
								}
							}
						}
						break;
				}
			}
			newID.clear();
			++step;
		}
		if(c_gameTDB.IsLoaded())
			c_gameTDB.CloseFile();
		coverList.clear();
		m_checksums.unload();
		m_newID.unload();
	}
	LWP_MutexLock(m_mutex);
	if (countFlat == 0)
		_setThrdMsg(wfmt(_fmt("dlmsg5", L"%i/%i files downloaded."), count, n), 1.f);
	else
		_setThrdMsg(wfmt(_fmt("dlmsg9", L"%i/%i files downloaded. %i are front covers only."), count + countFlat, n, countFlat), 1.f);
	LWP_MutexUnlock(m_mutex);
	m_thrdWorking = false;
	pluginCoverList.clear();
	free(buffer);
	return 0;
}

void CMenu::_download(string gameId)
{
	lwp_t thread = LWP_THREAD_NULL;
	int msg = 0;
	wstringEx prevMsg;

	bool _updateGametdb = false;

	SetupInput();
	_showDownload();
	m_btnMgr.setText(m_downloadBtnCancel, _t("dl1", L"Cancel"));
	m_thrdStop = false;
	m_thrdMessageAdded = false;

	if(gameId.size() && CoverFlow.getHdr()->type == TYPE_PLUGIN)
	{
		char gamePath[256];
		if(string(CoverFlow.getHdr()->path).find_last_of("/") != string::npos)
			strncpy(gamePath, &CoverFlow.getHdr()->path[string(CoverFlow.getHdr()->path).find_last_of("/")+1], sizeof(gamePath));
		else
			strncpy(gamePath, CoverFlow.getHdr()->path, sizeof(gamePath));
		m_coverDLGameId = gamePath;
	}
	else
		m_coverDLGameId = gameId;

	while(!m_exit)
	{
		_mainLoopCommon();
		if ((BTN_HOME_PRESSED || BTN_B_PRESSED) && !m_thrdWorking)
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
		else if (BTN_UP_PRESSED)
			m_btnMgr.up();
		else if (BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if ((BTN_A_PRESSED || !gameId.empty()) && !(m_thrdWorking && m_thrdStop))
		{
			if ((m_btnMgr.selected(m_downloadBtnAll) || m_btnMgr.selected(m_downloadBtnMissing) || !gameId.empty()) && !m_thrdWorking)
			{
				bool dlAll = m_btnMgr.selected(m_downloadBtnAll);
				m_btnMgr.show(m_downloadPBar);
				m_btnMgr.setProgress(m_downloadPBar, 0.f);
				_hideSettings();
				m_btnMgr.hide(m_downloadBtnAll);
				m_btnMgr.hide(m_downloadBtnMissing);
				m_btnMgr.hide(m_downloadBtnGameTDBDownload);
				m_btnMgr.hide(m_downloadLblCovers);
				m_btnMgr.hide(m_downloadLblGameTDBDownload);
				m_btnMgr.hide(m_downloadLblCoverSet);
				m_btnMgr.hide(m_downloadBtnCoverSet);

				m_thrdStop = false;
				m_thrdWorking = true;
				gameId.clear();
				if (dlAll)
					LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_coverDownloaderAll, (void *)this, 0, 8192, 40);
				else
					LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_coverDownloaderMissing, (void *)this, 0, 8192, 40);
			}
			else if (m_btnMgr.selected(m_downloadBtnPrioM) && !m_thrdWorking)
			{
				if( m_downloadPrioVal&C_TYPE_ONOR )
				{
					m_downloadPrioVal ^= C_TYPE_ONOR;
				}
				else
				{
					if( m_downloadPrioVal&C_TYPE_ONCU )
					{
						if( m_downloadPrioVal&C_TYPE_PRIOA )
						{
							if(m_downloadPrioVal&C_TYPE_PRIOB )
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
							if(m_downloadPrioVal&C_TYPE_PRIOB )
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
				m_cfg.setUInt( "GENERAL", "cover_prio", m_downloadPrioVal );
				m_cfg.save();
				_showSettings();
			}
			else if (m_btnMgr.selected(m_downloadBtnPrioP) && !m_thrdWorking)
			{
				if( m_downloadPrioVal&C_TYPE_ONOR )
				{
					m_downloadPrioVal ^= C_TYPE_ONOR;
					m_downloadPrioVal ^= C_TYPE_ONCU;
					m_downloadPrioVal ^= C_TYPE_PRIOA;
					m_downloadPrioVal ^= C_TYPE_PRIOB;
				}
				else
				{
					if( m_downloadPrioVal&C_TYPE_ONCU )
					{
						if( m_downloadPrioVal&C_TYPE_PRIOA )
						{
							if(m_downloadPrioVal&C_TYPE_PRIOB )
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
							if(m_downloadPrioVal&C_TYPE_PRIOB )
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
				m_cfg.save();
				_showSettings();
			}
			else if (m_btnMgr.selected(m_downloadBtnCoverSet) && !m_thrdWorking)
			{
				settingsmenu = true;
				_showSettings();				
			}
			else if (m_btnMgr.selected(m_downloadBtnBack) && !m_thrdWorking)
			{
				_hideSettings();
				_showDownload();
			}
			else if ((m_btnMgr.selected(m_downloadBtnEN) || m_btnMgr.selected(m_downloadBtnENs)) && !m_thrdWorking)
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_EN;
				m_cfg.setUInt( "GENERAL", "cover_prio", m_downloadPrioVal );
				m_cfg.save();
				_showSettings();
			}
			else if ((m_btnMgr.selected(m_downloadBtnJA) || m_btnMgr.selected(m_downloadBtnJAs)) && !m_thrdWorking)
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_JA;
				m_cfg.setUInt( "GENERAL", "cover_prio", m_downloadPrioVal );
				m_cfg.save();
				_showSettings();
			}
			else if ((m_btnMgr.selected(m_downloadBtnFR) || m_btnMgr.selected(m_downloadBtnFRs)) && !m_thrdWorking)
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_FR;
				m_cfg.setUInt( "GENERAL", "cover_prio", m_downloadPrioVal );
				m_cfg.save();
				_showSettings();
			}
			else if ((m_btnMgr.selected(m_downloadBtnDE) || m_btnMgr.selected(m_downloadBtnDEs)) && !m_thrdWorking)
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_DE;
				m_cfg.setUInt( "GENERAL", "cover_prio", m_downloadPrioVal );
				m_cfg.save();
				_showSettings();
			}
			else if ((m_btnMgr.selected(m_downloadBtnES) || m_btnMgr.selected(m_downloadBtnESs)) && !m_thrdWorking)
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_ES;
				m_cfg.setUInt( "GENERAL", "cover_prio", m_downloadPrioVal );
				m_cfg.save();
				_showSettings();
			}
			else if ((m_btnMgr.selected(m_downloadBtnIT) || m_btnMgr.selected(m_downloadBtnITs)) && !m_thrdWorking)
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_IT;
				m_cfg.setUInt( "GENERAL", "cover_prio", m_downloadPrioVal );
				m_cfg.save();
				_showSettings();
			}
			else if ((m_btnMgr.selected(m_downloadBtnNL) || m_btnMgr.selected(m_downloadBtnNLs)) && !m_thrdWorking)
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_NL;
				m_cfg.setUInt( "GENERAL", "cover_prio", m_downloadPrioVal );
				m_cfg.save();
				_showSettings();
			}
			else if ((m_btnMgr.selected(m_downloadBtnPT) || m_btnMgr.selected(m_downloadBtnPTs)) && !m_thrdWorking)
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_PT;
				m_cfg.setUInt( "GENERAL", "cover_prio", m_downloadPrioVal );
				m_cfg.save();
				_showSettings();
			}
			else if ((m_btnMgr.selected(m_downloadBtnRU) || m_btnMgr.selected(m_downloadBtnRUs)) && !m_thrdWorking)
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_RU;
				m_cfg.setUInt( "GENERAL", "cover_prio", m_downloadPrioVal );
				m_cfg.save();
				_showSettings();
			}
			else if ((m_btnMgr.selected(m_downloadBtnKO) || m_btnMgr.selected(m_downloadBtnKOs)) && !m_thrdWorking)
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_KO;
				m_cfg.setUInt( "GENERAL", "cover_prio", m_downloadPrioVal );
				m_cfg.save();
				_showSettings();
			}
			else if ((m_btnMgr.selected(m_downloadBtnZHCN) || m_btnMgr.selected(m_downloadBtnZHCNs)) && !m_thrdWorking)
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_ZHCN;
				m_cfg.setUInt( "GENERAL", "cover_prio", m_downloadPrioVal );
				m_cfg.save();
				_showSettings();
			}
			else if ((m_btnMgr.selected(m_downloadBtnAU) || m_btnMgr.selected(m_downloadBtnAUs)) && !m_thrdWorking)
			{
				_hideSettings();
				m_downloadPrioVal ^= C_TYPE_AU;
				m_cfg.setUInt( "GENERAL", "cover_prio", m_downloadPrioVal );
				m_cfg.save();				
				_showSettings();
			}

			else if (m_btnMgr.selected(m_downloadBtnGameTDBDownload) && !m_thrdWorking)
			{
				m_btnMgr.show(m_downloadPBar);
				m_btnMgr.setProgress(m_downloadPBar, 0.f);
				_hideSettings();
				m_btnMgr.hide(m_downloadBtnAll);
				m_btnMgr.hide(m_downloadBtnMissing);
				m_btnMgr.hide(m_downloadBtnGameTDBDownload);
				m_btnMgr.hide(m_downloadLblCovers);
				m_btnMgr.hide(m_downloadLblGameTDBDownload);
				m_btnMgr.hide(m_downloadLblCoverSet);
				m_btnMgr.hide(m_downloadBtnCoverSet);
				m_thrdStop = false;
				m_thrdWorking = true;

				_updateGametdb = true;

				LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_gametdbDownloader, (void *)this, 0, 8192, 40);		
			}
			else if (m_btnMgr.selected(m_downloadBtnCancel))
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
			if (m_thrdProgress == 1.f)
			{
				if (_updateGametdb)
					break;
				m_btnMgr.setText(m_downloadBtnCancel, _t("dl2", L"Back"));
			}
			if (prevMsg != m_thrdMessage)
			{
				prevMsg = m_thrdMessage;
				m_btnMgr.setText(m_downloadLblMessage[msg], m_thrdMessage, false);
				m_btnMgr.hide(m_downloadLblMessage[msg], 0, 0, -1.f, -1.f, true);
				m_btnMgr.show(m_downloadLblMessage[msg]);
				msg ^= 1;
				m_btnMgr.hide(m_downloadLblMessage[msg], 0, 0, -1.f, -1.f);
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
	_hideDownload();
	_hideSettings();
}

void CMenu::_initDownloadMenu()
{
	// Download menu
	_addUserLabels(m_downloadLblUser, ARRAY_SIZE(m_downloadLblUser), "DOWNLOAD");
	m_downloadBg = _texture("DOWNLOAD/BG", "texture", theme.bg, false);
	m_downloadLblTitle = _addTitle("DOWNLOAD/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_downloadPBar = _addProgressBar("DOWNLOAD/PROGRESS_BAR", 40, 200, 560, 20);
	m_downloadBtnCancel = _addButton("DOWNLOAD/CANCEL_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_downloadLblCovers = _addLabel("DOWNLOAD/COVERS", theme.btnFont, L"", 40, 130, 320, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadBtnAll = _addButton("DOWNLOAD/ALL_BTN", theme.btnFont, L"", 370, 130, 230, 56, theme.btnFontColor);
	m_downloadBtnMissing = _addButton("DOWNLOAD/MISSING_BTN", theme.btnFont, L"", 370, 190, 230, 56, theme.btnFontColor);
	m_downloadLblCoverSet = _addLabel("DOWNLOAD/COVERSSET", theme.btnFont, L"", 40, 250, 320, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadBtnCoverSet = _addButton("DOWNLOAD/COVERSET_BTN", theme.btnFont, L"", 370, 250, 230, 56, theme.btnFontColor);
	m_downloadLblGameTDBDownload = _addLabel("DOWNLOAD/GAMETDB_DOWNLOAD", theme.btnFont, L"", 40, 310, 320, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadBtnGameTDBDownload = _addButton("DOWNLOAD/GAMETDB_DOWNLOAD_BTN", theme.btnFont, L"", 370, 310, 230, 56, theme.btnFontColor);
	m_downloadLblGameTDB = _addLabel("DOWNLOAD/GAMETDB", theme.lblFont, L"", 40, 390, 370, 60, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadLblMessage[0] = _addLabel("DOWNLOAD/MESSAGE1", theme.lblFont, L"", 40, 228, 560, 100, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_downloadLblMessage[1] = _addLabel("DOWNLOAD/MESSAGE2", theme.lblFont, L"", 40, 228, 560, 100, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);

	// Cover settings
	m_downloadLblSetTitle = _addTitle("DOWNLOAD/SETTITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_downloadLblCoverPrio = _addLabel("DOWNLOAD/COVERPRIO", theme.lblFont, L"", 40, 100, 290, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadLblPrio = _addLabel("DOWNLOAD/PRIO_BTN", theme.btnFont, L"", 366, 100, 178, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_downloadBtnPrioM = _addPicButton("DOWNLOAD/PRIO_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 310, 100, 56, 56);
	m_downloadBtnPrioP = _addPicButton("DOWNLOAD/PRIO_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 100, 56, 56);
	m_downloadLblRegion = _addLabel("DOWNLOAD/REGION", theme.lblFont, L"", 40, 160, 600, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadBtnEN = _addPicButton("DOWNLOAD/EN", theme.btnENOff, theme.btnENOffs, 60, 220, 120, 56);
	m_downloadBtnJA = _addPicButton("DOWNLOAD/JA", theme.btnJAOff, theme.btnJAOffs, 195, 220, 120, 56);
	m_downloadBtnFR = _addPicButton("DOWNLOAD/FR", theme.btnFROff, theme.btnFROffs, 330, 220, 120, 56);
	m_downloadBtnDE = _addPicButton("DOWNLOAD/DE", theme.btnDEOff, theme.btnDEOffs, 465, 220, 120, 56);
	m_downloadBtnES = _addPicButton("DOWNLOAD/ES", theme.btnESOff, theme.btnESOffs, 60, 280, 120, 56);
	m_downloadBtnIT = _addPicButton("DOWNLOAD/IT", theme.btnITOff, theme.btnITOffs, 195, 280, 120, 56);
	m_downloadBtnNL = _addPicButton("DOWNLOAD/NL", theme.btnNLOff, theme.btnNLOffs, 330, 280, 120, 56);
	m_downloadBtnPT = _addPicButton("DOWNLOAD/PT", theme.btnPTOff, theme.btnPTOffs, 465, 280, 120, 56);
	m_downloadBtnRU = _addPicButton("DOWNLOAD/RU", theme.btnRUOff, theme.btnRUOffs, 60, 340, 120, 56);
	m_downloadBtnKO = _addPicButton("DOWNLOAD/KO", theme.btnKOOff, theme.btnKOOffs, 195, 340, 120, 56);
	m_downloadBtnZHCN = _addPicButton("DOWNLOAD/ZHCN", theme.btnZHCNOff, theme.btnZHCNOffs, 330, 340, 120, 56);
	m_downloadBtnAU = _addPicButton("DOWNLOAD/AU", theme.btnAUOff, theme.btnAUOffs, 465, 340, 120, 56);	
	m_downloadBtnENs = _addPicButton("DOWNLOAD/ENS", theme.btnENOn, theme.btnENOns, 60, 220, 120, 56);
	m_downloadBtnJAs = _addPicButton("DOWNLOAD/JAS", theme.btnJAOn, theme.btnJAOns, 195, 220, 120, 56);
	m_downloadBtnFRs = _addPicButton("DOWNLOAD/FRS", theme.btnFROn, theme.btnFROns, 330, 220, 120, 56);
	m_downloadBtnDEs = _addPicButton("DOWNLOAD/DES", theme.btnDEOn, theme.btnDEOns, 465, 220, 120, 56);
	m_downloadBtnESs = _addPicButton("DOWNLOAD/ESS", theme.btnESOn, theme.btnESOns, 60, 280, 120, 56);
	m_downloadBtnITs = _addPicButton("DOWNLOAD/ITS", theme.btnITOn, theme.btnITOns, 195, 280, 120, 56);
	m_downloadBtnNLs = _addPicButton("DOWNLOAD/NLS", theme.btnNLOn, theme.btnNLOns, 330, 280, 120, 56);
	m_downloadBtnPTs = _addPicButton("DOWNLOAD/PTS", theme.btnPTOn, theme.btnPTOns, 465, 280, 120, 56);
	m_downloadBtnRUs = _addPicButton("DOWNLOAD/RUS", theme.btnRUOn, theme.btnRUOns, 60, 340, 120, 56);
	m_downloadBtnKOs = _addPicButton("DOWNLOAD/KOS", theme.btnKOOn, theme.btnKOOns, 195, 340, 120, 56);
	m_downloadBtnZHCNs = _addPicButton("DOWNLOAD/ZHCNS", theme.btnZHCNOn, theme.btnZHCNOns, 330, 340, 120, 56);
	m_downloadBtnAUs = _addPicButton("DOWNLOAD/AUS", theme.btnAUOn, theme.btnAUOns, 465, 340, 120, 56);
	m_downloadBtnBack = _addButton("DOWNLOAD/BACK_BTN", theme.btnFont, L"", 420, 410, 200, 56, theme.btnFontColor);

	// Download menu
	_setHideAnim(m_downloadLblTitle, "DOWNLOAD/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadPBar, "DOWNLOAD/PROGRESS_BAR", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadLblCovers, "DOWNLOAD/COVERS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnCancel, "DOWNLOAD/CANCEL_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnAll, "DOWNLOAD/ALL_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnMissing, "DOWNLOAD/MISSING_BTN", 0, 0, -2.f, 0.f);	
	_setHideAnim(m_downloadLblCoverSet, "DOWNLOAD/COVERSSET", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnCoverSet, "DOWNLOAD/COVERSET_BTN", 0, 0, -2.f, 0.f);	
	_setHideAnim(m_downloadLblGameTDBDownload, "DOWNLOAD/GAMETDB_DOWNLOAD", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnGameTDBDownload, "DOWNLOAD/GAMETDB_DOWNLOAD_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadLblGameTDB, "DOWNLOAD/GAMETDB", 0, 0, -2.f, 0.f);

	// Cover settings	
	_setHideAnim(m_downloadLblSetTitle, "DOWNLOAD/SETTITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadLblCoverPrio, "DOWNLOAD/COVERPRIO", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadLblPrio, "DOWNLOAD/PRIO_BTN", 0, 0, -2.f, 0.f);		
	_setHideAnim(m_downloadBtnPrioM, "DOWNLOAD/PRIO_MINUS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnPrioP, "DOWNLOAD/PRIO_PLUS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadLblRegion, "DOWNLOAD/REGION", 0, 0, -2.f, 0.f);	
	_setHideAnim(m_downloadBtnEN, "DOWNLOAD/EN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnJA, "DOWNLOAD/JA", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnFR, "DOWNLOAD/FR", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnDE, "DOWNLOAD/DE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnES, "DOWNLOAD/ES", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnIT, "DOWNLOAD/IT", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnNL, "DOWNLOAD/NL", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnPT, "DOWNLOAD/PT", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnRU, "DOWNLOAD/RU", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnKO, "DOWNLOAD/KO", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnZHCN, "DOWNLOAD/ZHCN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnAU, "DOWNLOAD/AU", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnENs, "DOWNLOAD/ENS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnJAs, "DOWNLOAD/JAS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnFRs, "DOWNLOAD/FRS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnDEs, "DOWNLOAD/DES", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnESs, "DOWNLOAD/ESS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnITs, "DOWNLOAD/ITS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnNLs, "DOWNLOAD/NLS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnPTs, "DOWNLOAD/PTS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnRUs, "DOWNLOAD/RUS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnKOs, "DOWNLOAD/KOS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnZHCNs, "DOWNLOAD/ZHCNS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnAUs, "DOWNLOAD/AUS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnBack, "DOWNLOAD/BACK_BTN", 0, 0, -2.f, 0.f);

	m_downloadPrioVal = m_cfg.getUInt("GENERAL", "cover_prio", 0);

	_hideDownload(true);
	_textDownload();
}

void CMenu::_textDownload(void)
{
	m_btnMgr.setText(m_downloadBtnCancel, _t("dl1", L"Cancel"));
	m_btnMgr.setText(m_downloadBtnAll, _t("dl3", L"All"));
	m_btnMgr.setText(m_downloadBtnMissing, _t("dl4", L"Missing"));
	m_btnMgr.setText(m_downloadLblTitle, _t("dl5", L"Download"));
	m_btnMgr.setText(m_downloadBtnGameTDBDownload, _t("dl6", L"Download"));
	m_btnMgr.setText(m_downloadLblCovers, _t("dl8", L"Covers"));
	m_btnMgr.setText(m_downloadLblGameTDBDownload, _t("dl12", L"GameTDB"));
	m_btnMgr.setText(m_downloadLblGameTDB, _t("dl10", L"Please donate\nto GameTDB.com"));
	m_btnMgr.setText(m_downloadLblCoverPrio, _t("dl13", L"Download order"));
	m_btnMgr.setText(m_downloadLblRegion, _t("dl14", L"Select regions to check for covers:"));
	m_btnMgr.setText(m_downloadLblCoverSet, _t("dl15", L"Cover download settings"));
	m_btnMgr.setText(m_downloadBtnCoverSet, _t("dl16", L"Set"));
	m_btnMgr.setText(m_downloadLblSetTitle, _t("dl17", L"Cover download settings"));
	m_btnMgr.setText(m_downloadBtnEN, L"EN");
	m_btnMgr.setText(m_downloadBtnJA, L"JA");
	m_btnMgr.setText(m_downloadBtnFR, L"FR");
	m_btnMgr.setText(m_downloadBtnDE, L"DE");
	m_btnMgr.setText(m_downloadBtnES, L"ES");
	m_btnMgr.setText(m_downloadBtnIT, L"IT");
	m_btnMgr.setText(m_downloadBtnNL, L"NL");
	m_btnMgr.setText(m_downloadBtnPT, L"PT");
	m_btnMgr.setText(m_downloadBtnRU, L"RU");
	m_btnMgr.setText(m_downloadBtnKO, L"KO");
	m_btnMgr.setText(m_downloadBtnZHCN, L"ZHCN");
	m_btnMgr.setText(m_downloadBtnAU, L"AU");
	m_btnMgr.setText(m_downloadBtnENs, L"EN");
	m_btnMgr.setText(m_downloadBtnJAs, L"JA");
	m_btnMgr.setText(m_downloadBtnFRs, L"FR");
	m_btnMgr.setText(m_downloadBtnDEs, L"DE");
	m_btnMgr.setText(m_downloadBtnESs, L"ES");
	m_btnMgr.setText(m_downloadBtnITs, L"IT");
	m_btnMgr.setText(m_downloadBtnNLs, L"NL");
	m_btnMgr.setText(m_downloadBtnPTs, L"PT");
	m_btnMgr.setText(m_downloadBtnRUs, L"RU");
	m_btnMgr.setText(m_downloadBtnKOs, L"KO");
	m_btnMgr.setText(m_downloadBtnZHCNs, L"ZHCN");
	m_btnMgr.setText(m_downloadBtnAUs, L"AU");
	m_btnMgr.setText(m_downloadBtnBack, _t("dl18", L"Back"));
}

s8 CMenu::_versionTxtDownloaderInit(CMenu *m) //Handler to download versions txt file
{
	if (!m->m_thrdWorking) return 0;
	return m->_versionTxtDownloader();
}

s8 CMenu::_versionTxtDownloader() // code to download new version txt file
{
	u32 bufferSize = 0x010000;	// Maximum download size 64kb
	u8 *buffer = (u8*)MEM2_alloc(bufferSize);
	if(buffer == NULL)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(L"Not enough memory", 1.f);
		LWP_MutexUnlock(m_mutex);
		m_thrdWorking = false;
		return 0;
	}

	LWP_MutexLock(m_mutex);
	_setThrdMsg(_t("dlmsg1", L"Initializing network..."), 0.f);
	LWP_MutexUnlock(m_mutex);

	if (_initNetwork() < 0)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg2", L"Network initialization failed!"), 1.f);
		LWP_MutexUnlock(m_mutex);
	}
	else
	{
		// DLoad txt file
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg11", L"Downloading..."), 0.2f);
		LWP_MutexUnlock(m_mutex);

		m_thrdStep = 0.2f;
		m_thrdStepLen = 0.9f - 0.2f;
		gprintf("TXT update URL: %s\n\n", m_cfg.getString("GENERAL", "updatetxturl", UPDATE_URL_VERSION).c_str());
		download = downloadfile(buffer, bufferSize, m_cfg.getString("GENERAL", "updatetxturl", UPDATE_URL_VERSION).c_str(),CMenu::_downloadProgress, this);
		if (download.data == 0 || download.size < 19)
		{
			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg20", L"No version information found."), 1.f); // TODO: Check for 16
			LWP_MutexUnlock(m_mutex);
		}
		else
		{
			// txt download finished, now save file
			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg13", L"Saving..."), 0.9f);
			LWP_MutexUnlock(m_mutex);			
			
			FILE *file = fopen(m_ver.c_str(), "wb");
			if (file != NULL)
			{
				fwrite(download.data, 1, download.size, file);
				fclose(file);

				// version file valid, check for version with SVN_REV
				int svnrev = atoi(SVN_REV);
				gprintf("Installed Version: %d\n", svnrev);
				m_version.load(m_ver.c_str());
				int rev = m_version.getInt("GENERAL", "version", 0);
				gprintf("Latest available Version: %d\n", rev);
				if (svnrev < rev)
				{
					// new version available
					LWP_MutexLock(m_mutex);
					_setThrdMsg(_t("dlmsg19", L"New update available!"), 1.f);
					LWP_MutexUnlock(m_mutex);
				}
				else
				{
					// no new version available
					LWP_MutexLock(m_mutex);
					_setThrdMsg(_t("dlmsg17", L"No new updates found."), 1.f);
					LWP_MutexUnlock(m_mutex);
				}
			}
			else
			{
				LWP_MutexLock(m_mutex);
				_setThrdMsg(_t("dlmsg15", L"Saving failed!"), 1.f);
				LWP_MutexUnlock(m_mutex);
			}

		}
	}
	m_thrdWorking = false;
	free(buffer);
	return 0;
}

s8 CMenu::_versionDownloaderInit(CMenu *m) //Handler to download new dol
{
	if (!m->m_thrdWorking)
		return 0;
	return m->_versionDownloader();
}

s8 CMenu::_versionDownloader() // code to download new version
{
	char dol_backup[33];
	strcpy(dol_backup, m_dol.c_str());
	strcat(dol_backup, ".backup");

	if (m_app_update_size == 0)
		m_app_update_size	= 0x400000;
	if (m_data_update_size == 0)
		m_data_update_size	= 0x400000;

	// check for existing dol
	ifstream filestr;
	gprintf("DOL Path: %s\n", m_dol.c_str());
	filestr.open(m_dol.c_str());
	if (filestr.fail())
	{
		filestr.close();
		rename(dol_backup, m_dol.c_str());
		filestr.open(m_dol.c_str());
		if (filestr.fail())
		{
			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg18", L"boot.dol not found at default path!"), 1.f);
			LWP_MutexUnlock(m_mutex);
		}
		filestr.close();
		sleep(3);
		m_thrdWorking = false;
		return 0;
	}
	filestr.close();

	u32 bufferSize = max(m_app_update_size, m_data_update_size);	// Buffer for size of the biggest file.
	u8 *buffer = (u8*)MEM2_alloc(bufferSize);
	if(buffer == NULL)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(L"Not enough memory!", 1.f);
		LWP_MutexUnlock(m_mutex);
		sleep(3);
		m_thrdWorking = false;
		return 0;
	}

	LWP_MutexLock(m_mutex);
	_setThrdMsg(_t("dlmsg1", L"Initializing network..."), 0.f);
	LWP_MutexUnlock(m_mutex);
	
	if (_initNetwork() < 0)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg2", L"Network initialization failed!"), 1.f);
		LWP_MutexUnlock(m_mutex);
		sleep(3);
		m_thrdWorking = false;
		free(buffer);
		return 0;
	}

	// Load actual file
	LWP_MutexLock(m_mutex);
	_setThrdMsg(_t("dlmsg22", L"Updating application directory..."), 0.2f);
	LWP_MutexUnlock(m_mutex);

	m_thrdStep = 0.2f;
	m_thrdStepLen = 0.9f - 0.2f;
	gprintf("App Update URL: %s\n", m_app_update_url);
	gprintf("Data Update URL: %s\n", m_data_update_url);

	download = downloadfile(buffer, bufferSize, m_app_update_url, CMenu::_downloadProgress, this);
	if (download.data == 0 || download.size < m_app_update_size)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg12", L"Download failed!"), 1.f);
		LWP_MutexUnlock(m_mutex);
		sleep(3);
		m_thrdWorking = false;
		free(buffer);
		return 0;
	}

	// download finished, backup boot.dol and write new files.
	LWP_MutexLock(m_mutex);
	_setThrdMsg(_t("dlmsg13", L"Saving..."), 0.8f);
	LWP_MutexUnlock(m_mutex);			

	remove(dol_backup);
	rename(m_dol.c_str(), dol_backup);

	remove(m_app_update_zip.c_str());

	FILE *file = fopen(m_app_update_zip.c_str(), "wb");
	if (file != NULL)
	{
		fwrite(download.data, 1, download.size, file);
		fclose(file);

		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg24", L"Extracting..."), 0.8f);
		LWP_MutexUnlock(m_mutex);

		ZipFile zFile(m_app_update_zip.c_str());
		bool result = zFile.ExtractAll(m_app_update_drive);
		remove(m_app_update_zip.c_str());

		if (!result)
			goto fail;

		//Update apps dir succeeded, try to update the data dir.
		download.data = NULL;
		download.size = 0;

		memset(buffer, 0, bufferSize);  //should we be clearing the buffer of any possible data before downloading?

		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg23", L"Updating data directory..."), 0.2f);
		LWP_MutexUnlock(m_mutex);

		download = downloadfile(buffer, bufferSize, m_data_update_url, CMenu::_downloadProgress, this);
		if (download.data == 0 || download.size < m_data_update_size)
		{
			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg12", L"Download failed!"), 1.f);
			LWP_MutexUnlock(m_mutex);
			goto success;
		}

		// download finished, write new files.
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg13", L"Saving..."), 0.9f);
		LWP_MutexUnlock(m_mutex);

		remove(m_data_update_zip.c_str());

		file = fopen(m_data_update_zip.c_str(), "wb");
		if (file != NULL)
		{
			fwrite(download.data, 1, download.size, file);
			fclose(file);

			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg24", L"Extracting..."), 0.8f);
			LWP_MutexUnlock(m_mutex);

			ZipFile zDataFile(m_data_update_zip.c_str());
			result = zDataFile.ExtractAll(m_dataDir.c_str());
			remove(m_data_update_zip.c_str());

			if (!result)
			{
				LWP_MutexLock(m_mutex);
				_setThrdMsg(_t("dlmsg15", L"Saving failed!"), 1.f);
				LWP_MutexUnlock(m_mutex);
			}
		}
	}
	else
		goto fail;

success:
	LWP_MutexLock(m_mutex);
	_setThrdMsg(_t("dlmsg21", L"WiiFlow will now exit to allow the update to take effect."), 1.f);
	LWP_MutexUnlock(m_mutex);

	filestr.open(m_dol.c_str());
	if (filestr.fail())
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg25", L"Extraction must have failed! Renaming the backup to boot.dol"), 1.f);
		LWP_MutexUnlock(m_mutex);
		rename(dol_backup, m_dol.c_str());
	}
	else
		remove(dol_backup);
	filestr.close();

	m_exit = true;
	goto out;

fail:
	rename(dol_backup, m_dol.c_str());
	LWP_MutexLock(m_mutex);
	_setThrdMsg(_t("dlmsg15", L"Saving failed!"), 1.f);
	LWP_MutexUnlock(m_mutex);
out:
	free(buffer);
	sleep(3);
	m_thrdWorking = false;
	return 0;
}

int CMenu::_gametdbDownloader(CMenu *m)
{
	if (!m->m_thrdWorking) return 0;
	return m->_gametdbDownloaderAsync();
}

int CMenu::_gametdbDownloaderAsync()
{
	string langCode;

	u32 bufferSize = 0x800000; // 8 MB
	u8 *buffer = (u8*)MEM2_alloc(bufferSize);
	if (buffer == NULL)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(L"Not enough memory", 1.f);
		LWP_MutexUnlock(m_mutex);
		return 0;
	}
	langCode = m_loc.getString(m_curLanguage, "gametdb_code", "EN");
	LWP_MutexLock(m_mutex);
	_setThrdMsg(_t("dlmsg1", L"Initializing network..."), 0.f);
	LWP_MutexUnlock(m_mutex);
	if (_initNetwork() < 0)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg2", L"Network initialization failed!"), 1.f);
		LWP_MutexUnlock(m_mutex);
	}
	else
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg11", L"Downloading..."), 0.0f);
		LWP_MutexUnlock(m_mutex);
		m_thrdStep = 0.0f;
		m_thrdStepLen = 1.0f;
		download = downloadfile(buffer, bufferSize, fmt(GAMETDB_URL, langCode.c_str()), CMenu::_downloadProgress, this);
		if (download.data == 0)
		{
			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg12", L"Download failed!"), 1.f);
			LWP_MutexUnlock(m_mutex);
		}
		else
		{
			string zippath = fmt("%s/wiitdb.zip", m_settingsDir.c_str());

			gprintf("Downloading file to '%s'\n", zippath.c_str());

			remove(zippath.c_str());
			
			_setThrdMsg(wfmt(_fmt("dlmsg4", L"Saving %s"), "wiitdb.zip"), 1.f);
			FILE *file = fopen(zippath.c_str(), "wb");
			if (file == NULL)
			{
				gprintf("Can't save zip file\n");

				LWP_MutexLock(m_mutex);
				_setThrdMsg(_t("dlmsg15", L"Couldn't save ZIP file"), 1.f);
				LWP_MutexUnlock(m_mutex);
			}
			else
			{
				fwrite(download.data, download.size, 1, file);
				fclose(file);

				gprintf("Extracting zip file: ");
				
				ZipFile zFile(zippath.c_str());
				bool zres = zFile.ExtractAll(m_settingsDir.c_str());
				
				gprintf(zres ? "success\n" : "failed\n");

				// We don't need the zipfile anymore
				remove(zippath.c_str());

				// We should always remove the offsets file to make sure it's reloaded
				string offsetspath = fmt("%s/gametdb_offsets.bin", m_settingsDir.c_str());
				remove(offsetspath.c_str());

				// Update cache
				//m_gameList.SetLanguage(m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str());
				UpdateCache();
				
				LWP_MutexLock(m_mutex);
				_setThrdMsg(_t("dlmsg26", L"Updating cache..."), 0.f);
				LWP_MutexUnlock(m_mutex);

				m_GameTDBLoaded = true;

				_loadList();
				_initCF();
			}
		}
	}
	free(buffer);
	m_thrdWorking = false;
	return 0;
}
