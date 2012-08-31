#include "menu.hpp"

#include <string.h>
#include <gccore.h>

// Category menu
u16 m_categoryLblPage;
u16 m_categoryBtnPageM;
u16 m_categoryBtnPageP;
u16 m_categoryBtnClear;
u16 m_categoryBtnBack;
u16 m_categoryLblTitle;
u16 m_categoryLblCat[11];
u16 m_categoryBtnCat[11];
u16 m_categoryBtnCats[11];
u16 m_categoryBtnCatHid[11];
u16 m_categoryBtnCatReq[11];
u16 m_categoryLblUser[4];
STexture m_categoryBg;

u8 m_categories[51];
u8 curPage;
u8 lastBtn;
const char *catSettings;
string id;
bool gameSet;

void CMenu::_hideCategorySettings(bool instant)
{
	m_btnMgr.hide(m_categoryLblTitle, instant);
	m_btnMgr.hide(m_categoryLblPage, instant);
	m_btnMgr.hide(m_categoryBtnPageM, instant);
	m_btnMgr.hide(m_categoryBtnPageP, instant);
	m_btnMgr.hide(m_categoryBtnClear, instant);
	m_btnMgr.hide(m_categoryBtnBack, instant);
	
	for(u8 i = 0; i < ARRAY_SIZE(m_categoryLblUser); ++i)
	{
		if(m_categoryLblUser[i] != (u16)-1u)
			m_btnMgr.hide(m_categoryLblUser[i], instant);
	}

	for(int i = 1; i < 11; ++i)
	{
		m_btnMgr.hide(m_categoryLblCat[i]);
		m_btnMgr.hide(m_categoryBtnCat[i]);
		m_btnMgr.hide(m_categoryBtnCats[i]);
		m_btnMgr.hide(m_categoryBtnCatHid[i]);
		m_btnMgr.hide(m_categoryBtnCatReq[i]);
	}
}

void CMenu::_showCategorySettings(void)
{
	_setBg(m_categoryBg, m_categoryBg);
	
	for(u8 i = 0; i < ARRAY_SIZE(m_categoryLblUser); ++i)
	{
		if(m_categoryLblUser[i] != (u16)-1)
			m_btnMgr.show(m_categoryLblUser[i]);
	}
	
	m_btnMgr.show(m_categoryLblTitle);
	m_btnMgr.show(m_categoryBtnClear);
	m_btnMgr.show(m_categoryBtnBack);
	
	_updateCheckboxes();
}

void CMenu::_updateCheckboxes(void)
{
	for(u8 i = 1; i < 11; ++i)
	{
		m_btnMgr.hide(m_categoryBtnCat[i], true);
		m_btnMgr.hide(m_categoryBtnCats[i], true);
		m_btnMgr.hide(m_categoryBtnCatHid[i], true);
		m_btnMgr.hide(m_categoryBtnCatReq[i], true);
		m_btnMgr.hide(m_categoryLblCat[i], true);
	}
	
	if(m_max_categories > 11)
	{
		m_btnMgr.setText(m_categoryLblPage, wfmt(L"%i / %i", curPage, ((m_max_categories - 2) / 10) + 1));
		m_btnMgr.show(m_categoryLblPage);
		m_btnMgr.show(m_categoryBtnPageM);
		m_btnMgr.show(m_categoryBtnPageP);
	}

	for(int i = 1; i < 11; ++i)
	{
		int j = i + ((curPage - 1) * 10);
		if(j == m_max_categories)
			break;
		if(m_categories[0] == '1' && !gameSet)
			m_btnMgr.show(m_categoryBtnCat[i]);
		else
		{
			switch(m_categories[j])
			{
				case '0':
					m_btnMgr.show(m_categoryBtnCat[i]);
					break;
				case '1':
					m_btnMgr.show(m_categoryBtnCats[i]);
					break;
				case '2':
					m_btnMgr.show(m_categoryBtnCatHid[i]);
					break;
				default:
					m_btnMgr.show(m_categoryBtnCatReq[i]);
					break;
			}
		}

		m_btnMgr.setText(m_categoryLblCat[i], m_cat.getWString(fmt("%s/GENERAL", _domainFromView()), fmt("cat%d",j), wfmt(L"Category %i",j).c_str()));	
		m_btnMgr.show(m_categoryLblCat[i]);
	}

}

void CMenu::_CategorySettings(bool fromGameSet)
{
	SetupInput();
	curPage = 1;
	gameSet = fromGameSet;
	
	if(fromGameSet)
	{
		if(m_current_view != COVERFLOW_EMU)
			id = m_cf.getId();
		else
		{
			dir_discHdr *hdr = m_cf.getHdr();
			string tempname(hdr->path);
			tempname.erase(0, tempname.find_first_of('/')+1);
			string dirName = tempname.substr(0, tempname.find_first_of('/')+1);
			tempname.assign(&tempname[tempname.find_last_of('/') + 1]);
			if(tempname.find_last_of('.') != string::npos)
			tempname.erase(tempname.find_last_of('.'), tempname.size() - tempname.find_last_of('.'));
			id = dirName+tempname;
		}
		catSettings = m_cat.getString(_domainFromView(), id, "").c_str();
		m_btnMgr.setText(m_categoryLblTitle, m_cf.getTitle());
	}
	else
	{
		catSettings = m_cat.getString(fmt("%s/GENERAL", _domainFromView()), "categories").c_str();
		m_btnMgr.setText(m_categoryLblTitle, _t("cat1", L"Select Categories"));
	}
		
	memset(&m_categories, '0', m_max_categories);
	memcpy(&m_categories, catSettings, m_max_categories);
		
	_showCategorySettings();
	
	while(true)
	{
		_mainLoopCommon();
		m_cf.tick();
		
		if(!m_btnMgr.selected(lastBtn))
			m_btnMgr.noHover(false);
			
		if(BTN_HOME_PRESSED || BTN_B_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_categoryBtnBack)))
		{
			char newCatSettings[m_max_categories + 1];
			memset(&newCatSettings, 0, sizeof(newCatSettings));
			memcpy(&newCatSettings, &m_categories, sizeof(m_categories));

			if(!fromGameSet)
			{
				newCatSettings[0] = '1';
				for(u8 i = 1; i < m_max_categories; i++)
				{
					if(newCatSettings[i] != '0')
					{
						newCatSettings[0] = '0';
						break;
					}
				}
				m_cat.setString(fmt("%s/GENERAL", _domainFromView()), "categories", newCatSettings);
			}
			else
				m_cat.setString(_domainFromView(), id, newCatSettings);

			//m_cat.save();
			break;
		}
		
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
			
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		
		if(BTN_PLUS_PRESSED && fromGameSet)
		{
			char newCatSettings[m_max_categories + 1];
			memset(&newCatSettings, 0, sizeof(newCatSettings));
			memcpy(&newCatSettings, &m_categories, sizeof(m_categories));
			m_cat.setString(_domainFromView(), id, newCatSettings);
			
			_hideCategorySettings();
			m_cf.right();
			curPage = 1;
			if(m_current_view != COVERFLOW_EMU)
				id = m_cf.getId();
			else
			{
				dir_discHdr *hdr = m_cf.getHdr();
				string tempname(hdr->path);
				tempname.erase(0, tempname.find_first_of('/')+1);
				string dirName = tempname.substr(0, tempname.find_first_of('/')+1);
				tempname.assign(&tempname[tempname.find_last_of('/') + 1]);
				if(tempname.find_last_of('.') != string::npos)
					tempname.erase(tempname.find_last_of('.'), tempname.size() - tempname.find_last_of('.'));
				id = dirName+tempname;
			}
			catSettings = m_cat.getString(_domainFromView(), id, "").c_str();
			m_btnMgr.setText(m_categoryLblTitle, m_cf.getTitle());
			
			memset(&m_categories, '0', m_max_categories);
			memcpy(&m_categories, catSettings, m_max_categories);
		
			_showCategorySettings();
		}
		
		if(BTN_MINUS_PRESSED && fromGameSet)
		{
			char newCatSettings[m_max_categories + 1];
			memset(&newCatSettings, 0, sizeof(newCatSettings));
			memcpy(&newCatSettings, &m_categories, sizeof(m_categories));
			m_cat.setString(_domainFromView(), id, newCatSettings);
			
			_hideCategorySettings();
			m_cf.left();
			curPage = 1;
			if(m_current_view != COVERFLOW_EMU)
				id = m_cf.getId();
			else
			{
				dir_discHdr *hdr = m_cf.getHdr();
				string tempname(hdr->path);
				tempname.erase(0, tempname.find_first_of('/')+1);
				string dirName = tempname.substr(0, tempname.find_first_of('/')+1);
				tempname.assign(&tempname[tempname.find_last_of('/') + 1]);
				if(tempname.find_last_of('.') != string::npos)
					tempname.erase(tempname.find_last_of('.'), tempname.size() - tempname.find_last_of('.'));
				id = dirName+tempname;
			}
			catSettings = m_cat.getString(_domainFromView(), id, "").c_str();
			m_btnMgr.setText(m_categoryLblTitle, m_cf.getTitle());
			
			memset(&m_categories, '0', m_max_categories);
			memcpy(&m_categories, catSettings, m_max_categories);
		
			_showCategorySettings();
		}
	
		if((BTN_LEFT_PRESSED && m_max_categories>11) || (BTN_A_PRESSED && m_btnMgr.selected(m_categoryBtnPageM)))
		{
			lastBtn = m_categoryBtnPageM;
			m_btnMgr.noHover(true);
			
			curPage--;
			if(curPage < 1)
				curPage = ((m_max_categories - 2) / 10) + 1;
			
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_categoryBtnPageM);
				
			_updateCheckboxes();
		}
		else if((BTN_RIGHT_PRESSED && m_max_categories>11) || (BTN_A_PRESSED && m_btnMgr.selected(m_categoryBtnPageP)))
		{
			lastBtn = m_categoryBtnPageP;
			m_btnMgr.noHover(true);
			
			curPage++;
			if(curPage > ((m_max_categories - 2) / 10) + 1)
				curPage = 1;
			
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED)
				m_btnMgr.click(m_categoryBtnPageP);
				
			_updateCheckboxes();
		}
		if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_categoryBtnClear))
			{
				m_categories[0] = '1';
				for(int j = 1; j < m_max_categories; ++j)
					m_categories[j] = '0';
				_updateCheckboxes();
			}
			
			for(int i = 1; i < 11; ++i)
			{
				if(m_btnMgr.selected(m_categoryBtnCat[i]) || m_btnMgr.selected(m_categoryBtnCats[i]) || m_btnMgr.selected(m_categoryBtnCatHid[i]) || m_btnMgr.selected(m_categoryBtnCatReq[i]))
				{
					lastBtn = m_categoryBtnCat[i];
					if(m_btnMgr.selected(m_categoryBtnCats[i]))
						lastBtn = m_categoryBtnCats[i];
					else if(m_btnMgr.selected(m_categoryBtnCatHid[i]))
						lastBtn = m_categoryBtnCatHid[i];
					else if(m_btnMgr.selected(m_categoryBtnCatReq[i]))
						lastBtn = m_categoryBtnCatReq[i];
					m_btnMgr.noHover(true);
					
					int j = i + ((curPage - 1) * 10);
					if(fromGameSet)
					{
						m_categories[j] = m_categories[j] == '0' ? '1' : '0';
					}
					else
					{
						m_categories[j] = m_categories[j] == '0' ? '1' : m_categories[j] == '1' ? '2' : m_categories[j] == '2' ? '3' : '0';
						if(m_categories[0] == '1' && m_categories[j] != '0')
							m_categories[0] = '0';
					}
					
					m_btnMgr.hide(m_categoryBtnCat[i], true);
					m_btnMgr.hide(m_categoryBtnCats[i], true);
					m_btnMgr.hide(m_categoryBtnCatHid[i], true);
					m_btnMgr.hide(m_categoryBtnCatReq[i], true);
					
					if(m_categories[0] =='1' && !fromGameSet)
						m_btnMgr.show(m_categoryBtnCat[i]);
					else
					{
						switch(m_categories[j])
						{
							case '0':
								m_btnMgr.show(m_categoryBtnCat[i]);
								break;
							case '1':
								m_btnMgr.show(m_categoryBtnCats[i]);
								break;
							case '2':
								m_btnMgr.show(m_categoryBtnCatHid[i]);
								break;
							default:
								m_btnMgr.show(m_categoryBtnCatReq[i]);
								break;
						}
					}
					break;
				}
			}
		}
	}
	_hideCategorySettings();
}

void CMenu::_initCategorySettingsMenu(CMenu::SThemeData &theme)
{
	_addUserLabels(theme, m_categoryLblUser, ARRAY_SIZE(m_categoryLblUser), "CATEGORY");
	m_categoryBg = _texture(theme.texSet, "CATEGORY/BG", "texture", theme.bg);
	m_categoryLblTitle = _addTitle(theme, "CATEGORY/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_categoryBtnPageM = _addPicButton(theme, "CATEGORY/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 56, 56);
	m_categoryLblPage = _addLabel(theme, "CATEGORY/PAGE_BTN", theme.btnFont, L"", 76, 400, 100, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_categoryBtnPageP = _addPicButton(theme, "CATEGORY/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 176, 400, 56, 56);
	m_categoryBtnBack = _addButton(theme, "CATEGORY/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_categoryBtnClear = _addButton(theme, "CATEGORY/CLEAR_BTN", theme.btnFont, L"", 255, 400, 150, 56, theme.btnFontColor);	
	
	for(int i = 1; i < 6; ++i)
	{ 	// left half
		m_categoryBtnCat[i] = _addPicButton(theme, fmt("CATEGORY/CAT_%i_BTN", i), theme.checkboxoff, theme.checkboxoffs, 30, (42+i*58), 44, 48);
		m_categoryBtnCats[i] = _addPicButton(theme, fmt("CATEGORY/CAT_%i_BTNS", i), theme.checkboxon, theme.checkboxons, 30, (42+i*58), 44, 48);
		m_categoryBtnCatHid[i] = _addPicButton(theme, fmt("CATEGORY/CAT_%i_BTNHID", i), theme.checkboxHid, theme.checkboxHids, 30, (42+i*58), 44, 48);
		m_categoryBtnCatReq[i] = _addPicButton(theme, fmt("CATEGORY/CAT_%i_BTNREQ", i), theme.checkboxReq, theme.checkboxReqs, 30, (42+i*58), 44, 48);
		m_categoryLblCat[i] = _addLabel(theme, fmt("CATEGORY/CAT_%i", i), theme.lblFont, L"", 85, (42+i*58), 230, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
		// right half
		m_categoryBtnCat[i+5] = _addPicButton(theme, fmt("CATEGORY/CAT_%i_BTN", i+5), theme.checkboxoff, theme.checkboxoffs, 325, (42+i*58), 44, 48);
		m_categoryBtnCats[i+5] = _addPicButton(theme, fmt("CATEGORY/CAT_%i_BTNS", i+5), theme.checkboxon, theme.checkboxons, 325, (42+i*58), 44, 48);
		m_categoryBtnCatHid[i+5] = _addPicButton(theme, fmt("CATEGORY/CAT_%i_BTNHID", i+5), theme.checkboxHid, theme.checkboxHids, 325, (42+i*58), 44, 48);
		m_categoryBtnCatReq[i+5] = _addPicButton(theme, fmt("CATEGORY/CAT_%i_BTNREQ", i+5), theme.checkboxReq, theme.checkboxReqs, 325, (42+i*58), 44, 48);
		m_categoryLblCat[i+5] = _addLabel(theme, fmt("CATEGORY/CAT_%i", i+5), theme.txtFont, L"", 380, (42+i*58), 230, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	}
	_setHideAnim(m_categoryLblTitle, "CATEGORY/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_categoryLblPage, "CATEGORY/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_categoryBtnPageM, "CATEGORY/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_categoryBtnPageP, "CATEGORY/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_categoryBtnClear, "CATEGORY/CLEAR_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_categoryBtnBack, "CATEGORY/BACK_BTN", 0, 0, 1.f, -1.f);
	
	for(int i = 1; i < 11; ++i)
	{
		_setHideAnim(m_categoryBtnCat[i], fmt("CATEGORY/CAT_%i_BTN", i), 0, 0, 1.f, 0.f);
		_setHideAnim(m_categoryBtnCats[i], fmt("CATEGORY/CAT_%i_BTNS", i), 0, 0, 1.f, 0.f);
		_setHideAnim(m_categoryBtnCatHid[i], fmt("CATEGORY/CAT_%i_BTNHID", i), 0, 0, 1.f, 0.f);
		_setHideAnim(m_categoryBtnCatReq[i], fmt("CATEGORY/CAT_%i_BTNREQ", i), 0, 0, 1.f, 0.f);
		_setHideAnim(m_categoryLblCat[i], fmt("CATEGORY/CAT_%i", i), 0, 0, 1.f, 0.f);
	}
	_hideCategorySettings(true);
	_textCategorySettings();
}

void CMenu::_textCategorySettings(void)
{
	m_btnMgr.setText(m_categoryBtnClear, _t("cat2", L"Clear"));
	m_btnMgr.setText(m_categoryBtnBack, _t("cd1", L"Back"));
}
