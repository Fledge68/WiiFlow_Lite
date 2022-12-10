
#include "menu.hpp"

// Category menu
s16 m_categoryLblPage;
s16 m_categoryBtnPageM;
s16 m_categoryBtnPageP;
s16 m_categoryBtnClear;
s16 m_categoryBtnBack;
s16 m_categoryLblTitle;
s16 m_categoryLblUser[4];
TexData m_categoryBg;

vector<char> m_categories;
static u8 curPage;
char id[64];
const char *catDomain = NULL;
bool gameSet;
string genDomain;

void CMenu::_hideCategorySettings(bool instant)
{
	m_btnMgr.hide(m_categoryLblTitle, instant);
	m_btnMgr.hide(m_categoryLblPage, instant);
	m_btnMgr.hide(m_categoryBtnPageM, instant);
	m_btnMgr.hide(m_categoryBtnPageP, instant);
	m_btnMgr.hide(m_categoryBtnClear, instant);
	m_btnMgr.hide(m_categoryBtnBack, instant);

	for(u8 i = 0; i < ARRAY_SIZE(m_categoryLblUser); ++i)
		if(m_categoryLblUser[i] != -1)
			m_btnMgr.hide(m_categoryLblUser[i], instant);

	for(u8 i = 1; i < 11; ++i)
	{
		m_btnMgr.hide(m_checkboxLblTxt[i]);
		m_btnMgr.hide(m_checkboxBtn[i]);
	}
}

void CMenu::_showCategorySettings(void)
{
	_setBg(m_categoryBg, m_categoryBg);
	for(u8 i = 0; i < ARRAY_SIZE(m_categoryLblUser); ++i)
		if(m_categoryLblUser[i] != -1)
			m_btnMgr.show(m_categoryLblUser[i]);
			
	m_btnMgr.show(m_categoryLblTitle);
	m_btnMgr.show(m_categoryBtnClear);
	m_btnMgr.show(m_categoryBtnBack);
	_updateCatCheckboxes();
}

void CMenu::_setCatGenDomain()
{
	genDomain = "GENERAL";
	if(!m_cat.hasDomain("PLUGINS"))// if still using old style categories_lite.ini set as "GENERAL" and return
		return;
	if(gameSet)
	{
		const dir_discHdr *hdr = CoverFlow.getHdr();
		if(hdr->type == TYPE_PLUGIN)
			genDomain = "PLUGINS";
	}
	else if(m_current_view & COVERFLOW_PLUGIN)
	{
		for(u8 i = 0; m_plugin.PluginExist(i); ++i)// only set "PLUGINS" for real plugins
		{
			if(m_plugin.GetEnabledStatus(i))
			{
				strncpy(m_plugin.PluginMagicWord, fmt("%08x", m_plugin.GetPluginMagic(i)), 8);
				if(strncasecmp(m_plugin.PluginMagicWord, GC_PMAGIC, 8) == 0)//NGCM
					continue;
				else if(strncasecmp(m_plugin.PluginMagicWord, WII_PMAGIC, 8) == 0)//NWII
					continue;
				else if(strncasecmp(m_plugin.PluginMagicWord, NAND_PMAGIC, 8) == 0)//NAND
					continue;
				else if(strncasecmp(m_plugin.PluginMagicWord, ENAND_PMAGIC, 8) == 0)//EMUNAND
					continue;
				else if(strncasecmp(m_plugin.PluginMagicWord, HB_PMAGIC, 8) == 0)//HBRW
					continue;	
				else
				{
					genDomain = "PLUGINS";
					break;
				}
			}
		}
	}
}

void CMenu::_updateCatCheckboxes(void)
{
	for(u8 i = 1; i < 11; ++i)
	{
		m_btnMgr.hide(m_checkboxBtn[i], true);
		m_btnMgr.hide(m_checkboxLblTxt[i], true);
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
				m_btnMgr.setBtnTexture(m_checkboxBtn[i], theme.checkboxoff, theme.checkboxoffs, false);
				break;
			case '1':
				m_btnMgr.setBtnTexture(m_checkboxBtn[i], theme.checkboxon, theme.checkboxons, false);
				break;
			case '2':
				m_btnMgr.setBtnTexture(m_checkboxBtn[i], theme.checkboxHid, theme.checkboxHids, false);
				break;
			default:
				m_btnMgr.setBtnTexture(m_checkboxBtn[i], theme.checkboxReq, theme.checkboxReqs, false);
		}
		m_btnMgr.setText(m_checkboxLblTxt[i], m_cat.getWString(genDomain, fmt("cat%d",j), wfmt(L"Category %i",j).c_str()));
		m_btnMgr.show(m_checkboxLblTxt[i]);
		m_btnMgr.show(m_checkboxBtn[i]);
	}

}

void CMenu::_getGameCategories(void)
{
	const dir_discHdr *hdr = CoverFlow.getHdr();
	switch(hdr->type)
	{
		case TYPE_CHANNEL:
			catDomain = "NAND";
			break;
		case TYPE_EMUCHANNEL:
			catDomain = "CHANNELS";
			break;
		case TYPE_HOMEBREW:
			catDomain = "HOMEBREW";
			break;
		case TYPE_GC_GAME:
			catDomain = "GAMECUBE";
			break;
		case TYPE_WII_GAME:
			catDomain = "WII";
			break;
		default:
			catDomain = m_plugin.PluginMagicWord;
	}

	memset(id, 0, 64);
	if(NoGameID(hdr->type))
	{
		if(strrchr(hdr->path, '/') != NULL)
			wcstombs(id, hdr->title, sizeof(id) - 1);
		else
			strcpy(id, hdr->path);// scummvm
	}
	else
		strcpy(id, hdr->id);

	const char *gameCats = m_cat.getString(catDomain, id, "").c_str();
	if(strlen(gameCats) > 0)
	{
		for(u8 j = 0; j < strlen(gameCats); ++j)
		{
			int k = (static_cast<int>(gameCats[j])) - 32;
			m_categories.at(k) = '1';
		}
	}
	else
		m_cat.remove(catDomain, id);
	m_btnMgr.setText(m_categoryLblTitle, CoverFlow.getTitle());
}

void CMenu::_setGameCategories(void)
{
	string gameCats = "";
	for(int i = 1; i < m_max_categories; i++)
	{
		if(m_categories.at(i) == '1')
		{
			char cCh = static_cast<char>(i + 32);
			gameCats += cCh;
		}
	}
	m_cat.setString(catDomain, id, gameCats);
}
	
void CMenu::_CategorySettings(bool fromGameSet)
{
	SetupInput();
	curPage = 1;
	gameSet = fromGameSet;
	m_newGame = false;
	
	if(m_source.loaded() && m_catStartPage > 0)
		curPage = m_catStartPage;

	_setCatGenDomain();
	
	m_max_categories = m_cat.getInt(genDomain, "numcategories", 6);
	if(curPage < 1 || curPage > (((m_max_categories - 2)/ 10) + 1))
		curPage = 1;
	m_categories.resize(m_max_categories, '0');
	m_categories.assign(m_max_categories, '0');

	if(fromGameSet)
	{
		_getGameCategories();
	}
	else
	{
		string requiredCats = m_cat.getString(genDomain, "required_categories", "");
		string selectedCats = m_cat.getString(genDomain, "selected_categories", "");
		string hiddenCats = m_cat.getString(genDomain, "hidden_categories", "");
		u8 numReqCats = requiredCats.length();
		u8 numSelCats = selectedCats.length();
		u8 numHidCats = hiddenCats.length();
		
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
				m_cat.setString(genDomain, "selected_categories", newSelCats);
				m_cat.setString(genDomain, "hidden_categories", newHidCats);
				m_cat.setString(genDomain, "required_categories", newReqCats);
			}
			else
			{
				_setGameCategories();
			}
			break;
		}
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		if(BTN_PLUS_PRESSED && fromGameSet)
		{
			_setGameCategories();
			_hideCategorySettings();
			CoverFlow.right();
			curPage = 1;
			m_newGame = true;
			m_categories.assign(m_max_categories, '0');
			_playGameSound();//changes banner and game sound
			_getGameCategories();
			_showCategorySettings();
		}
		if(BTN_MINUS_PRESSED && fromGameSet)
		{
			_setGameCategories();
			_hideCategorySettings();
			CoverFlow.left();
			curPage = 1;
			m_newGame = true;
			m_categories.assign(m_max_categories, '0');
			_playGameSound();
			_getGameCategories();
			_showCategorySettings();
		}
		if((BTN_LEFT_PRESSED && m_max_categories>11) || (BTN_A_PRESSED && m_btnMgr.selected(m_categoryBtnPageM)))
		{
			curPage--;
			if(curPage < 1)
				curPage = ((m_max_categories - 2) / 10) + 1;
			if(BTN_LEFT_PRESSED || BTN_MINUS_PRESSED)
				m_btnMgr.click(m_categoryBtnPageM);
			_updateCatCheckboxes();
		}
		else if((BTN_RIGHT_PRESSED && m_max_categories>11) || (BTN_A_PRESSED && m_btnMgr.selected(m_categoryBtnPageP)))
		{
			curPage++;
			if(curPage > ((m_max_categories - 2) / 10) + 1)
				curPage = 1;
			if(BTN_RIGHT_PRESSED || BTN_PLUS_PRESSED)
				m_btnMgr.click(m_categoryBtnPageP);
			_updateCatCheckboxes();
		}
		if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_categoryBtnClear))
			{
				m_refreshGameList = true;
				bool hiddenCat = false;
				for(int j = 1; j < m_max_categories; ++j)
				{
					if(m_categories.at(j) == '2')
					{	
						hiddenCat = true;
						continue;
					}
					m_categories.at(j) = '0';
				}
				if(!hiddenCat)
					m_categories.at(0) = '1';
				_updateCatCheckboxes();
			}
			for(u8 i = 1; i < 11; ++i)
			{
				m_refreshGameList = true;
				if(m_btnMgr.selected(m_checkboxBtn[i]))
				{
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
					m_btnMgr.hide(m_checkboxBtn[i], true);
					switch(m_categories.at(j))
					{
						case '0':
							m_btnMgr.setBtnTexture(m_checkboxBtn[i], theme.checkboxoff, theme.checkboxoffs, false);
							break;
						case '1':
							m_btnMgr.setBtnTexture(m_checkboxBtn[i], theme.checkboxon, theme.checkboxons, false);
							break;
						case '2':
							m_btnMgr.setBtnTexture(m_checkboxBtn[i], theme.checkboxHid, theme.checkboxHids, false);
							break;
						default:
							m_btnMgr.setBtnTexture(m_checkboxBtn[i], theme.checkboxReq, theme.checkboxReqs, false);
					}
					m_btnMgr.show(m_checkboxBtn[i]);
					m_btnMgr.setSelected(m_checkboxBtn[i]);
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
	m_categoryLblTitle = _addLabel("CATEGORY/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_categoryBtnPageM = _addPicButton("CATEGORY/PAGE_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 20, 400, 48, 48);
	m_categoryLblPage = _addLabel("CATEGORY/PAGE_BTN", theme.btnFont, L"", 68, 400, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_categoryBtnPageP = _addPicButton("CATEGORY/PAGE_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 172, 400, 48, 48);
	m_categoryBtnBack = _addButton("CATEGORY/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);
	m_categoryBtnClear = _addButton("CATEGORY/CLEAR_BTN", theme.btnFont, L"", 230, 400, 180, 48, theme.btnFontColor);
	
	_setHideAnim(m_categoryLblTitle, "CATEGORY/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_categoryLblPage, "CATEGORY/PAGE_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_categoryBtnPageM, "CATEGORY/PAGE_MINUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_categoryBtnPageP, "CATEGORY/PAGE_PLUS", 0, 0, 1.f, -1.f);
	_setHideAnim(m_categoryBtnClear, "CATEGORY/CLEAR_BTN", 0, 0, 1.f, -1.f);
	_setHideAnim(m_categoryBtnBack, "CATEGORY/BACK_BTN", 0, 0, 1.f, -1.f);
	
	_hideCategorySettings(true);
	_textCategorySettings();
}

void CMenu::_textCategorySettings(void)
{
	m_btnMgr.setText(m_categoryBtnClear, _t("cat2", L"Clear"));
	m_btnMgr.setText(m_categoryBtnBack, _t("cd1", L"Back"));
}
