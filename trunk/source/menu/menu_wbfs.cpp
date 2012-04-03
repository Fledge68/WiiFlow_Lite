
#include "menu.hpp"
#include "loader/wbfs.h"
#include "lockMutex.hpp"
#include "loader/gc_disc.hpp"
#include "gc.h"
#include "fileOps.h"
#include "music/SoundHandler.hpp"
#include "channel/nand.hpp"
#include "defines.h"

using namespace std;

void CMenu::_hideWBFS(bool instant)
{
	m_btnMgr.hide(m_wbfsLblTitle, instant);
	m_btnMgr.hide(m_wbfsPBar, instant);
	m_btnMgr.hide(m_wbfsBtnBack, instant);
	m_btnMgr.hide(m_wbfsBtnGo, instant);
	m_btnMgr.hide(m_wbfsLblDialog);
	m_btnMgr.hide(m_wbfsLblMessage);
	for (u32 i = 0; i < ARRAY_SIZE(m_wbfsLblUser); ++i)
		if (m_wbfsLblUser[i] != -1u)
			m_btnMgr.hide(m_wbfsLblUser[i], instant);
}

void CMenu::_showWBFS(CMenu::WBFS_OP op)
{
	_setBg(m_wbfsBg, m_wbfsBg);
	switch (op)
	{
		case CMenu::WO_ADD_GAME:
				m_btnMgr.setText(m_wbfsLblTitle, _t("wbfsop1", L"Install Game"));
				break;
		case CMenu::WO_REMOVE_GAME:
				m_btnMgr.setText(m_wbfsLblTitle, _t("wbfsop2", L"Delete Game"));
				break;
		case CMenu::WO_FORMAT:
//				m_btnMgr.setText(m_wbfsLblTitle, _t("wbfsop3", L"Format"));
				break;
		case CMenu::WO_COPY_GAME:
				m_btnMgr.setText(m_wbfsLblTitle, _t("wbfsop11", L"Copy Game"));
				break;
	};
	m_btnMgr.show(m_wbfsLblTitle);
	m_btnMgr.show(m_wbfsBtnBack);
	m_btnMgr.show(m_wbfsBtnGo);
	m_btnMgr.show(m_wbfsLblDialog);
	for (u32 i = 0; i < ARRAY_SIZE(m_wbfsLblUser); ++i)
		if (m_wbfsLblUser[i] != -1u)
			m_btnMgr.show(m_wbfsLblUser[i]);
}

static void slotLight(bool state)
{
	if (state)
		*(u32 *)0xCD0000C0 |= 0x20;        
	else
		*(u32 *)0xCD0000C0 &= ~0x20;        
}

void CMenu::_addDiscProgress(int status, int total, void *user_data)
{
	CMenu &m = *(CMenu *)user_data;
	m.m_progress = total == 0 ? 0.f : (float)status / (float)total;
	// Don't synchronize too often
	if(m.m_progress - m.m_thrdProgress >= 0.01f)
	{
		LWP_MutexLock(m.m_mutex);
		m._setThrdMsg(L"", m.m_progress);
		LWP_MutexUnlock(m.m_mutex);
	}
}

void CMenu::_Messenger(int message, int info, char *cinfo, void *user_data)
{
	CMenu &m = *(CMenu *)user_data;
	LWP_MutexLock(m.m_mutex);
	if(message == 1)
		m._setThrdMsg(wfmt(m._fmt("wbfsop23", L"Calculating space needed for %s...\n Please insert disc %d to continue"), cinfo, info), m.m_progress);
	if(message == 2)
		m._setThrdMsg(wfmt(m._fmt("wbfsop15", L"Calculating space needed for %s"), cinfo), m.m_progress);
	if(message == 3)
		m._setThrdMsg(wfmt(m._fmt("wbfsop16", L"Installing %s"), cinfo), m.m_progress);
	if(message == 4)
		m._setThrdMsg(wfmt(m._fmt("wbfsop17", L"Installing %s disc %d/2"), cinfo, info), m.m_progress);
	if(message == 5)
		m._setThrdMsg(m._t("wbfsop18", L"Don't try to trick me with a Wii disc!!"), m.m_progress);
	if(message == 6)	
		m._setThrdMsg(m._t("wbfsop19", L"This is not a GC disc!!"), m.m_progress);
	if(message == 7)
		m._setThrdMsg(wfmt(m._fmt("wbfsop20", L"You inserted disc %d again!!"), info), m.m_progress);
	if(message == 8)
		m._setThrdMsg(m._t("wbfsop21", L"This is a disc of another game!!"), m.m_progress);
	if(message == 9)
		m._setThrdMsg(wfmt(m._fmt("wbfsop22", L"Installing %s...\n Please insert disc 2 to continue"), cinfo), m.m_progress);
	if(message == 10)	
		m._setThrdMsg(m._t("wbfsop25", L"Disc read error!! Please clean the disc"), m.m_progress);
	if(message == 11)	
		m._setThrdMsg(m._t("wbfsop26", L"Disc ejected!! Please insert disc again"), m.m_progress);
	LWP_MutexUnlock(m.m_mutex);
}

int CMenu::_gameInstaller(void *obj)
{
	CMenu &m = *(CMenu *)obj;
	int ret;

	if (!WBFS_Mounted())
	{
		m.m_thrdWorking = false;
		return -1;
	}

	u64 comp_size = 0, real_size = 0;
	f32 free, used;
	WBFS_DiskSpace(&used, &free);
	WBFS_DVD_Size(&comp_size, &real_size);
	
	if((f32)comp_size + (f32)128*1024 >= free * GB_SIZE)
	{
		LWP_MutexLock(m.m_mutex);
		m._setThrdMsg(wfmt(m._fmt("wbfsop10", L"Not enough space: %lld blocks needed, %i available"), comp_size, free), 0.f);
		LWP_MutexUnlock(m.m_mutex);
		ret = -1;
	}
	else
	{	
		LWP_MutexLock(m.m_mutex);
		m._setThrdMsg(L"", 0);
		LWP_MutexUnlock(m.m_mutex);
		
		ret = WBFS_AddGame(CMenu::_addDiscProgress, obj);
		LWP_MutexLock(m.m_mutex);
		if (ret == 0)
			m._setThrdMsg(m._t("wbfsop8", L"Game installed"), 1.f);
		else
			m._setThrdMsg(m._t("wbfsop9", L"An error has occurred"), 1.f);
		LWP_MutexUnlock(m.m_mutex);
		slotLight(true);
	}
	m.m_thrdWorking = false;
	return ret;
}

int CMenu::_GCgameInstaller(void *obj)
{
	CMenu &m = *(CMenu *)obj;
	GCDump m_gcdump;
	
	bool skip = m.m_cfg.getBool("DML", "skip_on_error", false);
	bool comp = m.m_cfg.getBool("DML", "compressed_dump", true);
	bool wexf = m.m_cfg.getBool("DML", "write_ex_files", true);
	bool alig = m.m_cfg.getBool("DML", "force_32k_align_files", false);
	u32 nretry = m.m_cfg.getUInt("DML", "num_retries", 5);
	u32 rsize = 1048576; //1MB
	
	if(skip)
		rsize = 8192; // Use small chunks when skip on error is enabled

	m_gcdump.Init(skip, comp, wexf, alig, nretry, rsize,DeviceName[currentPartition],m.m_DMLgameDir.c_str(), CMenu::_addDiscProgress, CMenu::_Messenger, obj);
	
	int ret;	
	m.m_progress = 0.f;

	if (!DeviceHandler::Instance()->IsInserted(currentPartition))
	{
		m.m_thrdWorking = false;
		return -1;
	}

	char partition[6];
	sprintf(partition,"%s:/",DeviceName[currentPartition]);

	u32 needed = 0;

	ret = m_gcdump.CheckSpace(&needed, comp);
	if(ret != 0)
	{
		m._setThrdMsg(m._t("wbfsop9", L"An error has occurred"), 1.f);
		m.m_thrdWorking = false;
		return ret;
	}
	
	if(m_gcdump.GetFreeSpace(partition, BL) <= needed)
	{
		gprintf("Free space available: %d Mb (%d blocks)\n", m_gcdump.GetFreeSpace(partition, MB), m_gcdump.GetFreeSpace(partition, BL));
		LWP_MutexLock(m.m_mutex);
		m._setThrdMsg(wfmt(m._fmt("wbfsop24", L"Not enough space: %d blocks needed, %d available"), needed, m_gcdump.GetFreeSpace(partition, BL)), 0.f);
		LWP_MutexUnlock(m.m_mutex);
		ret = -1;
	}
	else
	{
		gprintf("Free space available: %d Mb (%d blocks)\n", m_gcdump.GetFreeSpace(partition, MB), m_gcdump.GetFreeSpace(partition, BL));
		LWP_MutexLock(m.m_mutex);
		m._setThrdMsg(L"", 0);
		LWP_MutexUnlock(m.m_mutex);
		
		ret = m_gcdump.DumpGame();
		LWP_MutexLock(m.m_mutex);
		if(ret == 0)
			m._setThrdMsg(m._t("wbfsop8", L"Game installed"), 1.f);
		else if( ret >= 0x30200)
			m._setThrdMsg(wfmt(m._fmt("wbfsop12", L"DVDError(%d)"), ret), 1.f);
		else if( ret > 0)
			m._setThrdMsg(wfmt(m._fmt("wbfsop13", L"Game installed, but disc contains errors (%d)"), ret), 1.f);
		else
			m._setThrdMsg(m._t("wbfsop9", L"An error has occurred"), 1.f);
		LWP_MutexUnlock(m.m_mutex);
		slotLight(true);
	}
	m.m_thrdWorking = false;
	return ret;
}

int CMenu::_GCcopyGame(void *obj)
{
	CMenu &m = *(CMenu *)obj;
	char folder[50];
	char source[300];
	char target[300];
	snprintf(folder, sizeof(folder), m.m_DMLgameDir.c_str(), DeviceName[currentPartition]);
	snprintf(source, sizeof(source), "%s/%s", folder, m.m_cf.getHdr()->path);
	memset(folder, 0, sizeof(folder));
	snprintf(folder, sizeof(folder), DML_DIR, DeviceName[SD]);
	snprintf(target, sizeof(target), "%s/%s", folder, m.m_cf.getHdr()->path);

	LWP_MutexLock(m.m_mutex);
	m._setThrdMsg(L"", 0);
	gprintf("Copying from:\n%s\nto:\n%s\n",source,target);
	LWP_MutexUnlock(m.m_mutex);
	if (!fsop_DirExist(folder))
		makedir(folder);
	fsop_CopyFolder(source, target, CMenu::_addDiscProgress, obj);
	LWP_MutexLock(m.m_mutex);
	m._setThrdMsg(m._t("wbfsop14", L"Game copied, press Back to boot the game."), 1.f);
	gprintf("Game copied.\n");
	LWP_MutexUnlock(m.m_mutex);
	slotLight(true);

	m.m_thrdWorking = false;
	return 0;
}

bool CMenu::_wbfsOp(CMenu::WBFS_OP op)
{
	lwp_t thread = 0;
	static discHdr header ATTRIBUTE_ALIGN(32);
	static gc_discHdr gcheader ATTRIBUTE_ALIGN(32);
	bool done = false;
	bool upd_usb = false;
	bool upd_dml = false;
	bool out = false;
	struct AutoLight { AutoLight(void) { } ~AutoLight(void) { slotLight(false); } } aw;
	string cfPos = m_cf.getNextId();

	SetupInput();

	_showWBFS(op);
	switch (op)
	{
		case CMenu::WO_ADD_GAME:
			m_btnMgr.setText(m_wbfsLblDialog, _t("wbfsadddlg", L"Please insert the disc you want to copy, then click on Go."));
			break;
		case CMenu::WO_REMOVE_GAME:
			m_btnMgr.setText(m_wbfsLblDialog, wfmt(_fmt("wbfsremdlg", L"To permanently remove the game: %s, click on Go."), (u8*)m_cf.getTitle().toUTF8().c_str()));
			break;
		case CMenu::WO_FORMAT:
			break;
		case CMenu::WO_COPY_GAME:
			m_btnMgr.setText(m_wbfsLblDialog, _t("wbfscpydlg", L"If you are sure you want to copy this game to SD, click on Go."));
			break;
	}
	m_thrdStop = false;
	m_thrdMessageAdded = false;
	while (true)
	{
		_mainLoopCommon(false, m_thrdWorking);
		if ((BTN_HOME_PRESSED || BTN_B_PRESSED) && !m_thrdWorking)
		{
			break;
		}
		else if (BTN_UP_PRESSED)
			m_btnMgr.up();
		else if (BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if (BTN_A_PRESSED && !m_thrdWorking)
		{
			if (m_btnMgr.selected(m_wbfsBtnBack))
			{
				break;
			}
			else if (m_btnMgr.selected(m_wbfsBtnGo))
			{
				switch (op)
				{
					case CMenu::WO_ADD_GAME:
						m_btnMgr.show(m_wbfsPBar);
						m_btnMgr.setProgress(m_wbfsPBar, 0.f);
						m_btnMgr.hide(m_wbfsBtnGo);
						m_btnMgr.hide(m_wbfsBtnBack);
						m_btnMgr.show(m_wbfsLblMessage);
						m_btnMgr.setText(m_wbfsLblMessage, L"");
						Disc_SetUSB(NULL);
						if (Disc_Wait() < 0)
						{
							error(_t("wbfsoperr1", L"Disc_Wait failed"));
							out = true;
							break;
						}
						if (Disc_Open() < 0)
						{
							error(_t("wbfsoperr2", L"Disc_Open failed"));
							out = true;
							break;
						}
						if (Disc_IsWii() == 0)
						{
							Disc_ReadHeader(&header);
						
							if (_searchGamesByID((const char *) header.id).size() != 0)
							{
								error(_t("wbfsoperr4", L"Game already installed"));
								out = true;
								break;
							}
							cfPos = string((char *) header.id);
							m_btnMgr.setText(m_wbfsLblDialog, wfmt(_fmt("wbfsop6", L"Installing [%s] %s..."), string((const char *)header.id, sizeof header.id).c_str(), string((const char *)header.title, sizeof header.title).c_str()));
							done = true;
							upd_usb = true;
							m_thrdWorking = true;
							m_thrdProgress = 0.f;
							m_thrdMessageAdded = false;
							LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_gameInstaller, (void *)this, 0, 8 * 1024, 64);
						}
						else if(Disc_IsGC() == 0)
						{
							Disc_ReadGCHeader(&gcheader);
							
							char gcfolder[300];
							char dmlgamedir[50];
							snprintf(dmlgamedir, sizeof(dmlgamedir), "%s", (currentPartition != SD) ? m_DMLgameDir.c_str() : DML_DIR);
							snprintf(gcfolder, sizeof(gcfolder), "%s [%s]", gcheader.title, (char *)gcheader.id);
							if (_searchGamesByID((const char *) gcheader.id).size() != 0 || GC_GameIsInstalled((char *)gcheader.id, DeviceName[currentPartition], dmlgamedir) || GC_GameIsInstalled(gcfolder, DeviceName[currentPartition], dmlgamedir))
							{
								error(_t("wbfsoperr4", L"Game already installed"));
								out = true;
								break;
							}
							cfPos = string((char *) gcheader.id);
							m_btnMgr.setText(m_wbfsLblDialog, wfmt(_fmt("wbfsop6", L"Installing [%s] %s..."), string((const char *)gcheader.id, sizeof gcheader.id).c_str(), string((const char *)gcheader.title, sizeof gcheader.title).c_str()));
							done = true;
							upd_dml = true;
							m_thrdWorking = true;
							m_thrdProgress = 0.f;
							m_thrdMessageAdded = false;
							LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_GCgameInstaller, (void *)this, 0, 8 * 1024, 64);
						}
						else
						{
							error(_t("wbfsoperr3", L"This is not a Wii or GC disc!"));
							out = true;
							break;
						}
						break;
					case CMenu::WO_REMOVE_GAME:
						if(m_current_view == COVERFLOW_USB)
						{
							WBFS_RemoveGame((u8 *)m_cf.getId().c_str(), (char *) m_cf.getHdr()->path);
							upd_usb = true;
						}
						else
						{
							char source[300];
							snprintf(source, sizeof(source), "%s/%s", sfmt((currentPartition != SD) ? m_DMLgameDir.c_str() : DML_DIR, DeviceName[currentPartition]).c_str(), (char *)m_cf.getHdr()->path);
							fsop_deleteFolder(source);
							upd_dml = true;
						}
						if(m_cfg.getBool("GENERAL", "delete_cover_and_game", true))
							RemoveCover((char *)m_cf.getId().c_str());
						m_btnMgr.show(m_wbfsPBar);
						m_btnMgr.setProgress(m_wbfsPBar, 0.f, true);
						m_btnMgr.setProgress(m_wbfsPBar, 1.f);
						m_btnMgr.hide(m_wbfsLblDialog);
						m_btnMgr.hide(m_wbfsBtnGo);
						m_btnMgr.show(m_wbfsLblMessage);
						m_btnMgr.setText(m_wbfsLblMessage, _t("wbfsop7", L"Game deleted"));
						done = true;
						break;
					case CMenu::WO_FORMAT:
						break;
					case CMenu::WO_COPY_GAME:
						char folder[50];
						char source[300];
						snprintf(folder, sizeof(folder), m_DMLgameDir.c_str(), DeviceName[currentPartition]);
						snprintf(source, sizeof(source), "%s/%s", folder, m_cf.getHdr()->path);
						if(fsop_GetFreeSpaceKb((char*)"sd:/")<fsop_GetFolderKb(source))
						{
							m_btnMgr.hide(m_wbfsBtnGo);
							_setThrdMsg(wfmt(_fmt("wbfsop10", L"Not enough space: %d blocks needed, %d available"), fsop_GetFolderKb(source), fsop_GetFreeSpaceKb((char*)"sd:/")), 0.f);
							break;
						}

						m_btnMgr.show(m_wbfsPBar);
						m_btnMgr.setProgress(m_wbfsPBar, 0.f);
						m_btnMgr.hide(m_wbfsBtnGo);
						m_btnMgr.hide(m_wbfsBtnBack);
						m_btnMgr.show(m_wbfsLblMessage);
						m_btnMgr.setText(m_wbfsLblMessage, L"");
						cfPos = string((char*)m_cf.getHdr()->hdr.id);
						m_btnMgr.setText(m_wbfsLblDialog, wfmt(_fmt("wbfsop10", L"Copying [%s] %s..."), (u8*)m_cf.getHdr()->hdr.id, (u8*)m_cf.getTitle().toUTF8().c_str()));
						done = true;
						upd_dml = true;
						m_thrdWorking = true;
						m_thrdProgress = 0.f;
						m_thrdMessageAdded = false;
						m_cf.stopCoverLoader();
						_stopSounds();
						MusicPlayer::DestroyInstance();
						SoundHandler::DestroyInstance();
						soundDeinit();
						Nand::Instance()->Disable_Emu();
						Nand::DestroyInstance();
						LWP_CreateThread(&thread, (void *(*)(void *))CMenu::_GCcopyGame, (void *)this, 0, 8 * 1024, 64);
						break;
				}
				if (out)
					break;
			}
		}
		// 
		if (m_thrdMessageAdded)
		{
			LockMutex lock(m_mutex);
			m_thrdMessageAdded = false;
			if (!m_thrdMessage.empty())
				m_btnMgr.setText(m_wbfsLblDialog, m_thrdMessage);
			m_btnMgr.setProgress(m_wbfsPBar, m_thrdProgress);
			m_btnMgr.setText(m_wbfsLblMessage, wfmt(_fmt("wbfsprogress", L"%i%%"), (int)(m_thrdProgress * 100.f)));
			if (!m_thrdWorking)
				m_btnMgr.show(m_wbfsBtnBack);
		}
	}
	_hideWBFS();
	if (done && (op == CMenu::WO_REMOVE_GAME || op == CMenu::WO_ADD_GAME))
	{
		m_gameList.SetLanguage(m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str());
		if( upd_dml )
			UpdateCache(COVERFLOW_DML);
		
		if( upd_usb )
			UpdateCache(COVERFLOW_USB);

		_loadList();
		_initCF();
		m_cf.findId(cfPos.c_str(), true);
	}
	return done;
}

void CMenu::_initWBFSMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_wbfsLblUser, ARRAY_SIZE(m_wbfsLblUser), "WBFS");
	m_wbfsBg = _texture(theme.texSet, "WBFS/BG", "texture", theme.bg);
	m_wbfsLblTitle = _addTitle(theme, "WBFS/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_wbfsLblDialog = _addLabel(theme, "WBFS/DIALOG", theme.lblFont, L"", 40, 90, 560, 200, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_wbfsLblMessage = _addLabel(theme, "WBFS/MESSAGE", theme.lblFont, L"", 40, 300, 560, 100, theme.lblFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_TOP);
	m_wbfsPBar = _addProgressBar(theme, "WBFS/PROGRESS_BAR", 40, 270, 560, 20);
	m_wbfsBtnBack = _addButton(theme, "WBFS/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_wbfsBtnGo = _addButton(theme, "WBFS/GO_BTN", theme.btnFont, L"", 245, 260, 150, 56, theme.btnFontColor);

	_setHideAnim(m_wbfsLblTitle, "WBFS/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_wbfsLblDialog, "WBFS/DIALOG", 0, 0, -2.f, 0.f);
	_setHideAnim(m_wbfsLblMessage, "WBFS/MESSAGE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_wbfsPBar, "WBFS/PROGRESS_BAR", 0, 0, -2.f, 0.f);
	_setHideAnim(m_wbfsBtnBack, "WBFS/BACK_BTN", 0, 0, -2.f, 0.f);
	_setHideAnim(m_wbfsBtnGo, "WBFS/GO_BTN", 0, 0, -2.f, 0.f);
	_hideWBFS(true);
	_textWBFS();
}

void CMenu::_textWBFS(void)
{
	m_btnMgr.setText(m_wbfsBtnBack, _t("wbfsop4", L"Back"));
	m_btnMgr.setText(m_wbfsBtnGo, _t("wbfsop5", L"Go"));
}
