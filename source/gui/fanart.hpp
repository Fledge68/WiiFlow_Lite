// Fan Art

#ifndef __FANART_HPP
#define __FANART_HPP

#include <ogcsys.h>
#include <gccore.h>

#include "gui.hpp"
#include "texture.hpp"
#include "config/config.hpp"
#include "loader/disc.h"

class CFanartElement
{
public:
	CFanartElement(Config &cfg, const char *dir, int artwork);
	void Cleanup(void);
	
	void draw();
	void tick();
	
	bool IsValid();
	bool IsAnimationComplete();
private:
	TexData m_art;
	int m_artwork;
	int m_delay;
	int m_event_duration;
	
	int m_x;
	int m_y;
	int m_alpha;
	float m_scaleX;
	float m_scaleY;
	float m_angle;
	
	int m_event_x;
	int m_event_y;
	int m_event_alpha;
	float m_event_scaleX;
	float m_event_scaleY;
	float m_event_angle;
	
	float m_step_x;
	float m_step_y;
	float m_step_alpha;
	float m_step_scaleX;
	float m_step_scaleY;
	float m_step_angle;
	
	bool m_isValid;
};

class CFanart
{
public:
	CFanart(void);
	~CFanart(void);

	void unload();
	bool load(Config &m_wiiflowConfig, const char *path, const dir_discHdr *hdr);
	bool isAnimationComplete();
	bool isLoaded();

	void getBackground(const TexData * &hq, const TexData * &lq);
	void draw();
	void tick();
	bool noLoop();
	void reset();

private:
	std::vector<CFanartElement> m_elms;

	bool m_animationComplete;
	u16 m_delayAfterAnimation;
	u8 m_globalShowCoverAfterAnimation;
	u16 m_defaultDelay;
	bool m_loaded;
	Config m_faConfig;

	TexData m_bg;
	TexData m_bglq;
};

#endif // __FANART_HPP