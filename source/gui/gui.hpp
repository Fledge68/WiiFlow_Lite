
// Buttons

#ifndef __GUI_HPP
#define __GUI_HPP

#include <ogc/pad.h>
#include "wiiuse/wpad.h"

#include "video.hpp"
#include "FreeTypeGX.h"
#include "text.hpp"
#include "memory/smartptr.hpp"
#include "music/gui_sound.h"
#include "wstringEx/wstringEx.hpp"

struct SButtonTextureSet
{
	STexture left;
	STexture right;
	STexture center;
	STexture leftSel;
	STexture rightSel;
	STexture centerSel;
};

class CButtonsMgr
{
public:
	bool init(CVideo &vid);
	void setRumble(bool enabled) { m_rumbleEnabled = enabled; }
	void reserve(u32 capacity) { m_elts.reserve(capacity); }
	u16 addButton(SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color,
		const SButtonTextureSet &texSet, const SmartGuiSound &clickSound = _noSound, const SmartGuiSound &hoverSound = _noSound);
	u16 addLabel(SFont font, const wstringEx &text, int x, int y, u32 width, u32 height, const CColor &color, u16 style, const STexture &bg = _noTexture);
	u16 addPicButton(const u8 *pngNormal, const u8 *pngSelected, int x, int y, u32 width, u32 height,
		const SmartGuiSound &clickSound = _noSound, const SmartGuiSound &hoverSound = _noSound);
	u16 addPicButton(STexture &texNormal, STexture &texSelected, int x, int y, u32 width, u32 height,
		const SmartGuiSound &clickSound = _noSound, const SmartGuiSound &hoverSound = _noSound);
	u16 addProgressBar(int x, int y, u32 width, u32 height, SButtonTextureSet &texSet);
	void setText(u16 id, const wstringEx &text, bool unwrap = false);
	void setText(u16 id, const wstringEx &text, u32 startline, bool unwrap = false);
	void setBtnTexture(u16 id, STexture &texNormal, STexture &texSelected);
	void setTexture(u16 id ,STexture &bg);
	void setTexture(u16 id, STexture &bg, int width, int height);
	void setProgress(u16 id, float f, bool instant = false);
	void reset(u16 id, bool instant = false);
	void moveBy(u16 id, int x, int y, bool instant = false);
	void getDimensions(u16 id, int &x, int &y, u32 &width, u32 &height);
	void hide(u16 id, int dx, int dy, float scaleX, float scaleY, bool instant = false);
	void hide(u16 id, bool instant = false);
	void show(u16 id, bool instant = false);
	void mouse(int chan, int x, int y);
	void up(void);
	void down(void);
	void draw(void);
	void tick(void);
	void noClick(bool noclick = false);
	void noHover(bool nohover = false);
	void click(u16 id = (u32)-1);
	bool selected(u16 button = (u32)-1);
	void setRumble(int, bool wii = false, bool gc = false);
	void deselect(void){ for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--) m_selected[chan] = (u32)-1; }
	void stopSounds(void);
	void setSoundVolume(int vol);
private:
	struct SHideParam
	{
		int dx;
		int dy;
		float scaleX;
		float scaleY;
	public:
		SHideParam(void) : dx(0), dy(0), scaleX(1.f), scaleY(1.f) { }
	};
	enum EltType {
		GUIELT_BUTTON,
		GUIELT_LABEL,
		GUIELT_PROGRESS
	};
	struct SElement
	{
		SHideParam hideParam;
		EltType t;
		bool visible;
		int x;
		int y;
		int w;
		int h;
		Vector3D pos;
		Vector3D targetPos;
		u8 alpha;
		u8 targetAlpha;
		float scaleX;
		float scaleY;
		float targetScaleX;
		float targetScaleY;
		int moveByX;
		int moveByY;
	public:
		virtual ~SElement(void) { }
		virtual void tick(void);
	protected:
		SElement(void) { }
	};
	struct SButton : public SElement
	{
		SFont font;
		SButtonTextureSet tex;
		wstringEx text;
		CColor textColor;
		float click;
		SmartGuiSound clickSound;
		SmartGuiSound hoverSound;
	public:
		SButton(void) { t = GUIELT_BUTTON; }
		virtual void tick(void);
	};
	struct SLabel : public SElement
	{
		SFont font;
		CText text;
		CColor textColor;
		u16 textStyle;
		STexture texBg;
	public:
		SLabel(void) { t = GUIELT_LABEL; }
		virtual void tick(void);
	};
	struct SProgressBar : public SElement
	{
		SButtonTextureSet tex;
		float val;
		float targetVal;
	public:
		SProgressBar(void) { t = GUIELT_PROGRESS; }
		virtual void tick(void);
	};
private:
	vector<SmartPtr<SElement> > m_elts;
	u32 m_selected[WPAD_MAX_WIIMOTES];
	bool m_rumbleEnabled;
	u8 m_rumble[WPAD_MAX_WIIMOTES];
	bool wii_rumble[WPAD_MAX_WIIMOTES];
	bool gc_rumble[WPAD_MAX_WIIMOTES];
	SmartGuiSound m_sndHover;
	SmartGuiSound m_sndClick;
	u8 m_soundVolume;
	bool m_noclick;
	bool m_nohover;
	CVideo m_vid;
private:
	void _drawBtn(const SButton &b, bool selected, bool click);
	void _drawLbl(SLabel &b);
	void _drawPBar(const SProgressBar &b);
	static STexture _noTexture;
	static SmartGuiSound _noSound;
};

#endif // !defined(__GUI_HPP)
