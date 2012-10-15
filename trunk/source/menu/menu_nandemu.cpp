#include <dirent.h>
#include <unistd.h>

#include "menu.hpp"
#include "defines.h"
#include "lockMutex.hpp"
#include "channel/nand.hpp"
#include "fileOps/fileOps.h"
#include "loader/cios.h"
#include "loader/nk.h"
#include "loader/sys.h"

// NandEmulation menu
s16 m_nandemuLblTitle;
s16 m_nandemuBtnBack;
s16 m_nandemuLblEmulationVal;
s16 m_nandemuLblEmulation;
s16 m_nandemuBtnEmulationM;
s16 m_nandemuBtnEmulationP;
s16 m_nandemuLblSaveDump;
s16 m_nandemuBtnAll;
s16 m_nandemuBtnMissing;
s16 m_nandemuLblNandDump;
s16 m_nandemuBtnNandDump;
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
STexture m_nandemuBg;

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
	DIR *d;
	d = opendir(path);
	if(!d)
	{
		return false;
	}
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

	DIR *d;
	d = opendir(basepath);
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

int CMenu::_FindEmuPart(string *emuPath, int part, bool searchvalid)
{
	NandHandle.Disable_Emu();

	int emuPartition = -1;	
	string tmpPath;
	if(m_current_view == COVERFLOW_CHANNEL)
	{
		emuPartition = m_cfg.getInt("NAND", "partition", 0);
		tmpPath = m_cfg.getString("NAND", "path", "");
		if(tmpPath.size() == 0)
		{
			m_cfg.setString("NAND", "path", STDEMU_DIR);
			tmpPath = m_cfg.getString("NAND", "path", STDEMU_DIR);
		}
	}
	else if(m_current_view == COVERFLOW_USB)
	{
		emuPartition = m_cfg.getInt("GAMES", "savepartition", -1);
		if(emuPartition == -1)
			emuPartition = m_cfg.getInt("NAND", "partition", 0);
		tmpPath = m_cfg.getString("GAMES", "savepath", m_cfg.getString("NAND", "path", ""));
		if(tmpPath.size() == 0)
		{
			m_cfg.setString("GAMES", "savepath", STDEMU_DIR);
			tmpPath = m_cfg.getString("GAMES", "savepath", STDEMU_DIR);
		}
	}
	
	if(!DeviceHandle.IsInserted(emuPartition))
		DeviceHandle.Mount(emuPartition);
		
	if(_TestEmuNand(emuPartition, tmpPath.c_str(), true) && DeviceHandle.PartitionUsableForNandEmu(emuPartition))
	{
		*emuPath = tmpPath;
		return emuPartition;
	}
	else
	{
		bool fllscn = emuPartition == -1;
		for(u8 i = part; i <= USB8; ++i)
		{
			if(!DeviceHandle.IsInserted(i))
				DeviceHandle.Mount(i);

			if(!DeviceHandle.PartitionUsableForNandEmu(i))
				continue;

			if(_TestEmuNand(i, tmpPath.c_str(), true) || searchvalid)
			{
				if(m_current_view == COVERFLOW_CHANNEL)
					m_cfg.setInt("NAND", "partition", i);
				else if(m_current_view == COVERFLOW_USB)
					m_cfg.setInt("GAMES", "savepartition", i);
					
				*emuPath = tmpPath;
				m_cfg.save();
		
				return i;
			}
		
			if(i == USB8 && !fllscn)
			{
				i = -1;
				fllscn = true;
			}
		}
	}
	
	return -1;
}

bool CMenu::_checkSave(string id, bool nand)
{
	int savePath = id.c_str()[0] << 24 | id.c_str()[1] << 16 | id.c_str()[2] << 8 | id.c_str()[3];
	if(nand)
	{
		u32 temp = 0;	
		if(ISFS_ReadDir(sfmt("/title/00010000/%08x", savePath).c_str(), NULL, &temp) < 0)
			if(ISFS_ReadDir(sfmt("/title/00010004/%08x", savePath).c_str(), NULL, &temp) < 0)
				return false;
	}
	else
	{	
		int emuPartition = m_cfg.getInt("GAMES", "savepartition", -1);						
		string emuPath = m_cfg.getString("GAMES", "savepath", "");
		if(emuPartition < 0 || emuPath.size() == 0)
			return false;
			
		struct stat fstat;
		if((stat(sfmt("%s:%s/title/00010000/%08x", DeviceName[emuPartition], emuPath.c_str(), savePath).c_str(), &fstat) != 0 ) 
			&& (stat(sfmt("%s:%s/title/00010004/%08x", DeviceName[emuPartition], emuPath.c_str(), savePath).c_str(), &fstat) != 0))
			return false;
	}
	return true;
}

void CMenu::_enableNandEmu(bool fromconfig)
{
	_cfNeedsUpdate();
	bool disable = (m_cfg.getBool("NAND", "disable", true) || neek2o()) && m_current_view == COVERFLOW_CHANNEL && !m_tempView;	

	if(!disable)
	{
		bool isD2XnewerThanV6 = (CurrentIOS.Type == IOS_TYPE_NEEK2O);
		if(CurrentIOS.Revision > 6 && CurrentIOS.Type == IOS_TYPE_D2X)
			isD2XnewerThanV6 = true;
		if(m_current_view == COVERFLOW_CHANNEL && !m_cfg.getBool("NAND", "disable", true) && !neek2o() && !m_tempView)
			NandHandle.Enable_Emu();
		u8 limiter = 0;
		s8 direction = m_btnMgr.selected(m_configBtnPartitionP) ? 1 : -1;
		if (!fromconfig)
			direction = 0;
		currentPartition = loopNum(currentPartition + direction, (int)USB8);
		while(!DeviceHandle.IsInserted(currentPartition) ||
			(m_current_view == COVERFLOW_CHANNEL && (DeviceHandle.GetFSType(currentPartition) != PART_FS_FAT ||
				(!isD2XnewerThanV6 && DeviceHandle.PathToDriveType(m_appDir.c_str()) == currentPartition) ||
				(!isD2XnewerThanV6 && DeviceHandle.PathToDriveType(m_dataDir.c_str()) == currentPartition))) ||
			((m_current_view == COVERFLOW_HOMEBREW || m_current_view == COVERFLOW_DML) && DeviceHandle.GetFSType(currentPartition) == PART_FS_WBFS))
		{
			currentPartition = loopNum(currentPartition + direction, (int)USB8);
			if (limiter > 10) break;
			limiter++;
		}
		gprintf("Next item: %s\n", DeviceName[currentPartition]);
		if(m_tempView)
			m_cfg.setInt("GAMES", "savepartition", currentPartition);
		else
			m_cfg.setInt(_domainFromView(), "partition", currentPartition);
	}
}

void CMenu::_setDumpMsg(const wstringEx &msg, float totprog, float fileprog)
{
	if(m_thrdStop) return;
	if(msg != L"...") m_thrdMessage = msg;
	m_thrdMessageAdded = true;
	m_thrdProgress = totprog;
	m_fileProgress = fileprog;
}

void CMenu::_ShowProgress(int dumpstat, int dumpprog, int filesize, int fileprog, int files, int folders, char *tmess, void *user_data)
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
	m_btnMgr.hide(m_nandemuBtnExtract, instant);
	m_btnMgr.hide(m_nandemuBtnPartition, instant);
	m_btnMgr.hide(m_nandemuBtnDisable, instant);	
	m_btnMgr.hide(m_nandemuLblInit, instant); 
}

void CMenu::_showNandEmu(void)
{
	_setBg(m_nandemuBg, m_nandemuBg);
	m_btnMgr.setText(m_nandemuLblTitle, _t("cfgne10", L"NAND Emulation Settings"));
	m_btnMgr.show(m_nandemuLblTitle);
	m_btnMgr.show(m_nandemuBtnBack);
	int i;
	if(((m_current_view == COVERFLOW_CHANNEL && !m_cfg.getBool("NAND", "disable", true)) || m_current_view == COVERFLOW_USB) && !m_locked)
	{
		m_btnMgr.show(m_nandemuLblEmulation);
		m_btnMgr.show(m_nandemuLblEmulationVal);
		m_btnMgr.show(m_nandemuBtnEmulationP);
		m_btnMgr.show(m_nandemuBtnEmulationM);
	}
	
	if((m_current_view == COVERFLOW_CHANNEL || m_current_view == COVERFLOW_USB) && !m_locked)
	{		
		m_btnMgr.show(m_nandemuLblSaveDump);
		m_btnMgr.show(m_nandemuBtnAll);
		m_btnMgr.show(m_nandemuBtnMissing);
		m_btnMgr.show(m_nandemuLblNandDump);
		m_btnMgr.show(m_nandemuBtnNandDump);
		if (m_current_view == COVERFLOW_CHANNEL)
		{
			i = min(max(0, m_cfg.getInt("NAND", "emulation", 0)), (int)ARRAY_SIZE(CMenu::_NandEmu) - 1);
			m_btnMgr.setText(m_nandemuLblEmulationVal, _t(CMenu::_NandEmu[i].id, CMenu::_NandEmu[i].text));
		}
		else if (m_current_view == COVERFLOW_USB)
		{
			i = min(max(0, m_cfg.getInt("GAMES", "save_emulation", 0)), (int)ARRAY_SIZE(CMenu::_GlobalSaveEmu) - 1);
			m_btnMgr.setText(m_nandemuLblEmulationVal, _t(CMenu::_GlobalSaveEmu[i].id, CMenu::_GlobalSaveEmu[i].text));
		}
	}
}

int CMenu::_NandEmuCfg(void)
{	
	lwp_t thread = 0;
	SetupInput();
	_showNandEmu();

	m_thrdStop = false;
	m_thrdMessageAdded = false;
	m_nandext = false;

	while(!m_exit)
	{
		_mainLoopCommon();
		if((BTN_HOME_PRESSED || BTN_B_PRESSED) && !m_thrdWorking)
			break;
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if (BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnEmulationP) || m_btnMgr.selected(m_nandemuBtnEmulationM)))
		{
			s8 direction = m_btnMgr.selected(m_nandemuBtnEmulationP) ? 1 : -1;
			if(m_current_view == COVERFLOW_CHANNEL)
				m_cfg.setInt("NAND", "emulation", (int)loopNum((u32)m_cfg.getInt("NAND", "emulation", 0) + direction, ARRAY_SIZE(CMenu::_NandEmu)));
			else if(m_current_view == COVERFLOW_USB)
				m_cfg.setInt("GAMES", "save_emulation", (int)loopNum((u32)m_cfg.getInt("GAMES", "save_emulation", 0) + direction, ARRAY_SIZE(CMenu::_GlobalSaveEmu)));
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
	int emuPartition = m_cfg.getInt("GAMES", "savepartition", m_cfg.getInt("NAND", "partition", 0));			
	char basepath[MAX_FAT_PATH];	
	snprintf(basepath, sizeof(basepath), "%s:%s", DeviceName[emuPartition], m_cfg.getString("GAMES", "savepath", m_cfg.getString("NAND", "path", "")).c_str());	

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

	if(m_current_view == COVERFLOW_CHANNEL)
		m_partRequest = m_cfg.getInt("NAND", "partition", -1);	
	else if(m_current_view == COVERFLOW_USB)
		m_partRequest = m_cfg.getInt("GAMES", "savepartition", -1);	
	int emuPartition = _FindEmuPart(&emuPath, m_partRequest, false);
	
	if(emuPartition < 0)
		emuPartition = _FindEmuPart(&emuPath, m_partRequest, true);	
	
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
	string emuPath;
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
		if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnExtract)))
		{
			m_fulldump =  true;
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
		else if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnDisable)))
		{
			_hideNandEmu();			
			return 0;
		}
		else if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnPartition)))
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
	
	if(m.m_current_view == COVERFLOW_CHANNEL)
		m.m_partRequest = m.m_cfg.getInt("NAND", "partition", -1);	
	else if(m.m_current_view == COVERFLOW_USB)
		m.m_partRequest = m.m_cfg.getInt("GAMES", "savepartition", -1);
	
	int emuPartition = m._FindEmuPart(&emuPath, m.m_partRequest, false);	
	int flashID = m.m_saveExtGameId.c_str()[0] << 24 | m.m_saveExtGameId.c_str()[1] << 16 | m.m_saveExtGameId.c_str()[2] << 8 | m.m_saveExtGameId.c_str()[3];
	
	if(_saveExists(sfmt("%s:%s/title/00010000/%08x", DeviceName[emuPartition], emuPath.c_str(), flashID).c_str()))	
	{
		snprintf(source, sizeof(source), "%s:%s/title/00010000/%08x", DeviceName[emuPartition], emuPath.c_str(), flashID);
		snprintf(dest, sizeof(dest), "/title/00010000/%08x", flashID);
	}
	else if(_saveExists(sfmt("%s:%s/title/00010004/%08x", DeviceName[emuPartition], emuPath.c_str(), flashID).c_str()))
	{
		snprintf(source, sizeof(source), "%s:%s/title/00010004/%08x", DeviceName[emuPartition], emuPath.c_str(), flashID);
		snprintf(dest, sizeof(dest), "/title/00010004/%08x", flashID);
	}
	NandHandle.ResetCounters();
	m.m_nandexentry = 1;
	m.m_dumpsize = NandHandle.CalcFlashSize(source, CMenu::_ShowProgress, obj);
	m_nandext = true;
	NandHandle.FlashToNAND(source, dest, CMenu::_ShowProgress, obj);
	
	m.m_thrdWorking = false;
	LWP_MutexLock(m.m_mutex);
	m.m_btnMgr.hide(m_nandfilePBar);
	m.m_btnMgr.hide(m_nandfileLblMessage);
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

	if(m.m_current_view == COVERFLOW_CHANNEL)
		m.m_partRequest = m.m_cfg.getInt("NAND", "partition", -1);	
	else if(m.m_current_view == COVERFLOW_USB)
		m.m_partRequest = m.m_cfg.getInt("GAMES", "savepartition", -1);

	emuPartition = m._FindEmuPart(&emuPath, m.m_partRequest, true);

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
	m.m_btnMgr.hide(m_nandfilePBar);
	m.m_btnMgr.hide(m_nandfileLblMessage);
	m._setDumpMsg(m._t("cfgne19", L"Extraction finished!"), 1.f, 1.f);
	LWP_MutexUnlock(m.m_mutex);
	return 0;
}

void CMenu::_initNandEmuMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_nandemuLblUser, ARRAY_SIZE(m_nandemuLblUser), "NANDEMU");
	m_nandemuBg = _texture(theme.texSet, "NANDEMU/BG", "texture", theme.bg);
	m_nandemuLblTitle = _addTitle(theme, "NANDEMU/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_nandfileLblMessage = _addLabel(theme, "NANDEMU/FMESSAGE", theme.lblFont, L"", 40, 230, 560, 100, theme.lblFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_TOP);
	m_nandemuLblMessage = _addLabel(theme, "NANDEMU/MESSAGE", theme.lblFont, L"", 40, 350, 560, 100, theme.lblFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_TOP);
	m_nandfileLblDialog = _addLabel(theme, "NANDEMU/FDIALOG", theme.lblFont, L"", 40, 60, 560, 200, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandfinLblDialog = _addLabel(theme, "NANDEMU/FINDIALOG", theme.lblFont, L"", 40, 120, 560, 200, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuLblDialog = _addLabel(theme, "NANDEMU/DIALOG", theme.lblFont, L"", 40, 180, 560, 200, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandfilePBar = _addProgressBar(theme, "NANDEMU/FILEPROGRESS_BAR", 40, 200, 560, 20);
	m_nandemuPBar = _addProgressBar(theme, "NANDEMU/PROGRESS_BAR", 40, 320, 560, 20);
	m_nandemuLblEmulation = _addLabel(theme, "NANDEMU/EMU_SAVE", theme.lblFont, L"", 40, 130, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuLblEmulationVal = _addLabel(theme, "NANDEMU/EMU_SAVE_BTN_GLOBAL", theme.btnFont, L"", 400, 130, 144, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_nandemuBtnEmulationM = _addPicButton(theme, "NANDEMU/EMU_SAVE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 344, 130, 56, 56);
	m_nandemuBtnEmulationP = _addPicButton(theme, "NANDEMU/EMU_SAVE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 544, 130, 56, 56);
	m_nandemuLblSaveDump = _addLabel(theme, "NANDEMU/SAVE_DUMP", theme.lblFont, L"", 40, 190, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuBtnAll = _addButton(theme, "NANDEMU/ALL_BTN", theme.btnFont, L"", 350, 190, 250, 56, theme.btnFontColor);
	m_nandemuBtnMissing = _addButton(theme, "NANDEMU/MISSING_BTN", theme.btnFont, L"", 350, 250, 250, 56, theme.btnFontColor);
	m_nandemuLblNandDump = _addLabel(theme, "NANDEMU/NAND_DUMP", theme.lblFont, L"", 40, 310, 340, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_nandemuBtnNandDump = _addButton(theme, "NANDEMU/NAND_DUMP_BTN", theme.btnFont, L"", 350, 310, 250, 56, theme.btnFontColor);
	m_nandemuBtnBack = _addButton(theme, "NANDEMU/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_nandemuBtnExtract = _addButton(theme, "NANDEMU/EXTRACT", theme.titleFont, L"", 72, 180, 496, 56, theme.titleFontColor);
	m_nandemuBtnDisable = _addButton(theme, "NANDEMU/DISABLE", theme.titleFont, L"", 72, 270, 496, 56, theme.titleFontColor);
	m_nandemuBtnPartition = _addButton(theme, "NANDEMU/PARTITION", theme.titleFont, L"", 72, 360, 496, 56, theme.titleFontColor);
	m_nandemuLblInit = _addLabel(theme, "NANDEMU/INIT", theme.lblFont, L"", 40, 40, 560, 140, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);

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
	_setHideAnim(m_nandemuBtnBack, "NANDEMU/BACK_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuBtnExtract, "NANDEMU/EXTRACT", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuBtnPartition, "NANDEMU/PARTITION", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuBtnDisable, "NANDEMU/DISABLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblInit, "NANDEMU/INIT", 100, 0, -2.f, 0.f);

	_hideNandEmu(true);
	_textNandEmu();
}

void CMenu::_textNandEmu(void)
{	
	m_btnMgr.setText(m_nandemuLblEmulation, _t("cfgne1", L"NAND Emulation"));
	m_btnMgr.setText(m_nandemuLblSaveDump, _t("cfgne2", L"Extract Game Saves"));
	m_btnMgr.setText(m_nandemuBtnAll, _t("cfgne3", L"All"));
	m_btnMgr.setText(m_nandemuBtnMissing, _t("cfgne4", L"Missing"));	
	m_btnMgr.setText(m_nandemuLblNandDump, _t("cfgne5", L"Extract NAND"));
	m_btnMgr.setText(m_nandemuBtnNandDump, _t("cfgne6", L"Start"));	
	m_btnMgr.setText(m_nandemuBtnBack, _t("cfgne7", L"Back"));	
}
