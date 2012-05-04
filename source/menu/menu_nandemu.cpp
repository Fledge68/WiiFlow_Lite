
#include "menu.hpp"
#include "nand.hpp"
#include "sys.h"
#include "loader/cios.hpp"
#include "loader/alt_ios.h"
#include "lockMutex.hpp"
#include "gecko/gecko.h"
#include "defines.h"

using namespace std;

static inline int loopNum(int i, int s)
{
	return i < 0 ? (s - (-i % s)) % s : i % s;
}

static bool _saveExcists(const char *path)
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

static bool _nandSaveExcists(const char *npath)
{	
	u32 temp = 0;	
	
	s32 ret = ISFS_ReadDir(npath, NULL, &temp);
	if(ret < 0)
		return false;
		
	return true;
}

void CMenu::_enableNandEmu(bool fromconfig)
{
	_cfNeedsUpdate();
	bool disable = true;
	int i = m_current_view == COVERFLOW_CHANNEL && min(max(0, m_cfg.getInt("NAND", "emulation", 0)), (int)ARRAY_SIZE(CMenu::_NandEmu) - 1);
	gprintf("i: %i\n",i);
	if (i>0 || m_current_view != COVERFLOW_CHANNEL)
		disable = false;
	if(!disable)
	{
		Nand::Instance()->Disable_Emu();
		bool isD2XnewerThanV6 = false;
		iosinfo_t * iosInfo = cIOSInfo::GetInfo(mainIOS);
		if (iosInfo->version > 6)
			isD2XnewerThanV6 = true;
		if(m_current_view == COVERFLOW_CHANNEL && m_cfg.getInt("NAND", "emulation", 0) > 0)
			Nand::Instance()->Enable_Emu();
		u8 limiter = 0;
		s8 direction = m_btnMgr.selected(m_configBtnPartitionP) ? 1 : -1;
		if (!fromconfig)
			direction = 0;
		currentPartition = loopNum(currentPartition + direction, (int)USB8);
		while(!DeviceHandler::Instance()->IsInserted(currentPartition) ||
			(m_current_view == COVERFLOW_CHANNEL && (DeviceHandler::Instance()->GetFSType(currentPartition) != PART_FS_FAT ||
				(!isD2XnewerThanV6 && DeviceHandler::Instance()->PathToDriveType(m_appDir.c_str()) == currentPartition) ||
				(!isD2XnewerThanV6 && DeviceHandler::Instance()->PathToDriveType(m_dataDir.c_str()) == currentPartition))) ||
			((m_current_view == COVERFLOW_HOMEBREW || m_current_view == COVERFLOW_DML) && DeviceHandler::Instance()->GetFSType(currentPartition) == PART_FS_WBFS))
		{
			currentPartition = loopNum(currentPartition + direction, (int)USB8);
			if (limiter > 10) break;
			limiter++;
		}

		gprintf("Next item: %s\n", DeviceName[currentPartition]);
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
	if(m.m_nandext)
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
	m_btnMgr.hide(m_nandemuBtnDisable, instant);	
	m_btnMgr.hide(m_nandemuLblInit, instant); 
}

void CMenu::_showNandEmu(void)
{
	_setBg(m_nandemuBg, m_nandemuBg);
	m_btnMgr.setText(m_nandemuLblTitle, _t("cfgne10", L"Nand emulation settings"));
	m_btnMgr.show(m_nandemuLblTitle);
	m_btnMgr.show(m_nandemuBtnBack);
	int i;
	if((m_current_view == COVERFLOW_CHANNEL || m_current_view == COVERFLOW_USB) && !m_locked)
	{
		m_btnMgr.show(m_nandemuLblEmulation);
		m_btnMgr.show(m_nandemuLblEmulationVal);
		m_btnMgr.show(m_nandemuBtnEmulationP);
		m_btnMgr.show(m_nandemuBtnEmulationM);
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
	
	while(true)
	{
		_mainLoopCommon(false, m_thrdWorking);
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
			m_btnMgr.setText(m_nandemuLblDialog, _t("cfgne11", L"Overall progress:"));
			if(m_fulldump)
				m_btnMgr.setText(m_nandemuLblTitle, _t("cfgne12", L"Nand extractor"));
			else
				m_btnMgr.setText(m_nandemuLblTitle, _t("cfgne13", L"Game save extractor"));
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
			m_btnMgr.setText(m_nandfileLblMessage, wfmt(_fmt("fileprogress", L"%d / %dkb"), m_fileprog/0x400, m_filesize/0x400));
			m_btnMgr.setText(m_nandemuLblMessage, wfmt(_fmt("dumpprogress", L"%i%%"), (int)(m_thrdProgress*100.f)));			
			
			if (!m_thrdWorking)
			{
				
				if(m_sgdump)
					m_btnMgr.setText(m_nandfinLblDialog, wfmt(_fmt("cfgne14", L"Extracted: %d saves / %d files / %d folders"), m_nandexentry, m_filesdone, m_foldersdone));
				else
					m_btnMgr.setText(m_nandfinLblDialog, wfmt(_fmt("cfgne15", L"Extracted: %d files / %d folders"), m_filesdone, m_foldersdone));
				if(m_dumpsize/0x400 > 0x270f)
					m_btnMgr.setText(m_nandemuLblDialog, wfmt(_fmt("cfgne16", L"Total size: %uMb (%d blocks)"), (m_dumpsize/0x100000), (m_dumpsize/0x8000)>>2));
				else
					m_btnMgr.setText(m_nandemuLblDialog, wfmt(_fmt("cfgne17", L"Total size: %ukb (%d blocks)"), (m_dumpsize/0x400), (m_dumpsize/0x8000)>>2));
				
				
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
	int emuPartition = m_cfg.getInt("GAMES", "savepartition", m_cfg.getInt("NAND", "partition", 0));
	int savePath = gameId.c_str()[0] << 24 | gameId.c_str()[1] << 16 | gameId.c_str()[2] << 8 | gameId.c_str()[3];			
	string npath = sfmt("/title/00010000/%x", savePath);
	string path = sfmt("%s:%s/title/00010000/%x", DeviceName[emuPartition], m_cfg.getString("GAMES", "savepath", m_cfg.getString("NAND", "path", "")).c_str(), savePath);
	
	if(!_nandSaveExcists(npath.c_str()) || (!m_forceext && _saveExcists(path.c_str())))
		return 0;
		
	lwp_t thread = 0;
	SetupInput();
	m_thrdStop = false;
	m_thrdMessageAdded = false;
	m_nandext = false;	
	
	if(!m_forceext)
	{
		m_btnMgr.setText(m_nandemuBtnExtract, _t("cfgne24", L"Extract save"));
		m_btnMgr.setText(m_nandemuBtnDisable, _t("cfgne25", L"Create new save"));	
		m_btnMgr.setText(m_nandemuLblInit, _t("cfgne26", L"A save file for this game was created on real nand. Extract existing safe file from real nand or create new file for nand emulation?"));
		m_btnMgr.show(m_nandemuBtnExtract);
		m_btnMgr.show(m_nandemuBtnDisable);	
		m_btnMgr.show(m_nandemuLblInit);
	}

	m_saveExtGameId = gameId;
	
	while(true)
	{
		_mainLoopCommon(false, m_thrdWorking);
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
			m_btnMgr.setText(m_nandemuLblDialog, _t("cfgne11", L"Overall progress:"));
			m_btnMgr.setText(m_nandemuLblTitle, _t("cfgne13", L"Game save extractor"));
			m_thrdStop = false;
			m_thrdProgress = 0.f;
			m_thrdWorking = true;
			LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_NandDumper, (void *)this, 0, 32768, 40);
		}
		if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnDisable)))
		{
			_hideNandEmu();			
			return 0;
		}
		
		if(m_thrdMessageAdded)
		{
			LockMutex lock(m_mutex);
			m_thrdMessageAdded = false;
			if (!m_thrdMessage.empty())
				m_btnMgr.setText(m_nandfileLblDialog, m_thrdMessage);
			m_btnMgr.setProgress(m_nandfilePBar, m_fileProgress);
			m_btnMgr.setProgress(m_nandemuPBar, m_thrdProgress);
			m_btnMgr.setText(m_nandfileLblMessage, wfmt(_fmt("fileprogress", L"%d / %dkb"), m_fileprog/0x400, m_filesize/0x400));
			m_btnMgr.setText(m_nandemuLblMessage, wfmt(_fmt("dumpprogress", L"%i%%"), (int)(m_thrdProgress*100.f)));			
			
			if (!m_thrdWorking)
			{
				m_btnMgr.setText(m_nandfinLblDialog, wfmt(_fmt("cfgne14", L"Extracted: %d saves / %d files / %d folders"), m_nandexentry, m_filesdone, m_foldersdone));
				if(m_dumpsize/0x400 > 0x270f)
					m_btnMgr.setText(m_nandemuLblDialog, wfmt(_fmt("cfgne16", L"Total size: %uMb (%d blocks)"), (m_dumpsize/0x100000), (m_dumpsize/0x8000)>>2));
				else
					m_btnMgr.setText(m_nandemuLblDialog, wfmt(_fmt("cfgne17", L"Total size: %ukb (%d blocks)"), (m_dumpsize/0x400), (m_dumpsize/0x8000)>>2));	
				
				_hideNandEmu();
				return 1;
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
	
	m_btnMgr.setText(m_nandemuBtnExtract, _t("cfgne5", L"Extract nand"));
	m_btnMgr.setText(m_nandemuBtnDisable, _t("cfgne22", L"Disable nandemulation"));	
	m_btnMgr.setText(m_nandemuLblInit, _t("cfgne23", L"Welcome to WiiFlow. I have not found a valid nand for nand emulation. Click Extract to extract your nand, or click disable to disable nand emulation."));
	m_btnMgr.show(m_nandemuBtnExtract);
	m_btnMgr.show(m_nandemuBtnDisable);	
	m_btnMgr.show(m_nandemuLblInit);	
	
	while(true)
	{
		_mainLoopCommon(false, m_thrdWorking);
		if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnExtract)))
		{
			m_fulldump =  true;
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
			m_btnMgr.setText(m_nandemuLblDialog, _t("cfgne11", L"Overall progress:"));
			m_btnMgr.setText(m_nandemuLblTitle, _t("cfgne12", L"Nand extractor"));
			m_thrdStop = false;
			m_thrdProgress = 0.f;
			m_thrdWorking = true;
			LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_NandDumper, (void *)this, 0, 32768, 40);
		}
		if(BTN_A_PRESSED && (m_btnMgr.selected(m_nandemuBtnDisable)))
		{
			_hideNandEmu();			
			return 0;
		}
		
		if(m_thrdMessageAdded)
		{
			LockMutex lock(m_mutex);
			m_thrdMessageAdded = false;
			if (!m_thrdMessage.empty())
				m_btnMgr.setText(m_nandfileLblDialog, m_thrdMessage);
			m_btnMgr.setProgress(m_nandfilePBar, m_fileProgress);
			m_btnMgr.setProgress(m_nandemuPBar, m_thrdProgress);
			m_btnMgr.setText(m_nandfileLblMessage, wfmt(_fmt("fileprogress", L"%d / %dkb"), m_fileprog/0x400, m_filesize/0x400));
			m_btnMgr.setText(m_nandemuLblMessage, wfmt(_fmt("dumpprogress", L"%i%%"), (int)(m_thrdProgress*100.f)));			
			
			if (!m_thrdWorking)
			{
				m_btnMgr.setText(m_nandfinLblDialog, wfmt(_fmt("cfgne15", L"Extracted: %d files / %d folders"), m_filesdone, m_foldersdone));
				if(m_dumpsize/0x400 > 0x270f)
					m_btnMgr.setText(m_nandemuLblDialog, wfmt(_fmt("cfgne16", L"Total size: %uMb (%d blocks)"), (m_dumpsize/0x100000), (m_dumpsize/0x8000)>>2));
				else
					m_btnMgr.setText(m_nandemuLblDialog, wfmt(_fmt("cfgne17", L"Total size: %ukb (%d blocks)"), (m_dumpsize/0x400), (m_dumpsize/0x8000)>>2));	
				
				_hideNandEmu();
				return 1;
			}
		}
	}	
	_hideNandEmu();
	return 0;
}

int CMenu::_NandDumper(void *obj)
{
	CMenu &m = *(CMenu *)obj;
	string emuPath;
	int emuPartition = -1;
	bool emuPartIsValid = false;
	m.m_nandext = false;
	m.m_sgdump = false;
	m.m_dumpsize = 0;
	m.m_filesdone = 0;
	m.m_foldersdone = 0;
	
	Nand::Instance()->ResetCounters();
	
	if(m.m_current_view == COVERFLOW_CHANNEL)
	{
		emuPartition = m.m_cfg.getInt("NAND", "partition", 0);
		emuPath = m.m_cfg.getString("NAND", "path", "");
	}
	else if(m.m_current_view == COVERFLOW_USB)
	{
		emuPartition = m.m_cfg.getInt("GAMES", "savepartition", -1);
		if(emuPartition == -1)
			emuPartition = m.m_cfg.getInt("NAND", "partition", 0);
		emuPath = m.m_cfg.getString("GAMES", "savepath", m.m_cfg.getString("NAND", "path", ""));
	}
	
	for(u8 i = emuPartition; i <= USB8; ++i)
	{
		if(!DeviceHandler::Instance()->IsInserted(emuPartition) || DeviceHandler::Instance()->GetFSType(emuPartition) != PART_FS_FAT)
		{
			emuPartition++;
			continue;
		}
		else
		{
			emuPartIsValid = true;
			if(m.m_current_view == COVERFLOW_CHANNEL)
				m.m_cfg.setInt("NAND", "partition", emuPartition);
			else if(m.m_current_view == COVERFLOW_USB)
				m.m_cfg.setInt("GAMES", "savepartition", emuPartition);			
			break;
		}
	}
	
	if(!emuPartIsValid)
	{
		m.error(sfmt("No valid FAT partition found for nandemulation!"));
		m.m_thrdWorking = false;		
		m.m_btnMgr.hide(m.m_nandfilePBar);
		m.m_btnMgr.hide(m.m_nandfileLblMessage);
		LWP_MutexLock(m.m_mutex);
		m._setDumpMsg(m._t("cfgne20", L"Extraction failed!"), 1.f, 1.f);
		LWP_MutexUnlock(m.m_mutex);
		m._hideNandEmu();
		return 0;
	}
	
	if(emuPath.size() == 0)
	{
		Nand::Instance()->CreatePath("%s:/wiiflow", DeviceName[emuPartition]);
		Nand::Instance()->CreatePath("%s:/wiiflow/nandemu", DeviceName[emuPartition]);
		if(m.m_current_view == COVERFLOW_CHANNEL)
		{
			m.m_cfg.setString("NAND", "path", STDEMU_DIR);
			emuPath = m.m_cfg.getString("NAND", "path", STDEMU_DIR);
			
		}
		else if(m.m_current_view == COVERFLOW_USB)
		{
			m.m_cfg.setString("GAMES", "savepath", STDEMU_DIR);			
			emuPath = m.m_cfg.getString("GAMES", "savepath", STDEMU_DIR);
		}		
	}
	m.m_cfg.save();
	
	char basepath[64];
	snprintf(basepath, 64, "%s:%s", DeviceName[emuPartition], emuPath.c_str());	
	
	LWP_MutexLock(m.m_mutex);
	m._setDumpMsg(L"Calculating space needed for extraction...", 0.f, 0.f);
	LWP_MutexUnlock(m.m_mutex);
	
	if(m.m_fulldump)
	{
		m.m_dumpsize = Nand::Instance()->CalcDumpSpace("/", true, CMenu::_ShowProgress, obj);
		m.m_nandext = true;	
		Nand::Instance()->DoNandDump("/", basepath, true, CMenu::_ShowProgress, obj);
	}
	else
	{
		bool missingOnly = !m.m_saveall;		
		string path, npath;
		safe_vector<string> saveList;
		m.m_sgdump = true;
		
		if(m.m_saveExtGameId.empty())
		{
			m.m_nandexentry = 0;
			saveList.reserve(m.m_gameList.size());
			for(u32 i = 0; i < m.m_gameList.size() && !m.m_thrdStop; ++i)
			{
				LWP_MutexLock(m.m_mutex);
				m._setDumpMsg(m._t("cfgne18", L"Listing game saves to extract..."), 0.f, 0.f);
				LWP_MutexUnlock(m.m_mutex);					
				
				string id((const char *)m.m_gameList[i].hdr.id, 4);
						
				int savePath = id.c_str()[0] << 24 | id.c_str()[1] << 16 | id.c_str()[2] << 8 | id.c_str()[3];
				
				path = sfmt("%s/title/00010000/%x", basepath, savePath);
				npath = sfmt("/title/00010000/%x", savePath);
						
				if(!missingOnly || !_saveExcists(path.c_str()))
				{
					if(_nandSaveExcists(npath.c_str()))
					{
						m.m_nandexentry++;
						saveList.push_back(id);
					}
				}	

			}
		}
		else
			saveList.push_back(m.m_saveExtGameId);

		
		for(u32 i = 0; i < saveList.size() && !m.m_thrdStop; ++i)
		{
			char source[ISFS_MAXPATH];
			int savePath = saveList[i].c_str()[0] << 24 | saveList[i].c_str()[1] << 16 | saveList[i].c_str()[2] << 8 | saveList[i].c_str()[3];
			snprintf(source, sizeof(source), "/title/00010000/%x",  savePath);
			m.m_dumpsize = Nand::Instance()->CalcDumpSpace(source, false, CMenu::_ShowProgress, obj);	
		}		
		for(u32 i = 0; i < saveList.size() && !m.m_thrdStop; ++i)
		{
			char source[ISFS_MAXPATH];
			int savePath = saveList[i].c_str()[0] << 24 | saveList[i].c_str()[1] << 16 | saveList[i].c_str()[2] << 8 | saveList[i].c_str()[3];
			snprintf(source, sizeof(source), "/title/00010000/%x",  savePath);	
			m.m_nandext = true;	
			Nand::Instance()->DoNandDump(source, basepath, false, CMenu::_ShowProgress, obj);
		}
	}
	
	m.m_thrdWorking = false;
	LWP_MutexLock(m.m_mutex);
	m.m_btnMgr.hide(m.m_nandfilePBar);
	m.m_btnMgr.hide(m.m_nandfileLblMessage);
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
	m_nandemuBtnDisable = _addButton(theme, "NANDEMU/DISABLE", theme.titleFont, L"", 72, 290, 496, 56, theme.titleFontColor);
	m_nandemuLblInit = _addLabel(theme, "NANDEMU/INIT", theme.lblFont, L"", 40, 40, 560, 140, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	
	
	_setHideAnim(m_nandemuLblTitle, "NANDEMU/TITLE", 0, 0, -2.f, 0.f);
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
	_setHideAnim(m_nandemuBtnDisable, "NANDEMU/DISABLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_nandemuLblInit, "NANDEMU/INIT", 100, 0, -2.f, 0.f);

	_hideNandEmu(true);
	_textNandEmu();
}

void CMenu::_textNandEmu(void)
{	
	m_btnMgr.setText(m_nandemuLblEmulation, _t("cfgne1", L"Nand emulation"));
	m_btnMgr.setText(m_nandemuLblSaveDump, _t("cfgne2", L"Extract game saves"));
	m_btnMgr.setText(m_nandemuBtnAll, _t("cfgne3", L"All"));
	m_btnMgr.setText(m_nandemuBtnMissing, _t("cfgne4", L"Missing"));	
	m_btnMgr.setText(m_nandemuLblNandDump, _t("cfgne5", L"Extract nand"));
	m_btnMgr.setText(m_nandemuBtnNandDump, _t("cfgne6", L"Start"));	
	m_btnMgr.setText(m_nandemuBtnBack, _t("cfgne7", L"Back"));	
}