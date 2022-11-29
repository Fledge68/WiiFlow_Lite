
#include "menu.hpp"

s16 m_partitionsLblTitle;
s16 m_partitionsBtnBack;
s16 m_partitionsLblUser[4];

TexData m_partitionsBg;

void CMenu::_hidePartitionsCfg(bool instant)
{
	m_btnMgr.hide(m_partitionsLblTitle, instant);
	m_btnMgr.hide(m_partitionsBtnBack, instant);
	for(u8 i = 0; i < ARRAY_SIZE(m_partitionsLblUser); ++i)
		if(m_partitionsLblUser[i] != -1)
			m_btnMgr.hide(m_partitionsLblUser[i], instant);

	_hideConfigButtons(instant);
}

void CMenu::_showPartitionsCfg(void)
{
	_setBg(m_partitionsBg, m_partitionsBg);
	m_btnMgr.show(m_partitionsLblTitle);
	m_btnMgr.show(m_partitionsBtnBack);
	for(u8 i = 0; i < ARRAY_SIZE(m_partitionsLblUser); ++i)
		if(m_partitionsLblUser[i] != -1)
			m_btnMgr.show(m_partitionsLblUser[i]);

	m_btnMgr.setText(m_configLbl1, _t("part1", L"Wii Partition"));
	m_btnMgr.setText(m_configLbl2, _t("part2", L"GameCube Partition"));
	m_btnMgr.setText(m_configLbl3, _t("part3", L"Emu NANDS Partition"));
	m_btnMgr.setText(m_configLbl4, _t("part4", L"Plugins Default Partition"));
	
	const char *partitionname = DeviceName[m_cfg.getInt(WII_DOMAIN, "partition", 0)];
	m_btnMgr.setText(m_configLbl1Val, upperCase(partitionname));
	
	partitionname = DeviceName[m_cfg.getInt(GC_DOMAIN, "partition", 0)];
	m_btnMgr.setText(m_configLbl2Val, upperCase(partitionname));
	
	partitionname = DeviceName[m_cfg.getInt(CHANNEL_DOMAIN, "partition", 0)];
	m_btnMgr.setText(m_configLbl3Val, upperCase(partitionname));
	
	partitionname = DeviceName[m_cfg.getInt(PLUGIN_DOMAIN, "partition", 0)];
	m_btnMgr.setText(m_configLbl4Val, upperCase(partitionname));
	
	m_btnMgr.show(m_configLbl1);
	m_btnMgr.show(m_configLbl1Val);
	m_btnMgr.show(m_configBtn1P);
	m_btnMgr.show(m_configBtn1M);
	m_btnMgr.show(m_configLbl2);
	m_btnMgr.show(m_configLbl2Val);
	m_btnMgr.show(m_configBtn2P);
	m_btnMgr.show(m_configBtn2M);
	m_btnMgr.show(m_configLbl3);
	m_btnMgr.show(m_configLbl3Val);
	m_btnMgr.show(m_configBtn3P);
	m_btnMgr.show(m_configBtn3M);
	m_btnMgr.show(m_configLbl4);
	m_btnMgr.show(m_configLbl4Val);
	m_btnMgr.show(m_configBtn4P);
	m_btnMgr.show(m_configBtn4M);
}

void CMenu::_partitionsCfg(void)
{
	//int prevPartition = currentPartition;
	SetupInput();
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
			else if(m_btnMgr.selected(m_configBtn1P) || m_btnMgr.selected(m_configBtn1M))
			{
				s8 direction = m_btnMgr.selected(m_configBtn1P) ? 1 : -1;
				_setPartition(direction, m_cfg.getInt(WII_DOMAIN, "partition"), COVERFLOW_WII);
				_showPartitionsCfg();
				if(m_current_view & COVERFLOW_WII || 
					(m_current_view & COVERFLOW_PLUGIN && m_plugin.GetEnabledStatus(m_plugin.GetPluginPosition(strtoul("4E574949", NULL, 16)))))
				{
					m_refreshGameList = true;
					//prevPartition = currentPartition;
				}
			}
			else if(m_btnMgr.selected(m_configBtn2P) || m_btnMgr.selected(m_configBtn2M))
			{
				s8 direction = m_btnMgr.selected(m_configBtn2P) ? 1 : -1;
				_setPartition(direction, m_cfg.getInt(GC_DOMAIN, "partition"), COVERFLOW_GAMECUBE);
				_showPartitionsCfg();
				if(m_current_view & COVERFLOW_GAMECUBE || 
					(m_current_view & COVERFLOW_PLUGIN && m_plugin.GetEnabledStatus(m_plugin.GetPluginPosition(strtoul("4E47434D", NULL, 16)))))
				{
					m_refreshGameList = true;
					//prevPartition = currentPartition;
				}
			}
			else if(m_btnMgr.selected(m_configBtn3P) || m_btnMgr.selected(m_configBtn3M))
			{
				s8 direction = m_btnMgr.selected(m_configBtn3P) ? 1 : -1;
				_setPartition(direction, m_cfg.getInt(CHANNEL_DOMAIN, "partition"), COVERFLOW_CHANNEL);
				_showPartitionsCfg();
				// partition only for emu nands
				if(m_current_view & COVERFLOW_CHANNEL || 
					(m_current_view & COVERFLOW_PLUGIN && m_plugin.GetEnabledStatus(m_plugin.GetPluginPosition(strtoul("454E414E", NULL, 16)))))
				{
					m_refreshGameList = true;
					//prevPartition = currentPartition;
				}
			}
			else if(m_btnMgr.selected(m_configBtn4P) || m_btnMgr.selected(m_configBtn4M))
			{
				s8 direction = m_btnMgr.selected(m_configBtn4P) ? 1 : -1;
				_setPartition(direction, m_cfg.getInt(PLUGIN_DOMAIN, "partition"), COVERFLOW_PLUGIN);
				_showPartitionsCfg();
				if(m_current_view & COVERFLOW_PLUGIN)
				{
					m_refreshGameList = true;
					//prevPartition = currentPartition;
				}
			}
		}
	}
	//m_current_view = m_prev_view;
	//m_prev_view = 0;
	//currentPartition = prevPartition;
	_hidePartitionsCfg();
}

void CMenu::_initPartitionsCfgMenu()
{
	m_partitionsBg = _texture("PARTCFG/BG", "texture", theme.bg, false);
	
	_addUserLabels(m_partitionsLblUser, ARRAY_SIZE(m_partitionsLblUser), "PARTCFG");
	m_partitionsLblTitle = _addLabel("PARTCFG/TITLE", theme.titleFont, L"", 0, 10, 640, 60, theme.titleFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	m_partitionsBtnBack = _addButton("PARTCFG/BACK_BTN", theme.btnFont, L"", 420, 400, 200, 48, theme.btnFontColor);

	_setHideAnim(m_partitionsLblTitle, "PARTCFG/TITLE", 0, 0, -2.f, 0.f);
	_setHideAnim(m_partitionsBtnBack, "PARTCFG/BACK_BTN", 0, 0, 1.f, -1.f);

	_hidePartitionsCfg(true);
	_textPartitionsCfg();
}

void CMenu::_textPartitionsCfg(void)
{
	m_btnMgr.setText(m_partitionsLblTitle, _t("part5", L"Partition Settings"));
	m_btnMgr.setText(m_partitionsBtnBack, _t("cfg10", L"Back"));
}
