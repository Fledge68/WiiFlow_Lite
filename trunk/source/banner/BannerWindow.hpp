/****************************************************************************
 * Copyright (C) 2012 Dimok
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
#ifndef BANNERWINDOW_HPP_
#define BANNERWINDOW_HPP_

#include <gccore.h>

#include "disc.h"
#include "AnimatedBanner.h"
#include "gui_sound.h"
#include "video.hpp"
#include "channel/banner.h"

#define FAVORITE_STARS  5

typedef struct _GC_OpeningBnr
{
	u32 magic;					  // BNR1 or BNR2
	u8 pad[0x1C];
	u8 tpl_data[0x1800];			// 96x32 pixel format GX_TF_RGB5A3
	struct
	{
		u8 disc_title[0x20];		// Gamename
		u8 developer_short[0x20];   // Company/Developer
		u8 full_title[0x40];		// Full Game Title
		u8 developer[0x40];		 // Company/Developer Full name, or description
		u8 long_description[0x80];  // Game Description
	} description[6];			   // 6 only on BNR2 => English, German, French, Spanish, Italian, Dutch ??
} GC_OpeningBnr;

class BannerWindow
{
	public:
		BannerWindow();
		void DeleteBanner(bool gamechange = false);
		void LoadBanner(Banner *banner, CVideo *vid, u8 *font1, u8 *font2);
		void LoadBannerBin(u8 *bnr, u32 bnr_size, CVideo *vid, u8 *font1, u8 *font2);
		int GetSelectedGame() { return gameSelected; }
		bool GetZoomSetting() { return AnimZoom; }
		void CreateGCBanner(u8 *bnr, CVideo *vid, u8 *font1, u8 *font2, const wchar_t *title);
		void Draw(void);
		bool ToogleZoom(void);
		void SetBrightness(float b);
	protected:
		int MainLoop();
		void Animate(void);
		void ChangeGame(Banner *banner);
		void DrawRectangle(f32 x, f32 y, f32 width, f32 height, GXColor color);
		void ReSetup_GX(void);
		void Init(CVideo *vid, u8 *font1, u8 *font2);

		static const float fBannerWidth = 608.f;
		static const float fBannerHeight = 448.f;
		static const float fIconWidth = 128.f;
		static const float fIconHeight = 96.f;

		CVideo *video;
		bool reducedVol;
		int returnVal;
		int gameSelected;
		dir_discHdr dvdheader;
		float Brightness;
		int MaxAnimSteps;

		int AnimStep;
		float AnimPosX, AnimPosY;
		float fAnimScale;
		bool AnimZoom;
		bool AnimationRunning;
		bool changing;

		Mtx modelview;
		Mtx44 projection;
		Vec2f ScreenProps;

		AnimatedBanner *gameBanner;
		u8 *sysFont1;
		u8 *sysFont2;
		bool FontLoaded;
};

extern BannerWindow *m_banner;

#endif
