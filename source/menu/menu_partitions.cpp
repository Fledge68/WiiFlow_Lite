
#include "menu.hpp"

s16 m_partitionsLblTitle;
s16 m_partitionsBtnBack;
s16 m_partitionsLblUser[4];

s16 m_partitionsLblWii;
s16 m_partitionsLblWiiVal;
s16 m_partitionsBtnWiiP;
s16 m_partitionsBtnWiiM;

s16 m_partitionsLblGC;
s16 m_partitionsLblGCVal;
s16 m_partitionsBtnGCP;
s16 m_partitionsBtnGCM;

s16 m_partitionsLblChannels;
s16 m_partitionsLblChannelsVal;
s16 m_partitionsBtnChannelsP;
s16 m_partitionsBtnChannelsM;

s16 m_partitionsLblPlugin;
s16 m_partitionsLblPluginVal;
s16 m_partitionsBtnPluginP;
s16 m_partitionsBtnPluginM;

TexData m_partitionsBg;

void CMenu::_hidePartitionsCfg(bool instant)
{
	m_btnMgr.hide(m_partitionsLblTitle, instant);
	m_btnMgr.hide(m_partitionsBtnBack, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_partitionsLblUser); ++i)
		if(m_partitionsLblUser[i] != -1)
			m_btnMgr.hide(m_partitionsLblUser[i], instant);

	m_btnMgr.hide(m_partitionsLblWii, instant);
	m_btnMgr.hide(m_partitionsLblWiiVal, instant);
	m_btnMgr.hide(m_partitionsBtnWiiP, instant);
	m_btnMgr.hide(m_partitionsBtnWiiM, instant);
	m_btnMgr.hide(m_partitionsLblGC, instant);
	m_btnMgr.hide(m_partitionsLblGCVal, instant);
	m_btnMgr.hide(m_partitionsBtnGCP, instant);
	m_btnMgr.hide(m_partitionsBtnGCM, instant);
	m_btnMgr.hide(m_partitionsLblChannels, instant);
	m_btnMgr.hide(m_partitionsLblChannelsVal, instant);
	m_btnMgr.hide(m_partitionsBtnChannelsP, instant);
	m_btnMgr.hide(m_partitionsBtnChannelsM, instant);
	m_btnMgr.hide(m_partitionsLblPlugin, instant);
	m_btnMgr.hide(m_partitionsLblPluginVal, instant);
	m_btnMgr.hide(m_partitionsBtnPluginP, instant);
	m_btnMgr.hide(m_partitionsBtnPluginM, instant);
}

void CMenu::_showPartitionsCfg(void)
{
	m_btnMgr.show(m_partitionsLblTitle);
	m_btnMgr.show(m_partitionsBtnBack);
	for(u8 i = 0; i < ARRAY_SIZE(m_partitionsLblUser); ++i)
		if(m_partitionsLblUser[i] != -1)
			m_btnMgr.show(m_partitionsLblUser[i]);

	m_btnMgr.show(m_partitionsLblWii);
	m_btnMgr.show(m_partitionsLblWiiVal);
	m_btnMgr.show(m_partitionsBtnWiiP);
	m_btnMgr.show(m_partitionsBtnWiiM);
	m_btnMgr.show(m_partitionsLblGC);
	m_btnMgr.show(m_partitionsLblGCVal);
	m_btnMgr.show(m_partitionsBtnGCP);
	m_btnMgr.show(m_partitionsBtnGCM);
	m_btnMgr.show(m_partitionsLblChannels);
	m_btnMgr.show(m_partitionsLblChannelsVal);
	m_btnMgr.show(m_partitionsBtnChannelsP);
	m_btnMgr.show(m_partitionsBtnChannelsM);
	m_btnMgr.show(m_partitionsLblPlugin);
	m_btnMgr.show(m_partitionsLblPluginVal);
	m_btnMgr.show(m_partitionsBtnPluginP);
	m_btnMgr.show(m_partitionsBtnPluginM);

	const char *partitionname = DeviceName[m_cfg.getInt(WII_DOMAIN, "partition", 0)];
	m_btnMgr.setText(m_partitionsLblWiiVal, upperCase(partitionname));
	partitionname = DeviceName[m_cfg.getInt(GC_DOMAIN, "partition", 0)];
	m_btnMgr.setText(m_partitionsLblGCVal, upperCase(partitionname));
	partitionname = DeviceName[m_cfg.getInt(CHANNEL_DOMAIN, "partition", 0)];
	m_btnMgr.setText(m_partitionsLblChannelsVal, upperCase(partitionname));
	partitionname = DeviceName[m_cfg.getInt(PLUGIN_DOMAIN, "partition", 0)];
	m_btnMgr.setText(m_partitionsLblPluginVal, upperCase(partitionname));
}

void CMenu::_partitionsCfg(void)
{
	m_prev_view = m_current_view;
	int prevPartition = currentPartition;
	SetupInput();
	_setBg(m_partitionsBg, m_partitionsBg);
	_showPartitionsCfg();
	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_PRESSED)
			break;
		else if(BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if(BTN_A_PRESSED)
		{
			if(m_btnMgr.selected(m_partitionsBtnBack))
				break;
			else if(m_btnMgr.selected(m_partitionsBtnWiiP) || m_btnMgr.selected(m_partitionsBtnWiiM))
			{
				s8 direction = m_btnMgr.selected(m_partitionsBtnWiiP) ? 1 : -1;
				currentPartition = m_cfg.getInt(WII_DOMAIN, "partition");
				m_current_view = COVERFLOW_WII;
				_setPartition(direction);
				_showPartitionsCfg();
				if(m_prev_view & COVERFLOW_WII)
					m_refreshGameList = true;
			}
			else if(m_btnMgr.selected(m_partitionsBtnGCP) || m_btnMgr.selected(m_partitionsBtnGCM))
			{
				s8 direction = m_btnMgr.selected(m_partitionsBtnGCP) ? 1 : -1;
				currentPartition = m_cfg.getInt(GC_DOMAIN, "partition");
				m_current_view = COVERFLOW_GAMECUBE;
				_setPartition(direction);
				_showPartitionsCfg();
				if(m_prev_view & COVERFLOW_GAMECUBE)
					m_refreshGameList = true;
			}
			else if(m_btnMgr.selected(m_partitionsBtnChannelsP) || m_btnMgr.selected(m_partitionsBtnChannelsM))
			{
				s8 direction = m_btnMgr.selected(m_partitionsBtnChannelsP) ? 1 : -1;
				currentPartition = m_cfg.getInt(CHANNEL_DOMAIN, "partition");
				m_current_view = COVERFLOW_CHANNEL;
				_setPartition(direction);
				_showPartitionsCfg();
				if(m_prev_view & COVERFLOW_CHANNEL)
					m_refreshGameList = true;
			}
			else if(m_btnMgr.selected(m_partitionsBtnPluginP) || m_btnMgr.selected(m_partitionsBtnPluginM))
			{
				s8 direction = m_btnMgr.selected(m_partitionsBtnPluginP) ? 1 : -1;
				currentPartition = m_cfg.getInt(PLUGIN_DOMAIN, "partition");
				m_current_view = COVERFLOW_PLUGIN;
				_setPartition(direction);
				_showPartitionsCfg();
				if(m_prev_view & COVERFLOW_PLUGIN)
					m_refreshGameList = true;
			}
		}
	}
	m_current_view = m_prev_view;
	m_prev_view = 0;
	currentPartition = prevPartition;
	_hidePartitionsCfg();
}

void CMenu::_initPartitionsCfgMenu()
{
	m_partitionsBg = _texture("PARTCFG/BG", "texture", theme.bg, false);
	_addUserLabels(m_partitionsLblUser, ARRAY_SIZE(m_partitionsLblUser), "PARTCFG");
	m_partitionsLblTitle = _addTitle("PARTCFG/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_partitionsBtnBack = _addButton("PARTCFG/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);

	m_partitionsLblWii = _addLabel("PARTCFG/WII", theme.lblFont, L"", 20, 125, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_partitionsLblWiiVal = _addLabel("PARTCFG/WII_BTN", theme.btnFont, L"", 468, 130, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_partitionsBtnWiiM = _addPicButton("PARTCFG/WII_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 130, 48, 48);
	m_partitionsBtnWiiP = _addPicButton("PARTCFG/WII_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 130, 48, 48);
	m_partitionsLblGC = _addLabel("PARTCFG/GC", theme.lblFont, L"", 20, 185, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_partitionsLblGCVal = _addLabel("PARTCFG/GC_BTN", theme.btnFont, L"", 468, 190, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_partitionsBtnGCM = _addPicButton("PARTCFG/GC_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 190, 48, 48);
	m_partitionsBtnGCP = _addPicButton("PARTCFG/GC_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 190, 48, 48);
	m_partitionsLblChannels = _addLabel("PARTCFG/CHANNELS", theme.lblFont, L"", 20, 245, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_partitionsLblChannelsVal = _addLabel("PARTCFG/CHANNELS_BTN", theme.btnFont, L"", 468, 250, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_partitionsBtnChannelsM = _addPicButton("PARTCFG/CHANNELS_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 250, 48, 48);
	m_partitionsBtnChannelsP = _addPicButton("PARTCFG/CHANNELS_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 250, 48, 48);
	m_partitionsLblPlugin = _addLabel("PARTCFG/PLUGIN", theme.lblFont, L"", 20, 305, 385, 56, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);
	m_partitionsLblPluginVal = _addLabel("PARTCFG/PLUGIN_BTN", theme.btnFont, L"", 468, 310, 104, 48, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_partitionsBtnPluginM = _addPicButton("PARTCFG/PLUGIN_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 420, 310, 48, 48);
	m_partitionsBtnPluginP = _addPicButton("PARTCFG/PLUGIN_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 572, 310, 48, 48);

	_setHideAnim(m_partitionsLblTitle, "PARTCFG/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_partitionsBtnBack, "PARTCFG/BACK_BTN", 0, 0, 1.f, -1.f);

	_setHideAnim(m_partitionsLblWii, "PARTCFG/WII", 50, 0, -2.f, 0.f);
	_setHideAnim(m_partitionsLblWiiVal, "PARTCFG/WII_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_partitionsBtnWiiM, "PARTCFG/WII_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_partitionsBtnWiiP, "PARTCFG/WII_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_partitionsLblGC, "PARTCFG/GC", 50, 0, -2.f, 0.f);
	_setHideAnim(m_partitionsLblGCVal, "PARTCFG/GC_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_partitionsBtnGCM, "PARTCFG/GC_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_partitionsBtnGCP, "PARTCFG/GC_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_partitionsLblChannels, "PARTCFG/CHANNELS", 50, 0, -2.f, 0.f);
	_setHideAnim(m_partitionsLblChannelsVal, "PARTCFG/CHANNELS_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_partitionsBtnChannelsM, "PARTCFG/CHANNELS_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_partitionsBtnChannelsP, "PARTCFG/CHANNELS_PLUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_partitionsLblPlugin, "PARTCFG/PLUGIN", 50, 0, -2.f, 0.f);
	_setHideAnim(m_partitionsLblPluginVal, "PARTCFG/PLUGIN_BTN", -50, 0, 1.f, 0.f);
	_setHideAnim(m_partitionsBtnPluginM, "PARTCFG/PLUGIN_MINUS", -50, 0, 1.f, 0.f);
	_setHideAnim(m_partitionsBtnPluginP, "PARTCFG/PLUGIN_PLUS", -50, 0, 1.f, 0.f);
	_hidePartitionsCfg(true);
	_textPartitionsCfg();
}

void CMenu::_textPartitionsCfg(void)
{
	m_btnMgr.setText(m_partitionsLblTitle, _t("part5", L"Partition Settings"));
	m_btnMgr.setText(m_partitionsLblWii, _t("part1", L"Wii Partition"));
	m_btnMgr.setText(m_partitionsLblGC, _t("part2", L"GameCube Partition"));
	m_btnMgr.setText(m_partitionsLblChannels, _t("part3", L"Emu NANDS Partition"));
	m_btnMgr.setText(m_partitionsLblPlugin, _t("part4", L"Plugins Default Partition"));
	m_btnMgr.setText(m_partitionsBtnBack, _t("cfg10", L"Back"));
}
