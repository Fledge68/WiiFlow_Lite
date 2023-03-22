
#include "menu.hpp"

s16 m_partitionsLblTitle;
s16 m_partitionsBtnBack;
s16 m_partitionsLblUser[4];

TexData m_partitionsBg;

static inline int loopNum(int i, int s)
{
	return (i + s) % s;
}

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
	m_btnMgr.setText(m_configLbl3, _t("part4", L"Plugins Default Partition"));
	
	currentPartition = m_cfg.getInt(WII_DOMAIN, "partition", USB1);
	m_btnMgr.setText(m_configLbl1Val, currentPartition == 8 ? "SD/USB" : upperCase(DeviceName[currentPartition]));
	
	currentPartition = m_cfg.getInt(GC_DOMAIN, "partition", USB1);
	m_btnMgr.setText(m_configLbl2Val, currentPartition == 8 ? "SD/USB" : upperCase(DeviceName[currentPartition]));
	
	currentPartition = m_cfg.getInt(PLUGIN_DOMAIN, "partition", SD);
	m_btnMgr.setText(m_configLbl3Val, upperCase(DeviceName[currentPartition]));
	
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
}

void CMenu::_partitionsCfg(void)
{
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
				m_cfg.setInt(WII_DOMAIN, "partition", currentPartition);
				_showPartitionsCfg();
				if(m_current_view & COVERFLOW_WII || (m_current_view & COVERFLOW_PLUGIN && m_plugin.GetEnabledStatus(WII_PMAGIC)))
					m_refreshGameList = true;
			}
			else if(m_btnMgr.selected(m_configBtn2P) || m_btnMgr.selected(m_configBtn2M))
			{
				s8 direction = m_btnMgr.selected(m_configBtn2P) ? 1 : -1;
				_setPartition(direction, m_cfg.getInt(GC_DOMAIN, "partition"), COVERFLOW_GAMECUBE);
				m_cfg.setInt(GC_DOMAIN, "partition", currentPartition);
				_showPartitionsCfg();
				if(m_current_view & COVERFLOW_GAMECUBE || (m_current_view & COVERFLOW_PLUGIN && m_plugin.GetEnabledStatus(GC_PMAGIC)))
					m_refreshGameList = true;
			}
			else if(m_btnMgr.selected(m_configBtn3P) || m_btnMgr.selected(m_configBtn3M))
			{
				s8 direction = m_btnMgr.selected(m_configBtn3P) ? 1 : -1;
				_setPartition(direction, m_cfg.getInt(PLUGIN_DOMAIN, "partition"), COVERFLOW_PLUGIN);
				m_cfg.setInt(PLUGIN_DOMAIN, "partition", currentPartition);
				_showPartitionsCfg();
				if(m_current_view & COVERFLOW_PLUGIN)
					m_refreshGameList = true;
			}
		}
	}
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

void CMenu::_setPartition(s8 direction, u8 partition, u8 coverflow)// COVERFLOW_NONE is for emu saves nand
{
	currentPartition = partition;
	u8 prev_view = m_current_view;// save and restore later
	m_current_view = coverflow;
	int FS_Type = 0;
	bool NeedFAT = m_current_view == COVERFLOW_CHANNEL || m_current_view == COVERFLOW_GAMECUBE || m_current_view == COVERFLOW_NONE;
	u8 limiter = 0;
	
	if(direction != 0)// change partition if direction is not zero
	{
		do
		{
			currentPartition = loopNum(currentPartition + direction, 9);
			if(currentPartition == 8 && (m_current_view == COVERFLOW_WII || m_current_view == COVERFLOW_GAMECUBE))
				break;
			FS_Type = DeviceHandle.GetFSType(currentPartition);
			limiter++;
		}
		while(limiter < 9 && (!DeviceHandle.IsInserted(currentPartition) ||
			(m_current_view != COVERFLOW_WII && FS_Type == PART_FS_WBFS) ||
			(NeedFAT && FS_Type != PART_FS_FAT)));
	}
	m_current_view = prev_view;
}	
