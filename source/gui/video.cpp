#include "pngu.h"
#include "video.hpp"
#include <string.h>
#include <wiilight.h>
#include "gecko.h"

#define DEFAULT_FIFO_SIZE	(256 * 1024)

using namespace std;

extern const u8 wait_01_png[];
extern const u8 wait_02_png[];
extern const u8 wait_03_png[];
extern const u8 wait_04_png[];
extern const u8 wait_05_png[];
extern const u8 wait_06_png[];
extern const u8 wait_07_png[];
extern const u8 wait_08_png[];
extern const u8 wait_09_png[];
extern const u8 wait_10_png[];

const float CVideo::_jitter2[2][2] = {
	{ 0.246490f, 0.249999f },
	{ -0.246490f, -0.249999f }
};

const float  CVideo::_jitter3[3][2] = {
	{ -0.373411f, -0.250550f },
	{ 0.256263f, 0.368119f },
	{ 0.117148f, -0.117570f }
};

const float CVideo::_jitter4[4][2] = {
	{ -0.208147f, 0.353730f },
	{ 0.203849f, -0.353780f },
	{ -0.292626f, -0.149945f },
	{ 0.296924f, 0.149994f }
};

const float CVideo::_jitter5[5][2] = {
	{ 0.5f, 0.5f },
	{ 0.3f, 0.1f },
	{ 0.7f, 0.9f },
	{ 0.9f, 0.3f },
	{ 0.1f, 0.7f }
};

const float CVideo::_jitter6[6][2] = {
	{ 0.4646464646f, 0.4646464646f },
	{ 0.1313131313f, 0.7979797979f },
	{ 0.5353535353f, 0.8686868686f },
	{ 0.8686868686f, 0.5353535353f },
	{ 0.7979797979f, 0.1313131313f },
	{ 0.2020202020f, 0.2020202020f }
};

const float CVideo::_jitter8[8][2] = {
	{ -0.334818f, 0.435331f },
	{ 0.286438f, -0.393495f },
	{ 0.459462f, 0.141540f },
	{ -0.414498f, -0.192829f },
	{ -0.183790f, 0.082102f },
	{ -0.079263f, -0.317383f },
	{ 0.102254f, 0.299133f },
	{ 0.164216f, -0.054399f }
};

const int CVideo::_stencilWidth = 128;
const int CVideo::_stencilHeight = 128;

static lwp_t waitThread = LWP_THREAD_NULL;
SmartBuf waitThreadStack;

extern "C"
{
	extern __typeof(malloc) __real_malloc;
	extern __typeof(memalign) __real_memalign;
}

CVideo::CVideo(void) :
	m_rmode(NULL), m_frameBuf(), m_curFB(0), m_fifo(NULL),
	m_yScale(0.0f), m_xfbHeight(0), m_wide(false),
	m_width2D(640), m_height2D(480), m_x2D(0), m_y2D(0), m_aa(0), m_aaAlpha(false),
	m_aaWidth(0), m_aaHeight(0), m_showWaitMessage(false), m_showingWaitMessages(false)
{
	memset(m_frameBuf, 0, sizeof m_frameBuf);
}

CVideo::~CVideo(void)
{
	cleanup();
}

void CColor::blend(const CColor &src)
{
	if (src.a == 0) return;
	r = (u8)(((int)src.r * (int)src.a + (int)r * (0xFF - (int)src.a)) / 0xFF);
	g = (u8)(((int)src.g * (int)src.a + (int)g * (0xFF - (int)src.a)) / 0xFF);
	b = (u8)(((int)src.b * (int)src.a + (int)b * (0xFF - (int)src.a)) / 0xFF);
}

CColor CColor::interpolate(const CColor &c1, const CColor &c2, u8 n)
{
	CColor c;
	c.r = (u8)(((int)c2.r * (int)n + (int)c1.r * (0xFF - (int)n)) / 0xFF);
	c.g = (u8)(((int)c2.g * (int)n + (int)c1.g * (0xFF - (int)n)) / 0xFF);
	c.b = (u8)(((int)c2.b * (int)n + (int)c1.b * (0xFF - (int)n)) / 0xFF);
	c.a = (u8)(((int)c2.a * (int)n + (int)c1.a * (0xFF - (int)n)) / 0xFF);
	return c;
}

void CVideo::setAA(u8 aa, bool alpha, int width, int height)
{
	if (aa <= 8 && aa != 7 && width <= m_rmode->fbWidth && height <= m_rmode->efbHeight && ((width | height) & 3) == 0)
	{
		m_aa = aa;
		m_aaAlpha = alpha;
		m_aaWidth = width;
		m_aaHeight = height;
	}
}

void CVideo::init(void)
{
	VIDEO_Init();
	m_wide = CONF_GetAspectRatio() == CONF_ASPECT_16_9;
	m_rmode = VIDEO_GetPreferredMode(NULL);

	u32 type = CONF_GetVideo();

	m_rmode->viWidth = m_wide ? 700 : 672;

	//CONF_VIDEO_NTSC and CONF_VIDEO_MPAL and m_rmode TVEurgb60Hz480IntDf are the same max height and width.
	if (type == CONF_VIDEO_PAL && m_rmode != &TVEurgb60Hz480IntDf)
	{
		m_rmode->viHeight = VI_MAX_HEIGHT_PAL;
		m_rmode->viXOrigin = (VI_MAX_WIDTH_PAL - m_rmode->viWidth) / 2;
		m_rmode->viYOrigin = (VI_MAX_HEIGHT_PAL - m_rmode->viHeight) / 2;
	}
	else
	{
		m_rmode->viHeight = VI_MAX_HEIGHT_NTSC;
		m_rmode->viXOrigin = (VI_MAX_WIDTH_NTSC - m_rmode->viWidth) / 2;
		m_rmode->viYOrigin = (VI_MAX_HEIGHT_NTSC - m_rmode->viHeight) / 2;
	}

	s8 hoffset = 0;  //Use horizontal offset set in wii menu.
	if (CONF_GetDisplayOffsetH(&hoffset) == 0)
		m_rmode->viXOrigin += hoffset;

	m_frameBuf[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(m_rmode));
	m_frameBuf[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(m_rmode));
	VIDEO_Configure(m_rmode);
	m_curFB = 0;
	VIDEO_SetNextFramebuffer(m_frameBuf[m_curFB]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if (m_rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
	m_fifo = __real_memalign(32, DEFAULT_FIFO_SIZE);
	memset(m_fifo, 0, DEFAULT_FIFO_SIZE);
	GX_Init(m_fifo, DEFAULT_FIFO_SIZE);
	GX_SetCopyClear(CColor(0), 0x00FFFFFF);
	_setViewPort(0, 0, m_rmode->fbWidth, m_rmode->efbHeight);
	m_yScale = GX_GetYScaleFactor(m_rmode->efbHeight, m_rmode->xfbHeight);
	m_xfbHeight = GX_SetDispCopyYScale(m_yScale);
	GX_SetScissor(0, 0, m_rmode->fbWidth, m_rmode->efbHeight);
	GX_SetDispCopySrc(0, 0, m_rmode->fbWidth, m_rmode->efbHeight);
	GX_SetDispCopyDst(m_rmode->fbWidth, m_xfbHeight);
	GX_SetCopyFilter(m_rmode->aa, m_rmode->sample_pattern, GX_TRUE, m_rmode->vfilter);
	GX_SetFieldMode(m_rmode->field_rendering, ((m_rmode->viHeight == 2 * m_rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
	GX_SetCullMode(GX_CULL_NONE);
	GX_CopyDisp(m_frameBuf[m_curFB], GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGB8, 0);
	GX_SetNumTexGens(0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GX_SetNumChans(0);
	GX_SetZCompLoc(GX_ENABLE);
	setup2DProjection();
	m_stencil = smartMemAlign32(CVideo::_stencilWidth * CVideo::_stencilHeight);
	memset(m_stencil.get(), 0, CVideo::_stencilWidth * CVideo::_stencilHeight);
}

void CVideo::set2DViewport(u32 w, u32 h, int x, int y)
{
	m_width2D = std::min(std::max(512u, w), 800u);
	m_height2D = std::min(std::max(384u, h), 600u);
	m_x2D = std::min(std::max(-50, x), 50);
	m_y2D = std::min(std::max(-50, y), 50);
}

void CVideo::setup2DProjection(bool setViewPort, bool noScale)
{
	Mtx44 projMtx;
	float width2D = noScale ? 640.f : (int)m_width2D;
	float height2D = noScale ? 480.f : (int)m_height2D;
	float x = noScale ? 0.f : (float)(640 - width2D) * 0.5f + (float)m_x2D;
	float y = noScale ? 0.f : (float)(480 - height2D) * 0.5f + (float)m_y2D;

	if (setViewPort)
		_setViewPort(0, 0, m_rmode->fbWidth, m_rmode->efbHeight);
	guOrtho(projMtx, y, height2D + y, x, width2D + x, 0.f, 1000.0f);
	GX_LoadProjectionMtx(projMtx, GX_ORTHOGRAPHIC);
}

void CVideo::renderToTexture(STexture &tex, bool clear)
{
	if (!tex.data) tex.data = smartMem2Alloc(GX_GetTexBufferSize(tex.width, tex.height, tex.format, GX_FALSE, 0));
	if (!tex.data) return;
	GX_DrawDone();
	GX_SetCopyFilter(GX_FALSE, NULL, GX_FALSE, NULL);
	GX_SetTexCopySrc(0, 0, tex.width, tex.height);
	GX_SetTexCopyDst(tex.width, tex.height, tex.format, GX_FALSE);
	GX_CopyTex(tex.data.get(), clear ? GX_TRUE : GX_FALSE);
	GX_PixModeSync();
	GX_SetCopyFilter(m_rmode->aa, m_rmode->sample_pattern, GX_TRUE, m_rmode->vfilter);
	DCFlushRange(tex.data.get(), GX_GetTexBufferSize(tex.width, tex.height, tex.format, GX_FALSE, 0));
	GX_SetScissor(0, 0, m_rmode->fbWidth, m_rmode->efbHeight);
}

void CVideo::prepare(void)
{
	GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
	_setViewPort(0.f, 0.f, (float)m_rmode->fbWidth, (float)m_rmode->efbHeight);
	GX_SetScissor(0, 0, m_rmode->fbWidth, m_rmode->efbHeight);
	GX_InvVtxCache();
	GX_InvalidateTexAll();
}

void CVideo::cleanup(void)
{
	for (u32 i = 0; i < sizeof m_aaBuffer / sizeof m_aaBuffer[0]; ++i)
	{
		SMART_FREE(m_aaBuffer[i]);
		m_aaBufferSize[i] = 0;
	}
}

void CVideo::prepareAAPass(int aaStep)
{
	float x = 0.f;
	float y = 0.f;
	u32 w = m_aaWidth <= 0 ? m_rmode->fbWidth : (u32)m_aaWidth;
	u32 h = m_aaHeight <= 0 ? m_rmode->efbHeight: (u32)m_aaHeight;
	switch (m_aa)
	{
		case 2:
			x += CVideo::_jitter2[aaStep][0];
			y += CVideo::_jitter2[aaStep][1];
			break;
		case 3:
			x += CVideo::_jitter3[aaStep][0];
			y += CVideo::_jitter3[aaStep][1];
			break;
		case 4:
			x += CVideo::_jitter4[aaStep][0];
			y += CVideo::_jitter4[aaStep][1];
			break;
		case 5:
			x += CVideo::_jitter5[aaStep][0];
			y += CVideo::_jitter5[aaStep][1];
			break;
		case 6:
			x += CVideo::_jitter6[aaStep][0];
			y += CVideo::_jitter6[aaStep][1];
			break;
		case 8:
			x += CVideo::_jitter8[aaStep][0];
			y += CVideo::_jitter8[aaStep][1];
			break;
	}
	GX_SetPixelFmt(m_aaAlpha ? GX_PF_RGBA6_Z24 : GX_PF_RGB8_Z24, GX_ZC_LINEAR);
	_setViewPort(x, y, (float)w, (float)h);
	GX_SetScissor(0, 0, w, h);
	GX_InvVtxCache();
	GX_InvalidateTexAll();
}

void CVideo::renderAAPass(int aaStep)
{
	u8 texFmt = GX_TF_RGBA8;
	u32 w = m_aaWidth <= 0 ? m_rmode->fbWidth : (u32)m_aaWidth;
	u32 h = m_aaHeight <= 0 ? m_rmode->efbHeight: (u32)m_aaHeight;
	u32 bufLen = GX_GetTexBufferSize(w, h, texFmt, GX_FALSE, 0);

	if (!m_aaBuffer[aaStep] || m_aaBufferSize[aaStep] < bufLen)
	{
		m_aaBuffer[aaStep] = smartMem2Alloc(bufLen);
		if (!!m_aaBuffer[aaStep])
			m_aaBufferSize[aaStep] = bufLen;
	}
	if (!m_aaBuffer[aaStep] || m_aaBufferSize[aaStep] < bufLen)
		return;
	GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_TRUE);
	GX_DrawDone();
	GX_SetCopyFilter(GX_FALSE, NULL, GX_FALSE, NULL);
	GX_SetTexCopySrc(0, 0, w, h);
	GX_SetTexCopyDst(w, h, texFmt, GX_FALSE);
	GX_CopyTex(m_aaBuffer[aaStep].get(), GX_TRUE);
	GX_PixModeSync();
	GX_SetCopyFilter(m_rmode->aa, m_rmode->sample_pattern, GX_TRUE, m_rmode->vfilter);
}

void CVideo::drawAAScene(bool fs)
{
	GXTexObj texObj[8];
	Mtx modelViewMtx;
	u8 texFmt = GX_TF_RGBA8;
	u32 tw = m_aaWidth <= 0 ? m_rmode->fbWidth : (u32)m_aaWidth;
	u32 th = m_aaHeight <= 0 ? m_rmode->efbHeight: (u32)m_aaHeight;
	float w = fs ? 640.f : (float)tw;
	float h = fs ? 480.f : (float)th;
	float x = 0.f;
	float y = 0.f;
	int aa;

	if (m_aa <= 0 || m_aa > 8)
		return;
	// 
	for (aa = 0; aa < m_aa; ++aa)
		if (!m_aaBuffer[aa])
			break;
	if (aa == 7)
		aa = 6;
	// 
	GX_SetNumChans(0);
	for (int i = 0; i < aa; ++i)
	{
		GX_InitTexObj(&texObj[i], m_aaBuffer[i].get(), tw , th, texFmt, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_LoadTexObj(&texObj[i], GX_TEXMAP0 + i);
	}
	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetTevKColor(GX_KCOLOR0, CColor(0xFF / 1, 0xFF / 5, 0xFF, 0xFF));	// Renders better gradients than 0xFF / aa
	GX_SetTevKColor(GX_KCOLOR1, CColor(0xFF / 2, 0xFF / 6, 0xFF, 0xFF));
	GX_SetTevKColor(GX_KCOLOR2, CColor(0xFF / 3, 0xFF / 7, 0xFF, 0xFF));
	GX_SetTevKColor(GX_KCOLOR3, CColor(0xFF / 4, 0xFF / 8, 0xFF, 0xFF));
	for (int i = 0; i < aa; ++i)
	{
		GX_SetTevKColorSel(GX_TEVSTAGE0 + i, GX_TEV_KCSEL_K0_R + i);
		GX_SetTevKAlphaSel(GX_TEVSTAGE0 + i, GX_TEV_KASEL_K0_R + i);
		GX_SetTevOrder(GX_TEVSTAGE0 + i, GX_TEXCOORD0, GX_TEXMAP0 + i, GX_COLORNULL);
		GX_SetTevColorIn(GX_TEVSTAGE0 + i, i == 0 ? GX_CC_ZERO : GX_CC_CPREV, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
		GX_SetTevAlphaIn(GX_TEVSTAGE0 + i, i == 0 ? GX_CA_ZERO : GX_CA_APREV, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
		GX_SetTevColorOp(GX_TEVSTAGE0 + i, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GX_SetTevAlphaOp(GX_TEVSTAGE0 + i, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	}
	GX_SetNumTevStages(aa);
	// 
	GX_SetAlphaUpdate(GX_TRUE);
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_FALSE);
	GX_SetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	// 
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	guMtxIdentity(modelViewMtx);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(x, y, 0.f);
	GX_TexCoord2f32(0.f, 0.f);
	GX_Position3f32(x + w, y, 0.f);
	GX_TexCoord2f32(1.f, 0.f);
	GX_Position3f32(x + w, y + h, 0.f);
	GX_TexCoord2f32(1.f, 1.f);
	GX_Position3f32(x, y + h, 0.f);
	GX_TexCoord2f32(0.f, 1.f);
	GX_End();
	GX_SetNumChans(1);
	GX_SetNumTexGens(1);
	GX_SetNumTevStages(1);
}

void CVideo::_setViewPort(float x, float y, float w, float h)
{
	m_vpX = x;
	m_vpY = y;
	m_vpW = w;
	m_vpH = h;
	GX_SetViewport(x, y, w, h, 0.f, 1.f);
}

void CVideo::shiftViewPort(float x, float y)
{
	GX_SetViewport(m_vpX + x, m_vpY + y, m_vpW, m_vpH, 0.f, 1.f);
}

static inline u32 coordsI8(u32 x, u32 y, u32 w)
{ 
	return (((y >> 2) * (w >> 3) + (x >> 3)) << 5) + ((y & 3) << 3) + (x & 7);
}

int CVideo::stencilVal(int x, int y)
{
	if ((u32)x >= m_rmode->fbWidth || (u32)y >= m_rmode->efbHeight)
		return 0;
	x = x * CVideo::_stencilWidth / 640;
	y = y * CVideo::_stencilHeight / 480;
	u32 i = coordsI8(x, y, (u32)CVideo::_stencilWidth);
	if (i >= (u32)(CVideo::_stencilWidth * CVideo::_stencilHeight))
		return 0;
	return m_stencil.get()[i];
}

void CVideo::prepareStencil(void)
{
	GX_SetPixelFmt(GX_PF_Y8, GX_ZC_LINEAR);
	_setViewPort(0.f, 0.f, (float)CVideo::_stencilWidth, (float)CVideo::_stencilHeight);
	GX_SetScissor(0, 0, CVideo::_stencilWidth, CVideo::_stencilHeight);
	GX_InvVtxCache();
	GX_InvalidateTexAll();
}

void CVideo::renderStencil(void)
{
	GX_DrawDone();
	GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_SetCopyFilter(GX_FALSE, NULL, GX_FALSE, NULL);
	GX_SetTexCopySrc(0, 0, CVideo::_stencilWidth, CVideo::_stencilHeight);
	GX_SetTexCopyDst(CVideo::_stencilWidth, CVideo::_stencilHeight, GX_CTF_R8, GX_FALSE);
	GX_CopyTex(m_stencil.get(), GX_TRUE);
	GX_PixModeSync();
	DCFlushRange(m_stencil.get(), CVideo::_stencilWidth * CVideo::_stencilHeight);
	GX_SetCopyFilter(m_rmode->aa, m_rmode->sample_pattern, GX_TRUE, m_rmode->vfilter);
}

void CVideo::render(void)
{
	GX_DrawDone();
	GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_CopyDisp(MEM_K1_TO_K0(m_frameBuf[m_curFB]), GX_TRUE);
	DCFlushRange(m_frameBuf[m_curFB], 2 * m_rmode->fbWidth * m_rmode->xfbHeight);
	VIDEO_SetNextFramebuffer(m_frameBuf[m_curFB]);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	m_curFB ^= 1;
	GX_InvalidateTexAll();
}

void CVideo::_showWaitMessages(CVideo *m)
{
	m->m_showingWaitMessages = true;
	u32 frames = m->m_waitMessageDelay * 50;
	u32 waitFrames = frames;
	
	u8 fadeStep = 2 * (u32) (255.f / (waitFrames * m->m_waitMessages.size()));
	s8 fadeDirection = 1;
	s16 currentLightLevel = 0;

	safe_vector<STexture>::iterator waitItr = m->m_waitMessages.begin();

	gprintf("Going to show a wait message screen, delay: %d, # images: %d\n", waitFrames, m->m_waitMessages.size());

	m->waitMessage(*waitItr);
	waitItr++;

	if (m->m_useWiiLight)
	{
		WIILIGHT_SetLevel(0);
		WIILIGHT_TurnOn();
	}
	while (m->m_showWaitMessage)
	{
		if (m->m_useWiiLight)
		{
			currentLightLevel += (fadeStep * fadeDirection);
			if (currentLightLevel >= 255) 
			{
				currentLightLevel = 255;
				fadeDirection = -1;
			}
			else if (currentLightLevel <= 0)
			{
				currentLightLevel = 0;
				fadeDirection = 1;
			}
			WIILIGHT_SetLevel(currentLightLevel);
		}
		
		if (waitFrames == 0)
		{
			if (waitItr == m->m_waitMessages.end())
				waitItr = m->m_waitMessages.begin();
			
			while (!*waitItr->data) 
			{
				gprintf("Skipping one image, because loaded data is not valid\n");
				waitItr++;

				if (waitItr == m->m_waitMessages.end())
					waitItr = m->m_waitMessages.begin();
			}
			
			m->waitMessage(*waitItr);
			waitItr++;
			
			waitFrames = frames;
		}
		waitFrames--;
		VIDEO_WaitVSync();
	}
	if (m->m_useWiiLight)
	{
		WIILIGHT_SetLevel(0);
		WIILIGHT_TurnOff();
	}
	m->m_waitMessages.clear();
	gprintf("Stop showing images\n");
	m->m_showingWaitMessages = false;
}

void CVideo::hideWaitMessage(bool force)
{
	m_showWaitMessage = false;
	CheckWaitThread(force);
}

void CVideo::CheckWaitThread(bool force)
{
	if (force || (!m_showingWaitMessages && waitThread != LWP_THREAD_NULL))
	{
		m_showWaitMessage = false;

		if(LWP_ThreadIsSuspended(waitThread))
			LWP_ResumeThread(waitThread);

		LWP_JoinThread(waitThread, NULL);

		SMART_FREE(waitThreadStack);
		waitThread = LWP_THREAD_NULL;

		m_waitMessages.clear();
	}
}

void CVideo::waitMessage(float delay)
{
	waitMessage(safe_vector<STexture>(), delay);
}

void CVideo::waitMessage(const safe_vector<STexture> &tex, float delay, bool useWiiLight)
{
	hideWaitMessage(true);

	m_useWiiLight = useWiiLight;

	if (tex.size() == 0)
	{
		STexture m_wTextures[10];
		m_wTextures[0].fromPNG(wait_01_png);
		m_wTextures[1].fromPNG(wait_02_png);
		m_wTextures[2].fromPNG(wait_03_png);
		m_wTextures[3].fromPNG(wait_04_png);
		m_wTextures[4].fromPNG(wait_05_png);
		m_wTextures[5].fromPNG(wait_06_png);
		m_wTextures[6].fromPNG(wait_07_png);
		m_wTextures[7].fromPNG(wait_08_png);
		m_wTextures[8].fromPNG(wait_09_png);
		m_wTextures[9].fromPNG(wait_10_png);
		m_waitMessages.reserve(10);
		for (int i = 0; i < 10; i++)
			m_waitMessages.push_back(m_wTextures[i]);

		m_waitMessageDelay = 0.2f;
	}
	else
	{
		m_waitMessages = tex;
		m_waitMessageDelay = delay;
	}
	
	if (m_waitMessages.size() == 1)
		waitMessage(m_waitMessages[0]);
	else if (m_waitMessages.size() > 1)
	{
		m_showWaitMessage = true;
		unsigned int stack_size = (unsigned int)32768;  //Try 32768?
		SMART_FREE(waitThreadStack);
		waitThreadStack = SmartBuf((unsigned char *)__real_malloc(stack_size), SmartBuf::SRCALL_MALLOC);
		waitThreadStack = smartMem2Alloc(stack_size);
		LWP_CreateThread(&waitThread, (void *(*)(void *))CVideo::_showWaitMessages, (void *)this, waitThreadStack.get(), stack_size, 30);
	}
}

void CVideo::waitMessage(const STexture &tex)
{
	Mtx modelViewMtx;
	GXTexObj texObj;

	for (int i = 0; i < 3; ++i)
	{
		prepare();
		setup2DProjection();
		//prepareAAPass(i);
		//setup2DProjection(false, true);
		GX_SetNumChans(0);
		GX_ClearVtxDesc();
		GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
		GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
		GX_SetNumTexGens(1);
		GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
		GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
		GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
		GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
		GX_SetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
		GX_SetAlphaUpdate(GX_TRUE);
		GX_SetCullMode(GX_CULL_NONE);
		GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_FALSE);
		guMtxIdentity(modelViewMtx);
		GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
		GX_InitTexObj(&texObj, tex.data.get(), tex.width, tex.height, tex.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_LoadTexObj(&texObj, GX_TEXMAP0);
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32((float)((640 - tex.width) / 2), (float)((480 - tex.height) / 2), 0.f);
		GX_TexCoord2f32(0.f, 0.f);
		GX_Position3f32((float)((640 + tex.width) / 2), (float)((480 - tex.height) / 2), 0.f);
		GX_TexCoord2f32(1.f, 0.f);
		GX_Position3f32((float)((640 + tex.width) / 2), (float)((480 + tex.height) / 2), 0.f);
		GX_TexCoord2f32(1.f, 1.f);
		GX_Position3f32((float)((640 - tex.width) / 2), (float)((480 + tex.height) / 2), 0.f);
		GX_TexCoord2f32(0.f, 1.f);
		GX_End();
		render();
	}
	GX_SetNumChans(1);
}

s32 CVideo::TakeScreenshot(const char *path)
{
	IMGCTX ctx = PNGU_SelectImageFromDevice (path);
	s32 ret = PNGU_EncodeFromYCbYCr(ctx, m_rmode->fbWidth, m_rmode->efbHeight, m_frameBuf[m_curFB], 1);
	PNGU_ReleaseImageContext (ctx);
	return ret;
}