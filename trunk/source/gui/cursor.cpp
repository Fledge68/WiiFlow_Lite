
#include "cursor.hpp"
#include "pngu.h"
#include "memory/mem2.hpp"
#include <algorithm>

using namespace std;

extern const u8 player1_point_png[];
extern const u8 player2_point_png[];
extern const u8 player3_point_png[];
extern const u8 player4_point_png[];

static inline u32 coordsI8(u32 x, u32 y, u32 w)
{
	return (((y >> 2) * (w >> 3) + (x >> 3)) << 5) + ((y & 3) << 3) + (x & 7);
}

static inline u32 coordsRGBA8(u32 x, u32 y, u32 w)
{
	return ((((y >> 2) * (w >> 2) + (x >> 2)) << 5) + ((y & 3) << 2) + (x & 3)) << 1;
}

bool CCursor::init(const char *png, bool wideFix, CColor shadowColor, float shadowX, float shadowY, bool blur, int chan)
{
	bool ok = true;
	bool shadow = shadowColor.a > 0;

	m_wideFix = wideFix;
	m_x = -1;
	m_y = -1;
	if(TexHandle.fromImageFile(m_texture, png) != TE_OK)
	{
 		if(chan == 0)
			ok = (TexHandle.fromPNG(m_texture, player1_point_png) == TE_OK);
		else if(chan == 1)
			ok = (TexHandle.fromPNG(m_texture, player2_point_png) == TE_OK);
		else if(chan == 2)
			ok = (TexHandle.fromPNG(m_texture, player3_point_png) == TE_OK);
		else if(chan == 3)
			ok = (TexHandle.fromPNG(m_texture, player4_point_png) == TE_OK);
	}
	if (ok && shadow)
	{
		m_shadowColor = shadowColor;
		m_shadowX = shadowX;
		m_shadowY = shadowY;
		m_shadow.width = m_texture.width;
		m_shadow.height = m_texture.height;
		m_shadow.maxLOD = 0;
		m_shadow.format = GX_TF_I8;
		m_shadow.data = (u8*)MEM2_alloc(GX_GetTexBufferSize(m_shadow.width, m_shadow.height, m_shadow.format, GX_FALSE, 0));
		if(m_shadow.data != NULL)
		{
			const u8 *src = m_texture.data;
			u8 *dst = m_shadow.data;
			u32 w = m_shadow.width;
			for (u32 yy = 0; yy < m_shadow.height; ++yy)
				for (u32 xx = 0; xx < m_shadow.width; ++xx)
					dst[coordsI8(xx, yy, w)] = src[coordsRGBA8(xx, yy, w)];
			if (blur) _blur();
		}
	}
	return ok;
}

void CCursor::draw(int x, int y, float a)
{
	GXTexObj texObj;
	float w = (float)m_texture.width * 0.5f;
	float h = (float)m_texture.height * 0.5f;
	Mtx modelViewMtx;
	Vector3D v;
	float xScale = m_wideFix ? 0.75f : 1.f;

	m_x = x - m_texture.width / 2;
	m_y = y - m_texture.height / 2;
	// 
	GX_SetNumChans(1);
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetNumTexGens(1);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_TRUE);
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetZMode(GX_DISABLE, GX_LEQUAL, GX_TRUE);
	// Shadow
	if(m_shadow.data != NULL)
	{
		guMtxIdentity(modelViewMtx);
		guMtxTransApply(modelViewMtx, modelViewMtx, (float)x - w + m_shadowX * xScale, (float)y - h + m_shadowY, 0.f);
		GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
		GX_InitTexObj(&texObj, m_shadow.data, m_shadow.width, m_shadow.height, m_shadow.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_LoadTexObj(&texObj, GX_TEXMAP0);
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		v = Vector3D(w, h, 0.f).rotateZ(a);
		GX_Position3f32(v.x * xScale, v.y, v.z);
		GX_Color4u8(m_shadowColor.r, m_shadowColor.g, m_shadowColor.b, m_shadowColor.a);
		GX_TexCoord2f32(1.f, 1.f);
		v = Vector3D(-w, h, 0.f).rotateZ(a);
		GX_Position3f32(v.x * xScale, v.y, v.z);
		GX_Color4u8(m_shadowColor.r, m_shadowColor.g, m_shadowColor.b, m_shadowColor.a);
		GX_TexCoord2f32(0.f, 1.f);
		v = Vector3D(-w, -h, 0.f).rotateZ(a);
		GX_Position3f32(v.x * xScale, v.y, v.z);
		GX_Color4u8(m_shadowColor.r, m_shadowColor.g, m_shadowColor.b, m_shadowColor.a);
		GX_TexCoord2f32(0.f, 0.f);
		v = Vector3D(w, -h, 0.f).rotateZ(a);
		GX_Position3f32(v.x * xScale, v.y, v.z);
		GX_Color4u8(m_shadowColor.r, m_shadowColor.g, m_shadowColor.b, m_shadowColor.a);
		GX_TexCoord2f32(1.f, 0.f);
		GX_End();
	}
	// 
	guMtxIdentity(modelViewMtx);
	guMtxTransApply(modelViewMtx, modelViewMtx, (float)x - w, (float)y - h, 0.f);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	GX_InitTexObj(&texObj, m_texture.data, m_texture.width, m_texture.height, m_texture.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	v = Vector3D(w, h, 0.f).rotateZ(a);
	GX_Position3f32(v.x * xScale, v.y, v.z);
	GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
	GX_TexCoord2f32(1.f, 1.f);
	v = Vector3D(-w, h, 0.f).rotateZ(a);
	GX_Position3f32(v.x * xScale, v.y, v.z);
	GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
	GX_TexCoord2f32(0.f, 1.f);
	v = Vector3D(-w, -h, 0.f).rotateZ(a);
	GX_Position3f32(v.x * xScale, v.y, v.z);
	GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
	GX_TexCoord2f32(0.f, 0.f);
	v = Vector3D(w, -h, 0.f).rotateZ(a);
	GX_Position3f32(v.x * xScale, v.y, v.z);
	GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
	GX_TexCoord2f32(1.f, 0.f);
	GX_End();
}

void CCursor::_blur(void)
{
	int radius = 3;
	int w = m_shadow.width;
	int h = m_shadow.height;
	int div = 2 * radius + 1;
	u8 *pic = m_shadow.data;
	int sum;
	int yp;
	int yi;
	int pass = 2;
	s32 *xmin = (s32*)MEM2_alloc(w * sizeof(s32));
	s32 *xmax = (s32*)MEM2_alloc(w * sizeof(s32));
	s32 *ymin = (s32*)MEM2_alloc(h * sizeof(s32));
	s32 *ymax = (s32*)MEM2_alloc(h * sizeof(s32));
	u8 *r = (u8*)MEM2_alloc(m_shadow.width * m_shadow.height);
	if(!xmin || !xmax || !ymin || !ymax || !r)
		return;
	for (int i = 0; i < w; ++i)
	{
		xmax[i] = min(i + radius + 1, w - 1);
		xmin[i] = max(i - radius, 0);
	}
	for (int i = 0; i < h; ++i)
	{
		ymax[i] = min(i + radius + 1, h - 1) * w;
		ymin[i] = max(i - radius, 0) * w;
	}
	for (int k = 0; k < pass; ++k)	// 2 passes for much better quality
	{
		yi = 0;
		for (int y = 0; y < h; ++y)
		{
			sum = 0;
			for (int i = -radius; i <= radius; ++i)
				sum += pic[coordsI8(min(max(0, i), w - 1), y, w)];
			for (int x = 0; x < w; ++x)
			{
				r[yi] = sum / div;
				sum += pic[coordsI8(xmax[x], y, w)];
				sum -= pic[coordsI8(xmin[x], y, w)];
				++yi;
			}
		}
		for (int x = 0; x < w; ++x)
		{
			sum = 0;
			yp = -radius * w;
			for (int i = -radius; i <= radius; ++i)
			{
				yi = max(0, yp) + x;
				sum += r[yi];
				yp += w;
			}
			yi = x;
			for (int y = 0; y < h; ++y)
			{
				pic[coordsI8(x, y, w)] = sum / div;
				sum += r[x + ymax[y]] - r[x + ymin[y]];
				yi += w;
			}
		}
	}
	free(xmin);
	free(xmax);
	free(ymin);
	free(ymax);
	free(r);
}
