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

class BannerWindow
{
	public:
		BannerWindow();
		void DeleteBanner();
		void LoadBanner(Banner *banner, CVideo *vid, u8 *font1, u8 *font2);
		int GetSelectedGame() { return gameSelected; }
		void Draw(void);
		void ZoomIn(void);
		void PauseZoom(void);
		void ZoomOut(void);
	protected:
		int MainLoop();
		void Animate(void);
		void ChangeGame(Banner *banner);
		void DrawRectangle(f32 x, f32 y, f32 width, f32 height, GXColor color, u8 filled);

		static const float fBannerWidth = 608.f;
		static const float fBannerHeight = 448.f;
		static const float fIconWidth = 128.f;
		static const float fIconHeight = 96.f;

		CVideo *video;
		bool reducedVol;
		int returnVal;
		int gameSelected;
		dir_discHdr dvdheader;

		int MaxAnimSteps;

		int AnimStep;
		float AnimPosX, AnimPosY;
		float fAnimScale;
		bool AnimZoomIn;
		bool AnimZoomOut;
		bool AnimationRunning;
		bool oldAnimationRunning;
		bool firstRun;

		u8 BGAlpha;
		u8 BannerAlpha;

		Mtx modelview;
		Mtx44 projection;
		Mtx44 originalProjection;
		Vec2f ScreenProps;

		AnimatedBanner *gameBanner;
		u8 *sysFont1;
		u8 *sysFont2;
};

#endif
