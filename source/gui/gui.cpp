#include "gui.hpp"
#include <algorithm>

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

s16 CButtonsMgr::addLabel(SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, s16 style, const TexData &bg)
{
	SLabel *lbl = new SLabel;

	lbl->font = font;
	lbl->visible = false;
	lbl->textStyle = style;
	lbl->text.setText(lbl->font, text);
	lbl->text.setFrame(width, lbl->textStyle, false, true);
	lbl->textColor = color;
	lbl->x = x + width / 2;
	lbl->y = y + height / 2;
	lbl->w = width;
	lbl->h = height;
	lbl->alpha = 0;
	lbl->targetAlpha = 0;
	lbl->scaleX = 0.f;
	lbl->scaleY = 0.f;
	lbl->targetScaleX = 0.f;
	lbl->targetScaleY = 0.f;
	lbl->texBg = bg;
	lbl->moveByX = 0;
	lbl->moveByY = 0;

	u32 sz = m_elts.size();
	m_elts.push_back(lbl);

	return m_elts.size() > sz ? m_elts.size() - 1 : -2;
}

s16 CButtonsMgr::addProgressBar(int x, int y, u32 width, u32 height, SButtonTextureSet &texSet)
{
	SProgressBar *pb = new SProgressBar;

	pb->visible = false;
	pb->x = x + width / 2;
	pb->y = y + height / 2;
	pb->w = width;
	pb->h = height;
	pb->alpha = 0;
	pb->targetAlpha = 0;
	pb->scaleX = 0.f;
	pb->scaleY = 0.f;
	pb->targetScaleX = 0.f;
	pb->targetScaleY = 0.f;
	pb->tex = texSet;
	pb->val = 0.f;
	pb->targetVal = 0.f;
	pb->moveByX = 0;
	pb->moveByY = 0;

	u32 sz = m_elts.size();
	m_elts.push_back(pb);

	return m_elts.size() > sz ? m_elts.size() - 1 : -2;
}

void CButtonsMgr::setText(s16 id, const wstringEx &text, bool unwrap)// unwrap means no wrap
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
				if (unwrap) 
					lbl->text.setFrame(100000, lbl->textStyle, true, true);
				else
					lbl->text.setFrame(lbl->w, lbl->textStyle, false, !unwrap);
				break;
			case GUIELT_PROGRESS:
				break;
		}
	}
}

void CButtonsMgr::setText(s16 id, const wstringEx &text, u32 startline, bool unwrap)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		SButton *btn = NULL;
		SLabel *lbl = NULL;
		switch(m_elts[id]->t)
		{
			case GUIELT_BUTTON:
				btn = (SButton*)m_elts[id];
				btn->text.setText(btn->font, text);
				break;
			case GUIELT_LABEL:
				lbl = (SLabel*)m_elts[id];
				lbl->text.setText(lbl->font, text, startline);
				if (unwrap) 
					lbl->text.setFrame(100000, lbl->textStyle, true, true);
				else
					lbl->text.setFrame(lbl->w, lbl->textStyle, false, !unwrap);
				break;
			case GUIELT_PROGRESS:
				break;
		}
	}
}

void CButtonsMgr::setBtnTexture(s16 id, TexData &texNormal, TexData &texSelected, bool cleanup)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		SButton *b = (SButton*)m_elts[id];// buttons only. ie. source buttons and checkboxes
		if(cleanup)/* free old textures for source buttons */
		{
			TexHandle.Cleanup(b->tex.center);
			TexHandle.Cleanup(b->tex.centerSel);
		}
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
		SButton *b = (SButton*)m_elts[id];// for buttons only. ie. source buttons
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
				/* x and y are currently the center of the w and h plus x and y */
				/* we need to set x and y back to upper left corner */
				lbl->x = lbl->x - lbl->w / 2;
				lbl->y = lbl->y - lbl->h / 2;
				lbl->w = width;
				lbl->h = height;
				lbl->x = lbl->x + width / 2;// set to new center based on new w and h
				lbl->y = lbl->y + height / 2;
				lbl->targetPos = Vector3D((float)(lbl->x + lbl->hideParam.dx), (float)(lbl->y + lbl->hideParam.dy), 0.f);
				lbl->pos = lbl->targetPos;
				break;
			case GUIELT_PROGRESS:
				break;
		}
	}
}

void CButtonsMgr::setTexture(s16 id, TexData &bg, int x_pos, int y_pos, int width, int height)
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
				lbl->x = x_pos + width / 2;
				lbl->y = y_pos + height / 2;
				lbl->targetPos = Vector3D((float)(lbl->x + lbl->hideParam.dx), (float)(lbl->y + lbl->hideParam.dy), 0.f);
				lbl->pos = lbl->targetPos;
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
		SProgressBar *p = (SProgressBar*)m_elts[id];
		p->targetVal = std::min(std::max(0.f, f), 1.f);
		if (instant) p->val = p->targetVal;
	}
}

void CButtonsMgr::reset(s16 id, bool instant)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		SElement &e = *m_elts[id];
		e.x -= e.moveByX;
		e.y -= e.moveByY;
		if (instant)
		{
			e.pos.x -= e.moveByX;
			e.pos.y -= e.moveByY;
		}
		e.targetPos.x -= e.moveByX;
		e.targetPos.y -= e.moveByY;
		e.moveByX = 0;
		e.moveByY = 0;
	}
}

void CButtonsMgr::moveBy(s16 id, int x, int y, bool instant)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		CButtonsMgr::SElement &e = *m_elts[id];
		e.moveByX += x;
		e.moveByY += y;
		e.x += x;
		e.y += y;
		if (instant)
		{
			e.pos.x += x;
			e.pos.y += y;
		}
		e.targetPos.x += x;
		e.targetPos.y += y;
	}
}

void CButtonsMgr::getTotalHeight(s16 id, int &height)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		SLabel *l = (SLabel*)m_elts[id];
		height = l->text.getTotalHeight();
	}
}

void CButtonsMgr::getDimensions(s16 id, int &x, int &y, u32 &width, u32 &height)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		CButtonsMgr::SElement &e = *m_elts[id];
		x = e.targetPos.x;
		y = e.targetPos.y;
		width = e.w;
		height = e.h;
		if(e.t == GUIELT_LABEL)
		{
			SLabel *l = (SLabel*)m_elts[id];
			// Calculate height
			height = l->text.getTotalHeight();
		}
	}
}

void CButtonsMgr::hide(s16 id, int dx, int dy, float scaleX, float scaleY, bool instant)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		SElement &e = *m_elts[id];
		e.hideParam.dx = dx;
		e.hideParam.dy = dy;
		e.hideParam.scaleX = scaleX;
		e.hideParam.scaleY = scaleY;
		e.visible = false;
		e.targetScaleX = scaleX;
		e.targetScaleY = scaleY;
		e.targetPos = Vector3D((float)(e.x + dx), (float)(e.y + dy), 0.f);
		e.targetAlpha = 0x00;
		if (instant)
		{
			e.scaleX = e.targetScaleX;
			e.scaleY = e.targetScaleY;
			e.pos = e.targetPos;
			e.alpha = e.targetAlpha;
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
		CButtonsMgr::SElement &e = *m_elts[id];
		hide(id, e.hideParam.dx, e.hideParam.dy, e.hideParam.scaleX, e.hideParam.scaleY, instant);
	}
}

void CButtonsMgr::show(s16 id, bool instant)
{
	if (id == -1) return;
	if (id < (s32)m_elts.size())
	{
		SElement &e = *m_elts[id];
		e.visible = true;
		e.targetScaleX = 1.0f;
		e.targetScaleY = 1.0f;
		e.targetPos = Vector3D((float)e.x, (float)e.y, 0);
		e.targetAlpha = 0xFF;
		if (instant)
		{
			e.scaleX = e.targetScaleX;
			e.scaleY = e.targetScaleY;
			e.pos = e.targetPos;
			e.alpha = e.targetAlpha;
		}
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
	m_soundVolume = std::min(std::max(0, vol), 0xFF);
}

void CButtonsMgr::setRumble(int chan, bool wii, bool gc, bool wupc)
{
	wii_rumble[chan] = wii;
	gc_rumble[chan] = gc;
	wupc_rumble[chan] = wupc;
}

void CButtonsMgr::setMouse(bool enable)
{
	m_mouse = enable;
}

void CButtonsMgr::noClick(bool noclick)
{
	m_noclick = noclick;
}

// **********************************************************************************************
// * This makes the click sound when a button is selected unless m_noclick is true.             *
// * You check to see if a controller button pressed and then call m_btnMgr.selected(btn name)  *
// **********************************************************************************************
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

// we use this for checkboxes. when clicked it is hidden which looses the selected state. so when its re-shown we call this to also auto select it.
void CButtonsMgr::setSelected(s16 button)
{
	SElement &e = *m_elts[button];
	m_selected[0] = button;
	e.targetScaleX = 1.1f;
	e.targetScaleY = 1.1f;
}


// **********************************************************************************************
// * Plays the click sound for the function above.  Also sets rumble off and enlarges button    *
// **********************************************************************************************
void CButtonsMgr::click(s16 id)
{
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		WUPC_Rumble(chan, 0);
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

// ********************************************************************************************
// * This is for using the mouse/pointer to select a button. It slightly enlarges the button, *
// * makes the hover sound if it's newly selected and if m_noHover is not set, and uses		  *
// * rumble if set on.																		  *
// ********************************************************************************************
void CButtonsMgr::mouse(int chan, int x, int y)
{
	if (m_elts.empty()) return;

	float w, h;
	s32 start = -1;// used as the current or last button
	if(m_selected[chan] != -1 && m_selected[chan] < (s32)m_elts.size())
	{
		// return current or last button to its normal scale
		m_elts[m_selected[chan]]->targetScaleX = 1.f;
		m_elts[m_selected[chan]]->targetScaleY = 1.f;
		start = m_selected[chan];
	}
	m_selected[chan] = -1;
	for(int i = (int)m_elts.size() - 1; i >= 0; --i)
	{
		SElement &e = *m_elts[i];
		if(e.t == GUIELT_BUTTON)
		{
			SButton &but = *(SButton*)&e;
			w = (float)(but.w / 2);
			h = (float)(but.h / 2);
			if(but.visible && (float)x >= but.pos.x - w && (float)x < but.pos.x + w && (float)y >= but.pos.y - h && (float)y < but.pos.y + h)
			{
				m_selected[chan] = i;
				but.targetScaleX = 1.05f;
				but.targetScaleY = 1.05f;
				// 
				if(start != m_selected[chan])// if it's a new button play hover sound and rumble
				{
					if(m_soundVolume > 0)
						but.hoverSound->Play(m_soundVolume);
					if(m_rumbleEnabled)
					{
						m_rumble[chan] = 4;
						if(wupc_rumble[chan]) WUPC_Rumble(chan, 1);
						if(wii_rumble[chan]) WPAD_Rumble(chan, 1);
						if(gc_rumble[chan]) PAD_ControlMotor(chan, 1);
					}
				}
				break;
			}
		}
	}
}

// **************************************************************************************************
// * This is for moving backwards to the next available button when using the d-pad instead of the  *
// * pointer/mouse. The Button is slightly enlarged to show it's been selected. 		 			*
// **************************************************************************************************
void CButtonsMgr::up(void)
{
	if(m_elts.empty() || m_mouse)
		return;
	u32 start = 0;// set as first element. don't use -1.
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
	{
		// check if a element is selected and if so use it as start element
		if(m_selected[chan] != -1 && m_selected[chan] < (s32)m_elts.size())
		{
			// return current or last button to its normal scale
			m_elts[m_selected[chan]]->targetScaleX = 1.f;
			m_elts[m_selected[chan]]->targetScaleY = 1.f;
			start = m_selected[chan];
		}
		m_selected[chan] = -1;
	}
	for(u32 i = 1; i <= m_elts.size(); ++i)
	{
		u32 j = loopNum<u32>(start - i, m_elts.size());
		SElement &e = *m_elts[j];
		if(e.t == GUIELT_BUTTON && e.visible)
		{
			m_selected[0] = j;
			e.targetScaleX = 1.1f;// mouse only enlarges 1.05
			e.targetScaleY = 1.1f;
			SButton &but = *(SButton*)&e;
			if(m_soundVolume > 0)
				but.hoverSound->Play(m_soundVolume);
			break;
		}
	}
}

// **************************************************************************************************
// * This is for moving forwards to the next available button when using the d-pad instead of the 	*
// * pointer/mouse. The Button is slightly enlarged to show it's been selected. 		 			*
// **************************************************************************************************
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
		u32 j = loopNum<u32>(start + i, m_elts.size());
		SElement &e = *m_elts[j];
		if(e.t == GUIELT_BUTTON && e.visible)
		{
			m_selected[0] = j;
			e.targetScaleX = 1.1f;// mouse only enlarges 1.05
			e.targetScaleY = 1.1f;
			SButton &but = *(SButton*)&e;
			if(m_soundVolume > 0)
				but.hoverSound->Play(m_soundVolume);
			break;
		}
	}
}

void CButtonsMgr::tick(void)
{
	for (u32 i = 0; i < m_elts.size(); ++i)
		m_elts[i]->tick();
	for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
		if (m_rumble[chan] > 0 && --m_rumble[chan] == 0)
		{
			WUPC_Rumble(chan, 0);
			WPAD_Rumble(chan, 0);
			PAD_ControlMotor(chan, 0);
		}

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

void CButtonsMgr::SElement::tick(void)
{
	scaleX += (targetScaleX - scaleX) * (targetScaleX > scaleX ? 0.3f : 0.1f);
	scaleY += (targetScaleY - scaleY) * (targetScaleY > scaleY ? 0.3f : 0.1f);
	int alphaDist = (int)targetAlpha - (int)alpha;
	alpha += abs(alphaDist) >= 8 ? (u8)(alphaDist / 8) : (u8)alphaDist;
	pos += (targetPos - pos) * 0.1f;
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
	b.font.font->setXScale(scaleX);
	b.font.font->setYScale(scaleY);
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

	b.text.setColor(CColor(b.textColor.r, b.textColor.g, b.textColor.b, (u8)((int)b.textColor.a * (int)alpha / 0xFF)));
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
