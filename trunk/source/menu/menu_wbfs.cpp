
#include "menu.hpp"
#include "types.h"
#include "lockMutex.hpp"
#include "channel/nand.hpp"
#include "gc/gc.hpp"
#include "loader/wbfs.h"
#include "loader/wdvd.h"
#include "loader/gc_disc_dump.hpp"
#include "music/SoundHandler.hpp"

void CMenu::_hideWBFS(bool instant)
{
	m_btnMgr.hide(m_configLblPartitionName, instant);
	m_btnMgr.hide(m_configLblPartition, instant);
	m_btnMgr.hide(m_configBtnPartitionP, instant);
	m_btnMgr.hide(m_configBtnPartitionM, instant);
	m_btnMgr.hide(m_wbfsLblTitle, instant);
	m_btnMgr.hide(m_wbfsPBar, instant);
	m_btnMgr.hide(m_wbfsBtnGo, instant);
	m_btnMgr.hide(m_wbfsLblDialog);
	m_btnMgr.hide(m_wbfsLblMessage);
	for(u8 i = 0; i < ARRAY_SIZE(m_wbfsLblUser); ++i)
		if(m_wbfsLblUser[i] != -1)
			m_btnMgr.hide(m_wbfsLblUser[i], instant);
}

void CMenu::_showWBFS(CMenu::WBFS_OP op)
{
	_setBg(m_wbfsBg, m_wbfsBg);
	switch (op)
	{
		case WO_ADD_GAME:
				m_btnMgr.setText(m_wbfsLblTitle, _t("wbfsop1", L"Install Game"));
				m_btnMgr.setText(m_configLblPartition, upperCase(DeviceName[currentPartition]));
				m_btnMgr.show(m_configLblPartitionName);
				m_btnMgr.show(m_configLblPartition);
				m_btnMgr.show(m_configBtnPartitionP);
				m_btnMgr.show(m_configBtnPartitionM);
				break;
		case WO_REMOVE_GAME:
				m_btnMgr.setText(m_wbfsLblTitle, _t("wbfsop2", L"Delete Game"));
				break;
		case WO_FORMAT:
//				m_btnMgr.setText(m_wbfsLblTitle, _t("wbfsop3", L"Format"));
				break;
		case WO_COPY_GAME:
				m_btnMgr.setText(m_wbfsLblTitle, _t("wbfsop11", L"Copy Game"));
				break;
	};
	m_btnMgr.show(m_wbfsLblTitle);
	m_btnMgr.show(m_wbfsBtnGo);
	m_btnMgr.show(m_wbfsLblDialog);
	for(u8 i = 0; i < ARRAY_SIZE(m_wbfsLblUser); ++i)
		if(m_wbfsLblUser[i] != -1)
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

static inline bool _searchGamesByID(const char *gameId)
{
	for(vector<dir_discHdr>::const_iterator itr = m_gameList.begin(); itr != m_gameList.end(); ++itr)
	{
		if(strncmp(itr->id, gameId, 6) == 0)
			return true;
	}
	return false;
}

void CMenu::GC_Messenger(int message, int info, char *cinfo)
{
	if(message == 1)
		m_thrdMessage = wfmt(_fmt("wbfsop23", L"Calculating space needed for %s...\n Please insert disc %d to continue"), cinfo, info);
	else if(message == 2)
		m_thrdMessage = wfmt(_fmt("wbfsop15", L"Calculating space needed for %s"), cinfo);
	else if(message == 3)
		m_thrdMessage = wfmt(_fmt("wbfsop16", L"Installing %s"), cinfo);
	else if(message == 4)
		m_thrdMessage = wfmt(_fmt("wbfsop17", L"Installing %s disc %d/2"), cinfo, info);
	else if(message == 5)
		m_thrdMessage = _t("wbfsop18", L"Don't try to trick me with a Wii disc!!");
	else if(message == 6)
		m_thrdMessage = _t("wbfsop19", L"This is not a GC disc!!");
	else if(message == 7)
		m_thrdMessage = wfmt(_fmt("wbfsop20", L"You inserted disc %d again!!"), info);
	else if(message == 8)
		m_thrdMessage = _t("wbfsop21", L"This is a disc of another game!!");
	else if(message == 9)
		m_thrdMessage = wfmt(_fmt("wbfsop22", L"Installing %s...\n Please insert disc 2 to continue"), cinfo);
	else if(message == 10)
		m_thrdMessage = _t("wbfsop25", L"Disc read error!! Please clean the disc");
	else if(message == 11)
		m_thrdMessage = _t("wbfsop26", L"Disc ejected!! Please insert disc again");
	m_thrdMessageAdded = true;
}

int CMenu::_gameInstaller(void *obj)
{
	CMenu &m = *(CMenu *)obj;
	int ret;

	DeviceHandle.OpenWBFS(currentPartition);
	if(!WBFS_Mounted())
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
		
		ret = WBFS_AddGame(_addDiscProgress, obj);
		LWP_MutexLock(m.m_mutex);
		if (ret == 0)
			m._setThrdMsg(m._t("wbfsop8", L"Game installed, press B to exit."), 1.f);
		else
			m._setThrdMsg(m._t("wbfsop9", L"An error has occurred"), 1.f);
		LWP_MutexUnlock(m.m_mutex);
		slotLight(false);
	}
	WBFS_Close();
	m.m_thrdWorking = false;
	return ret;
}

int CMenu::_GCgameInstaller()
{
	GCDump m_gcdump;

	bool skip = m_cfg.getBool(GC_DOMAIN, "skip_on_error", false);
	bool comp = m_cfg.getBool(GC_DOMAIN, "compressed_dump", false);
	bool wexf = m_cfg.getBool(GC_DOMAIN, "write_ex_files", true);
	bool alig = m_cfg.getBool(GC_DOMAIN, "force_32k_align_files", false);
	u32 nretry = m_cfg.getUInt(GC_DOMAIN, "num_retries", 5);
	u32 rsize = 1048576; //1MB

	if(skip)
		rsize = 8192; // Use small chunks when skip on error is enabled

	m_gcdump.Init(skip, comp, wexf, alig, nretry, rsize, DeviceName[currentPartition], m_DMLgameDir.c_str());
	
	int ret;
	m_progress = 0.f;

	if(!DeviceHandle.IsInserted(currentPartition))
	{
		m_thrdWorking = false;
		return -1;
	}

	char partition[6];
	strncpy(partition, fmt("%s:/", DeviceName[currentPartition]), sizeof(partition));

	u32 needed = 0;

	ret = m_gcdump.CheckSpace(&needed, comp);
	if(ret != 0)
	{
		_setThrdMsg(_t("wbfsop9", L"An error has occurred"), 1.f);
		m_thrdWorking = false;
		return ret;
	}

	if(m_gcdump.GetFreeSpace(partition, BL) <= needed)
	{
		gprintf("Free space available: %d Mb (%d blocks)\n", m_gcdump.GetFreeSpace(partition, MB), m_gcdump.GetFreeSpace(partition, BL));
		_setThrdMsg(wfmt(_fmt("wbfsop24", L"Not enough space: %d blocks needed, %d available"), needed, m_gcdump.GetFreeSpace(partition, BL)), 0.f);
		ret = -1;
	}
	else
	{
		gprintf("Free space available: %d Mb (%d blocks)\n", m_gcdump.GetFreeSpace(partition, MB), m_gcdump.GetFreeSpace(partition, BL));
		_setThrdMsg(L"", 0);

		ret = m_gcdump.DumpGame();
		if(ret == 0)
			_setThrdMsg(_t("wbfsop8", L"Game installed"), 1.f);
		else if( ret >= 0x30200)
			_setThrdMsg(wfmt(_fmt("wbfsop12", L"DVDError(%d)"), ret), 1.f);
		else if( ret > 0)
			_setThrdMsg(wfmt(_fmt("wbfsop13", L"Game installed, but disc contains errors (%d)"), ret), 1.f);
		else
			_setThrdMsg(_t("wbfsop9", L"An error has occurred"), 1.f);
		slotLight(true);
	}
	m_thrdWorking = false;
	return ret;
}

int CMenu::_GCcopyGame(void *obj)
{
	CMenu &m = *(CMenu *)obj;

	string GC_Path(CoverFlow.getHdr()->path);
	if(strcasestr(GC_Path.c_str(), "boot.bin") != NULL)
		GC_Path.erase(GC_Path.end() - 13, GC_Path.end());
	else
		GC_Path.erase(GC_Path.end() - 9, GC_Path.end());

	char source[300];
	strncpy(source, GC_Path.c_str(), sizeof(source));
	source[299] = '\0';

	char folder[50];
	strncpy(folder, fmt(DML_DIR, DeviceName[SD]), sizeof(folder));
	folder[49] = '\0';

	char target[300];
	strncpy(target, fmt("%s/%s", folder, strrchr(source, '/') + 1), sizeof(target));
	target[299] = '\0';

	LWP_MutexLock(m.m_mutex);
	m._setThrdMsg(L"", 0);
	gprintf("Copying from:\n%s\nto:\n%s\n", source, target);
	LWP_MutexUnlock(m.m_mutex);
	fsop_MakeFolder(folder);
	fsop_CopyFolder(source, target, _addDiscProgress, obj);
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
	char GameID[7];
	GameID[6] = '\0';

	bool done = false;
	bool upd_usb = false;
	bool upd_dml = false;
	bool upd_emu = false;
	bool upd_chan = false;
	bool out = false;
	const dir_discHdr *CF_Hdr = CoverFlow.getHdr();
	char cfPos[7];
	cfPos[6] = '\0';
	strncpy(cfPos, CoverFlow.getNextId(), 6);

	SetupInput();
	_showWBFS(op);
	switch (op)
	{
		case WO_ADD_GAME:
			m_btnMgr.setText(m_wbfsLblDialog, _t("wbfsadddlg", L"Please insert the disc you want to copy, then click on Go."));
			break;
		case WO_REMOVE_GAME:
			m_btnMgr.setText(m_wbfsLblDialog, wfmt(_fmt("wbfsremdlg", L"To permanently remove the game: %s, click on Go."), CoverFlow.getTitle().toUTF8().c_str()));
			break;
		case WO_FORMAT:
			break;
		case WO_COPY_GAME:
			m_btnMgr.setText(m_wbfsLblDialog, _t("wbfscpydlg", L"If you are sure you want to copy this game to SD, click on Go."));
			break;
	}
	m_thrdStop = false;
	m_thrdMessageAdded = false;
	while(!m_exit)
	{
		_mainLoopCommon();
		if((BTN_HOME_PRESSED || BTN_B_PRESSED) && !m_thrdWorking)
			break;
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if(BTN_A_PRESSED && !m_thrdWorking)
		{
			if(m_btnMgr.selected(m_wbfsBtnGo))
			{
				switch(op)
				{
					case WO_ADD_GAME:
						MusicPlayer.Stop();
						TempLoadIOS();
						m_btnMgr.hide(m_configLblPartitionName, true);
						m_btnMgr.hide(m_configLblPartition, true);
						m_btnMgr.hide(m_configBtnPartitionP, true);
						m_btnMgr.hide(m_configBtnPartitionM, true);
						m_btnMgr.show(m_wbfsPBar, true);
						m_btnMgr.setProgress(m_wbfsPBar, 0.f, true);
						m_btnMgr.hide(m_wbfsBtnGo, true);
						m_btnMgr.show(m_wbfsLblMessage, true);
						m_btnMgr.setText(m_wbfsLblMessage, L"");
						if (Disc_Wait() < 0)
						{
							error(_t("wbfsoperr1", L"Disc_Wait failed"));
							out = true;
							break;
						}
						if (Disc_Open(false) < 0)
						{
							error(_t("wbfsoperr2", L"Disc_Open failed"));
							out = true;
							break;
						}
						if (Disc_IsWii() == 0)
						{
							Disc_ReadHeader(&wii_hdr);
							memcpy(GameID, wii_hdr.id, 6);
							if(_searchGamesByID(GameID))
							{
								error(_t("wbfsoperr4", L"Game already installed"));
								out = true;
								break;
							}
							CoverFlow.clear();
							strncpy(cfPos, GameID, 6);
							m_btnMgr.setText(m_wbfsLblDialog, wfmt(_fmt("wbfsop6", L"Installing [%s] %s..."), GameID, wii_hdr.title));
							done = true;
							upd_usb = true;
							m_thrdWorking = true;
							m_thrdProgress = 0.f;
							m_thrdMessageAdded = false;
							LWP_CreateThread(&thread, (void *(*)(void *))_gameInstaller, (void *)this, 0, 8 * 1024, 64);
						}
						else if(Disc_IsGC() == 0)
						{
							Disc_ReadGCHeader(&gc_hdr);
							memcpy(GameID, gc_hdr.id, 6);
							if(_searchGamesByID(GameID))
							{
								error(_t("wbfsoperr4", L"Game already installed"));
								out = true;
								break;
							}
							strncpy(cfPos, GameID, 6);
							done = true;
							upd_dml = true;
							m_thrdWorking = true;
							m_thrdProgress = 0.f;
							//LWP_CreateThread(&thread, (void *(*)(void *))_GCgameInstaller, (void *)this, 0, 8 * 1024, 64);
							_start_pThread();
							m_thrdMessage = wfmt(_fmt("wbfsop6", L"Installing [%s] %s..."), GameID, gc_hdr.title);
							m_thrdMessageAdded = true;
							_GCgameInstaller();
							_stop_pThread();
						}
						else
						{
							error(_t("wbfsoperr3", L"This is not a Wii or GC disc!"));
							out = true;
						}
						break;
					case WO_REMOVE_GAME:
						if(CF_Hdr->type == TYPE_GC_GAME)
						{
							char GC_Path[1024];
							GC_Path[1023] = '\0';
							if(strcasestr(CF_Hdr->path, "boot.bin") != NULL)
							{
								strncpy(GC_Path, CF_Hdr->path, 1023);
								*strrchr(GC_Path, '/') = '\0'; //boot.bin
								*strrchr(GC_Path, '/') = '\0'; //sys
								fsop_deleteFolder(GC_Path);
							}
							else
							{
								strncpy(GC_Path, CF_Hdr->path, 1023);
								*strrchr(GC_Path, '/') = '\0'; //iso path
								const char *cmp = fmt(currentPartition == SD ? DML_DIR : m_DMLgameDir.c_str(), DeviceName[currentPartition]);
								if(strcasecmp(GC_Path, cmp) == 0)
									fsop_deleteFile(CF_Hdr->path);
								else
									fsop_deleteFolder(GC_Path);
							}
							upd_dml = true;
						}
						else if(CF_Hdr->type == TYPE_PLUGIN)
						{
							fsop_deleteFile(CF_Hdr->path);
							upd_emu = true;
						}
						else if(CF_Hdr->type == TYPE_WII_GAME)
						{
							DeviceHandle.OpenWBFS(currentPartition);
							WBFS_RemoveGame((u8*)&CF_Hdr->id, (char*)&CF_Hdr->path);
							WBFS_Close();
							upd_usb = true;
						}
						else if(CF_Hdr->type == TYPE_CHANNEL && !m_cfg.getBool(CHANNEL_DOMAIN, "disable", true))
						{
							if(CF_Hdr->settings[0] != 0x00010001)
							{
								error(_t("wbfsoperr5", L"Deleting this Channel is not allowed!"));
								done = true;
								out = true;
								break;
							}
							const char *nand_base = NandHandle.GetPath();
							fsop_deleteFolder(fmt("%s/title/%08x/%08x", nand_base, CF_Hdr->settings[0], CF_Hdr->settings[1]));
							fsop_deleteFile(fmt("%s/ticket/%08x/%08x.tik", nand_base, CF_Hdr->settings[0], CF_Hdr->settings[1]));
							upd_chan = true;
						}
						else /*who knows how but just block it*/
						{
							done = true;
							out = true;
							break;
						}
						if(m_cfg.getBool("GENERAL", "delete_cover_and_game", false))
							RemoveCover(CF_Hdr->id);
						m_btnMgr.show(m_wbfsPBar);
						m_btnMgr.setProgress(m_wbfsPBar, 0.f, true);
						m_btnMgr.setProgress(m_wbfsPBar, 1.f);
						m_btnMgr.hide(m_wbfsLblDialog);
						m_btnMgr.hide(m_wbfsBtnGo);
						m_btnMgr.show(m_wbfsLblMessage);
						m_btnMgr.setText(m_wbfsLblMessage, _t("wbfsop7", L"Game deleted"));
						done = true;
						break;
					case WO_FORMAT:
						break;
					case WO_COPY_GAME:
						string GC_Path(CF_Hdr->path);
						if(strcasestr(GC_Path.c_str(), "boot.bin") != NULL)
							GC_Path.erase(GC_Path.end() - 13, GC_Path.end());
						else
							GC_Path.erase(GC_Path.end() - 9, GC_Path.end());
						if(fsop_GetFreeSpaceKb("sd:/") < fsop_GetFolderKb(GC_Path.c_str()))
						{
							m_btnMgr.hide(m_wbfsBtnGo);
							_setThrdMsg(wfmt(_fmt("wbfsop24", L"Not enough space: %d blocks needed, %d available"), fsop_GetFolderKb(GC_Path.c_str()), fsop_GetFreeSpaceKb("sd:/")), 0.f);
							break;
						}
						m_btnMgr.show(m_wbfsPBar, true);
						m_btnMgr.setProgress(m_wbfsPBar, 0.f, true);
						m_btnMgr.hide(m_wbfsBtnGo, true);
						m_btnMgr.show(m_wbfsLblMessage, true);
						m_btnMgr.setText(m_wbfsLblMessage, L"");
						strncpy(cfPos, CF_Hdr->id, 6);
						m_btnMgr.setText(m_wbfsLblDialog, wfmt(_fmt("wbfsop10", L"Copying [%s] %s..."), CF_Hdr->id, CoverFlow.getTitle().toUTF8().c_str()));
						done = true;
						upd_dml = true;
						m_thrdWorking = true;
						m_thrdProgress = 0.f;
						m_thrdMessageAdded = false;
						LWP_CreateThread(&thread, (void *(*)(void *))_GCcopyGame, (void *)this, 0, 8 * 1024, 64);
						break;
				}
				if(out)
				{
					TempLoadIOS(IOS_TYPE_NORMAL_IOS);
					break;
				}
			}
			else if((m_btnMgr.selected(m_configBtnPartitionP) || m_btnMgr.selected(m_configBtnPartitionM)))
			{
				s8 direction = m_btnMgr.selected(m_configBtnPartitionP) ? 1 : -1;
				_setPartition(direction);
				m_btnMgr.setText(m_configLblPartition, upperCase(DeviceName[currentPartition]));
			}
		}
		if(m_thrdMessageAdded)
		{
			LockMutex lock(m_mutex);
			m_thrdMessageAdded = false;
			if (!m_thrdMessage.empty())
				m_btnMgr.setText(m_wbfsLblDialog, m_thrdMessage);
			m_btnMgr.setProgress(m_wbfsPBar, m_thrdProgress);
			m_btnMgr.setText(m_wbfsLblMessage, wfmt(L"%i%%", (int)(m_thrdProgress * 100.f)));
			if(!m_thrdWorking && op == WO_ADD_GAME)
			{
				WDVD_StopMotor();
				MusicPlayer.Stop();
				TempLoadIOS(IOS_TYPE_NORMAL_IOS);
			}
		}
	}
	_hideWBFS();
	if(done && (op == WO_REMOVE_GAME || op == WO_ADD_GAME))
	{
		//m_gameList.SetLanguage(m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str());	
		_showWaitMessage();
		if(upd_dml)
			UpdateCache(COVERFLOW_GAMECUBE);
		if(upd_usb)
			UpdateCache(COVERFLOW_WII);
		if(upd_emu)
			UpdateCache(COVERFLOW_PLUGIN);
		if(upd_chan)
			UpdateCache(COVERFLOW_CHANNEL);
		_loadList();
		_hideWaitMessage();
		_initCF();
		CoverFlow.findId(cfPos, true);
		Close_Inputs();
		Open_Inputs();
		for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
					WPAD_SetVRes(chan, m_vid.width() + m_cursor[chan].width(), m_vid.height() + m_cursor[chan].height());
	}
	else 
	{
		if(done && op == WO_COPY_GAME)
		{
			UpdateCache(COVERFLOW_GAMECUBE);
			currentPartition = SD;
			UpdateCache(COVERFLOW_GAMECUBE);
		}
		_loadList();
		_initCF();
	}
	return done;
}

void CMenu::_initWBFSMenu()
{
	_addUserLabels(m_wbfsLblUser, ARRAY_SIZE(m_wbfsLblUser), "WBFS");
	m_wbfsBg = _texture("WBFS/BG", "texture", theme.bg, false);
	m_wbfsLblTitle = _addTitle("WBFS/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_wbfsLblDialog = _addLabel("WBFS/DIALOG", theme.lblFont, L"", 20, 75, 600, 200, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_wbfsLblMessage = _addLabel("WBFS/MESSAGE", theme.lblFont, L"", 20, 300, 600, 100, theme.lblFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_TOP);
	m_wbfsPBar = _addProgressBar("WBFS/PROGRESS_BAR", 40, 200, 560, 20);
	m_wbfsBtnGo = _addButton("WBFS/GO_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);

	_setHideAnim(m_wbfsLblTitle, "WBFS/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_wbfsLblDialog, "WBFS/DIALOG", 0, 0, -2.f, 0.f);
	_setHideAnim(m_wbfsLblMessage, "WBFS/MESSAGE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_wbfsPBar, "WBFS/PROGRESS_BAR", 0, 0, -2.f, 0.f);
	_setHideAnim(m_wbfsBtnGo, "WBFS/GO_BTN", 0, 0, 1.f, -1.f);
	_hideWBFS(true);
	_textWBFS();
}

void CMenu::_textWBFS(void)
{
	m_btnMgr.setText(m_wbfsBtnGo, _t("wbfsop5", L"Go"));
}
