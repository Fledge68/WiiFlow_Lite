#include "gui.hpp"
#include <algorithm>

using namespace std;

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

TexData CButtonsMgr::_noTexture;

CButtonsMgr m_btnMgr;

bool CButtonsMgr::init()
{
	m_elts.clear();
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		m_selected[chan] = -1;
		m_rumble[chan] = 0;
	}
	m_rumbleEnabled = false;
	m_soundVolume = 0xFF;
	m_noclick = false;
	m_nohover = false;
	m_mouse = false;
	soundInit();

	return true;
}

s16 CButtonsMgr::addButton(SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color,
	const SButtonTextureSet &texSet, GuiSound *clickSound, GuiSound *hoverSound)
{
	SButton *b = new SButton;

	b->font = font;
	b->visible = false;
	//b->text = text;
	b->text.setText(b->font, text);
	b->textColor = color;
	b->x = x + width / 2;
	b->y = y + height / 2;
	b->w = width;
	b->h = height;
	b->alpha = 0;
	b->targetAlpha = 0;
	b->scaleX = 0.f;
	b->scaleY = 0.f;
	b->targetScaleX = 0.f;
	b->targetScaleY = 0.f;
	b->click = 0.f;
	b->tex = texSet;
	b->clickSound = clickSound;
	b->hoverSound = hoverSound;
	b->moveByX = 0;
	b->moveByY = 0;

	u16 sz = m_elts.size();
	m_elts.push_back(b);

	return m_elts.size() > sz ? m_elts.size() - 1 : -2;
}

void CButtonsMgr::reset(s16 id, bool instant)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		SElement &b = *m_elts[id];
		b.x -= b.moveByX;
		b.y -= b.moveByY;
		if (instant)
		{
			b.pos.x -= b.moveByX;
			b.pos.y -= b.moveByY;
		}
		b.targetPos.x -= b.moveByX;
		b.targetPos.y -= b.moveByY;
		b.moveByX = 0;
		b.moveByY = 0;
	}
}

void CButtonsMgr::moveBy(s16 id, int x, int y, bool instant)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		CButtonsMgr::SElement &b = *m_elts[id];
		b.moveByX += x;
		b.moveByY += y;
		b.x += x;
		b.y += y;
		if (instant)
		{
			b.pos.x += x;
			b.pos.y += y;
		}
		b.targetPos.x += x;
		b.targetPos.y += y;
	}
}

void CButtonsMgr::getDimensions(s16 id, int &x, int &y, u32 &width, u32 &height)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		CButtonsMgr::SElement &b = *m_elts[id];
		x = b.targetPos.x;
		y = b.targetPos.y;
		width = b.w;
		height = b.h;
		if(b.t == GUIELT_LABEL)
		{
			SLabel *s = (SLabel*)m_elts[id];
			// Calculate height
			height = s->text.getTotalHeight();
		}
	}
}

void CButtonsMgr::hide(s16 id, int dx, int dy, float scaleX, float scaleY, bool instant)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		SElement &b = *m_elts[id];
		b.hideParam.dx = dx;
		b.hideParam.dy = dy;
		b.hideParam.scaleX = scaleX;
		b.hideParam.scaleY = scaleY;
		b.visible = false;
		b.targetScaleX = scaleX;
		b.targetScaleY = scaleY;
		b.targetPos = Vector3D((float)(b.x + dx), (float)(b.y + dy), 0.f);
		b.targetAlpha = 0x00;
		if (instant)
		{
			b.scaleX = b.targetScaleX;
			b.scaleY = b.targetScaleY;
			b.pos = b.targetPos;
			b.alpha = b.targetAlpha;
		}
		for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
			if (m_selected[chan] == id)
				m_selected[chan] = -1;
	}
}

void CButtonsMgr::hide(s16 id, bool instant)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		CButtonsMgr::SElement &b = *m_elts[id];
		hide(id, b.hideParam.dx, b.hideParam.dy, b.hideParam.scaleX, b.hideParam.scaleY, instant);
	}
}

void CButtonsMgr::stopSounds(void)
{
	for (u32 i = 0; i < m_elts.size(); ++i)
		if (m_elts[i]->t == GUIELT_BUTTON)
		{
			SButton *b = (SButton*)m_elts[i];
			b->hoverSound->Stop();
			b->clickSound->Stop();
		}
}

void CButtonsMgr::setSoundVolume(int vol)
{
	m_soundVolume = min(max(0, vol), 0xFF);
}

void CButtonsMgr::show(s16 id, bool instant)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		SElement &b = *m_elts[id];
		b.visible = true;
		b.targetScaleX = 1.0f;
		b.targetScaleY = 1.0f;
		b.targetPos = Vector3D((float)b.x, (float)b.y, 0);
		b.targetAlpha = 0xFF;
		if (instant)
		{
			b.scaleX = b.targetScaleX;
			b.scaleY = b.targetScaleY;
			b.pos = b.targetPos;
			b.alpha = b.targetAlpha;
		}
	}
}

void CButtonsMgr::setRumble(int chan, bool wii, bool gc)
{
	wii_rumble[chan] = wii;
	gc_rumble[chan] = gc;
}

void CButtonsMgr::mouse(int chan, int x, int y)
{
	if (m_elts.empty()) return;

	float w, h;
	u16 start = 0;
	if(m_selected[chan] != -1 && m_selected[chan] < (s32)m_elts.size())
	{
		m_elts[m_selected[chan]]->targetScaleX = 1.f;
		m_elts[m_selected[chan]]->targetScaleY = 1.f;
		start = (u16)m_selected[chan];
	}
	m_selected[chan] = -1;
	for(int i = (int)m_elts.size() - 1; i >= 0; --i)
	{
		SElement &b = *m_elts[i];
		if(b.t == GUIELT_BUTTON)
		{
			SButton &but = *(SButton*)&b;
			w = (float)(but.w / 2);
			h = (float)(but.h / 2);
			if(but.visible && (float)x >= but.pos.x - w && (float)x < but.pos.x + w && (float)y >= but.pos.y - h && (float)y < but.pos.y + h)
			{
				m_selected[chan] = i;
				but.targetScaleX = 1.05f;
				but.targetScaleY = 1.05f;
				// 
				if(start != m_selected[chan])
				{
					if(m_soundVolume > 0)
					{
						if(!m_nohover) 
							but.hoverSound->Play(m_soundVolume);
					}
					if(m_rumbleEnabled)
					{
						m_rumble[chan] = 4;
						if(wii_rumble[chan]) WPAD_Rumble(chan, 1);
						if(gc_rumble[chan]) PAD_ControlMotor(chan, 1);
					}
				}
				break;
			}
		}
	}
}

void CButtonsMgr::setMouse(bool enable)
{
	m_mouse = enable;
}

bool CButtonsMgr::selected(s16 button)
{
	for(int chan = WPAD_MAX_WIIMOTES - 1; chan >= 0; chan--)
	{
		if(m_selected[chan] == button)
		{
			if(m_selected[chan] != -1 && !m_noclick) 
				click(m_selected[chan]);
			return true;
		}
	}
	return false;
}

void CButtonsMgr::up(void)
{
	if(m_elts.empty() || m_mouse)
		return;
	u32 start = 0;
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if(m_selected[chan] != -1 && m_selected[chan] < (s32)m_elts.size())
		{
			m_elts[m_selected[chan]]->targetScaleX = 1.f;
			m_elts[m_selected[chan]]->targetScaleY = 1.f;
			start = m_selected[chan];
		}
		m_selected[chan] = -1;
	}
	for(u32 i = 1; i <= m_elts.size(); ++i)
	{
		u32 j = loopNum(start - i, m_elts.size());
		SElement &b = *m_elts[j];
		if (b.t == GUIELT_BUTTON && b.visible)
		{
			m_selected[0] = j;
			b.targetScaleX = 1.1f;
			b.targetScaleY = 1.1f;
			break;
		}
	}
}

void CButtonsMgr::down(void)
{
	if(m_elts.empty() || m_mouse)
		return;
	u32 start = 0;
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		if(m_selected[chan] != -1 && m_selected[chan] < (s32)m_elts.size())
		{
			m_elts[m_selected[chan]]->targetScaleX = 1.f;
			m_elts[m_selected[chan]]->targetScaleY = 1.f;
			start = m_selected[chan];
		}
		m_selected[chan] = -1;
	}
	for(u32 i = 1; i <= m_elts.size(); ++i)
	{
		u32 j = loopNum(start + i, m_elts.size());
		SElement &b = *m_elts[j];
		if (b.t == GUIELT_BUTTON && b.visible)
		{
			m_selected[0] = j;
			b.targetScaleX = 1.1f;
			b.targetScaleY = 1.1f;
			break;
		}
	}
}

void CButtonsMgr::noHover(bool nohover)
{
	m_nohover = nohover;
}

void CButtonsMgr::noClick(bool noclick)
{
	m_noclick = noclick;
}

void CButtonsMgr::click(s16 id)
{
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		WPAD_Rumble(chan, 0);
		PAD_ControlMotor(chan, 0);

		if (id == -1) id = m_selected[chan];
		if (id == -1) continue;
		if (id < (s32)m_elts.size() && m_elts[id]->t == GUIELT_BUTTON)
		{
			SButton *b = (SButton*)m_elts[id];
			b->click = 1.f;
			b->scaleX = 1.1f;
			b->scaleY = 1.1f;
			if (m_soundVolume > 0) b->clickSound->Play(m_soundVolume);
		}
	}
}

void CButtonsMgr::SElement::tick(void)
{
	scaleX += (targetScaleX - scaleX) * (targetScaleX > scaleX ? 0.3f : 0.1f);
	scaleY += (targetScaleY - scaleY) * (targetScaleY > scaleY ? 0.3f : 0.1f);
	int alphaDist = (int)targetAlpha - (int)alpha;
	alpha += abs(alphaDist) >= 8 ? (u8)(alphaDist / 8) : (u8)alphaDist;
	pos += (targetPos - pos) * 0.1f;
}

void CButtonsMgr::SLabel::tick(void)
{
	CButtonsMgr::SElement::tick();
	text.tick();
}

void CButtonsMgr::SButton::tick(void)
{
	CButtonsMgr::SElement::tick();
	click += -click * 0.2f;
	if (click < 0.01f) click = 0.f;
	text.tick();
}

void CButtonsMgr::SProgressBar::tick(void)
{
	CButtonsMgr::SElement::tick();
	val += (targetVal - val) * 0.1f;
}

void CButtonsMgr::tick(void)
{
	for (u32 i = 0; i < m_elts.size(); ++i)
		m_elts[i]->tick();
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
		if (m_rumble[chan] > 0 && --m_rumble[chan] == 0)
		{
			WPAD_Rumble(chan, 0);
			PAD_ControlMotor(chan, 0);
		}

}

s16 CButtonsMgr::addLabel(SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, s16 style, const TexData &bg)
{
	SLabel *b = new SLabel;

	b->font = font;
	b->visible = false;
	b->textStyle = style;
	b->text.setText(b->font, text);
	b->text.setFrame(width, b->textStyle, false, true);
	b->textColor = color;
	b->x = x + width / 2;
	b->y = y + height / 2;
	b->w = width;
	b->h = height;
	b->alpha = 0;
	b->targetAlpha = 0;
	b->scaleX = 0.f;
	b->scaleY = 0.f;
	b->targetScaleX = 0.f;
	b->targetScaleY = 0.f;
	b->texBg = bg;
	b->moveByX = 0;
	b->moveByY = 0;

	u32 sz = m_elts.size();
	m_elts.push_back(b);

	return m_elts.size() > sz ? m_elts.size() - 1 : -2;
}

s16 CButtonsMgr::addProgressBar(int x, int y, u32 width, u32 height, SButtonTextureSet &texSet)
{
	SProgressBar *b = new SProgressBar;

	b->visible = false;
	b->x = x + width / 2;
	b->y = y + height / 2;
	b->w = width;
	b->h = height;
	b->alpha = 0;
	b->targetAlpha = 0;
	b->scaleX = 0.f;
	b->scaleY = 0.f;
	b->targetScaleX = 0.f;
	b->targetScaleY = 0.f;
	b->tex = texSet;
	b->val = 0.f;
	b->targetVal = 0.f;
	b->moveByX = 0;
	b->moveByY = 0;

	u32 sz = m_elts.size();
	m_elts.push_back(b);

	return m_elts.size() > sz ? m_elts.size() - 1 : -2;
}

s16 CButtonsMgr::addPicButton(TexData &texNormal, TexData &texSelected, int x, int y, u32 width, u32 height, GuiSound *clickSound, GuiSound *hoverSound)
{
	SButtonTextureSet texSet;

	texSet.center = texNormal;
	texSet.centerSel = texSelected;
	return addButton(SFont(), wstringEx(), x, y, width, height, CColor(), texSet, clickSound, hoverSound);
}

s16 CButtonsMgr::addPicButton(const u8 *pngNormal, const u8 *pngSelected, int x, int y, u32 width, u32 height, GuiSound *clickSound, GuiSound *hoverSound)
{
	SButtonTextureSet texSet;

	TexHandle.fromPNG(texSet.center, pngNormal);
	TexHandle.fromPNG(texSet.centerSel, pngSelected);
	return addButton(SFont(), wstringEx(), x, y, width, height, CColor(), texSet, clickSound, hoverSound);
}

void CButtonsMgr::setText(s16 id, const wstringEx &text, bool unwrap)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		SLabel *lbl = NULL;
		SButton *btn = NULL;
		switch (m_elts[id]->t)
		{
			case GUIELT_BUTTON:
				btn = (SButton*)m_elts[id];
				btn->text.setText(btn->font, text);
				break;
			case GUIELT_LABEL:
				lbl = (SLabel*)m_elts[id];
				lbl->text.setText(lbl->font, text);
				if (unwrap) lbl->text.setFrame(100000, lbl->textStyle, true, true);
				lbl->text.setFrame(lbl->w, lbl->textStyle, false, !unwrap);
				break;
			case GUIELT_PROGRESS:
				break;
		}
	}
}

void CButtonsMgr::setText(s16 id, const wstringEx &text, u32 startline,bool unwrap)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		SButton *btn = NULL;
		SLabel *lbl = NULL;
		switch(m_elts[id]->t)
		{
			case GUIELT_BUTTON:
				//((CButtonsMgr::SButton *)m_elts[id].get())->text = text;
				btn = (SButton*)m_elts[id];
				btn->text.setText(btn->font, text);
				break;
			case GUIELT_LABEL:
				lbl = (SLabel*)m_elts[id];
				lbl->text.setText(lbl->font, text, startline);
				if (unwrap) lbl->text.setFrame(100000, lbl->textStyle, true, true);
				lbl->text.setFrame(lbl->w, lbl->textStyle, false, !unwrap);
				break;
			case GUIELT_PROGRESS:
				break;
		}
	}
}

void CButtonsMgr::setBtnTexture(s16 id, TexData &texNormal, TexData &texSelected)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		SButton *b = (SButton*)m_elts[id];
		/* free old textures */
		TexHandle.Cleanup(b->tex.center);
		TexHandle.Cleanup(b->tex.centerSel);
		/*change textures */
		b->tex.center = texNormal;
		b->tex.centerSel = texSelected;
	}
}

void CButtonsMgr::freeBtnTexture(s16 id)
{
	if(id == -1) return;
	if(id < (s32)m_elts.size())
	{
		SButton *b = (SButton*)m_elts[id];
		TexHandle.Cleanup(b->tex.center);
		TexHandle.Cleanup(b->tex.centerSel);
	}
}

void CButtonsMgr::setTexture(s16 id, TexData &bg)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		SLabel *lbl = NULL;
		switch(m_elts[id]->t)
		{
			case GUIELT_BUTTON:
				break;
			case GUIELT_LABEL:
				lbl = (SLabel*)m_elts[id];
				lbl->texBg = bg;//change texture
				break;
			case GUIELT_PROGRESS:
				break;
		}
	}
}

void CButtonsMgr::setTexture(s16 id, TexData &bg, int width, int height)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		SLabel *lbl = NULL;
		switch(m_elts[id]->t)
		{
			case GUIELT_BUTTON:
				break;
			case GUIELT_LABEL:
				lbl = (SLabel*)m_elts[id];
				lbl->texBg = bg;//change texture
				lbl->w = width;
				lbl->h = height;
				break;
			case GUIELT_PROGRESS:
				break;
		}
	}
}

void CButtonsMgr::setProgress(s16 id, float f, bool instant)
{
	if(m_elts[id]->t == GUIELT_PROGRESS)
	{
		SProgressBar *b = (SProgressBar*)m_elts[id];
		b->targetVal = std::min(std::max(0.f, f), 1.f);
		if (instant) b->val = b->targetVal;
	}
}

void CButtonsMgr::_drawBtn(CButtonsMgr::SButton &b, bool selected, bool click)
{
	GXTexObj texObjLeft;
	GXTexObj texObjCenter;
	GXTexObj texObjRight;
	Mtx modelViewMtx;
	u8 alpha = b.alpha;
	float w, h, wh, scaleX = b.scaleX, scaleY = b.scaleY;

	if (click)
	{
		alpha = (u8)(b.click * 255.f);
		scaleX = (1.f - b.click) * 1.6f;
		scaleY = (1.f - b.click) * 1.6f;
	}
	if (alpha == 0 || scaleX == 0.f || scaleY == 0.f) return;
	guMtxIdentity(modelViewMtx);
	guMtxTransApply(modelViewMtx, modelViewMtx, b.pos.x, b.pos.y, 0.f);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	if (!!b.tex.left.data && !!b.tex.right.data && !!b.tex.leftSel.data && !!b.tex.rightSel.data && !!b.tex.center.data && !!b.tex.centerSel.data)
	{
		if (selected)
		{
			GX_InitTexObj(&texObjLeft, b.tex.leftSel.data, b.tex.leftSel.width, b.tex.leftSel.height, b.tex.leftSel.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
			GX_InitTexObj(&texObjCenter, b.tex.centerSel.data, b.tex.centerSel.width, b.tex.centerSel.height, b.tex.centerSel.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
			GX_InitTexObj(&texObjRight, b.tex.rightSel.data, b.tex.rightSel.width, b.tex.rightSel.height, b.tex.rightSel.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		}
		else
		{
			GX_InitTexObj(&texObjLeft, b.tex.left.data, b.tex.left.width, b.tex.left.height, b.tex.left.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
			GX_InitTexObj(&texObjCenter, b.tex.center.data, b.tex.center.width, b.tex.center.height, b.tex.center.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
			GX_InitTexObj(&texObjRight, b.tex.right.data, b.tex.right.width, b.tex.right.height, b.tex.right.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		}
		w = (float)(b.w / 2) * scaleX;
		h = (float)(b.h / 2) * scaleY;
		wh = (float)(b.h / 2) * scaleX;
		GX_LoadTexObj(&texObjLeft, GX_TEXMAP0);
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32(-w, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 0.f);
		GX_Position3f32(2.f * wh - w, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 0.f);
		GX_Position3f32(2.f * wh - w, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 1.f);
		GX_Position3f32(-w, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 1.f);
		GX_End();
		GX_LoadTexObj(&texObjCenter, GX_TEXMAP0);
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32(2.f * wh - w, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 0.f);
		GX_Position3f32(w - 2.f * wh, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 0.f);
		GX_Position3f32(w - 2.f * wh, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 1.f);
		GX_Position3f32(2.f * wh - w, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 1.f);
		GX_End();
		GX_LoadTexObj(&texObjRight, GX_TEXMAP0);
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32(w - 2.f * wh, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 0.f);
		GX_Position3f32(w, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 0.f);
		GX_Position3f32(w, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 1.f);
		GX_Position3f32(w - 2.f * wh, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 1.f);
		GX_End();
	}
	else if (!!b.tex.center.data && !!b.tex.centerSel.data)
	{
		if (selected)
			GX_InitTexObj(&texObjLeft, b.tex.centerSel.data, b.tex.centerSel.width, b.tex.centerSel.height, b.tex.centerSel.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		else
			GX_InitTexObj(&texObjLeft, b.tex.center.data, b.tex.center.width, b.tex.center.height, b.tex.center.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		w = (float)(b.w / 2) * scaleX;
		h = (float)(b.h / 2) * scaleY;
		GX_LoadTexObj(&texObjLeft, GX_TEXMAP0);
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32(-w, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 0.f);
		GX_Position3f32(w, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 0.f);
		GX_Position3f32(w, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 1.f);
		GX_Position3f32(-w, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 1.f);
		GX_End();
	}
	if (!b.font.font) return;
	b.font.font->reset();
	//CColor txtColor(b.textColor.r, b.textColor.g, b.textColor.b, (u8)((int)b.textColor.a * (int)alpha / 0xFF));
	b.font.font->setXScale(scaleX);
	b.font.font->setYScale(scaleY);
	//b.font.font->drawText(0, 0, b.text.c_str(), txtColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE);
	b.text.setColor(CColor(b.textColor.r, b.textColor.g, b.textColor.b, (u8)((int)b.textColor.a * (int)alpha / 0xFF)));
	b.text.setFrame(b.w, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, true, true);
	b.text.draw();
}

void CButtonsMgr::_drawLbl(CButtonsMgr::SLabel &b)
{
	GXTexObj texObj;
	Mtx modelViewMtx;
	u8 alpha = b.alpha;
	float scaleX = b.scaleX;
	float scaleY = b.scaleY;

	if (alpha == 0 || scaleX == 0.f || scaleY == 0.f)
		return;
	float w = (float)(b.w / 2) * scaleX;
	float h = (float)(b.h / 2) * scaleY;
	guMtxIdentity(modelViewMtx);
	guMtxTransApply(modelViewMtx, modelViewMtx, b.pos.x, b.pos.y, 0.f);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	if (b.texBg.data != NULL)
	{
		GX_InitTexObj(&texObj, b.texBg.data, b.texBg.width, b.texBg.height, b.texBg.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_LoadTexObj(&texObj, GX_TEXMAP0);
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32(-w, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 0.f);
		GX_Position3f32(w, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 0.f);
		GX_Position3f32(w, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 1.f);
		GX_Position3f32(-w, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 1.f);
		GX_End();
	}

	if (!b.font.font) return;

	b.font.font->reset();
	b.text.setColor(CColor(b.textColor.r, b.textColor.g, b.textColor.b, (u8)((int)b.textColor.a * (int)alpha / 0xFF)));
	b.font.font->setXScale(scaleX);
	b.font.font->setYScale(scaleY);
	float posX = b.pos.x;
	float posY = b.pos.y;
	if ((b.textStyle & FTGX_JUSTIFY_CENTER) == 0)
	{
		if ((b.textStyle & FTGX_JUSTIFY_RIGHT) != 0)
			posX += w;
		else
			posX -= w;
	}
	if ((b.textStyle & FTGX_ALIGN_MIDDLE) == 0)
	{
		if ((b.textStyle & FTGX_ALIGN_BOTTOM) != 0)
			posY += h;
		else
			posY -= h;
	}
	guMtxIdentity(modelViewMtx);
	guMtxTransApply(modelViewMtx, modelViewMtx, posX, posY, 0.f);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	if (b.moveByX != 0 || b.moveByY != 0)
	GX_SetScissor(b.targetPos.x - b.moveByX - b.w/2, b.targetPos.y - b.moveByY - b.h/2, b.w, b.h);

	b.text.draw();
	if (b.moveByX != 0 || b.moveByY != 0)
		GX_SetScissor(0, 0, m_vid.width(), m_vid.height());
}

void CButtonsMgr::_drawPBar(const CButtonsMgr::SProgressBar &b)
{
	Mtx modelMtx, modelViewMtx, viewMtx;
	u8 alpha = b.alpha;
	float scaleX = b.scaleX;
	float scaleY = b.scaleY;

	if (alpha == 0 || scaleX == 0.f || scaleY == 0.f) return;

	guMtxIdentity(modelMtx);
	guMtxIdentity(viewMtx);
	guMtxTransApply(modelMtx, modelMtx, b.pos.x, b.pos.y, 0.f);
	guMtxConcat(viewMtx, modelMtx, modelViewMtx);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	if (!!b.tex.left.data && !!b.tex.right.data && !!b.tex.leftSel.data && !!b.tex.rightSel.data && !!b.tex.center.data && !!b.tex.centerSel.data)
	{
		GXTexObj texObjBg, texObjBgL, texObjBgR, texObjBar, texObjBarL, texObjBarR;
		float w, h, wh, x1,x2, tx;
		GX_InitTexObj(&texObjBarL, b.tex.leftSel.data, b.tex.leftSel.width, b.tex.leftSel.height, b.tex.leftSel.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_InitTexObj(&texObjBar, b.tex.centerSel.data, b.tex.centerSel.width, b.tex.centerSel.height, b.tex.centerSel.format, GX_REPEAT, GX_CLAMP, GX_FALSE);
		GX_InitTexObj(&texObjBarR, b.tex.rightSel.data, b.tex.rightSel.width, b.tex.rightSel.height, b.tex.rightSel.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_InitTexObj(&texObjBgL, b.tex.left.data, b.tex.left.width, b.tex.left.height, b.tex.left.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_InitTexObj(&texObjBg, b.tex.center.data, b.tex.center.width, b.tex.center.height, b.tex.center.format, GX_REPEAT, GX_CLAMP, GX_FALSE);
		GX_InitTexObj(&texObjBgR, b.tex.right.data, b.tex.right.width, b.tex.right.height, b.tex.right.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
		w = (float)(b.w / 2) * scaleX;
		h = (float)(b.h / 2) * scaleY;
		wh = (float)(b.h / 2) * scaleX;
		x1 = 2.f * wh - w;
		x2 = w - 2.f * wh;
		GX_LoadTexObj(&texObjBgL, GX_TEXMAP0);
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32(-w, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 0.f);
		GX_Position3f32(x1, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 0.f);
		GX_Position3f32(x1, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 1.f);
		GX_Position3f32(-w, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 1.f);
		GX_End();
		GX_LoadTexObj(&texObjBg, GX_TEXMAP0);
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32(x1, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 0.f);
		GX_Position3f32(x2, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 0.f);
		GX_Position3f32(x2, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 1.f);
		GX_Position3f32(x1, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 1.f);
		GX_End();
		GX_LoadTexObj(&texObjBgR, GX_TEXMAP0);
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32(x2, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 0.f);
		GX_Position3f32(w, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 0.f);
		GX_Position3f32(w, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 1.f);
		GX_Position3f32(x2, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 1.f);
		GX_End();
		// 
		x2 = x1 + (2.f * w - 4.f * wh) * b.val;
		tx = (2.f * b.w - 4.f * b.h) * b.val / b.h * b.tex.centerSel.height / b.tex.centerSel.width;
		GX_LoadTexObj(&texObjBarL, GX_TEXMAP0);
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32(-w, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 0.f);
		GX_Position3f32(x1, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 0.f);
		GX_Position3f32(x1, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 1.f);
		GX_Position3f32(-w, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 1.f);
		GX_End();
		GX_LoadTexObj(&texObjBar, GX_TEXMAP0);
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32(x1, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 0.f);
		GX_Position3f32(x2, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(tx, 0.f);
		GX_Position3f32(x2, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(tx, 1.f);
		GX_Position3f32(x1, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 1.f);
		GX_End();
		GX_LoadTexObj(&texObjBarR, GX_TEXMAP0);
		GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
		GX_Position3f32(x2, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 0.f);
		GX_Position3f32(x2 + wh * 2.f, -h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 0.f);
		GX_Position3f32(x2 + wh * 2.f, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(1.f, 1.f);
		GX_Position3f32(x2, h, 0.f);
		GX_Color4u8(0xFF, 0xFF, 0xFF, alpha);
		GX_TexCoord2f32(0.f, 1.f);
		GX_End();
	}
}

void CButtonsMgr::draw(void)
{
	if (m_elts.empty()) return;
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

	for(s32 i = 0; i < (s32)m_elts.size(); ++i)
	{
		SButton *btn = NULL;
		SLabel *lbl = NULL;
		SProgressBar *prg = NULL;
		switch(m_elts[i]->t)
		{
			case GUIELT_BUTTON:
			{
				btn = (SButton*)m_elts[i];
				bool drawSelected = false;
				for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
				{
					if (i == m_selected[chan])
					{
						drawSelected = true;
						break;
					}
				}

				_drawBtn(*btn, drawSelected, false);
				if (btn->click > 0.f)
					_drawBtn(*btn, true, true);
				break;
			}
			case GUIELT_LABEL:
			{
				lbl = (SLabel*)m_elts[i];
				_drawLbl(*lbl);
				break;
			}
			case CButtonsMgr::GUIELT_PROGRESS:
			{
				prg = (SProgressBar*)m_elts[i];
				_drawPBar(*prg);
				break;
			}
		}
	}
}
