/****************************************************************************
 * Copyright (C) 2013 FIX94
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "menu/menu.hpp"
#include "libwbfs/wiidisc.h"
#include "channel/nand.hpp"

extern "C" {
#include "loader/sha1.h"
extern void aes_set_key(u8 *key);
extern void aes_decrypt_partial(u8 *inbuf, u8 *outbuf, u8 block[16], 
								u8 *ctext_ptr, u32 tmp_blockno);
};

#define WAD_BUF 0x10000

struct _hdr {
	u32 header_len;
	u16 type;
	u16 padding;
	u32 certs_len;
	u32 crl_len;
	u32 tik_len;
	u32 tmd_len;
	u32 data_len;
	u32 footer_len;
} ATTRIBUTE_PACKED hdr;

TexData m_wadBg;
bool mios = false;
char ISFS_Path[ISFS_MAXPATH];

void skip_align(FILE *f, u32 size)
{
	size_t align_missing = (ALIGN(64, size) - size);
	if(align_missing == 0)
		return;
	fseek(f, align_missing, SEEK_CUR);
}

int isfs_WriteFile(const char *app_name, const void *content, u32 size)
{
	s32 fd = -1;
	memset(&ISFS_Path, 0, ISFS_MAXPATH);
	strcpy(ISFS_Path, app_name);
	ISFS_Delete(ISFS_Path);
	if(ISFS_CreateFile(ISFS_Path, 0, ISFS_OPEN_RW, ISFS_OPEN_RW, ISFS_OPEN_RW) == 0)
		fd = ISFS_Open(ISFS_Path, ISFS_OPEN_RW);
	if(fd < 0)
		return fd;
	s32 ret = ISFS_Write(fd, content, size);
	ISFS_Close(fd);
	gprintf("Writing %s returned %i\n", ISFS_Path, ret);
	return ret;
}

int installWad(const char *path)
{
	gprintf("Installing %s\n", path);
	u32 size = 0;
	fsop_GetFileSizeBytes(path, &size);

	FILE *wad_file = fopen(path, "rb");
	fread(&hdr, sizeof(hdr), 1, wad_file);
	skip_align(wad_file, sizeof(hdr));

	mainMenu.m_thrdTotal = (hdr.certs_len + hdr.crl_len + hdr.tik_len + hdr.tmd_len + hdr.data_len + hdr.footer_len);
	if(size < mainMenu.m_thrdTotal || hdr.tik_len == 0 || hdr.tmd_len == 0 || hdr.data_len == 0)
	{
		fclose(wad_file);
		return -3;
	}
	fseek(wad_file, ALIGN(64, hdr.certs_len), SEEK_CUR);
	fseek(wad_file, ALIGN(64, hdr.crl_len), SEEK_CUR);

	gprintf("Reading tik\n");
	u8 *tik_buf = (u8*)MEM2_alloc(hdr.tik_len);
	fread(tik_buf, hdr.tik_len, 1, wad_file);
	skip_align(wad_file, hdr.tik_len);

	gprintf("Decrypting key\n");
	u8 tik_key[16];
	decrypt_title_key(tik_buf, tik_key);
	aes_set_key(tik_key);

	gprintf("Reading tmd\n");
	signed_blob *tmd_buf = (signed_blob*)MEM2_alloc(hdr.tmd_len);
	fread(tmd_buf, hdr.tmd_len, 1, wad_file);
	skip_align(wad_file, hdr.tmd_len);

	const tmd *tmd_ptr = (const tmd*)SIGNATURE_PAYLOAD(tmd_buf);
	u64 tid = tmd_ptr->title_id;
	const char *EmuNAND = NULL;
	if(mios == false)
	{
		EmuNAND = NandHandle.GetPath();
		u32 uid_size = 0;
		u8 *uid_buf = fsop_ReadFile(fmt("%s/sys/uid.sys", EmuNAND), &uid_size);
		if(uid_buf == NULL)
		{
			gprintf("No uid.sys found!\n");
			free(tmd_buf);
			free(tik_buf);
			return -5;
		}
		else if(uid_size % 0xC != 0)
		{
			gprintf("uid.sys size is invalid!\n");
			free(tmd_buf);
			free(tik_buf);
			free(uid_buf);
			return -6;
		}

		bool chan_exist = false;
		uid *uid_file = (uid*)uid_buf;
		u32 chans = uid_size/sizeof(uid);
		for(u32 i = 0; i < chans; ++i)
		{
			if(uid_file[i].TitleID == tid)
				chan_exist = true;
		}
		if(chan_exist == false)
		{
			gprintf("Updating uid.sys\n");
			u32 new_uid_size = (chans+1)*sizeof(uid);
			u8 *new_uid_buf = (u8*)MEM2_alloc(new_uid_size);
			memset(new_uid_buf, 0, new_uid_size);
			/* copy over old uid */
			memcpy(new_uid_buf, uid_buf, chans*sizeof(uid));
			uid *new_uid_file = (uid*)new_uid_buf;
			new_uid_file[chans].TitleID = tid;
			new_uid_file[chans].uid = 0x1000+chans;
			fsop_WriteFile(fmt("%s/sys/uid.sys", EmuNAND), new_uid_file, new_uid_size);
		}
		/* clear old tik */
		fsop_deleteFile(fmt("%s/ticket/%08x/%08x.tik", EmuNAND, (u32)(tid>>32), (u32)tid&0xFFFFFFFF));
		/* clear old content */
		fsop_deleteFolder(fmt("%s/title/%08x/%08x/content", EmuNAND, (u32)(tid>>32), (u32)tid&0xFFFFFFFF));

		/* (re)create folder structure */
		fsop_MakeFolder(fmt("%s/ticket", EmuNAND));
		fsop_MakeFolder(fmt("%s/ticket/%08x", EmuNAND, (u32)(tid>>32)));

		fsop_MakeFolder(fmt("%s/title", EmuNAND));
		fsop_MakeFolder(fmt("%s/title/%08x", EmuNAND, (u32)(tid>>32)));
		fsop_MakeFolder(fmt("%s/title/%08x/%08x", EmuNAND, (u32)(tid>>32), (u32)tid&0xFFFFFFFF));
		fsop_MakeFolder(fmt("%s/title/%08x/%08x/content", EmuNAND, (u32)(tid>>32), (u32)tid&0xFFFFFFFF));
		fsop_MakeFolder(fmt("%s/title/%08x/%08x/data", EmuNAND, (u32)(tid>>32), (u32)tid&0xFFFFFFFF));
	}
	int hash_errors = 0;

	/* decrypt and write app files */
	for(u16 cnt = 0; cnt < tmd_ptr->num_contents; cnt++)
	{
		u8 iv[16];
		const tmd_content *content = &tmd_ptr->contents[cnt];
		u16 content_index = content->index;
		memset(iv, 0, 16);
		memcpy(iv, &content_index, 2);
		/* longass filename */
		FILE *app_file = NULL;
		s32 fd = -1;
		if(mios == false)
		{
			const char *app_name = fmt("%s/title/%08x/%08x/content/%08x.app", EmuNAND,
				(u32)(tid>>32), (u32)tid&0xFFFFFFFF, content->cid);
			app_file = fopen(app_name, "wb");
			gprintf("Writing Emu NAND File %s\n", app_name);
		}
		else
		{
			/* delete then create file */
			memset(&ISFS_Path, 0, ISFS_MAXPATH);
			const char *app_name = fmt("/title/%08x/%08x/content/%08x.app",
				(u32)(tid>>32), (u32)tid&0xFFFFFFFF, content->cid);
			strcpy(ISFS_Path, app_name);
			ISFS_Delete(ISFS_Path);
			if(ISFS_CreateFile(ISFS_Path, 0, ISFS_OPEN_RW, ISFS_OPEN_RW, ISFS_OPEN_RW) == 0)
				fd = ISFS_Open(ISFS_Path, ISFS_OPEN_RW);
			if(fd >= 0)
				gprintf("Writing Real NAND File %s\n", ISFS_Path);
		}
		u64 read = 0;
		u8 *encBuf = (u8*)MEM2_alloc(WAD_BUF);
		u8 *decBuf = (u8*)MEM2_alloc(WAD_BUF);

		u8 block[16];
		u8 prev_block[16];
		u8 *ctext_ptr = iv;

		SHA1_CTX ctx;
		SHA1Init(&ctx);

		u32 size_enc_full = ALIGN(16, content->size);
		while(read < size_enc_full)
		{
			u64 size_enc = (size_enc_full - read);
			if (size_enc > WAD_BUF)
				size_enc = WAD_BUF;

			memset(encBuf, 0, WAD_BUF);
			fread(encBuf, size_enc, 1, wad_file);

			u32 partnr = 0;
			u32 lastnr = (size_enc / sizeof(block));

			for(partnr = 0; partnr < lastnr; partnr++)
			{
				aes_decrypt_partial(encBuf, decBuf, block, ctext_ptr, partnr);
				memcpy(prev_block, encBuf + (partnr * 16), 16);
				ctext_ptr = prev_block;
			}
			/* we need to work with the real size here */
			u64 size_dec = (content->size - read);
			if(size_dec > WAD_BUF)
				size_dec = WAD_BUF;
			SHA1Update(&ctx, decBuf, size_dec);
			if(mios == false)
				fwrite(decBuf, size_dec, 1, app_file);
			else if(fd >= 0)
				ISFS_Write(fd, decBuf, size_dec);
			/* dont forget to increase the read size */
			read += size_enc;
			mainMenu.update_pThread(size_enc);
		}
		sha1 app_sha1;
		SHA1Final(app_sha1, &ctx);
		skip_align(wad_file, size_enc_full);

		if(mios == false)
			fclose(app_file);
		else if(fd >= 0)
			ISFS_Close(fd);

		if(memcmp(app_sha1, content->hash, sizeof(sha1)) == 0)
			gprintf("sha1 matches on %08x.app, success!\n", content->cid);
		else
		{
			gprintf("sha1 mismatch on %08x.app!\n", content->cid);
			hash_errors++;
		}
	}
	if(mios == false)
	{
		fsop_WriteFile(fmt("%s/ticket/%08x/%08x.tik", EmuNAND, (u32)(tid>>32), (u32)tid&0xFFFFFFFF), tik_buf, hdr.tik_len);
		fsop_WriteFile(fmt("%s/title/%08x/%08x/content/title.tmd", EmuNAND, (u32)(tid>>32), (u32)tid&0xFFFFFFFF), tmd_buf, hdr.tmd_len);
	}
	else
	{
		isfs_WriteFile(fmt("/ticket/%08x/%08x.tik", (u32)(tid>>32), (u32)tid&0xFFFFFFFF), tik_buf, hdr.tik_len);
		isfs_WriteFile(fmt("/title/%08x/%08x/content/title.tmd", (u32)(tid>>32), (u32)tid&0xFFFFFFFF), tmd_buf, hdr.tmd_len);
	}
	free(tik_buf);
	free(tmd_buf);

	return hash_errors;
}

s16 m_wadBtnInstall;
s16 m_wadLblTitle;
s16 m_wadLblDialog;
s16 m_wadLblUser[4];

void CMenu::_showWad()
{
	_setBg(m_wadBg, m_wadBg);
	m_btnMgr.show(m_wadBtnInstall);
	m_btnMgr.show(m_wadLblTitle);
	m_btnMgr.show(m_wadLblDialog);
	/* partition selection */
	if(mios == false)
	{
		m_btnMgr.show(m_configLblPartitionName);
		m_btnMgr.show(m_configLblPartition);
		m_btnMgr.show(m_configBtnPartitionP);
		m_btnMgr.show(m_configBtnPartitionM);
	}

	for(u8 i = 0; i < ARRAY_SIZE(m_wadLblUser); ++i)
		if(m_wadLblUser[i] != -1)
			m_btnMgr.show(m_wadLblUser[i]);
}

void CMenu::_hideWad(bool instant)
{
	m_btnMgr.hide(m_wadBtnInstall, instant);
	m_btnMgr.hide(m_wadLblTitle, instant);
	m_btnMgr.hide(m_wadLblDialog, instant);
	/* partition selection */
	if(mios == false)
	{
		m_btnMgr.hide(m_configLblPartitionName);
		m_btnMgr.hide(m_configLblPartition);
		m_btnMgr.hide(m_configBtnPartitionP);
		m_btnMgr.hide(m_configBtnPartitionM);
	}

	for(u8 i = 0; i < ARRAY_SIZE(m_wadLblUser); ++i)
		if(m_wadLblUser[i] != -1)
			m_btnMgr.hide(m_wadLblUser[i], instant);
}

int getTID(const char *path, u64 *tid)
{
	if(!fsop_FileExist(path))
		return -1;

	u32 size = 0;
	fsop_GetFileSizeBytes(path, &size);
	if(size < sizeof(hdr))
		return -2;

	FILE *wad_file = fopen(path, "rb");
	fread(&hdr, sizeof(hdr), 1, wad_file);

	/* skip to tmd */
	skip_align(wad_file, sizeof(hdr));
	fseek(wad_file, ALIGN(64, hdr.certs_len), SEEK_CUR);
	fseek(wad_file, ALIGN(64, hdr.crl_len), SEEK_CUR);
	fseek(wad_file, ALIGN(64, hdr.tik_len), SEEK_CUR);

	/* read tmd and close wad */
	signed_blob *tmd_buf = (signed_blob*)MEM2_alloc(hdr.tmd_len);
	fread(tmd_buf, hdr.tmd_len, 1, wad_file);
	fclose(wad_file);

	/* get its tid, return and free mem */
	const tmd *tmd_ptr = (const tmd*)SIGNATURE_PAYLOAD(tmd_buf);
	(*tid) = tmd_ptr->title_id;
	free(tmd_buf);

	return 0;
}

int CMenu::_pThread(void *obj)
{
	CMenu *m = (CMenu*)obj;
	while(m->m_thrdInstalling)
	{
		m->_mainLoopCommon();
		if(m->m_thrdUpdated)
		{
			m->m_thrdUpdated = false;
			m->_addDiscProgress(m->m_thrdWritten, m->m_thrdTotal, obj);
			m->m_thrdDone = true;
		}
		if(m->m_thrdMessageAdded)
		{
			m->m_thrdMessageAdded = false;
			if(!m->m_thrdMessage.empty())
				m_btnMgr.setText(m->m_wbfsLblDialog, m->m_thrdMessage);
			if(m->m_thrdProgress > 0.f)
			{
				m_btnMgr.setText(m->m_wbfsLblMessage, wfmt(L"%i%%", (int)(m->m_thrdProgress * 100.f)));
				m_btnMgr.setProgress(m->m_wbfsPBar, m->m_thrdProgress);
			}
		}
	}
	m->m_thrdWorking = false;
	return 0;
}

void CMenu::_start_pThread(void)
{
	m_thrdPtr = LWP_THREAD_NULL;
	m_thrdWorking = true;
	m_thrdMessageAdded = false;
	m_thrdInstalling = true;
	m_thrdUpdated = false;
	m_thrdDone = true;
	m_thrdProgress = 0.f;
	m_thrdWritten = 0;
	m_thrdTotal = 0;
	LWP_CreateThread(&m_thrdPtr, (void *(*)(void *))_pThread, (void*)this, 0, 8 * 1024, 64);
}

void CMenu::_stop_pThread(void)
{
	if(m_thrdPtr == LWP_THREAD_NULL)
		return;

	if(LWP_ThreadIsSuspended(m_thrdPtr))
		LWP_ResumeThread(m_thrdPtr);
	m_thrdInstalling = false;
	while(m_thrdWorking)
		usleep(50);
	LWP_JoinThread(m_thrdPtr, NULL);
	m_thrdPtr = LWP_THREAD_NULL;

	m_btnMgr.setProgress(m_wbfsPBar, 1.f);
	m_btnMgr.setText(m_wbfsLblMessage, L"100%");
}

void CMenu::update_pThread(u64 added)
{
	if(m_thrdDone)
	{
		m_thrdDone = false;
		m_thrdWritten += added;
		m_thrdUpdated = true;
	}
}

void CMenu::_Wad(const char *wad_path, bool autoInstall)
{
	if(wad_path == NULL)
		return;

	/* precheck */
	mios = false;
	u64 tid;
	if(getTID(wad_path, &tid) < 0)
		return;
	if((u32)(tid>>32) != 0x00010001)
	{
		if(tid == 0x0000000100000101ull)
		{
			gprintf("MIOS Detected\n");
			mios = true;
		}
		else
		{
			gprintf("No Wii Channel!\n");
			return;
		}
	}
	if(autoInstall)
	{
		if(mios == true)
			installWad(wad_path);
		MIOSisDML();
		return;
	}

	u8 part = currentPartition;
	m_btnMgr.setText(m_wadLblDialog, wfmt(_fmt("wad3", L"Selected %s, after the installation you return to the explorer."), (strrchr(wad_path, '/')+1)));
	m_btnMgr.setText(m_configLblPartition, upperCase(DeviceName[currentPartition]));
	_showWad();

	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_wadBtnInstall))
			{
				_hideWad(true);
				m_btnMgr.setProgress(m_wbfsPBar, 0.f);
				m_btnMgr.setText(m_wbfsLblMessage, L"");
				m_btnMgr.setText(m_wbfsLblDialog, L"");
				m_btnMgr.show(m_wbfsPBar);
				m_btnMgr.show(m_wbfsLblMessage);
				m_btnMgr.show(m_wbfsLblDialog);
				/* mios is real nand, chans are emu */
				if(mios == false)
				{
					const char *emu_char = m_cfg.getString(CHANNEL_DOMAIN, "path", "/").c_str();
					NandHandle.SetPaths(emu_char, DeviceName[currentPartition]);
				}
				_start_pThread();
				m_thrdMessage = _t("wad4", L"Installing WAD, please wait...");
				m_thrdMessageAdded = true;
				int result = installWad(wad_path);
				_stop_pThread();
				if(result < 0)
					m_btnMgr.setText(m_wbfsLblDialog, wfmt(_fmt("wad5", L"Installation error %i!"), result));
				else
					m_btnMgr.setText(m_wbfsLblDialog, wfmt(_fmt("wad6", L"Installation finished with %i hash fails."), result));
			}
			else if((m_btnMgr.selected(m_configBtnPartitionP) || m_btnMgr.selected(m_configBtnPartitionM)))
			{
				s8 direction = m_btnMgr.selected(m_configBtnPartitionP) ? 1 : -1;
				_setPartition(direction);
				m_btnMgr.setText(m_configLblPartition, upperCase(DeviceName[currentPartition]));
			}
		}
		
	}
	currentPartition = part;
	_hideWad();
	/* onscreen message might be onscreen still */
	m_btnMgr.hide(m_wbfsLblMessage);
	m_btnMgr.hide(m_wbfsLblDialog);
	m_btnMgr.hide(m_wbfsPBar);

	if(mios == true)
	{
		/* recheck after new mios install */
		MIOSisDML();
		if(m_show_dml == false)
			m_show_dml = (m_mios_ver > 0);
	}
}

void CMenu::_initWad()
{
	_addUserLabels(m_wadLblUser, ARRAY_SIZE(m_wadLblUser), "WAD");

	m_wadBg = _texture("WAD/BG", "texture", theme.bg, false);
	m_wadLblTitle = _addTitle("WAD/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_wadLblDialog = _addLabel("WAD/DIALOG", theme.lblFont, L"", 40, 90, 560, 200, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_wadBtnInstall = _addButton("WAD/INSTALL_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);

	_setHideAnim(m_wadLblTitle, "WAD/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_wadLblDialog, "WAD/DIALOG", 0, 0, -2.f, 0.f);
	_setHideAnim(m_wadBtnInstall, "WAD/INSTALL_BTN", 0, 0, -2.f, 0.f);

	_hideWad(true);
	_textWad();
}

void CMenu::_textWad()
{
	m_btnMgr.setText(m_wadLblTitle, _t("wad1", L"Install WAD"));
	m_btnMgr.setText(m_wadBtnInstall, _t("wad2", L"Go"));
}
