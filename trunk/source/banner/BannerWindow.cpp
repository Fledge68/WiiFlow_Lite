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
#include <unistd.h>
#include "BannerWindow.hpp"
#include "menu.hpp"
#include "utils.h"
#include "gx_addons.h"
#include "gecko.h"

void BannerWindow::LoadBanner(Banner *banner, CVideo *vid, u8 *font1, u8 *font2)
{
	changing = true;
	MaxAnimSteps = 30;
	returnVal = -1;
	reducedVol = false;
	ScreenProps.x = 620; //620
	ScreenProps.y = 400; //400
	sysFont1 = font1;
	sysFont2 = font2;

	video = vid;
	guMtxIdentity(modelview);
	guMtxTransApply(modelview, modelview, (!video->wide() || video->vid_50hz()) ? 0.0f : 2.0f, video->vid_50hz() ? -1.0f : 0.0f, 0.0F);

	AnimPosX = 0.5f * (ScreenProps.x - fIconWidth);
	AnimPosY = 0.5f * (ScreenProps.y - fIconHeight);
	AnimZoomIn = false;
	AnimZoomOut = false;
	AnimationRunning = false;
	BannerAlpha = 255.f;
	BGAlpha = 255.f;

	ChangeGame(banner);
	gameSelected = 1;
	changing = false;
}

void BannerWindow::DeleteBanner()
{
	gameSelected = 0;
	gameBanner->Clear();
}

BannerWindow::BannerWindow()
{
	changing = false;
	AnimStep = 20;
	gameSelected = 0;
	gameBanner = new AnimatedBanner;
}

void BannerWindow::ChangeGame(Banner *banner)
{
	gameBanner->Clear();
	gameBanner->LoadFont(sysFont1, sysFont2);
	gameBanner->LoadBanner(banner);
}

void BannerWindow::ZoomIn(void)
{
	AnimZoomIn = true;
	AnimZoomOut = false;
	if(AnimStep <= 20)
		AnimStep++;
}

void BannerWindow::PauseZoom(void)
{
	AnimZoomIn = false;
	AnimZoomOut = false;
}

void BannerWindow::ZoomOut(void)
{
	AnimZoomIn = false;
	AnimZoomOut = true;
	if(AnimStep >= MaxAnimSteps)
		AnimStep--;
}

void BannerWindow::Animate(void)
{
	// animation is on going
	if(AnimStep < MaxAnimSteps)
	{
		AnimationRunning = true;
		if(AnimZoomIn && AnimStep < MaxAnimSteps)
			AnimStep++;
		else if(AnimZoomOut && AnimStep > 20)
			AnimStep--;

		/*
		// zoom in animation
		if(AnimZoomIn) {
			BGAlpha = std::min(255.f * AnimStep * 2.f / MaxAnimSteps, 255.f);
			//if(AnimStep < 0.4f * MaxAnimSteps)
				//BannerAlpha = 0;
			//else
				//BannerAlpha = std::min(255.f * (AnimStep - 0.4f * MaxAnimSteps) / (0.6f * MaxAnimSteps), 255.f);
		}
		// zoom out animation
		else {
			BGAlpha = std::min(255.f * (MaxAnimSteps-AnimStep) * 2.f / MaxAnimSteps, 255.f);
			//if((MaxAnimSteps - AnimStep) < 0.4f * MaxAnimSteps)
				//BannerAlpha = 0;
			//else
				//BannerAlpha = std::min(255.f * ((MaxAnimSteps - AnimStep) - 0.4f * MaxAnimSteps) / (0.6f * MaxAnimSteps), 255.f);
		}
		*/

		float curAnimStep = ((float)(MaxAnimSteps - AnimStep)/(float)MaxAnimSteps);

		float stepx1 = -AnimPosX;
		float stepy1 = -AnimPosY;
		float stepx2 = (640 - 1) - (AnimPosX + fIconWidth);
		float stepy2 = (480 - 1) - (AnimPosY + fIconHeight);

		float top = AnimPosY + stepy1 * curAnimStep;
		float bottom = AnimPosY + fIconHeight + stepy2 * curAnimStep;
		float left = AnimPosX + stepx1 * curAnimStep;
		float right = AnimPosX + fIconWidth + stepx2 * curAnimStep;

		// set main projection of all GUI stuff if we are using the banner browser
		//if(dynamic_cast<GuiBannerGrid *>(browserMenu->GetGameBrowser()) != NULL)
		//	guOrtho(FSProjection2D, top, bottom, left, right, 0, 10000);

		float xDiff = 0.5f * (video->wide() ? (video->vid_50hz() ? 616 : 620.0f) : 608.0f);
		float yDiff = 0.5f * (video->vid_50hz() ? 448.0f : 470.0f);

		// this just looks better for banner/icon ratio
		float iconWidth = fIconWidth - 20;
		float iconHeight = fIconHeight - 20;

		f32 ratioX = xDiff * 2.f / iconWidth;
		f32 ratioY = yDiff * 2.f / iconHeight;
		stepx1 = ((ScreenProps.x * 0.1f - xDiff) - (AnimPosX + 0.5f * fIconWidth - 0.5f * iconWidth)) * ratioX;
		stepx2 = ((ScreenProps.x * 0.1f + xDiff) - (AnimPosX + 0.5f * fIconWidth + 0.5f * iconWidth)) * ratioX;
		stepy1 = ((ScreenProps.y * 0.9f - yDiff) - (AnimPosY + 0.5f * fIconHeight - 0.5f * iconHeight)) * ratioY;
		stepy2 = ((ScreenProps.y * 0.9f + yDiff) - (AnimPosY + 0.5f * fIconHeight + 0.5f * iconHeight)) * ratioY;

		//! This works good for banners
		top = (ScreenProps.y * 0.5f - yDiff) + stepy1 * curAnimStep;
		bottom = (ScreenProps.y * 0.5f + yDiff) + stepy2 * curAnimStep;
		left = (ScreenProps.x * 0.5f - xDiff) + stepx1 * curAnimStep;
		right = (ScreenProps.x * 0.5f + xDiff) + stepx2 * curAnimStep;

		// set banner projection
		guOrtho(projection,top, bottom, left, right,-100,10000);
	}
	// last animation step
	else if(AnimationRunning)
	{
		// set back original projection and stop animation/render of the browser (save some CPU ;P)
		//memcpy(&video->projMtx, &originalProjection, sizeof(Mtx44));
		AnimationRunning = false;
	}
}

void BannerWindow::Draw(void)
{
	// draw a black background image first
	if(AnimStep >= MaxAnimSteps)
		DrawRectangle(0.0f, 0.0f, video->width(), video->height(), (GXColor) {0, 0, 0, BGAlpha}, true);

	if(changing)
		return;

	// Run window animation
	Animate();

	// no banner alpha means its the start of the animation
	if(BannerAlpha == 0)
		return;

	// cut the unneeded crap
	Mtx mv1, mv2, mv3;
	guMtxIdentity(mv2);
	guMtxIdentity(mv3);
	guMtxScaleApply(modelview,mv1, 1.f, -1.f, 1.f);
	guMtxTransApply(mv1,mv1, 0.5f * ScreenProps.x, 0.5f * ScreenProps.y, 0.f);
	guMtxTransApply(mv2,mv2, -0.5f * fBannerWidth, 0.5f * fBannerHeight, 0.f);
	guMtxTransApply(mv3,mv3, 0.5f * fBannerWidth, -0.5f * fBannerHeight + 104.f, 0.f);
	guMtxConcat(mv1, mv2, mv2);
	guMtxConcat(mv1, mv3, mv3);

	f32 viewportv[6];
	f32 projectionv[7];

	GX_GetViewportv(viewportv, video->vid_mode());
	GX_GetProjectionv(projectionv, projection, GX_ORTHOGRAPHIC);

	guVector vecTL;
	guVector vecBR;
	GX_Project(0.0f, 0.0f, 0.0f, mv2, projectionv, viewportv, &vecTL.x, &vecTL.y, &vecTL.z);
	GX_Project(0.0f, 0.0f, 0.0f, mv3, projectionv, viewportv, &vecBR.x, &vecBR.y, &vecBR.z);

	// round up scissor box offset and round down the size
	u32 scissorX = (u32)(0.5f + std::max(vecTL.x, 0.0f));
	u32 scissorY = (u32)(0.5f + std::max(vecTL.y, 0.0f));
	u32 scissorW = (u32)std::max(vecBR.x - vecTL.x, 0.0f);
	u32 scissorH = (u32)std::max(vecBR.y - vecTL.y, 0.0f);

	GX_SetScissor(scissorX, scissorY, scissorW, scissorH);

	// load projection matrix
	GX_LoadProjectionMtx(projection, GX_ORTHOGRAPHIC);

	if(gameBanner->getBanner())
	{
		gameBanner->getBanner()->Render(modelview, ScreenProps, video->wide(), BannerAlpha);
		gameBanner->getBanner()->AdvanceFrame();
	}

	// Setup GX
	ReSetup_GX();

	//if(AnimationRunning)
	GX_SetScissor(0, 0, video->width(), video->height());

	// Clear and back to previous projection
	video->setup2DProjection();
	GX_InvVtxCache();
	GX_InvalidateTexAll();
}

void BannerWindow::DrawRectangle(f32 x, f32 y, f32 width, f32 height, GXColor color, u8 filled)
{
	Mtx modelViewMtx;
	guMtxIdentity(modelViewMtx);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);

	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GX_ClearVtxDesc();
	GX_InvVtxCache();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

	u8 fmt;
	long n;
	int i;
	f32 x2 = x + width;
	f32 y2 = y + height;
	guVector v[] = { { x, y, 0.0f }, { x2, y, 0.0f }, { x2, y2, 0.0f }, { x, y2, 0.0f }, { x, y, 0.0f } };

	if (!filled)
	{
		fmt = GX_LINESTRIP;
		n = 5;
	}
	else
	{
		fmt = GX_TRIANGLEFAN;
		n = 4;
	}

	GX_Begin(fmt, GX_VTXFMT0, n);
	for (i = 0; i < n; i++)
	{
		GX_Position3f32(v[i].x, v[i].y, v[i].z);
		GX_Color4u8(color.r, color.g, color.b, color.a);
	}
	GX_End();
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
}

void BannerWindow::ReSetup_GX(void)
{
	// channel control
	GX_SetNumChans(1);
	GX_SetChanCtrl(GX_COLOR0A0,GX_DISABLE,GX_SRC_REG,GX_SRC_VTX,GX_LIGHTNULL,GX_DF_NONE,GX_AF_NONE);

	// texture gen.
	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	// texture environment
	GX_SetNumTevStages(1);
	GX_SetNumIndStages(0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTevSwapMode(GX_TEVSTAGE0, GX_TEV_SWAP0, GX_TEV_SWAP0);
	GX_SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_1_4);
	GX_SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
	GX_SetTevDirect(GX_TEVSTAGE0);
	// swap table
	GX_SetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
	GX_SetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_ALPHA);
	GX_SetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_ALPHA);
	GX_SetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_BLUE, GX_CH_BLUE, GX_CH_BLUE, GX_CH_ALPHA);
	// alpha compare and blend mode
	GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
}
