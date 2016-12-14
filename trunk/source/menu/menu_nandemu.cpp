#include <dirent.h>
#include <unistd.h>
#include <algorithm>
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

s16 m_nandemuLblNandSelect;
s16 m_nandemuLblNandSelectVal;
s16 m_nandemuBtnNandSelectM;
s16 m_nandemuBtnNandSelectP;
s16 m_nandemuLblEmulation;
s16 m_nandemuLblEmulationVal;
s16 m_nandemuBtnEmulationM;
s16 m_nandemuBtnEmulationP;
s16 m_nandemuLblSaveNandSelect;
s16 m_nandemuLblSaveNandSelectVal;
s16 m_nandemuBtnSaveNandSelectM;
s16 m_nandemuBtnSaveNandSelectP;
s16 m_nandemuLblSaveEmulation;
s16 m_nandemuLblSaveEmulationVal;
s16 m_nandemuBtnSaveEmulationM;
s16 m_nandemuBtnSaveEmulationP;

s16 m_nandemuLblSaveDump;
s16 m_nandemuBtnAll;
s16 m_nandemuBtnMissing;
s16 m_nandemuLblNandDump;
s16 m_nandemuBtnNandDump;
s16 m_nandemuLblSavePartition;
s16 m_nandemuLblSavePartitionVal;
s16 m_nandemuBtnSavePartitionM;
s16 m_nandemuBtnSavePartitionP;

s16 m_nandemuLblInstallWad;
s16 m_nandemuBtnInstallWad;

s16 m_nandfileLblMessage;
s16 m_nandemuLblMessage;
s16 m_nandfileLblDialog;
s16 m_nandfinLblDialog;
s16 m_nandemuLblDialog;
s16 m_nandfilePBar;
s16 m_nandemuPBar;
s16 m_nandemuBtnExtract;
s16 m_nandemuBtnDisable;
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
	return (i + s) % s;
}

void CMenu::_listEmuNands(const char *path, vector<string> &emuNands)
{
	DIR *d;
	struct dirent *dir;
	emuNands.clear();
	bool def = true;

	d = opendir(path);
	if(d != 0)
	{
		while((dir = readdir(d)) != 0)
		{
			if(dir->d_name[0] == '.')
				continue;
			if(dir->d_type == DT_DIR)
			{
				emuNands.push_back(dir->d_name);
				string filename = dir->d_name;
				if(filename == "default")
					def = false;
			}
		}
		closedir(d);
	}
	if(def)
		emuNands.push_back("default");
	sort(emuNands.begin(), emuNands.end());
}

void CMenu::_checkEmuNandSettings(bool savesnand)
{
	/* make sure partition and directory are legal and legit */
	const char *emuNand = NULL;
	int emuPart;
	if(savesnand)
	{
		emuNand = m_cfg.getString(WII_DOMAIN, "current_save_emunand", "default").c_str();
		emuPart = m_cfg.getInt(WII_DOMAIN, "savepartition", 0);
	}
	else
	{
		emuNand = m_cfg.getString(CHANNEL_DOMAIN, "current_emunand", "default").c_str();
		emuPart = m_cfg.getInt(CHANNEL_DOMAIN, "partition", 0);
	}
	const char *tmpPath = fmt("/%s/%s", EMU_NANDS_DIR, emuNand);
	/* check if partition is FAT && emuNAND directory exist*/
	if(!DeviceHandle.PartitionUsableForNandEmu(emuPart) || !_TestEmuNand(emuPart, tmpPath, false))
	{
		if(savesnand)
		{
			m_cfg.setString(WII_DOMAIN, "current_save_emunand", "default");
			m_cfg.getInt(WII_DOMAIN, "savepartition", 0);
		}
		else
		{
			m_cfg.setString(CHANNEL_DOMAIN, "current_emunand", "default");
			m_cfg.getInt(CHANNEL_DOMAIN, "partition", 0);
		}
	}
}

int CMenu::_FindEmuPart(string &emuPath, bool skipchecks, bool savesnand)
{
	int emuPart;
	const char *tmpPath = NULL;
	if(savesnand)
	{
		emuPart = m_cfg.getInt(WII_DOMAIN, "savepartition");
		tmpPath = fmt("/%s/%s", EMU_NANDS_DIR, m_cfg.getString(WII_DOMAIN, "current_save_emunand").c_str());
	}
	else
	{
		emuPart = m_cfg.getInt(CHANNEL_DOMAIN, "partition");
		tmpPath = fmt("/%s/%s", EMU_NANDS_DIR, m_cfg.getString(CHANNEL_DOMAIN, "current_emunand").c_str());
	}
	if(!DeviceHandle.PartitionUsableForNandEmu(emuPart))
		return -1;
	else if((skipchecks || _TestEmuNand(emuPart, tmpPath, true)))// true should only be true if emulation set to full
	{
		NandHandle.SetNANDEmu(emuPart);
		NandHandle.SetPaths(tmpPath, DeviceName[emuPart]);
		emuPath = tmpPath;
		return emuPart;
	}
	return -2;
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
		snprintf(testpath, sizeof(testpath), "%s/shared2/sys/SYSCONF", basepath);
		if(!fsop_FileExist(testpath))
			return false;
		snprintf(testpath, sizeof(testpath), "%s/title/00000001/00000002/data/setting.txt", basepath);
		if(!fsop_FileExist(testpath))
			return false;
		// Check Mii's
		snprintf(testpath, sizeof(testpath), "%s/shared2/menu/FaceLib/RFL_DB.dat", basepath);
		if(!fsop_FileExist(testpath))
			return false;
	}
	return true;
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
		int emuPartition = m_cfg.getInt(WII_DOMAIN, "savepartition");
		const char *emuPath = fmt("/%s/%s", EMU_NANDS_DIR, m_cfg.getString(WII_DOMAIN, "current_save_emunand").c_str());
		if(emuPartition < 0 || emuPath == NULL)
			return false;
		struct stat fstat;
		if((stat(fmt("%s:%s/title/00010000/%08x", DeviceName[emuPartition], emuPath, savePath), &fstat) != 0) 
			&& (stat(fmt("%s:%s/title/00010004/%08x", DeviceName[emuPartition], emuPath, savePath), &fstat) != 0))
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
	
	m_btnMgr.hide(m_nandemuLblNandSelect, instant);
	m_btnMgr.hide(m_nandemuLblNandSelectVal, instant);
	m_btnMgr.hide(m_nandemuBtnNandSelectP, instant);
	m_btnMgr.hide(m_nandemuBtnNandSelectM, instant);
	m_btnMgr.hide(m_nandemuLblEmulation, instant);
	m_btnMgr.hide(m_nandemuLblEmulationVal, instant);
	m_btnMgr.hide(m_nandemuBtnEmulationP, instant);
	m_btnMgr.hide(m_nandemuBtnEmulationM, instant);

	m_btnMgr.hide(m_nandemuLblSaveNandSelect, instant);
	m_btnMgr.hide(m_nandemuLblSaveNandSelectVal, instant);
	m_btnMgr.hide(m_nandemuBtnSaveNandSelectP, instant);
	m_btnMgr.hide(m_nandemuBtnSaveNandSelectM, instant);
	m_btnMgr.hide(m_nandemuLblSaveEmulation, instant);
	m_btnMgr.hide(m_nandemuLblSaveEmulationVal, instant);
	m_btnMgr.hide(m_nandemuBtnSaveEmulationP, instant);
	m_btnMgr.hide(m_nandemuBtnSaveEmulationM, instant);

	m_btnMgr.hide(m_nandemuLblSaveDump, instant);
	m_btnMgr.hide(m_nandemuBtnAll, instant);
	m_btnMgr.hide(m_nandemuBtnMissing, instant);
	
	m_btnMgr.hide(m_nandemuLblNandDump, instant);
	m_btnMgr.hide(m_nandemuBtnNandDump, instant);

	m_btnMgr.hide(m_nandemuLblSavePartition, instant);
	m_btnMgr.hide(m_nandemuLblSavePartitionVal, instant);
	m_btnMgr.hide(m_nandemuBtnSavePartitionP, instant);
	m_btnMgr.hide(m_nandemuBtnSavePartitionM, instant);
	
	m_btnMgr.hide(m_nandemuLblInstallWad, instant);
	m_btnMgr.hide(m_nandemuBtnInstallWad, instant);

	m_btnMgr.hide(m_nandemuBtnExtract, instant);
	m_btnMgr.hide(m_nandemuBtnDisable, instant);
	m_btnMgr.hide(m_nandemuLblInit, instant);
	
	for(u8 i = 0; i < ARRAY_SIZE(m_nandemuLblUser); ++i)
		if(m_nandemuLblUser[i] != -1)
			m_btnMgr.hide(m_nandemuLblUser[i], instant); 
}

void CMenu::_hideNandEmuPg(void)
{
	//Does not hide title, page, back, or user labels
	if(nandemuPage == 1)
	{
		m_btnMgr.hide(m_nandemuLblNandSelect);
		m_btnMgr.hide(m_nandemuLblNandSelectVal);
		m_btnMgr.hide(m_nandemuBtnNandSelectP);
		m_btnMgr.hide(m_nandemuBtnNandSelectM);
		
		m_btnMgr.hide(m_nandemuLblEmulationVal);
		m_btnMgr.hide(m_nandemuLblEmulation);
		m_btnMgr.hide(m_nandemuBtnEmulationP);
		m_btnMgr.hide(m_nandemuBtnEmulationM);

		m_btnMgr.hide(m_nandemuLblSaveNandSelect);
		m_btnMgr.hide(m_nandemuLblSaveNandSelectVal);
		m_btnMgr.hide(m_nandemuBtnSaveNandSelectP);
		m_btnMgr.hide(m_nandemuBtnSaveNandSelectM);
		
		m_btnMgr.hide(m_nandemuLblSaveEmulationVal);
		m_btnMgr.hide(m_nandemuLblSaveEmulation);
		m_btnMgr.hide(m_nandemuBtnSaveEmulationP);
		m_btnMgr.hide(m_nandemuBtnSaveEmulationM);
	}
	else if(nandemuPage == 2)
	{
		m_btnMgr.hide(m_nandemuLblSaveDump);
		m_btnMgr.hide(m_nandemuBtnAll);
		m_btnMgr.hide(m_nandemuBtnMissing);
		
		m_btnMgr.hide(m_nandemuLblNandDump);
		m_btnMgr.hide(m_nandemuBtnNandDump);
		
		m_btnMgr.hide(m_nandemuLblInstallWad);
		m_btnMgr.hide(m_nandemuBtnInstallWad);
	}
	else
	{
		m_btnMgr.hide(m_nandemuLblSavePartition);
		m_btnMgr.hide(m_nandemuLblSavePartitionVal);
		m_btnMgr.hide(m_nandemuBtnSavePartitionP);
		m_btnMgr.hide(m_nandemuBtnSavePartitionM);
	}
}

void CMenu::_showNandEmu(void)
{
	_setBg(m_nandemuBg, m_nandemuBg);
	m_btnMgr.setText(m_nandemuLblTitle, _t("cfgne10", L"NAND Emulation Settings"));
	m_btnMgr.show(m_nandemuLblTitle);
	m_btnMgr.show(m_nandemuBtnBack);
	
	m_btnMgr.setText(m_nandemuLblPage, wfmt(L"%i / 3", nandemuPage));
	m_btnMgr.show(m_nandemuLblPage);
	m_btnMgr.show(m_nandemuBtnPageM);
	m_btnMgr.show(m_nandemuBtnPageP);

	if(nandemuPage == 1)
	{
		int i = min(max(0, m_cfg.getInt(CHANNEL_DOMAIN, "emulation", 0)), (int)ARRAY_SIZE(CMenu::_NandEmu) - 1);
		m_btnMgr.setText(m_nandemuLblEmulationVal, _t(CMenu::_NandEmu[i].id, CMenu::_NandEmu[i].text));
		
		i = min(max(0, m_cfg.getInt(WII_DOMAIN, "save_emulation", 0)), (int)ARRAY_SIZE(CMenu::_GlobalSaveEmu) - 1);
		m_btnMgr.setText(m_nandemuLblSaveEmulationVal, _t(CMenu::_GlobalSaveEmu[i].id, CMenu::_GlobalSaveEmu[i].text));
		
		m_btnMgr.setText(m_nandemuLblNandSelectVal, m_cfg.getString(CHANNEL_DOMAIN, "current_emunand"));
		m_btnMgr.setText(m_nandemuLblSaveNandSelectVal, m_cfg.getString(WII_DOMAIN, "current_save_emunand"));
		
		m_btnMgr.show(m_nandemuLblNandSelect);
		m_btnMgr.show(m_nandemuLblNandSelectVal);
		m_btnMgr.show(m_nandemuBtnNandSelectP);
		m_btnMgr.show(m_nandemuBtnNandSelectM);
		
		m_btnMgr.show(m_nandemuLblEmulation);
		m_btnMgr.show(m_nandemuLblEmulationVal);
		m_btnMgr.show(m_nandemuBtnEmulationP);
		m_btnMgr.show(m_nandemuBtnEmulationM);
			
		m_btnMgr.show(m_nandemuLblSaveNandSelect);
		m_btnMgr.show(m_nandemuLblSaveNandSelectVal);
		m_btnMgr.show(m_nandemuBtnSaveNandSelectP);
		m_btnMgr.show(m_nandemuBtnSaveNandSelectM);
		
		m_btnMgr.show(m_nandemuLblSaveEmulation);
		m_btnMgr.show(m_nandemuLblSaveEmulationVal);
		m_btnMgr.show(m_nandemuBtnSaveEmulationP);
		m_btnMgr.show(m_nandemuBtnSaveEmulationM);
	}
	else if(nandemuPage == 2)
	{
		m_btnMgr.show(m_nandemuLblSaveDump);
		m_btnMgr.show(m_nandemuBtnAll);
		m_btnMgr.show(m_nandemuBtnMissing);
		
		m_btnMgr.show(m_nandemuLblNandDump);
		m_btnMgr.show(m_nandemuBtnNandDump);
		
		m_btnMgr.show(m_nandemuLblInstallWad);
		m_btnMgr.show(m_nandemuBtnInstallWad);
	}
	else
	{
		const char *partitionname = DeviceName[m_cfg.getInt(WII_DOMAIN, "savepartition")];
		m_btnMgr.setText(m_nandemuLblSavePartitionVal, upperCase(partitionname));

		m_btnMgr.show(m_nandemuLblSavePartition);
		m_btnMgr.show(m_nandemuLblSavePartitionVal);
		m_btnMgr.show(m_nandemuBtnSavePartitionP);
		m_btnMgr.show(m_nandemuBtnSavePartitionM);
	}
	
	for(u8 i = 0; i < ARRAY_SIZE(m_nandemuLblUser); ++i)
		if(m_nandemuLblUser[i] != -1)
			m_btnMgr.show(m_nandemuLblUser[i]);
}

int CMenu::_NandEmuCfg(void)
{	
	nandemuPage = 1;
	string ExtNand = "";
	
	vector<string> emuNands;
	vector<string> savesNands;
	string emuNand = m_cfg.getString(CHANNEL_DOMAIN, "current_emunand");
	int emuPart = m_cfg.getInt(CHANNEL_DOMAIN, "partition");
	string savesNand = m_cfg.getString(WII_DOMAIN, "current_save_emunand");
	int savesPart = m_cfg.getInt(WII_DOMAIN, "savepartition");

	_listEmuNands(fmt("%s:/%s", DeviceName[emuPart], EMU_NANDS_DIR), emuNands);
	int curEmuNand = 0;
	for(u8 i = 0; i < emuNands.size(); ++i)
	{
		if(emuNands[i] == emuNand)
		{
			curEmuNand = i;
			break;
		}
	}

	_listEmuNands(fmt("%s:/%s", DeviceName[savesPart], EMU_NANDS_DIR), savesNands);
	int curSavesNand = 0;
	for(u8 i = 0; i < savesNands.size(); ++i)
	{
		if(savesNands[i] == savesNand)
		{
			curSavesNand = i;
			break;
		}
	}

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
			_hideNandEmuPg();
			if(nandemuPage == 1)
				nandemuPage = 3;
			else --nandemuPage;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_nandemuBtnPageM);
			_showNandEmu();
		}
		else if((BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED) || (BTN_A_PRESSED && m_btnMgr.selected(m_nandemuBtnPageP)))
		{
			_hideNandEmuPg();
			if(nandemuPage == 3)
				nandemuPage = 1;
			else
				++nandemuPage;
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED)
				m_btnMgr.click(m_nandemuBtnPageP);
			_showNandEmu();
		}
		else if((BTN_HOME_PRESSED || BTN_B_PRESSED))// && !m_thrdWorking)
			break;
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnEmulationP) || m_btnMgr.selected(m_nandemuBtnEmulationM)))
		{
			s8 direction = m_btnMgr.selected(m_nandemuBtnEmulationP) ? 1 : -1;
			m_cfg.setInt(CHANNEL_DOMAIN, "emulation", loopNum(m_cfg.getInt(CHANNEL_DOMAIN, "emulation", 0) + direction, ARRAY_SIZE(CMenu::_NandEmu)));
			_showNandEmu();
		}
		else if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnSaveEmulationP) || m_btnMgr.selected(m_nandemuBtnSaveEmulationM)))
		{
			s8 direction = m_btnMgr.selected(m_nandemuBtnSaveEmulationP) ? 1 : -1;
			m_cfg.setInt(WII_DOMAIN, "save_emulation", loopNum(m_cfg.getInt(WII_DOMAIN, "save_emulation", 0) + direction, ARRAY_SIZE(CMenu::_GlobalSaveEmu)));
			_showNandEmu();
		}
		else if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnSavePartitionP) || m_btnMgr.selected(m_nandemuBtnSavePartitionM)))
		{
			s8 direction = m_btnMgr.selected(m_nandemuBtnSavePartitionP) ? 1 : -1;
			currentPartition = m_cfg.getInt(WII_DOMAIN, "savepartition");
			m_emuSaveNand = true;
			_setPartition(direction);
			m_emuSaveNand = false;
			_showNandEmu();
		}
		else if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnNandDump) || m_btnMgr.selected(m_nandemuBtnAll) || m_btnMgr.selected(m_nandemuBtnMissing)))
		{
			m_fulldump = m_btnMgr.selected(m_nandemuBtnNandDump) ? true : false;
			m_saveall = m_btnMgr.selected(m_nandemuBtnAll) ? true : false;
			m_btnMgr.hide(m_nandemuBtnBack);
			_hideNandEmu(true);
			
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
			if(m_fulldump)
			{
				ExtNand = emuNands[curEmuNand];
				m_btnMgr.setText(m_nandemuLblTitle, _t("cfgne12", L"NAND Extractor"));
			}
			else
				m_btnMgr.setText(m_nandemuLblTitle, _t("cfgne13", L"Game Save Extractor"));
			m_thrdStop = false;
			m_thrdProgress = 0.f;
			m_thrdWorking = true;
			LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_NandDumper, (void *)this, 0, 32768, 40);
		}
		else if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnNandSelectP) || m_btnMgr.selected(m_nandemuBtnNandSelectM)))
		{
			s8 direction = m_btnMgr.selected(m_nandemuBtnNandSelectP) ? 1 : -1;
			curEmuNand = loopNum(curEmuNand + direction, emuNands.size());
			m_cfg.setString(CHANNEL_DOMAIN, "current_emunand", emuNands[curEmuNand]);
			_showNandEmu();
		}
		else if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnSaveNandSelectP) || m_btnMgr.selected(m_nandemuBtnSaveNandSelectM)))
		{
			s8 direction = m_btnMgr.selected(m_nandemuBtnSaveNandSelectP) ? 1 : -1;
			curSavesNand = loopNum(curSavesNand + direction, savesNands.size());
			m_cfg.setString(WII_DOMAIN, "current_save_emunand", savesNands[curSavesNand]);
			_showNandEmu();
		}
		else if(BTN_A_PRESSED && m_btnMgr.selected(m_nandemuBtnInstallWad))
		{
			_hideNandEmu();
			_wadExplorer();
			nandemuPage = 1;
			_showNandEmu();
		}			
		else if(BTN_A_PRESSED && m_btnMgr.selected(m_nandemuBtnBack))
		{
			m_cfg.save();
			break;
		}

		if(m_thrdMessageAdded)
		{
			LockMutex lock(m_mutex);
			m_thrdMessageAdded = false;
			if(!m_thrdMessage.empty())
				m_btnMgr.setText(m_nandfileLblDialog, m_thrdMessage);
			m_btnMgr.setProgress(m_nandfilePBar, m_fileProgress);
			m_btnMgr.setProgress(m_nandemuPBar, m_thrdProgress);
			m_btnMgr.setText(m_nandfileLblMessage, wfmt(_fmt("fileprogress", L"%d / %dKB"), m_fileprog/0x400, m_filesize/0x400));
			m_btnMgr.setText(m_nandemuLblMessage, wfmt(_fmt("dumpprogress", L"%i%%"), (int)(m_thrdProgress*100.f)));			

			if(!m_thrdWorking)
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
				while(!m_exit)
				{
					_mainLoopCommon();
					if(BTN_HOME_PRESSED || BTN_B_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_nandemuBtnBack)))
					{
						_hideNandEmu();
						nandemuPage = 1;
						_showNandEmu();
						break;
					}
				}
			}
		}
	}
	_hideNandEmu();
	if(emuNand != m_cfg.getString(CHANNEL_DOMAIN, "current_emunand") || emuNand == ExtNand)
	{
		m_cfg.setBool(CHANNEL_DOMAIN, "update_cache", true);
		if(m_current_view & COVERFLOW_CHANNEL)
			m_refreshGameList = true;
	}
	return 0;
}

int CMenu::_FlashSave(string gameId)
{
	int emuPartition = m_cfg.getInt(WII_DOMAIN, "savepartition");
	char basepath[MAX_FAT_PATH];
	snprintf(basepath, sizeof(basepath), "%s:/%s/%s", DeviceName[emuPartition], EMU_NANDS_DIR, m_cfg.getString(WII_DOMAIN, "current_save_emunand").c_str());

	if(!_checkSave(gameId, false))// if save not on emu nand
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
		else if(BTN_HOME_PRESSED || BTN_B_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_nandemuBtnBack)))
		{
			_hideNandEmu();
			return 1;
		}

		if(m_thrdMessageAdded)
		{
			LockMutex lock(m_mutex);
			m_thrdMessageAdded = false;
			if(!m_thrdMessage.empty())
				m_btnMgr.setText(m_nandfileLblDialog, m_thrdMessage);
			m_btnMgr.setProgress(m_nandfilePBar, m_fileProgress);
			m_btnMgr.setProgress(m_nandemuPBar, m_thrdProgress);
			m_btnMgr.setText(m_nandfileLblMessage, wfmt(_fmt("fileprogress", L"%d / %dKB"), m_fileprog/0x400, m_filesize/0x400));
			m_btnMgr.setText(m_nandemuLblMessage, wfmt(_fmt("dumpprogress", L"%i%%"), (int)(m_thrdProgress*100.f)));			

			if(!m_thrdWorking)
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

int CMenu::_AutoExtractSave(string gameId)// called from wii game config menu or launching wii game
{
	if(!_checkSave(gameId, true))//if save not on real nand
		return 0;

	if(!m_forceext && _checkSave(gameId, false))//if not force extract and save is already on emunand
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
		if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if((BTN_A_PRESSED && m_btnMgr.selected(m_nandemuBtnExtract)) || m_forceext)
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
		else if(BTN_A_PRESSED && m_btnMgr.selected(m_nandemuBtnDisable))//create new save
		{
			int emuPart = m_cfg.getInt(WII_DOMAIN, "savepartition");
			const char *emuPath = fmt("/%s/%s", EMU_NANDS_DIR, m_cfg.getString(WII_DOMAIN, "current_save_emunand").c_str());
			char basepath[MAX_FAT_PATH];
			snprintf(basepath, sizeof(basepath), "%s:%s", DeviceName[emuPart], emuPath);
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
		else if(BTN_A_PRESSED && m_btnMgr.selected(m_nandemuBtnBack))
		{
			_hideNandEmu();
			return 1;
		}

		if(m_thrdMessageAdded)
		{
			LockMutex lock(m_mutex);
			m_thrdMessageAdded = false;
			if(!m_thrdMessage.empty())
				m_btnMgr.setText(m_nandfileLblDialog, m_thrdMessage);
			m_btnMgr.setProgress(m_nandfilePBar, m_fileProgress);
			m_btnMgr.setProgress(m_nandemuPBar, m_thrdProgress);
			m_btnMgr.setText(m_nandfileLblMessage, wfmt(_fmt("fileprogress", L"%d / %dKB"), m_fileprog/0x400, m_filesize/0x400));
			m_btnMgr.setText(m_nandemuLblMessage, wfmt(_fmt("dumpprogress", L"%i%%"), (int)(m_thrdProgress*100.f)));			

			if(!m_thrdWorking)
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

int CMenu::_NandFlasher(void *obj)
{
	CMenu &m = *(CMenu *)obj;
	string emuPath;

	char source[MAX_FAT_PATH];
	char dest[ISFS_MAXPATH];

	const char *SaveGameID = m.m_saveExtGameId.c_str();
	int emuPartition = m._FindEmuPart(emuPath, false, true);	
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
	m_btnMgr.hide(m_nandemuPBar);
	m_btnMgr.hide(m_nandemuLblMessage);
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
	emuPartition = m._FindEmuPart(emuPath, true, !m_fulldump);

	if(emuPartition < 0)
	{
		//m.error(m._t("cfgne8", L"No valid FAT partition found for NAND Emulation!"));
		m.m_thrdWorking = false;
		LWP_MutexLock(m.m_mutex);
		m_btnMgr.hide(m_nandfilePBar);
		m_btnMgr.hide(m_nandfileLblMessage);
		m_btnMgr.hide(m_nandemuPBar);
		m_btnMgr.hide(m_nandemuLblMessage);
		m._setDumpMsg(m._t("cfgne8", L"No valid FAT partition found for NAND Emulation!"), 1.f, 1.f);
		LWP_MutexUnlock(m.m_mutex);
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
			LWP_MutexLock(m.m_mutex);
			m._setDumpMsg(m._t("cfgne18", L"Listing game saves to extract..."), 0.f, 0.f);
			LWP_MutexUnlock(m.m_mutex);
			m.m_nandexentry = 0;
			saveList.reserve(m.m_gameList.size());
			for(u32 i = 0; i < m.m_gameList.size() && !m.m_thrdStop; ++i)
			{

				if(m.m_gameList[i].type == TYPE_WII_GAME)
				{
					string id((const char *)m.m_gameList[i].id, 4);

					if(!missingOnly || !m._checkSave(id, false))// all or if save not on emu nand
					{
						if(m._checkSave(id, true))// if save on real nand
						{
							m.m_nandexentry++;
							saveList.push_back(id);
						}
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
	m_btnMgr.hide(m_nandemuPBar);
	m_btnMgr.hide(m_nandemuLblMessage);
	m._setDumpMsg(m._t("cfgne19", L"Extraction finished!"), 1.f, 1.f);
	LWP_MutexUnlock(m.m_mutex);
	return 0;
}

void CMenu::_initNandEmuMenu()
{
	_addUserLabels(m_nandemuLblUser, ARRAY_SIZE(m_nandemuLblUser), "NANDEMU");
	m_nandemuBg = _texture("NANDEMU/BG", "texture", theme.bg, false);
	m_nandemuLblTitle = _addTitle("NANDEMU/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);

	m_nandfilePBar = _addProgressBar("NANDEMU/FILEPROGRESS_BAR", 40, 80, 560, 20);
	m_nandfileLblMessage = _addLabel("NANDEMU/FMESSAGE", theme.lblFont, L"", 40, 110, 560, 100, theme.lblFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_TOP);
	m_nandfileLblDialog = _addLabel("NANDEMU/FDIALOG", theme.lblFont, L"", 40, 60, 560, 200, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuPBar = _addProgressBar("NANDEMU/PROGRESS_BAR", 40, 220, 560, 20);
	m_nandemuLblMessage = _addLabel("NANDEMU/MESSAGE", theme.lblFont, L"", 40, 280, 560, 100, theme.lblFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_TOP);
	m_nandemuLblDialog = _addLabel("NANDEMU/DIALOG", theme.lblFont, L"", 40, 190, 560, 200, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandfinLblDialog = _addLabel("NANDEMU/FINDIALOG", theme.lblFont, L"", 40, 120, 560, 200, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);

	m_nandemuLblNandSelect = _addLabel("NANDEMU/NAND_SELECT", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuLblNandSelectVal = _addLabel("NANDEMU/NAND_SELECT_BTN", theme.btnFont, L"", 468, 130, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_nandemuBtnNandSelectM = _addPicButton("NANDEMU/NAND_SELECT_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 130, 48, 48);
	m_nandemuBtnNandSelectP = _addPicButton("NANDEMU/NAND_SELECT_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 130, 48, 48);
	m_nandemuLblEmulation = _addLabel("NANDEMU/NAND_EMU", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuLblEmulationVal = _addLabel("NANDEMU/NAND_EMU_BTN", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_nandemuBtnEmulationM = _addPicButton("NANDEMU/NAND_EMU_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_nandemuBtnEmulationP = _addPicButton("NANDEMU/NAND_EMU_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);
	m_nandemuLblSaveNandSelect = _addLabel("NANDEMU/SAVE_NAND_SELECT", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuLblSaveNandSelectVal = _addLabel("NANDEMU/SAVE_NAND_SELECT_BTN", theme.btnFont, L"", 468, 250, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_nandemuBtnSaveNandSelectM = _addPicButton("NANDEMU/SAVE_NAND_SELECT_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 250, 48, 48);
	m_nandemuBtnSaveNandSelectP = _addPicButton("NANDEMU/SAVE_NAND_SELECT_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 250, 48, 48);
	m_nandemuLblSaveEmulation = _addLabel("NANDEMU/SAVE_EMU", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuLblSaveEmulationVal = _addLabel("NANDEMU/SAVE_EMU_BTN", theme.btnFont, L"", 468, 310, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_nandemuBtnSaveEmulationM = _addPicButton("NANDEMU/SAVE_EMU_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 310, 48, 48);
	m_nandemuBtnSaveEmulationP = _addPicButton("NANDEMU/SAVE_EMU_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 310, 48, 48);

	m_nandemuLblSaveDump = _addLabel("NANDEMU/SAVE_DUMP", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuBtnAll = _addButton("NANDEMU/ALL_BTN", theme.btnFont, L"", 420, 130, 200, 48, theme.btnFontColor);
	m_nandemuBtnMissing = _addButton("NANDEMU/MISSING_BTN", theme.btnFont, L"", 420, 190, 200, 48, theme.btnFontColor);
	m_nandemuLblNandDump = _addLabel("NANDEMU/NAND_DUMP", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuBtnNandDump = _addButton("NANDEMU/NAND_DUMP_BTN", theme.btnFont, L"", 420, 250, 200, 48, theme.btnFontColor);
	m_nandemuLblInstallWad = _addLabel("NANDEMU/INSTALL_WAD", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuBtnInstallWad = _addButton("NANDEMU/INSTALL_WAD_BTN", theme.btnFont, L"", 420, 310, 200, 48, theme.btnFontColor);

	m_nandemuLblSavePartition = _addLabel("NANDEMU/SAVE_PART", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuLblSavePartitionVal = _addLabel("NANDEMU/SAVE_PART_BTN", theme.btnFont, L"", 468,130, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_nandemuBtnSavePartitionM = _addPicButton("NANDEMU/SAVE_PART_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 130, 48, 48);
	m_nandemuBtnSavePartitionP = _addPicButton("NANDEMU/SAVE_PART_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 130, 48, 48);

	m_nandemuBtnBack = _addButton("NANDEMU/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	m_nandemuLblPage = _addLabel("NANDEMU/PAGE_BTN", theme.btnFont, L"", 68, 400, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_nandemuBtnPageM = _addPicButton("NANDEMU/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 48, 48);
	m_nandemuBtnPageP = _addPicButton("NANDEMU/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 172, 400, 48, 48);

	m_nandemuBtnExtract = _addButton("NANDEMU/EXTRACT", theme.titleFont, L"", 72, 180, 496, 48, theme.titleFontColor);
	m_nandemuBtnDisable = _addButton("NANDEMU/DISABLE", theme.titleFont, L"", 72, 270, 496, 48, theme.titleFontColor);
	m_nandemuLblInit = _addLabel("NANDEMU/INIT", theme.lblFont, L"", 40, 40, 560, 140, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);

	_setHideAnim(m_nandemuLblTitle, "NANDEMU/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandfileLblMessage, "NANDEMU/FMESSAGE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblMessage, "NANDEMU/MESSAGE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandfileLblDialog, "NANDEMU/FDIALOG", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandfinLblDialog, "NANDEMU/FINDIALOG", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblDialog, "NANDEMU/DIALOG", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandfilePBar, "NANDEMU/FILEPROGRESS_BAR", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuPBar, "NANDEMU/PROGRESS_BAR", 0, 0, -2.f, 0.f);
	
	_setHideAnim(m_nandemuLblNandSelect, "NANDEMU/NAND_SELECT", 50, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblNandSelectVal, "NANDEMU/NAND_SELECT_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_nandemuBtnNandSelectM, "NANDEMU/NAND_SELECT_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_nandemuBtnNandSelectP, "NANDEMU/NAND_SELECT_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_nandemuLblEmulation, "NANDEMU/NAND_EMU", 50, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblEmulationVal, "NANDEMU/NAND_EMU_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_nandemuBtnEmulationM, "NANDEMU/NAND_EMU_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_nandemuBtnEmulationP, "NANDEMU/NAND_EMU_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_nandemuLblSaveNandSelect, "NANDEMU/SAVE_NAND_SELECT", 50, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblSaveNandSelectVal, "NANDEMU/SAVE_NAND_SELECT_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_nandemuBtnSaveNandSelectM, "NANDEMU/SAVE_NAND_SELECT_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_nandemuBtnSaveNandSelectP, "NANDEMU/SAVE_NAND_SELECT_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_nandemuLblSaveEmulation, "NANDEMU/SAVE_EMU", 50, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblSaveEmulationVal, "NANDEMU/SAVE_EMU_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_nandemuBtnSaveEmulationM, "NANDEMU/SAVE_EMU_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_nandemuBtnSaveEmulationP, "NANDEMU/SAVE_EMU_PLUS", -50, 0, 1.f, 0.f);

	_setHideAnim(m_nandemuLblSaveDump, "NANDEMU/SAVE_DUMP", 50, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuBtnAll, "NANDEMU/ALL_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_nandemuBtnMissing, "NANDEMU/MISSING_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_nandemuLblNandDump, "NANDEMU/NAND_DUMP", 50, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuBtnNandDump, "NANDEMU/NAND_DUMP_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_nandemuLblSavePartition, "NANDEMU/SAVE_PART", 50, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblSavePartitionVal, "NANDEMU/SAVE_PART_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_nandemuBtnSavePartitionM, "NANDEMU/SAVE_PART_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_nandemuBtnSavePartitionP, "NANDEMU/SAVE_PART_PLUS", -50, 0, 1.f, 0.f);	
	
	_setHideAnim(m_nandemuLblInstallWad, "NANDEMU/INSTALL_WAD", 50, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuBtnInstallWad, "NANDEMU/INSTALL_WAD_BTN", -50, 0, 1.f, 0.f);
	
	_setHideAnim(m_nandemuBtnBack, "NANDEMU/BACK_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_nandemuLblPage, "NANDEMU/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_nandemuBtnPageM, "NANDEMU/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_nandemuBtnPageP, "NANDEMU/PAGE_PLUS", 0, 0, 1.f, -1.f);
	
	_setHideAnim(m_nandemuBtnExtract, "NANDEMU/EXTRACT", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuBtnDisable, "NANDEMU/DISABLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblInit, "NANDEMU/INIT", 0, 0, -2.f, 0.f);

	_hideNandEmu(true);
	_textNandEmu();
}

void CMenu::_textNandEmu(void)
{
	m_btnMgr.setText(m_nandemuLblNandSelect, _t("cfgne37", L"Select NAND"));
	m_btnMgr.setText(m_nandemuLblEmulation, _t("cfgne1", L"NAND Emulation"));
	m_btnMgr.setText(m_nandemuLblSaveNandSelect, _t("cfgne32", L"Select Saves NAND"));
	m_btnMgr.setText(m_nandemuLblSaveEmulation, _t("cfgne33", L"Saves NAND Emulation"));
	m_btnMgr.setText(m_nandemuLblSavePartition, _t("cfgne38", L"Saves NAND Partition"));
	m_btnMgr.setText(m_nandemuLblSaveDump, _t("cfgne2", L"Extract Game Saves"));
	m_btnMgr.setText(m_nandemuBtnAll, _t("cfgne3", L"All"));
	m_btnMgr.setText(m_nandemuBtnMissing, _t("cfgne4", L"Missing"));
	m_btnMgr.setText(m_nandemuLblNandDump, _t("cfgne5", L"Extract NAND"));
	m_btnMgr.setText(m_nandemuBtnNandDump, _t("cfgne6", L"Start"));
	m_btnMgr.setText(m_nandemuLblInstallWad, _t("cfgne98", L"Install Wad"));
	m_btnMgr.setText(m_nandemuBtnInstallWad, _t("cfgne99", L"Go"));
	m_btnMgr.setText(m_nandemuBtnBack, _t("cfgne7", L"Back"));
}
