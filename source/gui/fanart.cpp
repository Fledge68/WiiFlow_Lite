
#include "fanart.hpp"
#include "memory/mem2.hpp"
#include "types.h"

using namespace std;

static  guVector  _GRRaxisx = (guVector){1, 0, 0}; // DO NOT MODIFY!!!
static  guVector  _GRRaxisy = (guVector){0, 1, 0}; // Even at runtime
static  guVector  _GRRaxisz = (guVector){0, 0, 1}; // NOT ever!

CFanart::CFanart(void)
	: m_animationComplete(false), m_loaded(false), m_faConfig(), m_bg(), m_bglq()
{
}

CFanart::~CFanart(void)
{
}

void CFanart::unload()
{
	m_faConfig.unload();
	m_loaded = false;
	for(vector<CFanartElement>::iterator Elm = m_elms.begin(); Elm != m_elms.end(); Elm++)
		Elm->Cleanup();
	m_elms.clear();
	TexHandle.Cleanup(m_bg);
	TexHandle.Cleanup(m_bglq);
}

char fanartDir[164];
bool CFanart::load(Config &m_wiiflowConfig, const char *path, const dir_discHdr *hdr)
{
	if(!m_wiiflowConfig.getBool("FANART", "enable_fanart", true))
		return false;

	unload();

	char id[64];
	memset(id, 0, sizeof(id));
	if(NoGameID(hdr->type))
	{
		if(strrchr(hdr->path, '/') != NULL)
			wcstombs(id, hdr->title, sizeof(id) - 1);
		else
			strncpy(id, hdr->path, sizeof(id) - 1);// scummvm
	}
	else
		strcpy(id, hdr->id);
		
	fanartDir[163] = '\0';
	strncpy(fanartDir, fmt("%s/%s", path, id), 163);

	TexErr texErr = TexHandle.fromImageFile(m_bg, fmt("%s/background.png", fanartDir));
	if(texErr == TE_ERROR && !NoGameID(hdr->type))
	{
		strncpy(fanartDir, fmt("%s/%.3s", path, id), 163);
		texErr = TexHandle.fromImageFile(m_bg, fmt("%s/background.png", fanartDir));
	}
	if(texErr == TE_OK)
	{
		char faConfig_Path[164];
		faConfig_Path[163] = '\0';
		strncpy(faConfig_Path, fmt("%s/%s.ini", fanartDir, id), 163);
		m_faConfig.load(faConfig_Path);
		if(!m_faConfig.loaded() && !NoGameID(hdr->type))
		{
			strncpy(faConfig_Path, fmt("%s/%.3s.ini", fanartDir, id), 163);
			m_faConfig.load(faConfig_Path);
			if(!m_faConfig.loaded())
			{
				TexHandle.Cleanup(m_bg);
				return false;
			}
		}
		TexHandle.fromImageFile(m_bglq, fmt("%s/background_lq.png", fanartDir));
		for(int i = 1; i <= 6; i++)
		{
			CFanartElement elm(m_faConfig, fanartDir, i);
			if (elm.IsValid()) m_elms.push_back(elm);
		}
		m_loaded = true;
		m_defaultDelay = m_wiiflowConfig.getInt("FANART", "delay_after_animation", 200);
		m_delayAfterAnimation = m_faConfig.getInt("GENERAL", "delay_after_animation", m_defaultDelay);
		m_globalShowCoverAfterAnimation = m_wiiflowConfig.getOptBool("FANART", "show_cover_after_animation", 2);
	}
	return true;
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

void CFanart::reset()
{
	for(vector<CFanartElement>::iterator Elm = m_elms.begin(); Elm != m_elms.end(); Elm++)
		Elm->Cleanup();
	m_elms.clear();
	for(int i = 1; i <= 6; i++)
	{
		CFanartElement elm(m_faConfig, fanartDir, i);
		if (elm.IsValid()) m_elms.push_back(elm);
	}
}

bool CFanart::noLoop()
{
	if(m_globalShowCoverAfterAnimation != 2)
		return m_globalShowCoverAfterAnimation == 1;
	return m_faConfig.getBool("GENERAL", "show_cover_after_animation", false);
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
	if(!m_loaded) return;
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

void CFanart::draw()
{
	if(!m_loaded) return;

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
		m_elms[i].draw();
}

CFanartElement::CFanartElement(Config &cfg, const char *dir, int artwork)
	: m_artwork(artwork), m_isValid(false)
{
	m_isValid = (TexHandle.fromImageFile(m_art, fmt("%s/artwork%d.png", dir, artwork)) == TE_OK);
	if(!m_isValid)
		return;

	char *domain = fmt_malloc("artwork%d", artwork);
	if(domain == NULL)
		return;

	m_x = cfg.getInt(domain, "x", 0);
	m_y = cfg.getInt(domain, "y", 0);
	m_scaleX = cfg.getFloat(domain, "scale_x", 1.f);
	m_scaleY = cfg.getFloat(domain, "scale_y", 1.f);
	m_alpha = min(cfg.getInt(domain, "alpha", 255), 255);
	m_delay = (int) (cfg.getFloat(domain, "delay", 0.f) * 50);
	m_angle = cfg.getFloat(domain, "angle", 0.f);

	m_event_duration = (int) (cfg.getFloat(domain, "duration", 0.f) * 50);
	m_event_x = m_event_duration == 0 ? m_x : cfg.getInt(domain, "event_x", m_x);
	m_event_y = m_event_duration == 0 ? m_y : cfg.getInt(domain, "event_y", m_y);
	m_event_scaleX = m_event_duration == 0 ? m_scaleX : cfg.getInt(domain, "event_scale_x", m_scaleX);
	m_event_scaleY = m_event_duration == 0 ? m_scaleY : cfg.getInt(domain, "event_scale_y", m_scaleY);
	m_event_alpha = m_event_duration == 0 ? m_alpha : min(cfg.getInt(domain, "event_alpha", m_alpha), 255); // Not from m_alpha, because the animation can start less translucent than m_alpha
	m_event_angle = m_event_duration == 0 ? m_angle : cfg.getFloat(domain, "event_angle", m_angle);

	m_step_x = m_event_duration == 0 ? 0 : (m_x - m_event_x) / m_event_duration;
	m_step_y = m_event_duration == 0 ? 0 : (m_y - m_event_y) / m_event_duration;
	m_step_scaleX = m_event_duration == 0 ? 0 : (m_scaleX - m_event_scaleX) / m_event_duration;
	m_step_scaleY = m_event_duration == 0 ? 0 : (m_scaleY - m_event_scaleY) / m_event_duration;
	m_step_alpha = m_event_duration == 0 ? 0 : (m_alpha - m_event_alpha) / m_event_duration;
	m_step_angle = m_event_duration == 0 ? 0 : (m_angle - m_event_angle) / m_event_duration;

	MEM2_free(domain);
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
