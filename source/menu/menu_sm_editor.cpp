#include "menu.hpp"
#include "gui/text.hpp"

#include <string.h>
#include <gccore.h>
#include <cmath>

u8 CB_curPage;
u8 CB_numPages;
u8 max_checkbox;
u8 mode;
u8 curSource;

// Checkboxes menu
TexData m_checkboxesBg;
s16 m_checkboxesLblUser[4];
s16 m_checkboxesLblPage;
s16 m_checkboxesBtnPageM;
s16 m_checkboxesBtnPageP;
s16 m_checkboxesBtnBack;
s16 m_checkboxesLblTitle;

s16 m_checkboxLblTxt[11];
s16 m_checkboxBtn[11];
s16 m_checkboxBtnOff[11];
s16 m_checkboxBtnOn[11];

void CMenu::_hideCheckboxesMenu(bool instant)
{
	m_btnMgr.hide(m_checkboxesLblTitle, instant);
	m_btnMgr.hide(m_checkboxesBtnBack, instant);
	m_btnMgr.hide(m_checkboxesLblPage, instant);
	m_btnMgr.hide(m_checkboxesBtnPageM, instant);
	m_btnMgr.hide(m_checkboxesBtnPageP, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_checkboxesLblUser); ++i)
		if(m_checkboxesLblUser[i] != -1)
			m_btnMgr.hide(m_checkboxesLblUser[i], instant);
			
	for(u8 i = 0; i < 11; ++i)
	{
		m_btnMgr.hide(m_checkboxLblTxt[i]);
		m_btnMgr.hide(m_checkboxBtn[i]);
	}
}

void CMenu::_showCheckboxesMenu(void)
{
	_setBg(m_checkboxesBg, m_checkboxesBg);
	for(u8 i = 0; i < ARRAY_SIZE(m_checkboxesLblUser); ++i)
		if(m_checkboxesLblUser[i] != -1)
			m_btnMgr.show(m_checkboxesLblUser[i]);

	if(mode == 1)
		m_btnMgr.setText(m_checkboxesLblTitle, _t("smedit1", L"Hide Sources"));
	else if(mode == 2)
		m_btnMgr.setText(m_checkboxesLblTitle, _t("smedit2", L"Choose Source"));
	else if(mode == 3)
		m_btnMgr.setText(m_checkboxesLblTitle, _t("smedit3", L"Choose Plugin"));
	else
		m_btnMgr.setText(m_checkboxesLblTitle, _t("smedit4", L"Choose Plugins"));

	m_btnMgr.show(m_checkboxesLblTitle);
	m_btnMgr.show(m_checkboxesBtnBack);
	_updateCheckboxes();
}

void CMenu::_updateCheckboxesText(void)
{
	u32 firstCheckbox= (CB_curPage - 1) * 10;
	for(u8 i = 1; i < min(firstCheckbox + 10, (u32)max_checkbox) - firstCheckbox + 1; i++)
	{
		if(mode == 1 || mode == 2)
		{
			string button = sfmt("button_%i", firstCheckbox + i - 1);
			m_btnMgr.setText(m_checkboxLblTxt[i], m_source.getWString(button, "title", button));
		}
		else if(mode == 3 || mode == 4)
		{
			m_btnMgr.setText(m_checkboxLblTxt[i], m_plugin.GetPluginName(firstCheckbox + i - 1));
		}
	}
}

void CMenu::_updateCheckboxes(void)
{
	if(max_checkbox > 10)
	{
		m_btnMgr.setText(m_checkboxesLblPage, wfmt(L"%i / %i", CB_curPage, CB_numPages));
		m_btnMgr.show(m_checkboxesLblPage);
		m_btnMgr.show(m_checkboxesBtnPageM);
		m_btnMgr.show(m_checkboxesBtnPageP);
	}
	for(int i = 0; i < 11; ++i)
	{
		m_btnMgr.hide(m_checkboxBtn[i]);
		m_btnMgr.hide(m_checkboxLblTxt[i]);
	}

	vector<string> magicNums;
	if(mode == 4)
	{
		magicNums = m_source.getStrings(sfmt("button_%i", curSource), "magic", ',');
	}
	u32 firstCheckbox = (CB_curPage - 1) * 10;
	for(u8 i = 1; i < min(firstCheckbox + 10, (u32)max_checkbox) - firstCheckbox + 1; ++i)
	{
		if(mode == 1)
		{
			if(m_source.getBool(sfmt("button_%i", firstCheckbox + i - 1), "hidden", false))
				m_checkboxBtn[i] = m_checkboxBtnOn[i];
			else
				m_checkboxBtn[i] = m_checkboxBtnOff[i];
		}
		else if(mode == 2)
		{
			m_checkboxBtn[i] = m_checkboxBtnOff[i];// all sources off
		}
		else if(mode == 3)
		{
			m_checkboxBtn[i] = m_checkboxBtnOff[i];// all plugins off
		}
		else
		{
			bool found = false;
			string pluginMagic = sfmt("%08x", m_plugin.GetPluginMagic(firstCheckbox + i - 1));
			
			if(magicNums.size() > 0)
			{
				for(u8 j = 0; j < magicNums.size(); j++)
				{
					string tmp = lowerCase(magicNums[j]);
					if(tmp == pluginMagic)
					{
						found = true;
						break;
					}
				}
				if(found)
					m_checkboxBtn[i] = m_checkboxBtnOn[i];
				else
					m_checkboxBtn[i] = m_checkboxBtnOff[i];
			}
		}
		m_btnMgr.show(m_checkboxBtn[i]);
		m_btnMgr.show(m_checkboxLblTxt[i]);
	}
}

void CMenu::_checkboxesMenu(u8 md)
{
	mode = md;
	if(mode == 1 || mode == 2)
		max_checkbox = m_max_source_btn + 1;
	else
	{
		max_checkbox = 0;
		while(m_plugin.PluginExist(max_checkbox)) max_checkbox++;
	}
	
	CB_curPage = 1;
	CB_numPages = (max_checkbox / 10) + 1;
	
	SetupInput();
	_showCheckboxesMenu();
	_updateCheckboxesText();
	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_checkboxesBtnBack)))
		{
			if(mode == 4)
			{
				mode = 2;
				max_checkbox = m_max_source_btn;
				CB_curPage = (curSource + 1) / 10 + 1;
				CB_numPages = (max_checkbox / 10) + 1;
				m_btnMgr.hide(m_checkboxesLblTitle, true);
				for(int i = 0; i < 11; ++i)
				{
					m_btnMgr.hide(m_checkboxBtn[i], true);
					m_btnMgr.hide(m_checkboxLblTxt[i], true);
				}
				_updateCheckboxes();
				_updateCheckboxesText();
				m_btnMgr.setText(m_checkboxesLblTitle, _t("smedit2", L"Choose Source"));
				m_btnMgr.show(m_checkboxesLblTitle);
			}
			else
				break;
		}
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if((BTN_MINUS_PRESSED || BTN_LEFT_PRESSED) || (BTN_A_PRESSED && m_btnMgr.selected(m_checkboxesBtnPageM)))
		{
			CB_curPage--;
			if(CB_curPage == 0)
				CB_curPage = CB_numPages;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_checkboxesBtnPageM);
			_updateCheckboxes();
			_updateCheckboxesText();
		}
		else if(((BTN_PLUS_PRESSED || BTN_RIGHT_PRESSED)) || (BTN_A_PRESSED && m_btnMgr.selected(m_checkboxesBtnPageP)))
		{
			CB_curPage++;
			if(CB_curPage > CB_numPages)
				CB_curPage = 1;
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED)
				m_btnMgr.click(m_checkboxesBtnPageP);
			_updateCheckboxes();
			_updateCheckboxesText();
		}
		if(BTN_A_PRESSED)
		{
			u32 firstCheckbox = (CB_curPage - 1) * 10;
			for(u8 i = 1; i <= min(firstCheckbox + 10, (u32)max_checkbox) - firstCheckbox + 1; ++i)
			{
				if(m_btnMgr.selected(m_checkboxBtn[i]))
				{
					if(mode == 1)
					{
						string button = sfmt("button_%i", firstCheckbox + i - 1);
						bool val = !m_source.getBool(button, "hidden", false);
						m_source.setBool(button, "hidden", val);
						_updateCheckboxes();
						m_btnMgr.setSelected(m_checkboxBtn[i]);
						break;
					}
					else if(mode == 2)
					{
						string source = m_source.getString(sfmt("button_%i", firstCheckbox + i - 1), "source", "");
						if(source != "plugin")
						{
							_hideCheckboxesMenu();
							error(_t("smediterr", L"Not allowed!"));
							_showCheckboxesMenu();
						}
						else
						{
							mode = 4;
							curSource = firstCheckbox + i - 1;
							max_checkbox = 0;
							while(m_plugin.PluginExist(max_checkbox)) max_checkbox++;
							CB_curPage = 1;
							CB_numPages = (max_checkbox / 10) + 1;
							m_btnMgr.hide(m_checkboxesLblTitle, true);
							for(int i = 0; i < 11; ++i)
							{
								m_btnMgr.hide(m_checkboxBtn[i], true);
								m_btnMgr.hide(m_checkboxLblTxt[i], true);
							}
							_updateCheckboxes();
							_updateCheckboxesText();
							m_btnMgr.setText(m_checkboxesLblTitle, _t("smedit4", L"Choose Plugins"));
							m_btnMgr.show(m_checkboxesLblTitle);
						}
					}
					else if(mode == 4)
					{
						bool found = false;
						u8 pluginsCount = 0;
						string newMagics;
						string pluginMagic = sfmt("%08x", m_plugin.GetPluginMagic(firstCheckbox + i - 1));
						string button = sfmt("button_%i", curSource);
						vector<string> magicNums = m_source.getStrings(button, "magic", ',');
						if(magicNums.size() > 0)
						{
							for(u8 j = 0; j < magicNums.size(); j++)
							{
								string tmp = lowerCase(magicNums[j]);
								if(tmp == pluginMagic)
								{
									found = true;// and don't add it
								}
								else if(m_plugin.GetPluginPosition(strtoul(magicNums[i].c_str(), NULL, 16)) < 255)// make sure plugin exist
								{
									if(pluginsCount == 0)
										newMagics = magicNums[j];
									else
										newMagics.append(',' + magicNums[j]);
									pluginsCount++;
								}
							}
						}
						if(!found)// add it if not found
						{
							if(newMagics.empty())
								newMagics = pluginMagic;
							else
								newMagics.append(',' + pluginMagic);
						}
						if(!newMagics.empty())// to make sure at least one plugin is selected
							m_source.setString(button, "magic", newMagics);
						_updateCheckboxes();
					}
					else if(mode == 3)
					{
						_hideCheckboxesMenu();
						u8 pos = firstCheckbox + i - 1;
						bool plugin_ok = true;
						strncpy(m_plugin.PluginMagicWord, fmt("%08x", m_plugin.GetPluginMagic(pos)), 8);
						if(strncasecmp(m_plugin.PluginMagicWord, "484252", 6) == 0)//HBRW
							plugin_ok = false;
						else if(strncasecmp(m_plugin.PluginMagicWord, "4E47434D", 8) == 0)//NGCM
							plugin_ok = false;						
						else if(strncasecmp(m_plugin.PluginMagicWord, "4E574949", 8) == 0)//NWII
							plugin_ok = false;
						else if(strncasecmp(m_plugin.PluginMagicWord, "4E414E44", 8) == 0)//NAND
							plugin_ok = false;
						else if(strncasecmp(m_plugin.PluginMagicWord, "454E414E", 8) == 0)//ENAN
							plugin_ok = false;
						else if(strncasecmp(m_plugin.PluginMagicWord, "5343564D", 8) == 0)//scummvm
							plugin_ok = false;
							
						if(!plugin_ok)
						{
							error(_t("smediterr", L"Not allowed!"));
						}
						else
						{
							int romsPartition = m_plugin.GetRomPartition(pos);
							if(romsPartition < 0)
								romsPartition = m_cfg.getInt(PLUGIN_DOMAIN, "partition", 0);
							string romsDir(fmt("%s:/%s", DeviceName[romsPartition], m_plugin.GetRomDir(pos)));
							const char *path = _FolderExplorer(romsDir.c_str());
							if(strlen(path) > 0)
							{
								Config m_plugin_cfg;
								m_plugin_cfg.load(m_plugin.GetPluginPath(pos).c_str());
								if(m_plugin_cfg.loaded())
								{
									if(strncmp(path, "sd:/", 4) == 0)
									{
										romsPartition = 0;
										m_plugin_cfg.setInt(PLUGIN, "rompartition", 0);
										m_plugin.SetRomPartition(pos, 0);
									}
									else
									{
										romsPartition = atoi(path + 3);
										m_plugin_cfg.setInt(PLUGIN, "rompartition", romsPartition);
										m_plugin.SetRomPartition(pos, romsPartition);
									}
									string rd = sfmt("%s", strchr(path, '/') + 1);
									m_plugin_cfg.setString(PLUGIN, "romdir", rd);
									m_plugin.SetRomDir(pos, rd);
									m_plugin_cfg.save(true);
									string cachedListFile(fmt("%s/%s_%s.db", m_listCacheDir.c_str(), DeviceName[romsPartition], m_plugin.PluginMagicWord));
									fsop_deleteFile(cachedListFile.c_str());
									m_refreshGameList = true;
								}
							}
						}
						_showCheckboxesMenu();
					}
				}
			}
		}
	}
	m_source.save();
	_hideCheckboxesMenu();
}

void CMenu::_initCheckboxesMenu()
{
	_addUserLabels(m_checkboxesLblUser, ARRAY_SIZE(m_checkboxesLblUser), "CHECKBOX");
	m_checkboxesBg = _texture("CHECKBOX/BG", "texture", theme.bg, false);
	m_checkboxesLblTitle = _addLabel("CHECKBOX/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_checkboxesBtnBack = _addButton("CHECKBOX/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	m_checkboxesLblPage = _addLabel("CHECKBOX/PAGE_BTN", theme.btnFont, L"", 68, 400, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_checkboxesBtnPageM = _addPicButton("CHECKBOX/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 48, 48);
	m_checkboxesBtnPageP = _addPicButton("CHECKBOX/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 172, 400, 48, 48);
	
	_setHideAnim(m_checkboxesLblTitle, "CHECKBOX/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_checkboxesLblPage, "CHECKBOX/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_checkboxesBtnPageM, "CHECKBOX/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_checkboxesBtnPageP, "CHECKBOX/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_checkboxesBtnBack, "CHECKBOX/BACK_BTN", 0, 0, 1.f, -1.f);

	/* init checkboxes only here to be used in all menu's */
	m_checkboxBtnOff[0] = _addPicButton("CHECKBOX/CHECKBOX_0_OFF", theme.checkboxoff, theme.checkboxoffs, 270, 394, 44, 48);
	m_checkboxBtnOn[0] = _addPicButton("CHECKBOX/CHECKBOX_0_ON", theme.checkboxon, theme.checkboxons, 270, 394, 44, 48);
	m_checkboxLblTxt[0] = _addLabel("CHECKBOX/CHECKBOX_0_TXT", theme.lblFont, L"", 325, 397, 100, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	for(int i = 1; i < 6; ++i)
	{ 	// Page 1
		m_checkboxBtnOff[i] = _addPicButton(fmt("CHECKBOX/CHECKBOX_%i_OFF", i), theme.checkboxoff, theme.checkboxoffs, 30, (39+i*58), 44, 48);
		m_checkboxBtnOn[i] = _addPicButton(fmt("CHECKBOX/CHECKBOX_%i_ON", i), theme.checkboxon, theme.checkboxons, 30, (39+i*58), 44, 48);
		m_checkboxLblTxt[i] = _addLabel(fmt("CHECKBOX/CHECKBOX_%i_TXT", i), theme.lblFont, L"", 85, (42+i*58), 230, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
		// right half
		m_checkboxBtnOff[i+5] = _addPicButton(fmt("CHECKBOX/CHECKBOX_%i_OFF", i+5), theme.checkboxoff, theme.checkboxoffs, 325, (39+i*58), 44, 48);
		m_checkboxBtnOn[i+5] = _addPicButton(fmt("CHECKBOX/CHECKBOX_%i_ON", i+5), theme.checkboxon, theme.checkboxons, 325, (39+i*58), 44, 48);
		m_checkboxLblTxt[i+5] = _addLabel(fmt("CHECKBOX/CHECKBOX_%i_TXT", i+5), theme.lblFont, L"", 380, (42+i*58), 230, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	}
	for(u8 i = 0; i < 11; ++i)
	{
		_setHideAnim(m_checkboxBtnOff[i], fmt("CHECKBOX/CHECKBOX_%i_OFF", i), 0, 0, 1.f, 0.f);
		_setHideAnim(m_checkboxBtnOn[i], fmt("CHECKBOX/CHECKBOX_%i_ON", i), 0, 0, 1.f, 0.f);
		_setHideAnim(m_checkboxLblTxt[i], fmt("CHECKBOX/CHECKBOX_%i_TXT", i), 0, 0, 1.f, 0.f);
		m_checkboxBtn[i] = m_checkboxBtnOff[i];
	}
	_hideCheckboxesMenu(true);
	_textCheckboxesMenu();
}

void CMenu::_textCheckboxesMenu(void)
{
	m_btnMgr.setText(m_checkboxesBtnBack, _t("cd1", L"Back"));
	m_btnMgr.setText(m_checkboxLblTxt[0], _t("dl25", L"All"));
}

/********************************************************************************************************************/

void CMenu::_hideSM_Editor(bool instant)
{
	m_btnMgr.hide(m_configLblTitle, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	
	m_btnMgr.hide(m_config7Lbl1, instant);
	m_btnMgr.hide(m_config7Btn1, instant);
	m_btnMgr.hide(m_config7Lbl2, instant);
	m_btnMgr.hide(m_config7Btn2, instant);
	m_btnMgr.hide(m_config7Lbl3, instant);
	m_btnMgr.hide(m_config7Btn3, instant);
	//m_btnMgr.hide(m_config7Lbl4, instant);
	//m_btnMgr.hide(m_config7Btn4, instant);
	
	for(u8 i = 0; i < ARRAY_SIZE(m_config7LblUser); ++i)
		if(m_config7LblUser[i] != -1)
			m_btnMgr.hide(m_config7LblUser[i], instant);
}

void CMenu::_showSM_Editor()
{
	m_btnMgr.hide(m_config7Lbl1, true);
	m_btnMgr.hide(m_config7Btn1, true);
	m_btnMgr.hide(m_config7Lbl2, true);
	m_btnMgr.hide(m_config7Btn2, true);
	m_btnMgr.hide(m_config7Lbl3, true);
	m_btnMgr.hide(m_config7Btn3, true);
	//m_btnMgr.hide(m_config7Lbl4, true);
	//m_btnMgr.hide(m_config7Btn4, true);
	
	_setBg(m_config7Bg, m_config7Bg);
	for(u32 i = 0; i < ARRAY_SIZE(m_config7LblUser); ++i)
		if(m_config7LblUser[i] != -1)
			m_btnMgr.show(m_config7LblUser[i]);

	m_btnMgr.show(m_configLblTitle);
	m_btnMgr.show(m_configBtnBack);

	m_btnMgr.show(m_config7Lbl1);
	m_btnMgr.show(m_config7Btn1);
	m_btnMgr.show(m_config7Lbl2);
	m_btnMgr.show(m_config7Btn2);
	m_btnMgr.show(m_config7Lbl3);
	m_btnMgr.show(m_config7Btn3);
	//m_btnMgr.show(m_config7Lbl4);
	//m_btnMgr.show(m_config7Btn4);
	
	m_btnMgr.setText(m_config7Lbl1, _t("smedit5", L"Hide source buttons"));
	m_btnMgr.setText(m_config7Btn1, _t("cfg14", L"Set"));
	m_btnMgr.setText(m_config7Lbl2, _t("smedit6", L"Link source buttons to plugins"));
	m_btnMgr.setText(m_config7Btn2, _t("cfg14", L"Set"));
	m_btnMgr.setText(m_config7Lbl3, _t("smedit7", L"Set plugin ROMs path"));
	m_btnMgr.setText(m_config7Btn3, _t("cfg14", L"Set"));
	//m_btnMgr.setText(m_config7Lbl4, _t("smedit8", L"Sourceflow settings"));
	//m_btnMgr.setText(m_config7Btn4, _t("cfg14", L"Set"));
}

void CMenu::_SM_Editor()
{
	m_btnMgr.setText(m_configLblTitle, _t("smedit9", L"Source Menu Setup"));// edit sources
	SetupInput();
	_showSM_Editor();
	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_configBtnBack)))
		{
			break;
		}
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_config7Btn1))
			{
				_hideSM_Editor();
				_checkboxesMenu(1);
				_showSM_Editor();
			}
			else if(m_btnMgr.selected(m_config7Btn2))
			{
				_hideSM_Editor();
				_checkboxesMenu(2);
				_showSM_Editor();
			}
			else if(m_btnMgr.selected(m_config7Btn3))
			{
				_hideSM_Editor();
				_checkboxesMenu(3);
				_showSM_Editor();
			}
			/*else if(m_btnMgr.selected(m_config7Btn4))
			{
				_hideSM_Editor();
				_CfgSrc();
				_showSM_Editor();
			}*/
		}
	}
	_hideSM_Editor();
	m_btnMgr.setText(m_configLblTitle, _t("cfg1", L"Settings"));
}
