
#include "menu.hpp"
#include "gui/GameTDB.hpp"

wstringEx gameinfo_Synopsis_w;
wstringEx gameinfo_Title_w;

bool titlecheck = false;
u8 cnt_controlsreq = 0, cnt_controls = 0;
const int pixels_to_skip = 10;

void CMenu::_gameinfo(void)
{ 
	bool first = true;
	SetupInput();
	_showGameInfo();

	u8 page = 0;
	
	int amount_of_skips = 0;
	
	int synopsis_x = 0, synopsis_y = 0;
	u32 synopsis_w = 0, synopsis_h = 0;

	do
	{
		_mainLoopCommon();

		if (amount_of_skips == 0)
		{
			// Check dimensions in the loop, because the animation can have an effect
			m_btnMgr.getDimensions(m_gameinfoLblSynopsis, synopsis_x, synopsis_y, synopsis_w, synopsis_h); // Get original dimensions
		}	
		if(first && page == 1)
		{
			m_btnMgr.moveBy(m_gameinfoLblSynopsis, 0, -1);
			amount_of_skips++;
			first = false;
		}

		if ((BTN_DOWN_PRESSED || BTN_DOWN_HELD) && !(m_thrdWorking && m_thrdStop) && page == 1)
		{
			if (synopsis_h - (amount_of_skips * pixels_to_skip) > (m_vid.height2D() - (35 + synopsis_y)))
			  {
				m_btnMgr.moveBy(m_gameinfoLblSynopsis, 0, -pixels_to_skip);
				amount_of_skips++;
			}
		}
		else if ((BTN_UP_PRESSED || BTN_UP_HELD) && !(m_thrdWorking && m_thrdStop) && page == 1)
		{
			if (amount_of_skips > 1)
			{
				m_btnMgr.moveBy(m_gameinfoLblSynopsis, 0, pixels_to_skip);
				amount_of_skips--;
			}
		}
		else if (BTN_RIGHT_PRESSED && !(m_thrdWorking && m_thrdStop) && page == 0 && !gameinfo_Synopsis_w.empty())
		{
			page = 1;
			amount_of_skips = 0;

			m_btnMgr.hide(m_gameinfoLblID, true);
			m_btnMgr.hide(m_gameinfoLblDev, true);
			m_btnMgr.hide(m_gameinfoLblRegion, true);
			m_btnMgr.hide(m_gameinfoLblPublisher, true);
			m_btnMgr.hide(m_gameinfoLblRlsdate, true);
			m_btnMgr.hide(m_gameinfoLblGenre, true);
			m_btnMgr.hide(m_gameinfoLblRating, true);
			m_btnMgr.hide(m_gameinfoLblWifiplayers, true);

			for(u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControlsReq); ++i)
				if(m_gameinfoLblControlsReq[i] != -1)
					m_btnMgr.hide(m_gameinfoLblControlsReq[i], true);

			for(u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControls); ++i)
				if(m_gameinfoLblControls[i] != -1)
					m_btnMgr.hide(m_gameinfoLblControls[i], true);

			// When showing synopsis, only show user labels 2 and 3
			for(u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblUser); ++i)
				if(i < ARRAY_SIZE(m_gameinfoLblUser) / 2)
					m_btnMgr.hide(m_gameinfoLblUser[i], true);
				else
					m_btnMgr.show(m_gameinfoLblUser[i]);

			m_btnMgr.reset(m_gameinfoLblSynopsis);
			m_btnMgr.show(m_gameinfoLblSynopsis, false);
		}
		else if (BTN_LEFT_PRESSED && !(m_thrdWorking && m_thrdStop))
		{
			page = 0;
			first = true;
			m_btnMgr.show(m_gameinfoLblID);
			m_btnMgr.show(m_gameinfoLblDev);
			m_btnMgr.show(m_gameinfoLblRegion);	
			m_btnMgr.show(m_gameinfoLblPublisher);
			m_btnMgr.show(m_gameinfoLblRlsdate);
			m_btnMgr.show(m_gameinfoLblGenre);
			m_btnMgr.show(m_gameinfoLblRating);
			m_btnMgr.show(m_gameinfoLblWifiplayers);

			for(u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControlsReq); ++i)
				if(m_gameinfoLblControlsReq[i] != -1 && i < cnt_controlsreq)
					m_btnMgr.show(m_gameinfoLblControlsReq[i]);

			for(u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControls); ++i)
				if(m_gameinfoLblControls[i] != -1 && i < cnt_controls)
					m_btnMgr.show(m_gameinfoLblControls[i]);

			// When showing synopsis, only show user labels 2 and 3
			for(u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblUser); ++i)
				if(i < ARRAY_SIZE(m_gameinfoLblUser) / 2)
					m_btnMgr.show(m_gameinfoLblUser[i]);
				else
					m_btnMgr.hide(m_gameinfoLblUser[i], true);

			m_btnMgr.hide(m_gameinfoLblSynopsis,true);
		}

	} while (!BTN_HOME_PRESSED && !BTN_B_PRESSED);

	_hideGameInfo(false);
}

void CMenu::_hideGameInfo(bool instant)
{
	m_btnMgr.hide(m_gameinfoLblID, instant);
	m_btnMgr.hide(m_gameinfoLblTitle, instant);
	m_btnMgr.hide(m_gameinfoLblSynopsis, instant);
	m_btnMgr.hide(m_gameinfoLblDev, instant);
	m_btnMgr.hide(m_gameinfoLblRegion, instant);
	m_btnMgr.hide(m_gameinfoLblPublisher, instant);
	m_btnMgr.hide(m_gameinfoLblRlsdate, instant);
	m_btnMgr.hide(m_gameinfoLblGenre, instant);
	m_btnMgr.hide(m_gameinfoLblRating, instant);
	m_btnMgr.hide(m_gameinfoLblWifiplayers, instant);
	
	for(u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControlsReq); ++i)
		if(m_gameinfoLblControlsReq[i] != -1)
			m_btnMgr.hide(m_gameinfoLblControlsReq[i], instant);

	for(u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblUser); ++i)
		m_btnMgr.hide(m_gameinfoLblUser[i], instant);

	for(u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControls); ++i)
		if(m_gameinfoLblControls[i] != -1)
			m_btnMgr.hide(m_gameinfoLblControls[i], instant);
}

void CMenu::_showGameInfo(void)
{
	_setBg(m_gameinfoBg, m_gameinfoBg);

	_textGameInfo();
	
	if(titlecheck)
	{
		m_btnMgr.show(m_gameinfoLblID);
		m_btnMgr.show(m_gameinfoLblTitle);
		m_btnMgr.show(m_gameinfoLblRating);
		m_btnMgr.show(m_gameinfoLblRegion);	
		m_btnMgr.show(m_gameinfoLblDev);
		m_btnMgr.show(m_gameinfoLblPublisher);
		m_btnMgr.show(m_gameinfoLblRlsdate);
		m_btnMgr.show(m_gameinfoLblGenre);
		m_btnMgr.show(m_gameinfoLblWifiplayers);

		for(u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblUser); ++i)
			if(i < ARRAY_SIZE(m_gameinfoLblUser) / 2)
				m_btnMgr.show(m_gameinfoLblUser[i]);
		
		for(u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControlsReq); ++i)
			if(m_gameinfoLblControlsReq[i] != -1 && i < cnt_controlsreq)
				m_btnMgr.show(m_gameinfoLblControlsReq[i]);
			
		for(u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControls); ++i)
			if(m_gameinfoLblControls[i] != -1 && i < cnt_controls)
				m_btnMgr.show(m_gameinfoLblControls[i]);
	}
}

void CMenu::_initGameInfoMenu()
{
	TexData emptyTex;
	_addUserLabels(m_gameinfoLblUser, 0, 1, "GAMEINFO");
	_addUserLabels(m_gameinfoLblUser, 2, 1, "GAMEINFO");
	
	m_gameinfoBg = _texture("GAMEINFO/BG", "texture", theme.bg, false);
	m_gameinfoLblID = _addText("GAMEINFO/GAMEID", theme.txtFont, L"", 40, 110, 420, 75, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_gameinfoLblGenre = _addText("GAMEINFO/GENRE", theme.txtFont, L"", 40, 140, 460, 56, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_gameinfoLblDev = _addText("GAMEINFO/DEVELOPER", theme.txtFont, L"", 40, 170, 460, 56, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_gameinfoLblPublisher = _addText("GAMEINFO/PUBLISHER", theme.txtFont, L"", 40, 200, 460, 56, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_gameinfoLblRlsdate = _addText("GAMEINFO/RLSDATE", theme.txtFont, L"", 40, 230, 460, 56, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_gameinfoLblRegion = _addText("GAMEINFO/REGION", theme.txtFont, L"", 40, 260, 460, 56, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_gameinfoLblRating = _addLabel("GAMEINFO/RATING", theme.titleFont, L"", 550, 380, 48, 60, theme.titleFontColor, 0, m_rating);
	m_gameinfoLblSynopsis = _addText("GAMEINFO/SYNOPSIS", theme.txtFont, L"", 40, 120, 560, 280, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	m_gameinfoLblWifiplayers = _addLabel("GAMEINFO/WIFIPLAYERS", theme.txtFont, L"", 550, 110, 68, 60, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP,m_wifi);

	_addUserLabels(m_gameinfoLblUser, 1, 1, "GAMEINFO");
	_addUserLabels(m_gameinfoLblUser, 3, 2, "GAMEINFO");

	m_gameinfoLblTitle = _addTitle("GAMEINFO/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);

	for(u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControlsReq); ++i)
	{
		string dom(fmt("GAMEINFO/CONTROLSREQ%i", i + 1));
		m_gameinfoLblControlsReq[i] = _addLabel(dom.c_str(), theme.txtFont, L"", 40 + (i*60), 310, 60, 40, theme.txtFontColor, 0, emptyTex);
		_setHideAnim(m_gameinfoLblControlsReq[i], dom.c_str(), 0, -100, 0.f, 0.f);
	}

	for(u8 i = 0; i < ARRAY_SIZE(m_gameinfoLblControls); ++i)
	{
		string dom(fmt("GAMEINFO/CONTROLS%i", i + 1));
		m_gameinfoLblControls[i] = _addLabel(dom.c_str(), theme.txtFont, L"", 40 + (i*60), 380, 60, 40, theme.txtFontColor, 0, emptyTex);
		_setHideAnim(m_gameinfoLblControls[i], dom.c_str(), 0, -100, 0.f, 0.f);
	}
	// 
	_setHideAnim(m_gameinfoLblID, "GAMEINFO/GAMEID",0, -100, 0.f, 0.f);
	_setHideAnim(m_gameinfoLblTitle, "GAMEINFO/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_gameinfoLblRating, "GAMEINFO/RATING", 0, -100, 0.f, 0.f);
	_setHideAnim(m_gameinfoLblSynopsis, "GAMEINFO/SYNOPSIS", 0, 700, 1.f, 1.f);
	_setHideAnim(m_gameinfoLblRegion, "GAMEINFO/REGION", 0, -100, 0.f, 0.f);
	_setHideAnim(m_gameinfoLblDev, "GAMEINFO/DEVELOPER", 0, -100, 0.f, 0.f);
	_setHideAnim(m_gameinfoLblPublisher, "GAMEINFO/PUBLISHER", 0, -100, 0.f, 0.f);
	_setHideAnim(m_gameinfoLblRlsdate, "GAMEINFO/RLSDATE", 0, -100, 0.f, 0.f);
	_setHideAnim(m_gameinfoLblGenre, "GAMEINFO/GENRE", 0, -100, 0.f, 0.f);
	_setHideAnim(m_gameinfoLblWifiplayers, "GAMEINFO/WIFIPLAYERS", 0, -100, 0.f, 0.f);
	// 
	_hideGameInfo(true);
}

void CMenu::_textGameInfo(void)
{
	cnt_controlsreq = 0;
	cnt_controls = 0;

	GameTDB gametdb;
	gametdb.OpenFile(fmt("%s/wiitdb.xml", m_settingsDir.c_str()));
	gametdb.SetLanguageCode(m_loc.getString(m_curLanguage, "gametdb_code", "EN").c_str());
	const char *TMP_Char = NULL;
	titlecheck = gametdb.IsLoaded();
	if(titlecheck)
	{
		char GameID[7];
		GameID[6] = '\0';
		strncpy(GameID, CoverFlow.getId(), 6);
		if(gametdb.GetTitle(GameID, TMP_Char))
		{
			gameinfo_Title_w.fromUTF8(TMP_Char);
			m_btnMgr.setText(m_gameinfoLblTitle, gameinfo_Title_w);
		}
		if(gametdb.GetSynopsis(GameID, TMP_Char))
		{
			gameinfo_Synopsis_w.fromUTF8(TMP_Char);
			m_btnMgr.setText(m_gameinfoLblSynopsis, gameinfo_Synopsis_w);
		}
		m_btnMgr.setText(m_gameinfoLblID, wfmt(L"GameID: %s", GameID), true);
		if(gametdb.GetDeveloper(GameID, TMP_Char))
			m_btnMgr.setText(m_gameinfoLblDev, wfmt(_fmt("gameinfo1",L"Developer: %s"), TMP_Char), true);
		if(gametdb.GetPublisher(GameID, TMP_Char))
			m_btnMgr.setText(m_gameinfoLblPublisher, wfmt(_fmt("gameinfo2",L"Publisher: %s"), TMP_Char), true);
		if(gametdb.GetRegion(GameID, TMP_Char))
			m_btnMgr.setText(m_gameinfoLblRegion, wfmt(_fmt("gameinfo3",L"Region: %s"), TMP_Char), true);
		if(gametdb.GetGenres(GameID, TMP_Char))
			m_btnMgr.setText(m_gameinfoLblGenre, wfmt(_fmt("gameinfo5",L"Genre: %s"), TMP_Char), true);

		int PublishDate = gametdb.GetPublishDate(GameID);
		int year = PublishDate >> 16;
		int day = PublishDate & 0xFF;
		int month = (PublishDate >> 8) & 0xFF;
		switch(CONF_GetRegion())
		{
			case 0:
			case 4:
			case 5:
				m_btnMgr.setText(m_gameinfoLblRlsdate, wfmt(_fmt("gameinfo4",L"Release Date: %i-%i-%i"), year, month, day), true);
				break;
			case 1:
				m_btnMgr.setText(m_gameinfoLblRlsdate, wfmt(_fmt("gameinfo4",L"Release Date: %i-%i-%i"), month, day, year), true);
				break;
			case 2:
				m_btnMgr.setText(m_gameinfoLblRlsdate, wfmt(_fmt("gameinfo4",L"Release Date: %i-%i-%i"), day, month, year), true);
				break;
		}
		//Ratings
		TexHandle.fromImageFile(m_rating, fmt("%s/norating.png", m_imgsDir.c_str()));
		const char *RatingValue = NULL;
		if(gametdb.GetRatingValue(GameID, RatingValue))
		{
			switch(gametdb.GetRating(GameID))
			{
				case GAMETDB_RATING_TYPE_CERO:
					if(RatingValue[0] == 'A')
						TexHandle.fromImageFile(m_rating, fmt("%s/cero_a.png", m_imgsDir.c_str()));
					else if(RatingValue[0] == 'B')
						TexHandle.fromImageFile(m_rating, fmt("%s/cero_b.png", m_imgsDir.c_str()));
					else if(RatingValue[0] == 'D')
						TexHandle.fromImageFile(m_rating, fmt("%s/cero_d.png", m_imgsDir.c_str()));
					else if(RatingValue[0] == 'C')
						TexHandle.fromImageFile(m_rating, fmt("%s/cero_c.png", m_imgsDir.c_str()));
					else if(RatingValue[0] == 'Z')
						TexHandle.fromImageFile(m_rating, fmt("%s/cero_z.png", m_imgsDir.c_str()));
					break;
				case GAMETDB_RATING_TYPE_ESRB:
					if(RatingValue[0] == 'E')
						TexHandle.fromImageFile(m_rating, fmt("%s/esrb_e.jpg", m_imgsDir.c_str()));
					else if(memcmp(RatingValue, "EC", 2) == 0)
						TexHandle.fromImageFile(m_rating, fmt("%s/esrb_ec.jpg", m_imgsDir.c_str()));
					else if(memcmp(RatingValue, "E10+", 4) == 0)
						TexHandle.fromImageFile(m_rating, fmt("%s/esrb_eten.jpg", m_imgsDir.c_str()));
					else if(RatingValue[0] == 'T')
						TexHandle.fromImageFile(m_rating, fmt("%s/esrb_t.jpg", m_imgsDir.c_str()));
					else if(RatingValue[0] == 'M')
						TexHandle.fromImageFile(m_rating, fmt("%s/esrb_m.jpg", m_imgsDir.c_str()));
					else if(memcmp(RatingValue, "AO", 2) == 0)
						TexHandle.fromImageFile(m_rating, fmt("%s/esrb_ao.jpg", m_imgsDir.c_str()));
					break;
				case GAMETDB_RATING_TYPE_PEGI:
					if(RatingValue[0] == '3')
						TexHandle.fromImageFile(m_rating, fmt("%s/pegi_3.png", m_imgsDir.c_str()));
					else if(RatingValue[0] == '7')
						TexHandle.fromImageFile(m_rating, fmt("%s/pegi_7.png", m_imgsDir.c_str()));
					else if(memcmp(RatingValue, "12", 2) == 0)
						TexHandle.fromImageFile(m_rating, fmt("%s/pegi_12.png", m_imgsDir.c_str()));
					else if(memcmp(RatingValue, "16", 2) == 0)
						TexHandle.fromImageFile(m_rating, fmt("%s/pegi_16.png", m_imgsDir.c_str()));
					else if(memcmp(RatingValue, "18", 2) == 0)
						TexHandle.fromImageFile(m_rating, fmt("%s/pegi_18.png", m_imgsDir.c_str()));
					break;
				case GAMETDB_RATING_TYPE_GRB:
					if(RatingValue[0] == 'A')
						TexHandle.fromImageFile(m_rating, fmt("%s/grb_a.png", m_imgsDir.c_str()));
					else if(memcmp(RatingValue, "12", 2) == 0)
						TexHandle.fromImageFile(m_rating, fmt("%s/grb_12.png", m_imgsDir.c_str()));
					else if(memcmp(RatingValue, "15", 2) == 0)
						TexHandle.fromImageFile(m_rating, fmt("%s/grb_15.png", m_imgsDir.c_str()));
					else if(memcmp(RatingValue, "18", 2) == 0)
						TexHandle.fromImageFile(m_rating, fmt("%s/grb_18.png", m_imgsDir.c_str()));
					break;
				default:
					break;
			}
		}
		m_btnMgr.setTexture(m_gameinfoLblRating, m_rating);
		//Wifi players
		int WifiPlayers = gametdb.GetWifiPlayers(GameID);
		TexData emptyTex;
		if(WifiPlayers == 1)
			TexHandle.fromImageFile(m_wifi, fmt("%s/wifi1.png", m_imgsDir.c_str()));
		else if(WifiPlayers == 2)
			TexHandle.fromImageFile(m_wifi, fmt("%s/wifi2.png", m_imgsDir.c_str()));
		else if(WifiPlayers == 4)
			TexHandle.fromImageFile(m_wifi, fmt("%s/wifi4.png", m_imgsDir.c_str()));
		else if(WifiPlayers == 8)
			TexHandle.fromImageFile(m_wifi, fmt("%s/wifi8.png", m_imgsDir.c_str()));
		else if(WifiPlayers == 10)
			TexHandle.fromImageFile(m_wifi, fmt("%s/wifi10.png", m_imgsDir.c_str()));
		else if(WifiPlayers == 12)
			TexHandle.fromImageFile(m_wifi, fmt("%s/wifi12.png", m_imgsDir.c_str()));
		else if(WifiPlayers == 16)
			TexHandle.fromImageFile(m_wifi, fmt("%s/wifi16.png", m_imgsDir.c_str()));
		else if(WifiPlayers == 18)
			TexHandle.fromImageFile(m_wifi, fmt("%s/wifi18.png", m_imgsDir.c_str()));
		else if(WifiPlayers == 32)
			TexHandle.fromImageFile(m_wifi, fmt("%s/wifi32.png", m_imgsDir.c_str()));
		if(WifiPlayers > 0)
			m_btnMgr.setTexture(m_gameinfoLblWifiplayers, m_wifi);
		else 
			m_btnMgr.setTexture(m_gameinfoLblWifiplayers, emptyTex);

		//check required controlls
		bool wiimote = false;
		bool nunchuk = false;
		bool classiccontroller = false;
		bool balanceboard = false;
		bool dancepad = false;
		bool guitar = false;
		bool gamecube = false;
		bool motionplus = false;
		bool drums = false;
		bool microphone = false;
		bool wheel = false;
		bool keyboard = false;
		bool udraw = false;
		bool zapper = false;

		vector<Accessory> Accessories;
		gametdb.GetAccessories(GameID, Accessories);
		for(vector<Accessory>::iterator acc_itr = Accessories.begin(); acc_itr != Accessories.end(); acc_itr++)
		{
			if(!acc_itr->Required)
				continue;
			if(strcmp((acc_itr->Name).c_str(), "wiimote") == 0)
				wiimote = true;
			else if(strcmp((acc_itr->Name).c_str(), "nunchuk") == 0)
				nunchuk = true;
			else if(strcmp((acc_itr->Name).c_str(), "guitar") == 0)
				guitar = true;
			else if(strcmp((acc_itr->Name).c_str(), "drums") == 0)
				drums = true;
			else if(strcmp((acc_itr->Name).c_str(), "dancepad") == 0)
				dancepad = true;
			else if(strcmp((acc_itr->Name).c_str(), "motionplus") == 0)
				motionplus = true;
			else if(strcmp((acc_itr->Name).c_str(), "microphone") == 0)
				microphone = true;
			else if(strcmp((acc_itr->Name).c_str(), "balanceboard") == 0)
				balanceboard = true;
			else if(strcmp((acc_itr->Name).c_str(), "udraw") == 0)
				udraw = true;
		}
		u8 x = 0;
		u8 max_controlsReq = ARRAY_SIZE(m_gameinfoLblControlsReq);
		
		if(wiimote && x < max_controlsReq)
		{
			u8 players = gametdb.GetPlayers(GameID);
			if(players >= 10)
				players /= 10;
			if(players == 1)
				TexHandle.fromImageFile(m_controlsreq[x], fmt("%s/wiimote1.png", m_imgsDir.c_str()));
			else if(players == 2)
				TexHandle.fromImageFile(m_controlsreq[x], fmt("%s/wiimote2.png", m_imgsDir.c_str()));
			else if(players == 3)
				TexHandle.fromImageFile(m_controlsreq[x], fmt("%s/wiimote3.png", m_imgsDir.c_str()));
			else if(players == 4)
				TexHandle.fromImageFile(m_controlsreq[x], fmt("%s/wiimote4.png", m_imgsDir.c_str()));
			else if(players == 6)
				TexHandle.fromImageFile(m_controlsreq[x], fmt("%s/wiimote6.png", m_imgsDir.c_str()));
			else if(players == 8)
				TexHandle.fromImageFile(m_controlsreq[x], fmt("%s/wiimote8.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControlsReq[x] ,m_controlsreq[x], 20, 60);
			x++;
		}
		if(nunchuk && x < max_controlsReq)
		{
			TexHandle.fromImageFile(m_controlsreq[x], fmt("%s/nunchukR.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControlsReq[x] ,m_controlsreq[x], 52, 60);
			x++;
		}
		if(guitar && x < max_controlsReq)
		{
			TexHandle.fromImageFile(m_controlsreq[x], fmt("%s/guitarR.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControlsReq[x] ,m_controlsreq[x], 52, 60);
			x++;
		}
		if(drums && x < max_controlsReq)
		{
			TexHandle.fromImageFile(m_controlsreq[x], fmt("%s/drumsR.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControlsReq[x] ,m_controlsreq[x], 52, 60);
			x++;
		}
		if(motionplus && x < max_controlsReq)
		{
			TexHandle.fromImageFile(m_controlsreq[x], fmt("%s/motionplusR.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControlsReq[x] ,m_controlsreq[x], 20, 60);
			x++;
		}
		if(dancepad && x < max_controlsReq)
		{
			TexHandle.fromImageFile(m_controlsreq[x], fmt("%s/dancepadR.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControlsReq[x] ,m_controlsreq[x], 52, 60);
			x++;
		}
		if(microphone && x < max_controlsReq)
		{
			TexHandle.fromImageFile(m_controlsreq[x], fmt("%s/microphoneR.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControlsReq[x] ,m_controlsreq[x], 52, 60);
			x++;
		}
		if(balanceboard && x < max_controlsReq)
		{
			TexHandle.fromImageFile(m_controlsreq[x], fmt("%s/balanceboardR.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControlsReq[x] ,m_controlsreq[x], 52, 60);
			x++;
		}
		if(udraw && x < max_controlsReq)
		{
			TexHandle.fromImageFile(m_controlsreq[x], fmt("%s/udrawR.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControlsReq[x] ,m_controlsreq[x], 52, 60);
			x++;
		}
		cnt_controlsreq = x;

		//check optional controlls
		wiimote = false;
		nunchuk = false;
		classiccontroller = false;
		balanceboard = false;
		dancepad = false;
		guitar = false;
		gamecube = false;
		motionplus = false;
		drums = false;
		microphone = false;
		wheel = false;
		keyboard = false;
		udraw = false;
		zapper = false;
		for(vector<Accessory>::iterator acc_itr = Accessories.begin(); acc_itr != Accessories.end(); acc_itr++)
		{
			if(acc_itr->Required)
				continue;
			if(strcmp((acc_itr->Name).c_str(), "classiccontroller") == 0)
				classiccontroller = true;
			else if(strcmp((acc_itr->Name).c_str(), "nunchuk") == 0)
				nunchuk = true;
			else if(strcmp((acc_itr->Name).c_str(), "guitar") == 0)
				guitar = true;
			else if(strcmp((acc_itr->Name).c_str(), "drums") == 0)
				drums = true;
			else if(strcmp((acc_itr->Name).c_str(), "dancepad") == 0)
				dancepad = true;
			else if(strcmp((acc_itr->Name).c_str(), "motionplus") == 0)
				motionplus = true;
			else if(strcmp((acc_itr->Name).c_str(), "balanceboard") == 0)
				balanceboard = true;
			else if(strcmp((acc_itr->Name).c_str(), "microphone") == 0)
				microphone = true;
			else if(strcmp((acc_itr->Name).c_str(), "gamecube") == 0)
				gamecube = true;
			else if(strcmp((acc_itr->Name).c_str(), "keyboard") == 0)
				keyboard = true;
			else if(strcmp((acc_itr->Name).c_str(), "zapper") == 0)
				zapper = true;
			else if(strcmp((acc_itr->Name).c_str(), "wheel") == 0)
				wheel = true;
			else if(strcmp((acc_itr->Name).c_str(), "udraw") == 0)
				udraw = true;
		}
		x = 0;
		u8 max_controls = ARRAY_SIZE(m_gameinfoLblControls);
		if(classiccontroller && x < max_controls)
		{
			TexHandle.fromImageFile(m_controls[x], fmt("%s/classiccontroller.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 52, 60);
			x++;
		}
		if(nunchuk && x < max_controls)
		{
			TexHandle.fromImageFile(m_controls[x], fmt("%s/nunchuk.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 52, 60);
			x++;
		}
		if(guitar && x < max_controls)
		{
			TexHandle.fromImageFile(m_controls[x], fmt("%s/guitar.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 52, 60);
			x++;
		}
		if(drums && x < max_controls)
		{
			TexHandle.fromImageFile(m_controls[x], fmt("%s/drums.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 52, 60);
			x++;
		}
		if(dancepad && x < max_controls)
		{
			TexHandle.fromImageFile(m_controls[x], fmt("%s/dancepad.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 52, 60);
			x++;
		}
		if(motionplus && x < max_controls)
		{
			TexHandle.fromImageFile(m_controls[x], fmt("%s/motionplus.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 20, 60);
			x++;
		}
		if(balanceboard && x < max_controls)
		{
			TexHandle.fromImageFile(m_controls[x], fmt("%s/balanceboard.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 52, 60);
			x++;
		}
		if(microphone && x < max_controls)
		{
			TexHandle.fromImageFile(m_controls[x], fmt("%s/microphone.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 48, 60);
			x++;
		}
		if(gamecube && x < max_controls)
		{
			TexHandle.fromImageFile(m_controls[x], fmt("%s/gcncontroller.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 48, 60);
			x++;
		}
		if(keyboard && x < max_controls)
		{
			TexHandle.fromImageFile(m_controls[x], fmt("%s/keyboard.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 52, 60);
			x++;
		}
		if(udraw && x < max_controls)
		{
			TexHandle.fromImageFile(m_controls[x], fmt("%s/udraw.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 52, 60);
			x++;
		}
		if(zapper && x < max_controls)
		{
			TexHandle.fromImageFile(m_controls[x], fmt("%s/zapper.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 52, 70);
			x++;
		}
		if(wheel && x < max_controls)
		{
			TexHandle.fromImageFile(m_controls[x], fmt("%s/wheel.png", m_imgsDir.c_str()));
			m_btnMgr.setTexture(m_gameinfoLblControls[x] ,m_controls[x], 52, 60);
			x++;
		}
		cnt_controls = x;
	}
	else
		m_btnMgr.setText(m_gameinfoLblTitle, wfmt(_fmt("gameinfo6",L"No Gameinfo"), true));

	gametdb.CloseFile();
}
