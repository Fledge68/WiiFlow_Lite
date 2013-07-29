#include "menu.hpp"

#include <string.h>
#include <gccore.h>

// Category menu
s16 m_categoryLblPage;
s16 m_categoryBtnPageM;
s16 m_categoryBtnPageP;
s16 m_categoryBtnClear;
s16 m_categoryBtnBack;
s16 m_categoryLblTitle;
s16 m_categoryLblCat[11];
s16 m_categoryBtnCat[11];
s16 m_categoryBtnCats[11];
s16 m_categoryBtnCatHid[11];
s16 m_categoryBtnCatReq[11];
s16 m_categoryLblUser[4];
TexData m_categoryBg;

vector<char> m_categories;
u8 curPage;
u8 lastBtn;
const char *catSettings;
string id;
string catDomain;
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
		if(m_categoryLblUser[i] != -1)
			m_btnMgr.hide(m_categoryLblUser[i], instant);
	}

	for(u8 i = 1; i < 11; ++i)
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
		if(m_categoryLblUser[i] != -1)
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
	for(u8 i = 1; i < 11; ++i)
	{
		int j = i + ((curPage - 1) * 10);
		if(j == m_max_categories)
			break;
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
		}

		m_btnMgr.setText(m_categoryLblCat[i], m_cat.getWString("GENERAL", fmt("cat%d",j), wfmt(L"Category %i",j).c_str()));	
		m_btnMgr.show(m_categoryLblCat[i]);
	}

}

void CMenu::_getIDCats(void)
{
	const dir_discHdr *hdr = CoverFlow.getHdr();
	switch(hdr->type)
	{
		case TYPE_CHANNEL:
			catDomain = "NAND";
			break;
		case TYPE_HOMEBREW:
			catDomain = "HOMEBREW";
			break;
		case TYPE_GC_GAME:
			catDomain = "DML";
			break;
		case TYPE_WII_GAME:
			catDomain = "GAMES";
			break;
		default:
			catDomain = (m_plugin.GetPluginName(m_plugin.GetPluginPosition(hdr->settings[0]))).toUTF8();
	}
	id = _getId();
	const char *idCats = m_cat.getString(catDomain, id, "").c_str();
	u8 numIdCats = strlen(idCats);
	if(numIdCats != 0)
	{
		for(u8 j = 0; j < numIdCats; ++j)
		{
			int k = (static_cast<int>(idCats[j])) - 32;
			m_categories.at(k) = '1';
		}
	}
	m_btnMgr.setText(m_categoryLblTitle, CoverFlow.getTitle());
}

void CMenu::_setIDCats(void)
{
	string newIdCats = "";
	for(int i = 1; i < m_max_categories; i++)
	{
		if(m_categories.at(i) == '1')
		{
			char cCh = static_cast<char>( i + 32);
			newIdCats = newIdCats + cCh;
		}
	}
	m_cat.setString(catDomain, id, newIdCats);
}
	
void CMenu::_CategorySettings(bool fromGameSet)
{
	SetupInput();
	curPage = 1;
	gameSet = fromGameSet;
	
	if(m_source.loaded() && m_catStartPage > 0)
		curPage = m_catStartPage;
	
	m_max_categories = m_cat.getInt("GENERAL", "numcategories", 6);
	if(curPage < 1 || curPage > (((m_max_categories - 2)/ 10) + 1))
		curPage = 1;
	m_categories.resize(m_max_categories, '0');
	m_categories.assign(m_max_categories, '0');

	if(fromGameSet)
	{
		_getIDCats();
	}
	else
	{
		const char *requiredCats = m_cat.getString("GENERAL", "required_categories").c_str();
		const char *selectedCats = m_cat.getString("GENERAL", "selected_categories").c_str();
		const char *hiddenCats = m_cat.getString("GENERAL", "hidden_categories").c_str();
		u8 numReqCats = strlen(requiredCats);
		u8 numSelCats = strlen(selectedCats);
		u8 numHidCats = strlen(hiddenCats);
		
		if(numReqCats != 0)
		{
			for(u8 j = 0; j < numReqCats; ++j)
			{
				int k = (static_cast<int>(requiredCats[j])) - 32;
				m_categories.at(k) = '3';
			}
		}
		if(numSelCats != 0)
		{
			for(u8 j = 0; j < numSelCats; ++j)
			{
				int k = (static_cast<int>(selectedCats[j])) - 32;
				m_categories.at(k) = '1';
			}
		}
		if(numHidCats != 0)
		{
			for(u8 j = 0; j < numHidCats; ++j)
			{
				int k = (static_cast<int>(hiddenCats[j])) - 32;
				m_categories.at(k) = '2';
			}
		}		
		m_btnMgr.setText(m_categoryLblTitle, _t("cat1", L"Select Categories"));
	}
	_showCategorySettings();
	
	while(!m_exit)
	{
		_mainLoopCommon();
		CoverFlow.tick();
		if(!m_btnMgr.selected(lastBtn))
			m_btnMgr.noHover(false);
			
		if(BTN_HOME_PRESSED || BTN_B_PRESSED || (BTN_A_PRESSED && m_btnMgr.selected(m_categoryBtnBack)))
		{
			if(!fromGameSet)
			{
				string newReqCats = "";
				string newSelCats = "";
				string newHidCats = "";
				for(int i = 1; i < m_max_categories; i++)
				{
					if(m_categories.at(i) == '1')
					{
						char cCh = static_cast<char>( i + 32);
						newSelCats = newSelCats + cCh;
					}
					else if(m_categories.at(i) == '2')
					{
						char cCh = static_cast<char>( i + 32);
						newHidCats = newHidCats + cCh;
					}
					else if(m_categories.at(i) == '3')
					{
						char cCh = static_cast<char>( i + 32);
						newReqCats = newReqCats + cCh;
					}
				}
				m_cat.setString("GENERAL", "selected_categories", newSelCats);
				m_cat.setString("GENERAL", "hidden_categories", newHidCats);
				m_cat.setString("GENERAL", "required_categories", newReqCats);
			}
			else
				_setIDCats();

			m_cat.save();
			break;
		}
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if(BTN_PLUS_PRESSED && fromGameSet)
		{
			_setIDCats();
			_hideCategorySettings();
			CoverFlow.right();
			curPage = 1;
			m_categories.assign(m_max_categories, '0');
			_getIDCats();
			_showCategorySettings();
		}
		if(BTN_MINUS_PRESSED && fromGameSet)
		{
			_setIDCats();
			_hideCategorySettings();
			CoverFlow.left();
			curPage = 1;
			m_categories.assign(m_max_categories, '0');
			_getIDCats();
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
				bool hiddenCat = false;
				for(int j = 1; j < m_max_categories; ++j)
				{
					if(m_categories.at(j) == '2' && m_locked)
					{	
						hiddenCat = true;
						continue;
					}
					m_categories.at(j) = '0';
				}
				if(!hiddenCat)
					m_categories.at(0) = '1';
				_updateCheckboxes();
			}
			for(u8 i = 1; i < 11; ++i)
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
						m_categories.at(j) = m_categories.at(j) == '0' ? '1' : '0';
					}
					else
					{
						if(m_locked && m_categories.at(j) == '2')
							m_categories.at(j) = '1';
						else if(m_locked && m_categories.at(j) == '1')
							m_categories.at(j) = '2';
						m_categories.at(j) = m_categories.at(j) == '0' ? '1' : m_categories.at(j) == '1' ? '2' : m_categories.at(j) == '2' ? '3' : '0';
						if(m_categories.at(0) == '1' && m_categories.at(j) != '0')
							m_categories.at(0) = '0';
					}
					m_btnMgr.hide(m_categoryBtnCat[i], true);
					m_btnMgr.hide(m_categoryBtnCats[i], true);
					m_btnMgr.hide(m_categoryBtnCatHid[i], true);
					m_btnMgr.hide(m_categoryBtnCatReq[i], true);
					switch(m_categories.at(j))
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
					break;
				}
			}
		}
	}
	_hideCategorySettings();
}

void CMenu::_initCategorySettingsMenu()
{
	_addUserLabels(m_categoryLblUser, ARRAY_SIZE(m_categoryLblUser), "CATEGORY");
	m_categoryBg = _texture("CATEGORY/BG", "texture", theme.bg, false);
	m_categoryLblTitle = _addTitle("CATEGORY/TITLE", theme.titleFont, L"", 20, 30, 600, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_categoryBtnPageM = _addPicButton("CATEGORY/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 56, 56);
	m_categoryLblPage = _addLabel("CATEGORY/PAGE_BTN", theme.btnFont, L"", 76, 400, 100, 56, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_categoryBtnPageP = _addPicButton("CATEGORY/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 176, 400, 56, 56);
	m_categoryBtnBack = _addButton("CATEGORY/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 56, theme.btnFontColor);
	m_categoryBtnClear = _addButton("CATEGORY/CLEAR_BTN", theme.btnFont, L"", 255, 400, 150, 56, theme.btnFontColor);
	for(u8 i = 1; i < 6; ++i)
	{ 	// left half
		m_categoryBtnCat[i] = _addPicButton(fmt("CATEGORY/CAT_%i_BTN", i), theme.checkboxoff, theme.checkboxoffs, 30, (42+i*58), 44, 48);
		m_categoryBtnCats[i] = _addPicButton(fmt("CATEGORY/CAT_%i_BTNS", i), theme.checkboxon, theme.checkboxons, 30, (42+i*58), 44, 48);
		m_categoryBtnCatHid[i] = _addPicButton(fmt("CATEGORY/CAT_%i_BTNHID", i), theme.checkboxHid, theme.checkboxHids, 30, (42+i*58), 44, 48);
		m_categoryBtnCatReq[i] = _addPicButton(fmt("CATEGORY/CAT_%i_BTNREQ", i), theme.checkboxReq, theme.checkboxReqs, 30, (42+i*58), 44, 48);
		m_categoryLblCat[i] = _addLabel(fmt("CATEGORY/CAT_%i", i), theme.lblFont, L"", 85, (42+i*58), 230, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
		// right half
		m_categoryBtnCat[i+5] = _addPicButton(fmt("CATEGORY/CAT_%i_BTN", i+5), theme.checkboxoff, theme.checkboxoffs, 325, (42+i*58), 44, 48);
		m_categoryBtnCats[i+5] = _addPicButton(fmt("CATEGORY/CAT_%i_BTNS", i+5), theme.checkboxon, theme.checkboxons, 325, (42+i*58), 44, 48);
		m_categoryBtnCatHid[i+5] = _addPicButton(fmt("CATEGORY/CAT_%i_BTNHID", i+5), theme.checkboxHid, theme.checkboxHids, 325, (42+i*58), 44, 48);
		m_categoryBtnCatReq[i+5] = _addPicButton(fmt("CATEGORY/CAT_%i_BTNREQ", i+5), theme.checkboxReq, theme.checkboxReqs, 325, (42+i*58), 44, 48);
		m_categoryLblCat[i+5] = _addLabel(fmt("CATEGORY/CAT_%i", i+5), theme.txtFont, L"", 380, (42+i*58), 230, 48, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	}
	_setHideAnim(m_categoryLblTitle, "CATEGORY/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_categoryLblPage, "CATEGORY/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_categoryBtnPageM, "CATEGORY/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_categoryBtnPageP, "CATEGORY/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_categoryBtnClear, "CATEGORY/CLEAR_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_categoryBtnBack, "CATEGORY/BACK_BTN", 0, 0, 1.f, -1.f);
	for(u8 i = 1; i < 11; ++i)
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
