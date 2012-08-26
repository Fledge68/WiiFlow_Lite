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
#include "gx_addons.h"
#include "gecko/gecko.h"
#include "loader/utils.h"
#include "menu/menu.hpp"

BannerWindow *m_banner;

extern const u8 custombanner_bin[];
extern const u32 custombanner_bin_size;

void BannerWindow::Init(CVideo *vid, u8 *font1, u8 *font2)
{
	MaxAnimSteps = 30;
	returnVal = -1;
	reducedVol = false;
	ScreenProps.x = 620; //620
	ScreenProps.y = 400; //400
	sysFont1 = font1;
	sysFont2 = font2;
	ShowBanner = true;

	video = vid;
	guMtxIdentity(modelview);
	guMtxTransApply(modelview, modelview, (!video->wide() || video->vid_50hz()) ? 0.0f : 2.0f, video->vid_50hz() ? -1.0f : 0.0f, 0.0F);

	AnimPosX = 0.5f * (ScreenProps.x - fIconWidth);
	AnimPosY = 0.5f * (ScreenProps.y - fIconHeight);
	AnimationRunning = false;
	Brightness = 0.f;

	// this just looks better for banner/icon ratio
	xDiff = 0.5f * (video->wide() ? (video->vid_50hz() ? 616 : 620.0f) : 608.0f);
	yDiff = 0.5f * (video->vid_50hz() ? 448.0f : 470.0f);

	iconWidth = fIconWidth - 20;
	iconHeight = fIconHeight - 20;

	ratioX = xDiff * 2.f / iconWidth;
	ratioY = yDiff * 2.f / iconHeight;

	stepx1 = ((ScreenProps.x * 0.1f - xDiff) - (AnimPosX + 0.5f * fIconWidth - 0.5f * iconWidth)) * ratioX;
	stepx2 = ((ScreenProps.x * 0.1f + xDiff) - (AnimPosX + 0.5f * fIconWidth + 0.5f * iconWidth)) * ratioX;
	stepy1 = ((ScreenProps.y * 0.9f - yDiff) - (AnimPosY + 0.5f * fIconHeight - 0.5f * iconHeight)) * ratioY;
	stepy2 = ((ScreenProps.y * 0.9f + yDiff) - (AnimPosY + 0.5f * fIconHeight + 0.5f * iconHeight)) * ratioY;

	gameBanner->Clear();
	if(!FontLoaded)
	{
		gameBanner->LoadFont(sysFont1, sysFont2);
		FontLoaded = true;
	}
}

void BannerWindow::LoadBanner(Banner *banner, CVideo *vid, u8 *font1, u8 *font2)
{
	changing = true;
	Init(vid, font1, font2);
	gameBanner->LoadBanner(banner);
	gameSelected = 1;
	changing = false;
	ShowBanner = true;
}

void BannerWindow::DeleteBanner(bool gamechange)
{
	if(!gamechange)
		gameSelected = 0;
	gameBanner->Clear();
}

BannerWindow::BannerWindow()
{
	ShowBanner = true;
	FontLoaded = false;
	changing = false;
	AnimZoom = false;
	AnimStep = 20;
	gameSelected = 0;
	gameBanner = new AnimatedBanner;
}

void BannerWindow::LoadBannerBin(u8 *bnr, u32 bnr_size, CVideo *vid, u8 *font1, u8 *font2)
{
	changing = true;
	Init(vid, font1, font2);
	gameBanner->LoadBannerBin(bnr, bnr_size);
	gameSelected = 1;
	changing = false;
	ShowBanner = true;
}

void BannerWindow::CreateGCBanner(u8 *bnr, CVideo *vid, u8 *font1, u8 *font2, const wchar_t *title)
{
	GC_OpeningBnr *openingBnr = (GC_OpeningBnr *)bnr;
	LoadBannerBin((u8*)custombanner_bin, (u32)custombanner_bin_size, vid, font1, font2);
	gameBanner->SetBannerTexture("GCIcon.tpl", openingBnr->tpl_data, 96, 32, GX_TF_RGB5A3);
	gameBanner->SetBannerText("T_GameTitle", title);
}

bool BannerWindow::ToogleZoom(void)
{
	if(AnimZoom)
	{
		AnimStep = 30;
		AnimZoom = false;
	}
	else
	{
		AnimStep = 20;
		AnimZoom = true;
	}
	return AnimZoom;
}

void BannerWindow::Animate(void)
{
	// animation is on going
	if(AnimStep <= MaxAnimSteps)
	{
		AnimationRunning = true;
		if(AnimZoom && AnimStep < MaxAnimSteps)
			AnimStep++;
		else if(!AnimZoom && AnimStep > 20)
			AnimStep--;
		float curAnimStep = ((float)(MaxAnimSteps - AnimStep)/(float)MaxAnimSteps);

		//! This works good for banners
		float top = (ScreenProps.y * 0.5f - yDiff) + stepy1 * curAnimStep;
		float bottom = (ScreenProps.y * 0.5f + yDiff) + stepy2 * curAnimStep;
		float left = (ScreenProps.x * 0.5f - xDiff) + stepx1 * curAnimStep;
		float right = (ScreenProps.x * 0.5f + xDiff) + stepx2 * curAnimStep;

		// set banner projection
		guOrtho(projection, top, bottom, left, right, -100, 10000);
	}
	// last animation step
	else if(AnimationRunning)
		AnimationRunning = false;
}

void BannerWindow::Draw(void)
{
	if(!ShowBanner)
		return;

	// draw a black background image first
	if(AnimStep >= MaxAnimSteps)
		DrawRectangle(0.0f, 0.0f, video->width(), video->height(), (GXColor) {0, 0, 0, 255.f});

	if(changing)
		return;

	// Run window animation
	Animate();

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
		gameBanner->getBanner()->Render(modelview, ScreenProps, video->wide(), 255.f);
		gameBanner->getBanner()->AdvanceFrame();
	}

	// Setup GX
	ReSetup_GX();
	GX_SetScissor(0, 0, video->width(), video->height());

	// Clear and back to previous projection
	video->setup2DProjection();

	// If wanted
	if(Brightness > 1.f)
		DrawRectangle(0.0f, 0.0f, video->width(), video->height(), (GXColor) {0, 0, 0, Brightness});
}

void BannerWindow::ToogleGameSettings()
{
	ToogleZoom();
	Brightness = (Brightness > 1.f ? 0.f : 200.f);
}

void BannerWindow::DrawRectangle(f32 x, f32 y, f32 width, f32 height, GXColor color)
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

	int i;
	f32 x2 = x + width;
	f32 y2 = y + height;
	guVector v[] = { { x, y, 0.0f }, { x2, y, 0.0f }, { x2, y2, 0.0f }, { x, y2, 0.0f }, { x, y, 0.0f } };

	GX_Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
	for(i = 0; i < 4; i++)
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
	for(u8 i = 0; i < video->getAA(); i++)
	{
		GX_SetTevOp(i, GX_MODULATE);
		GX_SetTevOrder(i, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
		GX_SetTevSwapMode(i, GX_TEV_SWAP0, GX_TEV_SWAP0);
		GX_SetTevKColorSel(i, GX_TEV_KCSEL_1_4);
		GX_SetTevKAlphaSel(i, GX_TEV_KASEL_1);
		GX_SetTevDirect(i);
	}
	// swap table
	GX_SetTevSwapModeTable(GX_TEV_SWAP0, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_ALPHA);
	GX_SetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_RED, GX_CH_RED, GX_CH_ALPHA);
	GX_SetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_GREEN, GX_CH_GREEN, GX_CH_GREEN, GX_CH_ALPHA);
	GX_SetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_BLUE, GX_CH_BLUE, GX_CH_BLUE, GX_CH_ALPHA);
	// alpha compare and blend mode
	GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_SET);
}
