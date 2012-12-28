#include "fanart.hpp"
#include "pngu.h"
#include "boxmesh.hpp"
#include "text.hpp"
#include "gecko/gecko.hpp"

using namespace std;

static  guVector  _GRRaxisx = (guVector){1, 0, 0}; // DO NOT MODIFY!!!
static  guVector  _GRRaxisy = (guVector){0, 1, 0}; // Even at runtime
static  guVector  _GRRaxisz = (guVector){0, 0, 1}; // NOT ever!

CFanart::CFanart(void)
	: m_animationComplete(false), m_loaded(false), m_cfg(), m_bg(), m_bglq()
{
}

CFanart::~CFanart(void)
{
}

void CFanart::unload()
{
	m_cfg.unload();
	m_loaded = false;
	for(vector<CFanartElement>::iterator Elm = m_elms.begin(); Elm != m_elms.end(); Elm++)
		Elm->Cleanup();
	m_elms.clear();
	TexHandle.Cleanup(m_bg);
	TexHandle.Cleanup(m_bglq);
}

bool CFanart::load(Config &m_globalConfig, const char *path, const char *id)
{
	bool retval = false;

	if(!m_globalConfig.getBool("FANART", "enable_fanart", true))
		return retval;

	unload();

	char dir[64];
	dir[63] = '\0';
	strncpy(dir, fmt("%s/%s", path, id), 63);

	TexErr texErr = TexHandle.fromImageFile(m_bg, fmt("%s/background.png", dir));
	if(texErr == TE_ERROR)
	{
		strncpy(dir, fmt("%s/%.3s", path, id), 63);
		texErr = TexHandle.fromImageFile(m_bg, fmt("%s/background.png", dir));
	}
	if(texErr == TE_OK)
	{
		char cfg_char[64];
		cfg_char[63] = '\0';
		strncpy(cfg_char, fmt("%s/%s.ini", dir, id), 63);
		m_cfg.load(cfg_char);
		if(!m_cfg.loaded())
		{
			strncpy(cfg_char, fmt("%s/%.3s.ini", dir, id), 63);
			m_cfg.load(cfg_char);
		}
		TexHandle.fromImageFile(m_bglq, fmt("%s/background_lq.png", dir));
		for(int i = 1; i <= 6; i++)
		{
			CFanartElement elm(m_cfg, dir, i);
			if (elm.IsValid()) m_elms.push_back(elm);
		}
		m_loaded = true;
		retval = true;
		m_defaultDelay = m_globalConfig.getInt("FANART", "delay_after_animation", 200);
		m_delayAfterAnimation = m_cfg.getInt("GENERAL", "delay_after_animation", m_defaultDelay);
		m_allowArtworkOnTop = m_globalConfig.getBool("FANART", "allow_artwork_on_top", true);
		m_globalHideCover = m_globalConfig.getOptBool("FANART", "hidecover", 2); // 0 is false, 1 is true, 2 is default
		m_globalShowCoverAfterAnimation = m_globalConfig.getOptBool("FANART", "show_cover_after_animation", 2);
	}
	return retval;
}

void CFanart::getBackground(const TexData * &hq, const TexData * &lq)
{
	if(m_loaded)
	{
		hq = &m_bg;
		lq = &m_bglq;
	}
	if(lq == NULL || lq->data == NULL)
		lq = hq;
}

CColor CFanart::getTextColor(CColor themeTxtColor)
{
	return m_loaded ? m_cfg.getColor("GENERAL", "textcolor", CColor(themeTxtColor)) : themeTxtColor;
}

bool CFanart::hideCover()
{
	if(!m_loaded)
		return false; // If no fanart is loaded, return false
/*

fanart_hidecover defaults to True
fanart_showafter defaults to False

 hideCover | fanart_hideCover | showAfter | fanart_showAfter | animating | hide
1   True              *             *              *               *       True
2   False             *             *              *               *       False
3  default          False           *              *               *       False
4  default      default/True      True             *             True      True 
5  default      default/True      True             *             False     False
6  default      default/True      False            *               *       True
7  default      default/True     default          True           True      True
8  default      default/True     default          True           False     False
9    *                *               *            *               *       True   
*/
	// rules 1 and 2
	if(m_globalHideCover != 2)
		return m_globalHideCover == 1;
	// rule 3
	if(!m_cfg.getBool("GENERAL", "hidecover", true))
		return false;
	// rules 4, 5 and 6
	if(m_globalShowCoverAfterAnimation != 2)
		return m_globalShowCoverAfterAnimation == 0 || !isAnimationComplete();
	// rules 7 and 8
	if(m_cfg.getBool("GENERAL", "show_cover_after_animation", false))
		return !isAnimationComplete();
	// rule 9
	return true;
}

bool CFanart::isLoaded()
{
	return m_loaded;
}

bool CFanart::isAnimationComplete()
{
	return m_animationComplete && m_delayAfterAnimation <= 0;
}

void CFanart::tick()
{
	m_animationComplete = true;
	for(u32 i = 0; i < m_elms.size(); ++i)
	{
		m_elms[i].tick();
		if(!m_elms[i].IsAnimationComplete())
			m_animationComplete = false;
	}
	if(m_animationComplete && m_delayAfterAnimation > 0)
		m_delayAfterAnimation--;
}

void CFanart::draw(bool front)
{
	if(!m_loaded) return; //derp
	if(!m_allowArtworkOnTop && front)
		return;	// It's not allowed to draw fanart on top, it has already been drawn

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

	for(u32 i = 0; i < m_elms.size(); ++i)
		if(!m_allowArtworkOnTop || ((front && m_elms[i].ShowOnTop()) || !front))
			m_elms[i].draw();
}

CFanartElement::CFanartElement(Config &cfg, const char *dir, int artwork)
	: m_artwork(artwork), m_isValid(false)
{
	m_isValid = (TexHandle.fromImageFile(m_art, fmt("%s/artwork%d.png", dir, artwork)) == TE_OK);
	if (!m_isValid)	return;

	const char *section = fmt("artwork%d", artwork);

	m_show_on_top = cfg.getBool(section, "show_on_top", true);

	m_x = cfg.getInt(section, "x", 0);
	m_y = cfg.getInt(section, "y", 0);
	m_scaleX = cfg.getFloat(section, "scale_x", 1.f);
	m_scaleY = cfg.getFloat(section, "scale_y", 1.f);
	m_alpha = min(cfg.getInt(section, "alpha", 255), 255);
	m_delay = (int) (cfg.getFloat(section, "delay", 0.f) * 50);
	m_angle = cfg.getFloat(section, "angle", 0.f);

	m_event_duration = (int) (cfg.getFloat(section, "duration", 0.f) * 50);
	m_event_x = m_event_duration == 0 ? m_x : cfg.getInt(section, "event_x", m_x);
	m_event_y = m_event_duration == 0 ? m_y : cfg.getInt(section, "event_y", m_y);
	m_event_scaleX = m_event_duration == 0 ? m_scaleX : cfg.getInt(section, "event_scale_x", m_scaleX);
	m_event_scaleY = m_event_duration == 0 ? m_scaleY : cfg.getInt(section, "event_scale_y", m_scaleY);
	m_event_alpha = m_event_duration == 0 ? m_alpha : min(cfg.getInt(section, "event_alpha", m_alpha), 255); // Not from m_alpha, because the animation can start less translucent than m_alpha
	m_event_angle = m_event_duration == 0 ? m_angle : cfg.getFloat(section, "event_angle", m_angle);

	m_step_x = m_event_duration == 0 ? 0 : (m_x - m_event_x) / m_event_duration;
	m_step_y = m_event_duration == 0 ? 0 : (m_y - m_event_y) / m_event_duration;
	m_step_scaleX = m_event_duration == 0 ? 0 : (m_scaleX - m_event_scaleX) / m_event_duration;
	m_step_scaleY = m_event_duration == 0 ? 0 : (m_scaleY - m_event_scaleY) / m_event_duration;
	m_step_alpha = m_event_duration == 0 ? 0 : (m_alpha - m_event_alpha) / m_event_duration;
	m_step_angle = m_event_duration == 0 ? 0 : (m_angle - m_event_angle) / m_event_duration;
}

void CFanartElement::Cleanup(void)
{
	TexHandle.Cleanup(m_art);
}

bool CFanartElement::IsValid()
{
	return m_isValid;
}

bool CFanartElement::IsAnimationComplete()
{
	return m_event_duration == 0;
}

bool CFanartElement::ShowOnTop()
{
	return m_show_on_top;
}

void CFanartElement::tick()
{
	if(m_delay > 0)
	{
		m_delay--;
		return;
	}

	if((m_step_x < 0 && m_event_x > m_x) || (m_step_x > 0 && m_event_x < m_x))
		m_event_x = (int) (m_event_x + m_step_x);
	if((m_step_y < 0 && m_event_y > m_y) || (m_step_y > 0 && m_event_y < m_y))
		m_event_y = (int) (m_event_y + m_step_y);
	if((m_step_alpha < 0 && m_event_alpha > m_alpha) || (m_step_alpha > 0 && m_event_alpha < m_alpha))
		m_event_alpha = (int) (m_event_alpha + m_step_alpha);
	if((m_step_scaleX < 0 && m_event_scaleX > m_scaleX) || (m_step_scaleX > 0 && m_event_scaleX < m_scaleX))
		m_event_scaleX += m_step_scaleX;
	if((m_step_scaleY < 0 && m_event_scaleY > m_scaleY) || (m_step_scaleY > 0 && m_event_scaleY < m_scaleY))
		m_event_scaleY += m_step_scaleY;
	if((m_step_angle < 0 && m_event_angle > m_angle) || (m_step_angle > 0 && m_event_angle < m_angle))
		m_event_angle = (int) (m_event_angle + m_step_angle);

	if(m_event_duration > 0)
		m_event_duration--;
}

void CFanartElement::draw()
{
	if(m_event_alpha == 0 || m_event_scaleX == 0.f || m_event_scaleY == 0.f || m_delay > 0)
		return;

	GXTexObj artwork;
	Mtx modelViewMtx, idViewMtx, rotViewMtxZ;

	guMtxIdentity(idViewMtx);
	guMtxScaleApply(idViewMtx, idViewMtx, m_event_scaleX, m_event_scaleY, 1.f);

	guMtxRotAxisDeg(rotViewMtxZ, &_GRRaxisz, m_event_angle);
	guMtxConcat(rotViewMtxZ, idViewMtx, modelViewMtx);

	guMtxTransApply(modelViewMtx, modelViewMtx, m_event_x, m_event_y, 0.f);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);

	GX_InitTexObj(&artwork, m_art.data, m_art.width, m_art.height, m_art.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_LoadTexObj(&artwork, GX_TEXMAP0);

	float w = (float)(m_art.width / 2); // * m_event_scaleX;
	float h = (float)(m_art.height / 2); // * m_event_scaleY;

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);

	// Draw top left
	GX_Position3f32(-w, -h, 0.f);
	GX_Color4u8(0xFF, 0xFF, 0xFF, m_event_alpha);
	GX_TexCoord2f32(0.f, 0.f);

	// Draw top right
	GX_Position3f32(w, -h, 0.f);
	GX_Color4u8(0xFF, 0xFF, 0xFF, m_event_alpha);
	GX_TexCoord2f32(1.f, 0.f);

	// Draw bottom right
	GX_Position3f32(w, h, 0.f);
	GX_Color4u8(0xFF, 0xFF, 0xFF, m_event_alpha);
	GX_TexCoord2f32(1.f, 1.f);

	// Draw bottom left
	GX_Position3f32(-w, h, 0.f);
	GX_Color4u8(0xFF, 0xFF, 0xFF, m_event_alpha);
	GX_TexCoord2f32(0.f, 1.f);
	GX_End();
}