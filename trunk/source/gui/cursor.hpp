
#ifndef __CURSOR_HPP
#define __CURSOR_HPP

#include "video.hpp"

class CCursor
{
public:
	bool init(const char *png, bool wideFix, CColor shadowColor, float shadowX, float shadowY, bool blur, int chan);
	void draw(int x, int y, float a);
	u32 width(void) const { return m_texture.height; }
	u32 height(void) const { return m_texture.width; }
	int x(void) const { return m_x; }
	int y(void) const { return m_y; }
private:
	TexData m_texture;
	TexData m_shadow;
	Mtx m_projMtx;
	Mtx m_viewMtx;
	int m_x;
	int m_y;
	bool m_wideFix;
	float m_shadowX;
	float m_shadowY;
	CColor m_shadowColor;
private:
	void _blur(void);
};

#endif // !defined()__CURSOR_HPP
