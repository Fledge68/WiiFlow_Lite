#include <dirent.h>
#include <unistd.h>

#include "menu.hpp"
#include "defines.h"
#include "lockMutex.hpp"
#include "channel/nand.hpp"
#include "loader/cios.h"
#include "loader/nk.h"

// NandEmulation menu
s16 m_nandemuLblTitle;
s16 m_nandemuBtnBack;
s16 m_nandemuLblPage;
s16 m_nandemuBtnPageM;
s16 m_nandemuBtnPageP;
s16 m_nandemuLblEmulationVal;
s16 m_nandemuLblEmulation;
s16 m_nandemuBtnEmulationM;
s16 m_nandemuBtnEmulationP;
s16 m_nandemuLblSaveDump;
s16 m_nandemuBtnAll;
s16 m_nandemuBtnMissing;
s16 m_nandemuLblNandDump;
s16 m_nandemuBtnNandDump;
s16 m_nandemuLblNandFolder;
s16 m_nandemuBtnNandFolder;
s16 m_nandemuLblNandSavesFolder;
s16 m_nandemuBtnNandSavesFolder;
s16 m_nandfileLblMessage;
s16 m_nandemuLblMessage;
s16 m_nandfileLblDialog;
s16 m_nandfinLblDialog;
s16 m_nandemuLblDialog;
s16 m_nandfilePBar;
s16 m_nandemuPBar;
s16 m_nandemuBtnExtract;
s16 m_nandemuBtnDisable;
s16 m_nandemuBtnPartition;
s16 m_nandemuLblInit;
s16 m_nandemuLblUser[4];
TexData m_nandemuBg;

int nandemuPage = 1;

bool m_nandext;
bool m_fulldump;
bool m_sgdump;
bool m_saveall;

static inline int loopNum(int i, int s)
{
	return i < 0 ? (s - (-i % s)) % s : i % s;
}

static bool _saveExists(const char *path)
{	
	DIR *d = opendir(path);
	if(!d)
		return false;
	else
	{
		closedir(d);
		return true;
	}
}

bool CMenu::_TestEmuNand(int epart, const char *path, bool indept)
{
	char basepath[64];
	char testpath[MAX_FAT_PATH];
	snprintf(basepath, sizeof(basepath), "%s:%s", DeviceName[epart], path);

	DIR *d = opendir(basepath);
	if(!d)
		return false;
	else
		closedir(d);

	if(indept)
	{
		// Check Wiimotes && Region
		snprintf(testpath, sizeof(testpath), "%s:%s/shared2/sys/SYSCONF", DeviceName[epart], path);
		if(!fsop_FileExist(testpath))
			return false;
		snprintf(testpath, sizeof(testpath), "%s:%s/title/00000001/00000002/data/setting.txt", DeviceName[epart], path);
		if(!fsop_FileExist(testpath))
			return false;
		// Check Mii's
		snprintf(testpath, sizeof(testpath), "%s:%s/shared2/menu/FaceLib/RFL_DB.dat", DeviceName[epart], path);
		if(!fsop_FileExist(testpath))
			return false;
	}
	return true;
}

int CMenu::_FindEmuPart(string &emuPath, bool skipchecks)
{
	int emuPart = -1;
	const char *tmpPath = NULL;
	if(m_current_view == COVERFLOW_CHANNEL)
	{
		emuPart = m_cfg.getInt(CHANNEL_DOMAIN, "partition", 0);
		tmpPath = m_cfg.getString(CHANNEL_DOMAIN, "path", "").c_str();
		if(strlen(tmpPath) == 0)
		{
			tmpPath = STDEMU_DIR;
			m_cfg.setString(CHANNEL_DOMAIN, "path", STDEMU_DIR);
		}
	}
	else if(m_current_view == COVERFLOW_USB)
	{
		emuPart = m_cfg.getInt(WII_DOMAIN, "savepartition", -1);
		if(emuPart == -1)
			emuPart = m_cfg.getInt(CHANNEL_DOMAIN, "partition", 0);
		tmpPath = m_cfg.getString(WII_DOMAIN, "savepath", m_cfg.getString(CHANNEL_DOMAIN, "path", "")).c_str();
		if(strlen(tmpPath) == 0)
		{
			tmpPath = STDEMU_DIR;
			m_cfg.setString(WII_DOMAIN, "savepath", STDEMU_DIR);
		}
	}
	if(!DeviceHandle.PartitionUsableForNandEmu(emuPart))
		return -1;
	else if((skipchecks || _TestEmuNand(emuPart, tmpPath, true)))
	{
		NandHandle.SetNANDEmu(emuPart);
		NandHandle.SetPaths(tmpPath, DeviceName[emuPart]);
		emuPath = tmpPath;
		return emuPart;
	}
	return -2;
}

bool CMenu::_checkSave(string id, bool nand)
{
	int savePath = id.c_str()[0] << 24 | id.c_str()[1] << 16 | id.c_str()[2] << 8 | id.c_str()[3];
	if(nand)
	{
		u32 temp = 0;
		if(ISFS_ReadDir(fmt("/title/00010000/%08x", savePath), NULL, &temp) < 0)
			if(ISFS_ReadDir(fmt("/title/00010004/%08x", savePath), NULL, &temp) < 0)
				return false;
	}
	else
	{
		int emuPartition = m_cfg.getInt(WII_DOMAIN, "savepartition", -1);
		string emuPath = m_cfg.getString(WII_DOMAIN, "savepath", "");
		if(emuPartition < 0 || emuPath.size() == 0)
			return false;
		struct stat fstat;
		if((stat(fmt("%s:%s/title/00010000/%08x", DeviceName[emuPartition], emuPath.c_str(), savePath), &fstat) != 0 ) 
			&& (stat(fmt("%s:%s/title/00010004/%08x", DeviceName[emuPartition], emuPath.c_str(), savePath), &fstat) != 0))
			return false;
	}
	return true;
}

void CMenu::_setDumpMsg(const wstringEx &msg, float totprog, float fileprog)
{
	if(m_thrdStop) return;
	if(msg != L"...") m_thrdMessage = msg;
	m_thrdMessageAdded = true;
	m_thrdProgress = totprog;
	m_fileProgress = fileprog;
}

void CMenu::_ShowProgress(int dumpstat, int dumpprog, int filesize, int fileprog, int files, int folders, const char *tmess, void *user_data)
{
	CMenu &m = *(CMenu *)user_data;
	m.m_progress = dumpprog == 0 ? 0.f : (float)dumpstat / (float)dumpprog;
	m.m_fprogress = filesize == 0 ? 0.f : (float)fileprog / (float)filesize;
	m.m_fileprog = fileprog;
	m.m_filesize = filesize;
	m.m_filesdone = files;
	m.m_foldersdone = folders;
	LWP_MutexLock(m.m_mutex);
	if(m_nandext)
		m._setDumpMsg(wfmt(m._fmt("cfgne9", L"Current file: %s"), tmess), m.m_progress, m.m_fprogress);
	else
		m._setDumpMsg(L"...", m.m_progress, m.m_fprogress);
	LWP_MutexUnlock(m.m_mutex);
}

void CMenu::_hideNandEmu(bool instant)
{
	m_btnMgr.hide(m_nandemuLblTitle, instant);
	m_btnMgr.hide(m_nandemuBtnBack, instant);
	m_btnMgr.hide(m_nandemuLblPage, instant);
	m_btnMgr.hide(m_nandemuBtnPageM, instant);
	m_btnMgr.hide(m_nandemuBtnPageP, instant);
	m_btnMgr.hide(m_nandfilePBar, instant);
	m_btnMgr.hide(m_nandemuPBar, instant);
	m_btnMgr.hide(m_nandfileLblMessage, instant);
	m_btnMgr.hide(m_nandemuLblMessage, instant);
	m_btnMgr.hide(m_nandfileLblDialog, instant);
	m_btnMgr.hide(m_nandemuLblDialog, instant);
	m_btnMgr.hide(m_nandfinLblDialog, instant);
	m_btnMgr.hide(m_nandemuLblEmulationVal, instant);
	m_btnMgr.hide(m_nandemuLblEmulation, instant);
	m_btnMgr.hide(m_nandemuBtnEmulationP, instant);
	m_btnMgr.hide(m_nandemuBtnEmulationM, instant);
	m_btnMgr.hide(m_nandemuLblSaveDump, instant);
	m_btnMgr.hide(m_nandemuBtnAll, instant);
	m_btnMgr.hide(m_nandemuBtnMissing, instant);
	m_btnMgr.hide(m_nandemuLblNandDump, instant);
	m_btnMgr.hide(m_nandemuBtnNandDump, instant);
	m_btnMgr.hide(m_nandemuLblNandFolder, instant);
	m_btnMgr.hide(m_nandemuBtnNandFolder, instant);
	m_btnMgr.hide(m_nandemuLblNandSavesFolder, instant);
	m_btnMgr.hide(m_nandemuBtnNandSavesFolder, instant);
	m_btnMgr.hide(m_nandemuBtnExtract, instant);
	m_btnMgr.hide(m_nandemuBtnPartition, instant);
	m_btnMgr.hide(m_nandemuBtnDisable, instant);
	m_btnMgr.hide(m_nandemuLblInit, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_nandemuLblUser); ++i)
		if(m_nandemuLblUser[i] != -1)
			m_btnMgr.hide(m_nandemuLblUser[i], instant); 
}

void CMenu::_showNandEmu(void)
{
	_setBg(m_nandemuBg, m_nandemuBg);
	m_btnMgr.setText(m_nandemuLblTitle, _t("cfgne10", L"NAND Emulation Settings"));
	m_btnMgr.show(m_nandemuLblTitle);
	m_btnMgr.show(m_nandemuBtnBack);
	
	m_btnMgr.setText(m_nandemuLblPage, wfmt(L"%i / 2", nandemuPage));
	m_btnMgr.show(m_nandemuLblPage);
	m_btnMgr.show(m_nandemuBtnPageM);
	m_btnMgr.show(m_nandemuBtnPageP);

	if(nandemuPage == 1)
	{
		int i;
		if(((m_current_view == COVERFLOW_CHANNEL && !m_cfg.getBool(CHANNEL_DOMAIN, "disable", true)) || m_current_view == COVERFLOW_USB))
		{
			m_btnMgr.show(m_nandemuLblEmulation);
			m_btnMgr.show(m_nandemuLblEmulationVal);
			m_btnMgr.show(m_nandemuBtnEmulationP);
			m_btnMgr.show(m_nandemuBtnEmulationM);
		}
	
		if((m_current_view == COVERFLOW_CHANNEL || m_current_view == COVERFLOW_USB))
		{
			m_btnMgr.show(m_nandemuLblSaveDump);
			m_btnMgr.show(m_nandemuBtnAll);
			m_btnMgr.show(m_nandemuBtnMissing);
			m_btnMgr.show(m_nandemuLblNandDump);
			m_btnMgr.show(m_nandemuBtnNandDump);
			if (m_current_view == COVERFLOW_CHANNEL)
			{
				i = min(max(0, m_cfg.getInt(CHANNEL_DOMAIN, "emulation", 0)), (int)ARRAY_SIZE(CMenu::_NandEmu) - 1);
				m_btnMgr.setText(m_nandemuLblEmulationVal, _t(CMenu::_NandEmu[i].id, CMenu::_NandEmu[i].text));
			}
			else if (m_current_view == COVERFLOW_USB)
			{
				i = min(max(0, m_cfg.getInt(WII_DOMAIN, "save_emulation", 0)), (int)ARRAY_SIZE(CMenu::_GlobalSaveEmu) - 1);
				m_btnMgr.setText(m_nandemuLblEmulationVal, _t(CMenu::_GlobalSaveEmu[i].id, CMenu::_GlobalSaveEmu[i].text));
			}
		}
	}
	else
	{
		m_btnMgr.show(m_nandemuLblNandFolder);
		m_btnMgr.show(m_nandemuBtnNandFolder);
		m_btnMgr.show(m_nandemuLblNandSavesFolder);
		m_btnMgr.show(m_nandemuBtnNandSavesFolder);
	}
	for(u8 i = 0; i < ARRAY_SIZE(m_nandemuLblUser); ++i)
		if(m_nandemuLblUser[i] != -1)
			m_btnMgr.show(m_nandemuLblUser[i]);
}

int CMenu::_NandEmuCfg(void)
{	
	string path = "";
	nandemuPage = 1;

	lwp_t thread = 0;
	SetupInput();
	_showNandEmu();

	m_thrdStop = false;
	m_thrdMessageAdded = false;
	m_nandext = false;
	
	while(!m_exit)
	{
		_mainLoopCommon();
		if((BTN_MINUS_PRESSED || BTN_LEFT_PRESSED) || (BTN_A_PRESSED && m_btnMgr.selected(m_nandemuBtnPageM)))
		{
			m_btnMgr.hide(m_nandemuLblEmulationVal, true);
			m_btnMgr.hide(m_nandemuLblEmulation, true);
			m_btnMgr.hide(m_nandemuBtnEmulationP, true);
			m_btnMgr.hide(m_nandemuBtnEmulationM, true);
			m_btnMgr.hide(m_nandemuLblSaveDump, true);
			m_btnMgr.hide(m_nandemuBtnAll, true);
			m_btnMgr.hide(m_nandemuBtnMissing, true);
			m_btnMgr.hide(m_nandemuLblNandDump, true);
			m_btnMgr.hide(m_nandemuBtnNandDump, true);
			m_btnMgr.hide(m_nandemuLblNandFolder, true);
			m_btnMgr.hide(m_nandemuBtnNandFolder, true);
			m_btnMgr.hide(m_nandemuLblNandSavesFolder, true);
			m_btnMgr.hide(m_nandemuBtnNandSavesFolder, true);
			
			nandemuPage = nandemuPage == 1 ? 2 : 1;
			_showNandEmu();
		}
		else if((BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED) || (BTN_A_PRESSED && m_btnMgr.selected(m_nandemuBtnPageP)))
		{
			m_btnMgr.hide(m_nandemuLblEmulationVal, true);
			m_btnMgr.hide(m_nandemuLblEmulation, true);
			m_btnMgr.hide(m_nandemuBtnEmulationP, true);
			m_btnMgr.hide(m_nandemuBtnEmulationM, true);
			m_btnMgr.hide(m_nandemuLblSaveDump, true);
			m_btnMgr.hide(m_nandemuBtnAll, true);
			m_btnMgr.hide(m_nandemuBtnMissing, true);
			m_btnMgr.hide(m_nandemuLblNandDump, true);
			m_btnMgr.hide(m_nandemuBtnNandDump, true);
			m_btnMgr.hide(m_nandemuLblNandFolder, true);
			m_btnMgr.hide(m_nandemuBtnNandFolder, true);
			m_btnMgr.hide(m_nandemuLblNandSavesFolder, true);
			m_btnMgr.hide(m_nandemuBtnNandSavesFolder, true);
			
			nandemuPage = nandemuPage == 1 ? 2 : 1;
			_showNandEmu();
		}
		else if((BTN_HOME_PRESSED || BTN_B_PRESSED) && !m_thrdWorking)
			break;
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if (BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnEmulationP) || m_btnMgr.selected(m_nandemuBtnEmulationM)))
		{
			s8 direction = m_btnMgr.selected(m_nandemuBtnEmulationP) ? 1 : -1;
			if(m_current_view == COVERFLOW_CHANNEL)
				m_cfg.setInt(CHANNEL_DOMAIN, "emulation", (int)loopNum((u32)m_cfg.getInt(CHANNEL_DOMAIN, "emulation", 0) + direction, ARRAY_SIZE(CMenu::_NandEmu)));
			else if(m_current_view == COVERFLOW_USB)
				m_cfg.setInt(WII_DOMAIN, "save_emulation", (int)loopNum((u32)m_cfg.getInt(WII_DOMAIN, "save_emulation", 0) + direction, ARRAY_SIZE(CMenu::_GlobalSaveEmu)));
			_showNandEmu();
		}	
		else if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnNandDump) || m_btnMgr.selected(m_nandemuBtnAll) || m_btnMgr.selected(m_nandemuBtnMissing)))
		{
			m_fulldump = m_btnMgr.selected(m_nandemuBtnNandDump) ? true : false;
			m_saveall = m_btnMgr.selected(m_nandemuBtnAll) ? true : false;
			m_btnMgr.hide(m_nandemuBtnBack);
			m_btnMgr.hide(m_nandemuLblEmulationVal);
			m_btnMgr.hide(m_nandemuLblEmulation);
			m_btnMgr.hide(m_nandemuBtnEmulationP);
			m_btnMgr.hide(m_nandemuBtnEmulationM);
			m_btnMgr.hide(m_nandemuLblSaveDump);
			m_btnMgr.hide(m_nandemuBtnAll);
			m_btnMgr.hide(m_nandemuBtnMissing);
			m_btnMgr.hide(m_nandemuLblNandDump);
			m_btnMgr.hide(m_nandemuBtnNandDump);
			m_btnMgr.hide(m_nandemuLblPage);
			m_btnMgr.hide(m_nandemuBtnPageM);
			m_btnMgr.hide(m_nandemuBtnPageP);
			
			m_btnMgr.show(m_nandfilePBar);
			m_btnMgr.show(m_nandemuPBar);
			m_btnMgr.show(m_nandfileLblMessage);
			m_btnMgr.show(m_nandemuLblMessage);
			m_btnMgr.show(m_nandfileLblDialog);
			m_btnMgr.show(m_nandemuLblDialog);
			m_btnMgr.setText(m_nandemuLblMessage, L"");
			m_btnMgr.setText(m_nandfileLblMessage, L"");
			m_btnMgr.setText(m_nandemuLblDialog, _t("cfgne11", L"Overall Progress:"));
			if(m_fulldump)
				m_btnMgr.setText(m_nandemuLblTitle, _t("cfgne12", L"NAND Extractor"));
			else
				m_btnMgr.setText(m_nandemuLblTitle, _t("cfgne13", L"Game Save Extractor"));
			m_thrdStop = false;
			m_thrdProgress = 0.f;
			m_thrdWorking = true;
			LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_NandDumper, (void *)this, 0, 32768, 40);
		}
		else if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnNandFolder)))
		{
			_hideNandEmu(true);
			u8 tmpView = m_current_view;
			m_current_view = COVERFLOW_CHANNEL;
			string emuPath;
			_FindEmuPart(emuPath, true);
			const char *path = _FolderExplorer(NandHandle.GetPath());
			m_current_view = tmpView;
			if(strlen(path) > 0)
			{
				if(strncmp(path, "sd:/", 4) == 0)
					m_cfg.setInt(CHANNEL_DOMAIN, "partition", 0);
				else
				{
					const char *partval = &path[3];
					m_cfg.setInt(CHANNEL_DOMAIN, "partition", atoi(partval));
				}
				char tmpPath[MAX_FAT_PATH];
				strncpy(tmpPath, strchr(path, '/'), MAX_FAT_PATH-1);
				m_cfg.setString(CHANNEL_DOMAIN, "path", tmpPath);
				m_cfg.setBool(CHANNEL_DOMAIN, "update_cache", true);
				if(m_cfg.getBool(CHANNEL_DOMAIN, "source"))
					m_load_view = true;

			}
			_showNandEmu();
		}
		else if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnNandSavesFolder)))
		{
			_hideNandEmu(true);
			u8 tmpView = m_current_view;
			m_current_view = COVERFLOW_USB;
			string emuPath;
			_FindEmuPart(emuPath, true);
			const char *path = _FolderExplorer(NandHandle.GetPath());
			m_current_view = tmpView;
			if(strlen(path) > 0)
			{
				if(strncmp(path, "sd:/", 4) == 0)
					m_cfg.setInt(WII_DOMAIN, "savepartition", 0);
				else
				{
					const char *partval = &path[3];
					m_cfg.setInt(WII_DOMAIN, "savepartition", atoi(partval));
				}
				char tmpPath[MAX_FAT_PATH];
				strncpy(tmpPath, strchr(path, '/'), MAX_FAT_PATH-1);
				m_cfg.setString(WII_DOMAIN, "savepath", tmpPath);
			}
			_showNandEmu();
		}
		else if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnBack)))
		{
			m_cfg.save();
			break;
		}

		if (m_thrdMessageAdded)
		{
			LockMutex lock(m_mutex);
			m_thrdMessageAdded = false;
			if (!m_thrdMessage.empty())
				m_btnMgr.setText(m_nandfileLblDialog, m_thrdMessage);
			m_btnMgr.setProgress(m_nandfilePBar, m_fileProgress);
			m_btnMgr.setProgress(m_nandemuPBar, m_thrdProgress);
			m_btnMgr.setText(m_nandfileLblMessage, wfmt(_fmt("fileprogress", L"%d / %dKB"), m_fileprog/0x400, m_filesize/0x400));
			m_btnMgr.setText(m_nandemuLblMessage, wfmt(_fmt("dumpprogress", L"%i%%"), (int)(m_thrdProgress*100.f)));			

			if (!m_thrdWorking)
			{
				if(m_sgdump)
					m_btnMgr.setText(m_nandfinLblDialog, wfmt(_fmt("cfgne14", L"Extracted: %d saves / %d files / %d folders"), m_nandexentry, m_filesdone, m_foldersdone));
				else
					m_btnMgr.setText(m_nandfinLblDialog, wfmt(_fmt("cfgne15", L"Extracted: %d files / %d folders"), m_filesdone, m_foldersdone));
				if(m_dumpsize/0x400 > 0x270f)
					m_btnMgr.setText(m_nandemuLblDialog, wfmt(_fmt("cfgne16", L"Total size: %uMB (%d blocks)"), (m_dumpsize/0x100000), (m_dumpsize/0x8000)>>2));
				else
					m_btnMgr.setText(m_nandemuLblDialog, wfmt(_fmt("cfgne17", L"Total size: %uKB (%d blocks)"), (m_dumpsize/0x400), (m_dumpsize/0x8000)>>2));
				m_btnMgr.show(m_nandemuBtnBack);
				m_btnMgr.show(m_nandfinLblDialog);
			}
		}
	}
	_hideNandEmu();
	return 0;
}

int CMenu::_FlashSave(string gameId)
{
	int emuPartition = m_cfg.getInt(WII_DOMAIN, "savepartition", m_cfg.getInt(CHANNEL_DOMAIN, "partition", 0));
	char basepath[MAX_FAT_PATH];
	snprintf(basepath, sizeof(basepath), "%s:%s", DeviceName[emuPartition], m_cfg.getString(WII_DOMAIN, "savepath", m_cfg.getString(CHANNEL_DOMAIN, "path", "")).c_str());

	if(!_checkSave(gameId, false))
		return 0;

	lwp_t thread = 0;
	SetupInput();
	m_thrdStop = false;
	m_thrdMessageAdded = false;
	m_nandext = false;

	m_saveExtGameId = gameId;

	while(!m_exit)
	{
		_mainLoopCommon();
		if(m_forceext)
		{
			m_forceext = false;
			m_btnMgr.hide(m_nandemuLblInit);
			m_btnMgr.show(m_nandemuLblTitle);
			m_btnMgr.show(m_nandfilePBar);
			m_btnMgr.show(m_nandemuPBar);
			m_btnMgr.show(m_nandfileLblMessage);
			m_btnMgr.show(m_nandemuLblMessage);
			m_btnMgr.show(m_nandfileLblDialog);
			m_btnMgr.show(m_nandemuLblDialog);
			m_btnMgr.setText(m_nandemuLblMessage, L"");
			m_btnMgr.setText(m_nandfileLblMessage, L"");
			m_btnMgr.setText(m_nandemuLblDialog, _t("cfgne11", L"Overall Progress:"));
			m_btnMgr.setText(m_nandemuLblTitle, _t("cfgne28", L"Game Save Flasher"));
			m_thrdStop = false;
			m_thrdProgress = 0.f;
			m_thrdWorking = true;
			LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_NandFlasher, (void *)this, 0, 32768, 40);
		}
		else if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnBack)))
		{
			m_cfg.save();
			_hideNandEmu();
				return 1;
		}

		if(m_thrdMessageAdded)
		{
			LockMutex lock(m_mutex);
			m_thrdMessageAdded = false;
			if (!m_thrdMessage.empty())
				m_btnMgr.setText(m_nandfileLblDialog, m_thrdMessage);
			m_btnMgr.setProgress(m_nandfilePBar, m_fileProgress);
			m_btnMgr.setProgress(m_nandemuPBar, m_thrdProgress);
			m_btnMgr.setText(m_nandfileLblMessage, wfmt(_fmt("fileprogress", L"%d / %dKB"), m_fileprog/0x400, m_filesize/0x400));
			m_btnMgr.setText(m_nandemuLblMessage, wfmt(_fmt("dumpprogress", L"%i%%"), (int)(m_thrdProgress*100.f)));			

			if (!m_thrdWorking)
			{
				m_btnMgr.setText(m_nandfinLblDialog, wfmt(_fmt("cfgne29", L"Flashed: %d saves / %d files / %d folders"), m_nandexentry, m_filesdone, m_foldersdone));
				if(m_dumpsize/0x400 > 0x270f)
					m_btnMgr.setText(m_nandemuLblDialog, wfmt(_fmt("cfgne16", L"Total size: %uMB (%d blocks)"), (m_dumpsize/0x100000), (m_dumpsize/0x8000)>>2));
				else
					m_btnMgr.setText(m_nandemuLblDialog, wfmt(_fmt("cfgne17", L"Total size: %uKB (%d blocks)"), (m_dumpsize/0x400), (m_dumpsize/0x8000)>>2));

				m_btnMgr.show(m_nandemuBtnBack);
				m_btnMgr.show(m_nandfinLblDialog);
			}
		}
	}
	_hideNandEmu();
	return 0;
}

int CMenu::_AutoExtractSave(string gameId)
{
	string emuPath;
	int emuPartition = _FindEmuPart(emuPath, false);
	if(emuPartition < 0)
		emuPartition = _FindEmuPart(emuPath, true);
	if(!_checkSave(gameId, true))
		return 1;

	if(!m_forceext && _checkSave(gameId, false))
		return 1;

	lwp_t thread = 0;
	SetupInput();
	m_thrdStop = false;
	m_thrdMessageAdded = false;
	m_nandext = false;

	if(!m_forceext)
	{
		m_btnMgr.setText(m_nandemuBtnExtract, _t("cfgne24", L"Extract save"));
		m_btnMgr.setText(m_nandemuBtnDisable, _t("cfgne25", L"Create new save"));
		m_btnMgr.setText(m_nandemuLblInit, _t("cfgne26", L"A save file for this game was created on real NAND. Extract existing save file from real NAND or create new file for NAND Emulation?"));
		m_btnMgr.show(m_nandemuBtnExtract);
		m_btnMgr.show(m_nandemuBtnDisable);	
		m_btnMgr.show(m_nandemuLblInit);
	}

	m_saveExtGameId = gameId;

	while(!m_exit)
	{
		_mainLoopCommon();
		if((BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnExtract))) || m_forceext)
		{
			m_forceext = false;
			m_fulldump = false;
			m_btnMgr.hide(m_nandemuBtnExtract);
			m_btnMgr.hide(m_nandemuBtnDisable);	
			m_btnMgr.hide(m_nandemuLblInit);
			m_btnMgr.show(m_nandemuLblTitle);
			m_btnMgr.show(m_nandfilePBar);
			m_btnMgr.show(m_nandemuPBar);
			m_btnMgr.show(m_nandfileLblMessage);
			m_btnMgr.show(m_nandemuLblMessage);
			m_btnMgr.show(m_nandfileLblDialog);
			m_btnMgr.show(m_nandemuLblDialog);
			m_btnMgr.setText(m_nandemuLblMessage, L"");
			m_btnMgr.setText(m_nandfileLblMessage, L"");
			m_btnMgr.setText(m_nandemuLblDialog, _t("cfgne11", L"Overall Progress:"));
			m_btnMgr.setText(m_nandemuLblTitle, _t("cfgne13", L"Game Save Extractor"));
			m_thrdStop = false;
			m_thrdProgress = 0.f;
			m_thrdWorking = true;
			LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_NandDumper, (void *)this, 0, 32768, 40);
		}
		else if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnDisable)))
		{
			char basepath[MAX_FAT_PATH];
			snprintf(basepath, sizeof(basepath), "%s:%s", DeviceName[emuPartition], emuPath.c_str());
			NandHandle.CreatePath("%s/import", basepath);
			NandHandle.CreatePath("%s/meta", basepath);
			NandHandle.CreatePath("%s/shared1", basepath);
			NandHandle.CreatePath("%s/shared2", basepath);
			NandHandle.CreatePath("%s/sys", basepath);
			NandHandle.CreatePath("%s/title", basepath);
			NandHandle.CreatePath("%s/ticket", basepath);
			NandHandle.CreatePath("%s/tmp", basepath);
			_hideNandEmu();
			return 0;
		}
		else if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnBack)))
		{
			m_cfg.save();
			_hideNandEmu();
				return 1;
		}

		if(m_thrdMessageAdded)
		{
			LockMutex lock(m_mutex);
			m_thrdMessageAdded = false;
			if (!m_thrdMessage.empty())
				m_btnMgr.setText(m_nandfileLblDialog, m_thrdMessage);
			m_btnMgr.setProgress(m_nandfilePBar, m_fileProgress);
			m_btnMgr.setProgress(m_nandemuPBar, m_thrdProgress);
			m_btnMgr.setText(m_nandfileLblMessage, wfmt(_fmt("fileprogress", L"%d / %dKB"), m_fileprog/0x400, m_filesize/0x400));
			m_btnMgr.setText(m_nandemuLblMessage, wfmt(_fmt("dumpprogress", L"%i%%"), (int)(m_thrdProgress*100.f)));			

			if (!m_thrdWorking)
			{
				m_btnMgr.setText(m_nandfinLblDialog, wfmt(_fmt("cfgne14", L"Extracted: %d saves / %d files / %d folders"), m_nandexentry, m_filesdone, m_foldersdone));
				if(m_dumpsize/0x400 > 0x270f)
					m_btnMgr.setText(m_nandemuLblDialog, wfmt(_fmt("cfgne16", L"Total size: %uMB (%d blocks)"), (m_dumpsize/0x100000), (m_dumpsize/0x8000)>>2));
				else
					m_btnMgr.setText(m_nandemuLblDialog, wfmt(_fmt("cfgne17", L"Total size: %uKB (%d blocks)"), (m_dumpsize/0x400), (m_dumpsize/0x8000)>>2));
				
				m_btnMgr.show(m_nandemuBtnBack);
				m_btnMgr.show(m_nandfinLblDialog);
			}
		}
	}
	_hideNandEmu();
	return 0;
}

int CMenu::_AutoCreateNand(void)
{
	lwp_t thread = 0;
	SetupInput();
	m_thrdStop = false;
	m_thrdMessageAdded = false;
	m_nandext = false;
	m_tempView = false;

	m_btnMgr.setText(m_nandemuBtnExtract, _t("cfgne5", L"Extract NAND"));
	m_btnMgr.setText(m_nandemuBtnDisable, _t("cfgne22", L"Disable NAND Emulation"));
	m_btnMgr.setText(m_nandemuBtnPartition, _t("cfgne31", L"Select Partition"));
	m_btnMgr.setText(m_nandemuLblInit, _t("cfgne23", L"Welcome to WiiFlow. I have not found a valid NAND for NAND Emulation. Click Extract to extract your NAND, or click disable to disable NAND Emulation."));
	m_btnMgr.show(m_nandemuBtnExtract);
	m_btnMgr.show(m_nandemuBtnDisable);
	m_btnMgr.show(m_nandemuBtnPartition);
	m_btnMgr.show(m_nandemuLblInit);

	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_nandemuBtnExtract))
			{
				m_fulldump = true;
				m_btnMgr.hide(m_nandemuBtnExtract);
				m_btnMgr.hide(m_nandemuBtnDisable);
				m_btnMgr.hide(m_nandemuBtnPartition);
				m_btnMgr.hide(m_nandemuLblInit);
				m_btnMgr.show(m_nandemuLblTitle);
				m_btnMgr.show(m_nandfilePBar);
				m_btnMgr.show(m_nandemuPBar);
				m_btnMgr.show(m_nandfileLblMessage);
				m_btnMgr.show(m_nandemuLblMessage);
				m_btnMgr.show(m_nandfileLblDialog);
				m_btnMgr.show(m_nandemuLblDialog);
				m_btnMgr.setText(m_nandemuLblMessage, L"");
				m_btnMgr.setText(m_nandfileLblMessage, L"");
				m_btnMgr.setText(m_nandemuLblDialog, _t("cfgne11", L"Overall Progress:"));
				m_btnMgr.setText(m_nandemuLblTitle, _t("cfgne12", L"NAND Extractor"));
				m_thrdStop = false;
				m_thrdProgress = 0.f;
				m_thrdWorking = true;
				LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_NandDumper, (void *)this, 0, 32768, 40);
			}
			else if(m_btnMgr.selected(m_nandemuBtnDisable))
			{
				_hideNandEmu();
				return 0;
			}
			else if(m_btnMgr.selected(m_nandemuBtnPartition))
			{
				if(m_current_view == COVERFLOW_USB)
				{
					m_tempView = true;
					m_current_view = COVERFLOW_CHANNEL;
				}
				_hideNandEmu();
				_config(1);
				if(m_tempView)
				{
					m_current_view = COVERFLOW_USB;
					m_tempView = false;
					return 0;
				}
				return 1;
			}
			else if(m_btnMgr.selected(m_nandemuBtnBack))
			{
				m_cfg.save();
				_hideNandEmu();
					return 1;
			}
		}

		if(m_thrdMessageAdded)
		{
			LockMutex lock(m_mutex);
			m_thrdMessageAdded = false;
			if (!m_thrdMessage.empty())
				m_btnMgr.setText(m_nandfileLblDialog, m_thrdMessage);
			m_btnMgr.setProgress(m_nandfilePBar, m_fileProgress);
			m_btnMgr.setProgress(m_nandemuPBar, m_thrdProgress);
			m_btnMgr.setText(m_nandfileLblMessage, wfmt(_fmt("fileprogress", L"%d / %dKB"), m_fileprog/0x400, m_filesize/0x400));
			m_btnMgr.setText(m_nandemuLblMessage, wfmt(_fmt("dumpprogress", L"%i%%"), (int)(m_thrdProgress*100.f)));

			if (!m_thrdWorking)
			{
				m_btnMgr.setText(m_nandfinLblDialog, wfmt(_fmt("cfgne15", L"Extracted: %d files / %d folders"), m_filesdone, m_foldersdone));
				if(m_dumpsize/0x400 > 0x270f)
					m_btnMgr.setText(m_nandemuLblDialog, wfmt(_fmt("cfgne16", L"Total size: %uMB (%d blocks)"), (m_dumpsize/0x100000), (m_dumpsize/0x8000)>>2));
				else
					m_btnMgr.setText(m_nandemuLblDialog, wfmt(_fmt("cfgne17", L"Total size: %uKB (%d blocks)"), (m_dumpsize/0x400), (m_dumpsize/0x8000)>>2));
				m_btnMgr.show(m_nandemuBtnBack);
				m_btnMgr.show(m_nandfinLblDialog);
			}
		}
	}
	_hideNandEmu();
	return 0;
}

int CMenu::_NandFlasher(void *obj)
{
	CMenu &m = *(CMenu *)obj;
	string emuPath;

	char source[MAX_FAT_PATH];
	char dest[ISFS_MAXPATH];

	const char *SaveGameID = m.m_saveExtGameId.c_str();
	int emuPartition = m._FindEmuPart(emuPath, false);	
	int flashID = SaveGameID[0] << 24 | SaveGameID[1] << 16 | SaveGameID[2] << 8 | SaveGameID[3];

	if(_saveExists(fmt("%s:%s/title/00010000/%08x", DeviceName[emuPartition], emuPath.c_str(), flashID)))
	{
		snprintf(source, sizeof(source), "%s:%s/title/00010000/%08x", DeviceName[emuPartition], emuPath.c_str(), flashID);
		snprintf(dest, sizeof(dest), "/title/00010000/%08x", flashID);
	}
	else if(_saveExists(fmt("%s:%s/title/00010004/%08x", DeviceName[emuPartition], emuPath.c_str(), flashID)))
	{
		snprintf(source, sizeof(source), "%s:%s/title/00010004/%08x", DeviceName[emuPartition], emuPath.c_str(), flashID);
		snprintf(dest, sizeof(dest), "/title/00010004/%08x", flashID);
	}
	NandHandle.ResetCounters();
	m.m_nandexentry = 1;
	m.m_dumpsize = NandHandle.CalcFlashSize(source, _ShowProgress, obj);
	m_nandext = true;
	NandHandle.FlashToNAND(source, dest, _ShowProgress, obj);

	m.m_thrdWorking = false;
	LWP_MutexLock(m.m_mutex);
	m_btnMgr.hide(m_nandfilePBar);
	m_btnMgr.hide(m_nandfileLblMessage);
	m._setDumpMsg(m._t("cfgne30", L"Flashing save files finished!"), 1.f, 1.f);
	LWP_MutexUnlock(m.m_mutex);
	return 0;
}

int CMenu::_NandDumper(void *obj)
{
	CMenu &m = *(CMenu *)obj;
	string emuPath;
	int emuPartition = -1;
	m_nandext = false;
	m_sgdump = false;
	m.m_dumpsize = 0;
	m.m_filesdone = 0;
	m.m_foldersdone = 0;

	NandHandle.ResetCounters();
	emuPartition = m._FindEmuPart(emuPath, true);

	if(emuPartition < 0)
	{
		m.error(m._t("cfgne8", L"No valid FAT partition found for NAND Emulation!"));
		return 0;
	}

	char basepath[64];
	snprintf(basepath, sizeof(basepath), "%s:%s", DeviceName[emuPartition], emuPath.c_str());

	LWP_MutexLock(m.m_mutex);
	m._setDumpMsg(m._t("cfgne27", L"Calculating space needed for extraction..."), 0.f, 0.f);
	LWP_MutexUnlock(m.m_mutex);

	if(m_fulldump)
	{
		m.m_dumpsize = NandHandle.CalcDumpSpace("/", CMenu::_ShowProgress, obj);
		m_nandext = true;
		NandHandle.DoNandDump("/", basepath, CMenu::_ShowProgress, obj);
	}
	else
	{
		bool missingOnly = !m_saveall;
		vector<string> saveList;
		m_sgdump = true;

		if(m.m_saveExtGameId.empty())
		{
			m.m_nandexentry = 0;
			saveList.reserve(m_gameList.size());
			for(u32 i = 0; i < m_gameList.size() && !m.m_thrdStop; ++i)
			{
				LWP_MutexLock(m.m_mutex);
				m._setDumpMsg(m._t("cfgne18", L"Listing game saves to extract..."), 0.f, 0.f);
				LWP_MutexUnlock(m.m_mutex);

				string id((const char *)m_gameList[i].id, 4);

				if(!missingOnly || !m._checkSave(id, false))
				{
					if(m._checkSave(id, true))
					{
						m.m_nandexentry++;
						saveList.push_back(id);
					}
				}
			}
		}
		else
		{
			m.m_nandexentry = 1;
			saveList.push_back(m.m_saveExtGameId);
		}

		for(u32 i = 0; i < saveList.size() && !m.m_thrdStop; ++i)
		{
			char source[ISFS_MAXPATH];
			int savePath = saveList[i].c_str()[0] << 24 | saveList[i].c_str()[1] << 16 | saveList[i].c_str()[2] << 8 | saveList[i].c_str()[3];
			snprintf(source, sizeof(source), "/title/00010000/%08x", savePath);
			if(!m._checkSave(saveList[i], true))
				snprintf(source, sizeof(source), "/title/00010004/%08x", savePath);

			m.m_dumpsize = NandHandle.CalcDumpSpace(source, CMenu::_ShowProgress, obj);	
		}
		for(u32 i = 0; i < saveList.size() && !m.m_thrdStop; ++i)
		{
			char source[ISFS_MAXPATH];
			int savePath = saveList[i].c_str()[0] << 24 | saveList[i].c_str()[1] << 16 | saveList[i].c_str()[2] << 8 | saveList[i].c_str()[3];
			snprintf(source, sizeof(source), "/title/00010000/%08x",  savePath);
			if(!m._checkSave(saveList[i], true))
				snprintf(source, sizeof(source), "/title/00010004/%08x",  savePath);

			m_nandext = true;
			NandHandle.DoNandDump(source, basepath, CMenu::_ShowProgress, obj);
		}
	}

	m.m_thrdWorking = false;
	LWP_MutexLock(m.m_mutex);
	m_btnMgr.hide(m_nandfilePBar);
	m_btnMgr.hide(m_nandfileLblMessage);
	m._setDumpMsg(m._t("cfgne19", L"Extraction finished!"), 1.f, 1.f);
	LWP_MutexUnlock(m.m_mutex);
	return 0;
}

void CMenu::_initNandEmuMenu()
{
	_addUserLabels(m_nandemuLblUser, ARRAY_SIZE(m_nandemuLblUser), "NANDEMU");
	m_nandemuBg = _texture("NANDEMU/BG", "texture", theme.bg, false);
	m_nandemuLblTitle = _addTitle("NANDEMU/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);

	m_nandfileLblMessage = _addLabel("NANDEMU/FMESSAGE", theme.lblFont, L"", 40, 230, 560, 100, theme.lblFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_TOP);
	m_nandemuLblMessage = _addLabel("NANDEMU/MESSAGE", theme.lblFont, L"", 40, 350, 560, 100, theme.lblFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_TOP);
	m_nandfileLblDialog = _addLabel("NANDEMU/FDIALOG", theme.lblFont, L"", 40, 60, 560, 200, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandfinLblDialog = _addLabel("NANDEMU/FINDIALOG", theme.lblFont, L"", 40, 120, 560, 200, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuLblDialog = _addLabel("NANDEMU/DIALOG", theme.lblFont, L"", 40, 180, 560, 200, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandfilePBar = _addProgressBar("NANDEMU/FILEPROGRESS_BAR", 40, 200, 560, 20);
	m_nandemuPBar = _addProgressBar("NANDEMU/PROGRESS_BAR", 40, 320, 560, 20);

	m_nandemuLblEmulation = _addLabel("NANDEMU/EMU_SAVE", theme.lblFont, L"", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuLblEmulationVal = _addLabel("NANDEMU/EMU_SAVE_BTN_GLOBAL", theme.btnFont, L"", 400, 130, 144, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_nandemuBtnEmulationM = _addPicButton("NANDEMU/EMU_SAVE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 344, 130, 56, 56);
	m_nandemuBtnEmulationP = _addPicButton("NANDEMU/EMU_SAVE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);
	m_nandemuLblSaveDump = _addLabel("NANDEMU/SAVE_DUMP", theme.lblFont, L"", 40, 190, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuBtnAll = _addButton("NANDEMU/ALL_BTN", theme.btnFont, L"", 350, 190, 250, 56, theme.btnFontColor);
	m_nandemuBtnMissing = _addButton("NANDEMU/MISSING_BTN", theme.btnFont, L"", 350, 250, 250, 56, theme.btnFontColor);
	m_nandemuLblNandDump = _addLabel("NANDEMU/NAND_DUMP", theme.lblFont, L"", 40, 310, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuBtnNandDump = _addButton("NANDEMU/NAND_DUMP_BTN", theme.btnFont, L"", 350, 310, 250, 56, theme.btnFontColor);

	m_nandemuLblNandFolder = _addLabel("NANDEMU/NAND_FOLDER", theme.lblFont, L"", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuBtnNandFolder = _addButton("NANDEMU/NAND_FOLDER_BTN", theme.btnFont, L"", 350, 130, 250, 56, theme.btnFontColor);
	m_nandemuLblNandSavesFolder = _addLabel("NANDEMU/NAND_SAVES_FOLDER", theme.lblFont, L"", 40, 190, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuBtnNandSavesFolder = _addButton("NANDEMU/NAND_SAVES_FOLDER_BTN", theme.btnFont, L"", 350, 190, 250, 56, theme.btnFontColor);

	m_nandemuBtnBack = _addButton("NANDEMU/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_nandemuLblPage = _addLabel("NANDEMU/PAGE_BTN", theme.btnFont, L"", 62, 400, 98, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_nandemuBtnPageM = _addPicButton("NANDEMU/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 10, 400, 52, 56);
	m_nandemuBtnPageP = _addPicButton("NANDEMU/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 160, 400, 52, 56);

	m_nandemuBtnExtract = _addButton("NANDEMU/EXTRACT", theme.titleFont, L"", 72, 180, 496, 56, theme.titleFontColor);
	m_nandemuBtnDisable = _addButton("NANDEMU/DISABLE", theme.titleFont, L"", 72, 270, 496, 56, theme.titleFontColor);
	m_nandemuBtnPartition = _addButton("NANDEMU/PARTITION", theme.titleFont, L"", 72, 360, 496, 56, theme.titleFontColor);
	m_nandemuLblInit = _addLabel("NANDEMU/INIT", theme.lblFont, L"", 40, 40, 560, 140, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);

	_setHideAnim(m_nandemuLblTitle, "NANDEMU/TITLE", 0, -100, 0.f, 0.f);
	_setHideAnim(m_nandfileLblMessage, "NANDEMU/FMESSAGE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblMessage, "NANDEMU/MESSAGE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandfileLblDialog, "NANDEMU/FDIALOG", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandfinLblDialog, "NANDEMU/FINDIALOG", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblDialog, "NANDEMU/DIALOG", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandfilePBar, "NANDEMU/FILEPROGRESS_BAR", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuPBar, "NANDEMU/PROGRESS_BAR", 0, 0, -2.f, 0.f);
	
	_setHideAnim(m_nandemuLblEmulation, "NANDEMU/EMU_SAVE", 100, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblEmulationVal, "NANDEMU/EMU_SAVE_BTN_GLOBAL", 0, 0, 1.f, -1.f);
	_setHideAnim(m_nandemuBtnEmulationM, "NANDEMU/EMU_SAVE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_nandemuBtnEmulationP, "NANDEMU/EMU_SAVE_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_nandemuLblSaveDump, "NANDEMU/SAVE_DUMP", 100, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuBtnAll, "NANDEMU/ALL_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuBtnMissing, "NANDEMU/MISSING_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblNandDump, "NANDEMU/NAND_DUMP", 100, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuBtnNandDump, "NANDEMU/NAND_DUMP_BTN", 0, 0, -2.f, 0.f);
	
	_setHideAnim(m_nandemuLblNandFolder, "NANDEMU/NAND_FOLDER", 100, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuBtnNandFolder, "NANDEMU/NAND_FOLDER_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblNandSavesFolder, "NANDEMU/NAND_SAVES_FOLDER", 100, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuBtnNandSavesFolder, "NANDEMU/NAND_SAVES_FOLDER_BTN", 0, 0, -2.f, 0.f);
	
	_setHideAnim(m_nandemuBtnBack, "NANDEMU/BACK_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblPage, "NANDEMU/PAGE_BTN", 0, 0, -1.f, 1.f);
	_setHideAnim(m_nandemuBtnPageM, "NANDEMU/PAGE_MINUS", 0, 0, -1.f, 1.f);
	_setHideAnim(m_nandemuBtnPageP, "NANDEMU/PAGE_PLUS", 0, 0, -1.f, 1.f);
	
	_setHideAnim(m_nandemuBtnExtract, "NANDEMU/EXTRACT", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuBtnPartition, "NANDEMU/PARTITION", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuBtnDisable, "NANDEMU/DISABLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblInit, "NANDEMU/INIT", 100, 0, -2.f, 0.f);

	_hideNandEmu(true);
	_textNandEmu();
}

void CMenu::_textNandEmu(void)
{
	m_btnMgr.setText(m_nandemuLblNandFolder, _t("cfgne32", L"Change Nand"));
	m_btnMgr.setText(m_nandemuLblNandSavesFolder, _t("cfgne33", L"Change Saves Nand"));
	m_btnMgr.setText(m_nandemuBtnNandFolder, _t("dl16", L"Set"));
	m_btnMgr.setText(m_nandemuBtnNandSavesFolder, _t("dl16", L"Set"));
	m_btnMgr.setText(m_nandemuLblEmulation, _t("cfgne1", L"NAND Emulation"));
	m_btnMgr.setText(m_nandemuLblSaveDump, _t("cfgne2", L"Extract Game Saves"));
	m_btnMgr.setText(m_nandemuBtnAll, _t("cfgne3", L"All"));
	m_btnMgr.setText(m_nandemuBtnMissing, _t("cfgne4", L"Missing"));
	m_btnMgr.setText(m_nandemuLblNandDump, _t("cfgne5", L"Extract NAND"));
	m_btnMgr.setText(m_nandemuBtnNandDump, _t("cfgne6", L"Start"));
	m_btnMgr.setText(m_nandemuBtnBack, _t("cfgne7", L"Back"));
}
