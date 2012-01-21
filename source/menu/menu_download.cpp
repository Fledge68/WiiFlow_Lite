#include "menu.hpp"
#include "svnrev.h"
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "http.h"
#include "pngu.h"
#include "defines.h"

#include "loader/fs.h"
#include "loader/wdvd.h"
#include "usbstorage.h"
#include "unzip/ZipFile.h"

#include <network.h>

#include "gecko.h"
#include "wifi_gecko.h"
#include <fstream>
#include "lockMutex.hpp"

#include <ogc/lwp_watchdog.h>
#include <time.h>

#define TAG_GAME_ID		"{gameid}"
#define TAG_LOC			"{loc}"
#define TITLES_URL		"http://www.gametdb.com/titles.txt?LANG=%s"
#define GAMETDB_URL		"http://www.gametdb.com/wiitdb.zip?LANG=%s&FALLBACK=TRUE&WIIWARE=TRUE"
#define UPDATE_URL_VERSION	"http://update.wiiflow.org/txt/versions.txt"

using namespace std;

static const char FMT_BPIC_URL[] = "http://art.gametdb.com/wii/coverfullHQ/{loc}/{gameid}.png"\
"|http://art.gametdb.com/wii/coverfull/{loc}/{gameid}.png";
static const char FMT_PIC_URL[] = "http://art.gametdb.com/wii/cover/{loc}/{gameid}.png";

static block download = { 0, 0 };
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
	url.replace(url.find(TAG_LOC), strlen(TAG_LOC), country.c_str());
	url.replace(url.find(TAG_GAME_ID), strlen(TAG_GAME_ID), gameId.c_str());

	return url;
}

void CMenu::_hideDownload(bool instant)
{
	m_btnMgr.hide(m_downloadLblTitle, instant);
	m_btnMgr.hide(m_downloadBtnCancel, instant);
	m_btnMgr.hide(m_downloadBtnAll, instant);
	m_btnMgr.hide(m_downloadBtnMissing, instant);
	m_btnMgr.hide(m_downloadBtnGameTDBDownload, instant);
	m_btnMgr.hide(m_downloadPBar, instant);
	m_btnMgr.hide(m_downloadLblMessage[0], 0, 0, -2.f, 0.f, instant);
	m_btnMgr.hide(m_downloadLblMessage[1], 0, 0, -2.f, 0.f, instant);
	m_btnMgr.hide(m_downloadLblCovers, instant);
	m_btnMgr.hide(m_downloadLblGameTDBDownload, instant);
	m_btnMgr.hide(m_downloadLblGameTDB, instant);
	for (u32 i = 0; i < ARRAY_SIZE(m_downloadLblUser); ++i)
		if (m_downloadLblUser[i] != -1u)
			m_btnMgr.hide(m_downloadLblUser[i], instant);
}

void CMenu::_showDownload(void)
{
	_setBg(m_downloadBg, m_downloadBg);
	m_btnMgr.show(m_downloadLblGameTDB);
	m_btnMgr.show(m_downloadLblTitle);
	m_btnMgr.show(m_downloadBtnCancel);
	m_btnMgr.show(m_downloadBtnAll);
	m_btnMgr.show(m_downloadBtnMissing);
	m_btnMgr.show(m_downloadLblCovers);
	if (!m_locked)
	{
		m_btnMgr.show(m_downloadLblGameTDBDownload);
		m_btnMgr.show(m_downloadBtnGameTDBDownload);
	}
	for (u32 i = 0; i < ARRAY_SIZE(m_downloadLblUser); ++i)
		if (m_downloadLblUser[i] != -1u)
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
	SmartBuf buffer;
	FILE *file = fopen(filename, "rb");
	if (file == NULL) return false;
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	if (fileSize > 0)
	{
		buffer = smartAnyAlloc(fileSize);
		if (!!buffer) fread(buffer.get(), 1, fileSize, file);
	}
	SAFE_CLOSE(file);
	return !buffer ? false : checkPNGBuf(buffer.get());
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
	u8 *buf = ISFS_GetFile((u8 *) "/shared2/sys/net/02/config.dat", &size, -1);
	if (buf && size > 4)
	{
		retval = buf[4] > 0; // There is a valid connection defined.
		SAFE_FREE(buf);
	}
	return retval;
}

s32 CMenu::_networkComplete(s32 ok, void *usrData)
{
	CMenu *m = (CMenu *) usrData;
	
	m->m_networkInit = ok == 0;
	m->m_thrdNetwork = false;

	bool wifigecko = m->m_cfg.getBool("GENERAL", "wifi_gecko", false);
	gprintf("NET: Network init complete, enabled wifi_gecko: %s\n", wifigecko ? "yes" : "no");

	if (wifigecko)
	{
		// Get ip
		std::string ip = m->m_cfg.getString("GENERAL", "wifi_gecko_ip");
		u16 port = m->m_cfg.getInt("GENERAL", "wifi_gecko_port");

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
	while (net_get_status() == -EBUSY || m_thrdNetwork) {}; // Async initialization may be busy, wait to see if it succeeds.
	if (m_networkInit) return 0;
	if (!_isNetworkAvailable()) return -2;

	char ip[16];
	int val = if_config(ip, NULL, NULL, true);
	
	m_networkInit = !val;
	return val;
}

#define STACK_ALIGN(type, name, cnt, alignment)         \
					u8 _al__##name[((sizeof(type)*(cnt)) + (alignment) + \
					(((sizeof(type)*(cnt))%(alignment)) > 0 ? ((alignment) - \
					((sizeof(type)*(cnt))%(alignment))) : 0))]; \
					type *name = (type*)(((u32)(_al__##name)) + ((alignment) - (( \
					(u32)(_al__##name))&((alignment)-1))))

void CMenu::_deinitNetwork()
{
	net_wc24cleanup();
	net_deinit();
	m_networkInit = false;
}

int CMenu::_coverDownloader(bool missingOnly)
{
	string path;
	safe_vector<string> coverList;
	int count = 0, countFlat = 0;
	float listWeight = missingOnly ? 0.125f : 0.f;	// 1/8 of the progress bar for testing the PNGs we already have
	float dlWeight = 1.f - listWeight;

	u32 bufferSize = 0x280000;	// Maximum download size 2 MB
	SmartBuf buffer = smartAnyAlloc(bufferSize);
	if (!buffer)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(L"Not enough memory!", 1.f);
		LWP_MutexUnlock(m_mutex);
		m_thrdWorking = false;
		return 0;
	}
	bool savePNG = m_cfg.getBool("GENERAL", "keep_png", true);

	safe_vector<string> fmtURLBox = stringToVector(m_cfg.getString("GENERAL", "url_full_covers", FMT_BPIC_URL), '|');
	safe_vector<string> fmtURLFlat = stringToVector(m_cfg.getString("GENERAL", "url_flat_covers", FMT_PIC_URL), '|');

	u32 nbSteps = m_gameList.size();
	u32 step = 0;
	if (m_coverDLGameId.empty())
	{
		coverList.reserve(m_gameList.size());
		for (u32 i = 0; i < m_gameList.size() && !m_thrdStop; ++i)
		{
			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg7", L"Listing covers to download..."), listWeight * (float)step / (float)nbSteps);
			LWP_MutexUnlock(m_mutex);
			++step;
			string id((const char *)m_gameList[i].hdr.id, sizeof m_gameList[i].hdr.id);
			path = sfmt("%s/%s.png", m_boxPicDir.c_str(), id.c_str());
			if (!missingOnly || (!m_cf.fullCoverCached(id.c_str()) && !checkPNGFile(path.c_str())))
				coverList.push_back(id);
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
			return 0;
		}
		m_thrdStepLen = dlWeight / (float)nbSteps;

		Config m_newID;
		m_newID.load(sfmt("%s/newid.ini", m_settingsDir.c_str()).c_str());
		m_newID.setString("CHANNELS", "WFSF", "DWFA");

		for (u32 i = 0; i < coverList.size() && !m_thrdStop; ++i)
		{
			// Try to get the full cover
			string url;
			const char *domain = _domainFromView();
			bool success = false;
			FILE *file = NULL;

			string newID = m_newID.getString(domain, coverList[i], coverList[i]);

 			if(!newID.empty() && strncasecmp(newID.c_str(), coverList[i].c_str(), m_current_view != COVERFLOW_USB ? 4 : 6) == 0)
				m_newID.remove(domain, coverList[i]);
			else if(!newID.empty())
			{
				 gprintf("old id = %s\nnew id = %s\n", coverList[i].c_str(), newID.c_str());
			}

			for (u32 j = 0; !success && j < fmtURLBox.size() && !m_thrdStop; ++j)
			{

				url = makeURL(fmtURLBox[j], newID, countryCode(newID));
				if (j == 0) ++step;
				m_thrdStep = listWeight + dlWeight * (float)step / (float)nbSteps;
				LWP_MutexLock(m_mutex);
				_setThrdMsg(wfmt(_fmt("dlmsg3", L"Downloading from %s"), url.c_str()), m_thrdStep);
				LWP_MutexUnlock(m_mutex);
				download = downloadfile(buffer.get(), bufferSize, url.c_str(), CMenu::_downloadProgress, this);

				if (download.data == NULL && newID[3] != 'E')
				{
					url = makeURL(fmtURLBox[j], newID, "US");
					LWP_MutexLock(m_mutex);
					_setThrdMsg(wfmt(_fmt("dlmsg3", L"Downloading from %s"), url.c_str()), m_thrdStep);
					LWP_MutexUnlock(m_mutex);
					download = downloadfile(buffer.get(), bufferSize, url.c_str(), CMenu::_downloadProgress, this);
				}

				if (download.data != NULL)
				{
					if (savePNG)
					{
						path = sfmt("%s/%s.png", m_boxPicDir.c_str(), coverList[i].c_str());
						LWP_MutexLock(m_mutex);
						_setThrdMsg(wfmt(_fmt("dlmsg4", L"Saving %s"), path.c_str()), listWeight + dlWeight * (float)(step + 1) / (float)nbSteps);
						LWP_MutexUnlock(m_mutex);
						file = fopen(path.c_str(), "wb");
						if (file != NULL)
						{
							fwrite(download.data, download.size, 1, file);
							SAFE_CLOSE(file);
						}
					}

					LWP_MutexLock(m_mutex);
					_setThrdMsg(wfmt(_fmt("dlmsg10", L"Making %s"), sfmt("%s.wfc", coverList[i].c_str()).c_str()), listWeight + dlWeight * (float)(step + 1) / (float)nbSteps);
					LWP_MutexUnlock(m_mutex);
					if (m_cf.preCacheCover(coverList[i].c_str(), download.data, true))
					{
						++count;
						success = true;
					}
				}
			}
			if (!success && !m_thrdStop)
			{
				path = sfmt("%s/%s.png", m_picDir.c_str(), coverList[i].c_str());
				if (!checkPNGFile(path.c_str()))
				{
					// Try to get the front cover
					if (m_thrdStop) break;
					for (u32 j = 0; !success && j < fmtURLFlat.size() && !m_thrdStop; ++j)
					{
						url = makeURL(fmtURLFlat[j], newID, countryCode(newID));
						LWP_MutexLock(m_mutex);
						_setThrdMsg(wfmt(_fmt("dlmsg8", L"Full cover not found. Downloading from %s"), url.c_str()), listWeight + dlWeight * (float)step / (float)nbSteps);
						LWP_MutexUnlock(m_mutex);
						download = downloadfile(buffer.get(), bufferSize, url.c_str(), CMenu::_downloadProgress, this);
						if (download.data == NULL && newID[3] != 'E')
						{
							url = makeURL(fmtURLFlat[j], newID, "EN");
							LWP_MutexLock(m_mutex);
							_setThrdMsg(wfmt(_fmt("dlmsg3", L"Downloading from %s"), url.c_str()), m_thrdStep);
							LWP_MutexUnlock(m_mutex);
							download = downloadfile(buffer.get(), bufferSize, url.c_str(), CMenu::_downloadProgress, this);
						}
						if (download.data != NULL)
						{
							if (savePNG)
							{
								LWP_MutexLock(m_mutex);
								_setThrdMsg(wfmt(_fmt("dlmsg4", L"Saving %s"), path.c_str()), listWeight + dlWeight * (float)(step + 1) / (float)nbSteps);
								LWP_MutexUnlock(m_mutex);
								file = fopen(path.c_str(), "wb");
								if (file != NULL)
								{
									fwrite(download.data, download.size, 1, file);
									SAFE_CLOSE(file);
								}
							}

							LWP_MutexLock(m_mutex);
							_setThrdMsg(wfmt(_fmt("dlmsg10", L"Making %s"), sfmt("%s.wfc", coverList[i].c_str()).c_str()), listWeight + dlWeight * (float)(step + 1) / (float)nbSteps);
							LWP_MutexUnlock(m_mutex);
							if (m_cf.preCacheCover(coverList[i].c_str(), download.data, false))
							{
								++countFlat;
								success = true;
							}
						}
					}
				}
			}
			newID.clear();
			++step;
		}
		coverList.clear();
		m_newID.unload();
	}
	LWP_MutexLock(m_mutex);
	if (countFlat == 0)
		_setThrdMsg(wfmt(_fmt("dlmsg5", L"%i/%i files downloaded."), count, n), 1.f);
	else
		_setThrdMsg(wfmt(_fmt("dlmsg9", L"%i/%i files downloaded. %i are front covers only."), count + countFlat, n, countFlat), 1.f);
	LWP_MutexUnlock(m_mutex);
	m_thrdWorking = false;
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
	m_coverDLGameId = gameId;
	while (true)
	{
		_mainLoopCommon(false, m_thrdWorking);
		if ((BTN_HOME_PRESSED || BTN_B_PRESSED) && !m_thrdWorking)
			break;
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
				m_btnMgr.hide(m_downloadBtnAll);
				m_btnMgr.hide(m_downloadBtnMissing);
				m_btnMgr.hide(m_downloadBtnGameTDBDownload);
				m_btnMgr.hide(m_downloadLblCovers);
				m_btnMgr.hide(m_downloadLblGameTDBDownload);
				m_thrdStop = false;
				m_thrdWorking = true;
				gameId.clear();
				if (dlAll)
					LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_coverDownloaderAll, (void *)this, 0, 8192, 40);
				else
					LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_coverDownloaderMissing, (void *)this, 0, 8192, 40);
			}
			else if (m_btnMgr.selected(m_downloadBtnGameTDBDownload) && !m_thrdWorking)
			{
//				bool dlAll = m_btnMgr.selected(m_downloadBtnAllTitles);
				m_btnMgr.show(m_downloadPBar);
				m_btnMgr.setProgress(m_downloadPBar, 0.f);
				m_btnMgr.hide(m_downloadBtnAll);
				m_btnMgr.hide(m_downloadBtnMissing);
				m_btnMgr.hide(m_downloadBtnGameTDBDownload);
				m_btnMgr.hide(m_downloadLblCovers);
				m_btnMgr.hide(m_downloadLblGameTDBDownload);
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
		// 
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
}

void CMenu::_initDownloadMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_downloadLblUser, ARRAY_SIZE(m_downloadLblUser), "DOWNLOAD");
	m_downloadBg = _texture(theme.texSet, "DOWNLOAD/BG", "texture", theme.bg);
	m_downloadLblTitle = _addLabel(theme, "DOWNLOAD/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_downloadPBar = _addProgressBar(theme, "DOWNLOAD/PROGRESS_BAR", 40, 200, 560, 20);
	m_downloadBtnCancel = _addButton(theme, "DOWNLOAD/CANCEL_BTN", theme.btnFont, L"", 420, 410, 200, 56, theme.btnFontColor);
	m_downloadLblCovers = _addLabel(theme, "DOWNLOAD/COVERS", theme.btnFont, L"", 40, 150, 320, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadBtnAll = _addButton(theme, "DOWNLOAD/ALL_BTN", theme.btnFont, L"", 370, 150, 230, 56, theme.btnFontColor);
	m_downloadBtnMissing = _addButton(theme, "DOWNLOAD/MISSING_BTN", theme.btnFont, L"", 370, 210, 230, 56, theme.btnFontColor);
	m_downloadLblGameTDBDownload = _addLabel(theme, "DOWNLOAD/GAMETDB_DOWNLOAD", theme.btnFont, L"", 40, 270, 320, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadBtnGameTDBDownload = _addButton(theme, "DOWNLOAD/GAMETDB_DOWNLOAD_BTN", theme.btnFont, L"", 370, 270, 230, 56, theme.btnFontColor);
	m_downloadLblGameTDB = _addLabel(theme, "DOWNLOAD/GAMETDB", theme.btnFont, L"", 40, 400, 370, 60, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_downloadLblMessage[0] = _addLabel(theme, "DOWNLOAD/MESSAGE1", theme.lblFont, L"", 40, 228, 560, 100, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_downloadLblMessage[1] = _addLabel(theme, "DOWNLOAD/MESSAGE2", theme.lblFont, L"", 40, 228, 560, 100, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	//
	_setHideAnim(m_downloadLblTitle, "DOWNLOAD/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadPBar, "DOWNLOAD/PROGRESS_BAR", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadLblCovers, "DOWNLOAD/COVERS", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnCancel, "DOWNLOAD/CANCEL_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnAll, "DOWNLOAD/ALL_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnMissing, "DOWNLOAD/MISSING_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadLblGameTDBDownload, "DOWNLOAD/GAMETDB_DOWNLOAD", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadBtnGameTDBDownload, "DOWNLOAD/GAMETDB_DOWNLOAD_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_downloadLblGameTDB, "DOWNLOAD/GAMETDB", 0, 0, -2.f, 0.f);
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
}

s8 CMenu::_versionTxtDownloaderInit(CMenu *m) //Handler to download versions txt file
{
	if (!m->m_thrdWorking) return 0;
	return m->_versionTxtDownloader();
}

s8 CMenu::_versionTxtDownloader() // code to download new version txt file
{
	u32 bufferSize = 0x001000;	// Maximum download size 4kb
	SmartBuf buffer = smartAnyAlloc(bufferSize);
	if (!buffer)
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
		download = downloadfile(buffer.get(), bufferSize, m_cfg.getString("GENERAL", "updatetxturl", UPDATE_URL_VERSION).c_str(),CMenu::_downloadProgress, this);
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
				SAFE_CLOSE(file);

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
	return 0;
}

s8 CMenu::_versionDownloaderInit(CMenu *m) //Handler to download new dol
{
	if (!m->m_thrdWorking) return 0;
	return m->_versionDownloader();
}

s8 CMenu::_versionDownloader() // code to download new version
{
	char dol_backup[33];
	strcpy(dol_backup, m_dol.c_str());
	strcat(dol_backup, ".backup");

	if (m_app_update_size == 0)	 m_app_update_size	= 0x400000;
	if (m_data_update_size == 0) m_data_update_size	= 0x400000;
	
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
	SmartBuf buffer = smartAnyAlloc(bufferSize);
	if (!buffer)
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
		return 0;
	}

	// Load actual file
	LWP_MutexLock(m_mutex);
	_setThrdMsg(_t("dlmsg22", L"Updating application directory..."), 0.2f);
	LWP_MutexUnlock(m_mutex);

	m_thrdStep = 0.2f;
	m_thrdStepLen = 0.9f - 0.2f;
	gprintf("App Update URL: %s\n", m_app_update_url);
	gprintf("Data Update URL: %s\n", m_app_update_url);
	
	download = downloadfile(buffer.get(), bufferSize, m_app_update_url, CMenu::_downloadProgress, this);
	if (download.data == 0 || download.size < m_app_update_size)
	{
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg12", L"Download failed!"), 1.f);
		LWP_MutexUnlock(m_mutex);
		sleep(3);
		m_thrdWorking = false;
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
		SAFE_CLOSE(file);
		
		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg24", L"Extracting..."), 0.8f);
		LWP_MutexUnlock(m_mutex);

		ZipFile zFile(m_app_update_zip.c_str());
		bool result = zFile.ExtractAll(m_appDir.c_str());
		remove(m_app_update_zip.c_str());

		if (!result) goto fail;

		//Update apps dir succeeded, try to update the data dir.
		download.data = NULL;
		download.size = 0;

		//memset(&buffer, 0, bufferSize);  should we be clearing the buffer of any possible data before downloading?

		LWP_MutexLock(m_mutex);
		_setThrdMsg(_t("dlmsg23", L"Updating data directory..."), 0.2f);
		LWP_MutexUnlock(m_mutex);

		download = downloadfile(buffer.get(), bufferSize, m_data_update_url, CMenu::_downloadProgress, this);
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
			SAFE_CLOSE(file);
			
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
    filestr.close();

	m_exit = true;
	goto out;

fail:
	rename(dol_backup, m_dol.c_str());
	LWP_MutexLock(m_mutex);
	_setThrdMsg(_t("dlmsg15", L"Saving failed!"), 1.f);
	LWP_MutexUnlock(m_mutex);
out:
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
	SmartBuf buffer = smartAnyAlloc(bufferSize);
	if (!buffer)
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
		download = downloadfile(buffer.get(), bufferSize, sfmt(GAMETDB_URL, langCode.c_str()).c_str(), CMenu::_downloadProgress, this);
		if (download.data == 0)
		{
			LWP_MutexLock(m_mutex);
			_setThrdMsg(_t("dlmsg12", L"Download failed!"), 1.f);
			LWP_MutexUnlock(m_mutex);
		}
		else
		{
			string zippath = sfmt("%s/wiitdb.zip", m_settingsDir.c_str());

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
				SAFE_CLOSE(file);

				gprintf("Extracting zip file: ");
				
				ZipFile zFile(zippath.c_str());
				bool zres = zFile.ExtractAll(m_settingsDir.c_str());
				
				gprintf(zres ? "success\n" : "failed\n");

				// We don't need the zipfile anymore
				remove(zippath.c_str());
				
				// Update cache
				m_gameList.SetLanguage(m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str());
				UpdateCache();
				
				LWP_MutexLock(m_mutex);
				_setThrdMsg(_t("dlmsg26", L"Updating cache..."), 0.f);
				LWP_MutexUnlock(m_mutex);

				_loadList();
				_initCF();
			}
		}
	}
	m_thrdWorking = false;
	return 0;
}