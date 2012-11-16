// Coverflow

#ifndef __COVERFLOW_HPP
#define __COVERFLOW_HPP

#include <ogcsys.h>
#include <gccore.h>
#include <string>

#include "wiiuse/wpad.h"
#include <ogc/pad.h>

#include "video.hpp"
#include "FreeTypeGX.h"
#include "text.hpp"
#include "config/config.hpp"
#include "loader/disc.h"
#include "loader/utils.h"
#include "music/gui_sound.h"

using namespace std;

enum Sorting
{
	SORT_ALPHA,
	SORT_GAMEID,
	SORT_PLAYCOUNT,
	SORT_LASTPLAYED,
	SORT_WIFIPLAYERS,
	SORT_PLAYERS,
	SORT_MAX,
	SORT_ESRB,
	SORT_CONTROLLERS,
};

class CCoverFlow
{
public:
	CCoverFlow(void);
	~CCoverFlow(void);
	// 
	bool init(const u8 *font, const u32 font_size, bool vid_50hz);
	// Cover list management
	void clear(void);
	void shutdown(void);
	void reserve(u32 capacity);
	void addItem(dir_discHdr *hdr, const char *picPath, const char *boxPicPath, const char *blankBoxPicPath, int playcount = 0, unsigned int lastPlayed = 0);
	bool empty(void) const { return m_items.empty(); }
	// 
	bool start(const char *id = NULL);
	void stopCoverLoader(bool empty = false);
	void startCoverLoader(void);
	// 
	void simulateOtherScreenFormat(bool s);
	// Commands
	void tick(void);
	bool findId(const char *id, bool instant = false);
	void pageUp(void);
	void pageDown(void);
	void nextLetter(wchar_t *c);
	void prevLetter(wchar_t *c);
	void nextPlayers(bool wifi, wchar_t *c);
	void prevPlayers(bool wifi, wchar_t *c);
	void nextID(wchar_t *c);
	void prevID(wchar_t *c);
	void left(void);
	void right(void);
	void up(void);
	void down(void);
	bool select(void);
	void flip(bool force = false, bool f = true);
	void cancel(void);
	bool selected(void) const { return m_selected; }
	void makeEffectTexture(const STexture * &bg);
	void drawText(bool withRectangle = false);
	void draw(void);
	void drawEffect(void);
	void hideCover(void);
	void showCover(void);
	void mouse(int chan, int x, int y);
	bool mouseOver(int x, int y);
	// Accessors for settings
	void setCompression(bool enable) { m_compressTextures = enable; }
	bool getBoxMode(void) const { return m_box;}
	void setBufferSize(u32 numCovers);
	void setTextures(const string &loadingPic, const string &loadingPicFlat, const string &noCoverPic, const string &noCoverPicFlat);
	void setFont(const SFont &font, const CColor &color);
	void setRange(u32 rows, u32 columns);
	void setBoxMode(bool box);
	void setHQcover(bool HQ);
	void setTextureQuality(float lodBias, int aniso, bool edgeLOD);
	void setCameraPos(bool selected, const Vector3D &pos, const Vector3D &aim);
	void setCameraOsc(bool selected, const Vector3D &speed, const Vector3D &amp);
	void setCoverScale(bool selected, const Vector3D &left, const Vector3D &right, const Vector3D &center, const Vector3D &rowCenter);
	void setCoverPos(bool selected, const Vector3D &left, const Vector3D &right, const Vector3D &center, const Vector3D &rowCenter);
	void setCoverAngleOsc(bool selected, const Vector3D &speed, const Vector3D &amp);
	void setCoverPosOsc(bool selected, const Vector3D &speed, const Vector3D &amp);
	void setSpacers(bool selected, const Vector3D &left, const Vector3D &right);
	void setDeltaAngles(bool selected, const Vector3D &left, const Vector3D &right);
	void setAngles(bool selected, const Vector3D &left, const Vector3D &right, const Vector3D &center, const Vector3D &rowCenter);
	void setTitleAngles(bool selected, float left, float right, float center);
	void setTitlePos(bool selected, const Vector3D &left, const Vector3D &right, const Vector3D &center);
	void setTitleWidth(bool selected, float side, float center);
	void setTitleStyle(bool selected, u16 side, u16 center);
	void setColors(bool selected, const CColor &begColor, const CColor &endColor, const CColor &offColor);
	void setFanartPlaying(const bool isPlaying);
	void setFanartTextColor(const CColor textColor);
	void setShadowColors(bool selected, const CColor &centerColor, const CColor &begColor, const CColor &endColor, const CColor &offColor);
	void setShadowPos(float scale, float x, float y);
	void setMirrorAlpha(float cover, float title);
	void setMirrorBlur(bool blur);
	void setRowSpacers(bool selected, const Vector3D &top, const Vector3D &bottom);
	void setRowDeltaAngles(bool selected, const Vector3D &top, const Vector3D &bottom);
	void setRowAngles(bool selected, const Vector3D &top, const Vector3D &bottom);
	void setCoverFlipping(const Vector3D &pos, const Vector3D &angle, const Vector3D &scale);
	void setBlur(u32 blurResolution, u32 blurRadius, float blurFactor);
	void setSorting(Sorting sorting);
	// 
	void setSounds(GuiSound *flipSound, GuiSound *hoverSound, GuiSound *selectSound, GuiSound *cancelSound);
	void setSoundVolume(u8 vol);
	void stopSound(void);
	// 
	void applySettings(void);
	void setCachePath(const char *path, bool deleteSource, bool compress);
	bool fullCoverCached(const char *id);
	bool preCacheCover(const char *id, const u8 *png, bool full);
	// 
	string getId(void) const;
	string getNextId(void) const;
	dir_discHdr * getHdr(void) const;
	dir_discHdr * getNextHdr(void) const;
	wstringEx getTitle(void) const;
	u64 getChanTitle(void) const;
	//
	bool getRenderTex(void);
	void setRenderTex(bool);
	void RenderTex(void);
private:
	enum DrawMode { CFDR_NORMAL, CFDR_STENCIL, CFDR_SHADOW };
	struct SLayout
	{
		Vector3D camera;
		Vector3D cameraAim;
		Vector3D leftScale;
		Vector3D rightScale;
		Vector3D centerScale;
		Vector3D rowCenterScale;
		Vector3D leftPos;
		Vector3D rightPos;
		Vector3D centerPos;
		Vector3D rowCenterPos;
		Vector3D leftAngle;
		Vector3D rightAngle;
		Vector3D centerAngle;
		Vector3D rowCenterAngle;
		Vector3D leftSpacer;
		Vector3D rightSpacer;
		Vector3D leftDeltaAngle;
		Vector3D rightDeltaAngle;
		float txtLeftAngle;
		float txtRightAngle;
		float txtCenterAngle;
		Vector3D txtLeftPos;
		Vector3D txtRightPos;
		Vector3D txtCenterPos;
		float txtSideWidth;
		float txtCenterWidth;
		u16 txtSideStyle;
		u16 txtCenterStyle;
		Vector3D cameraOscSpeed;
		Vector3D cameraOscAmp;
		Vector3D coverOscASpeed;
		Vector3D coverOscAAmp;
		Vector3D coverOscPSpeed;
		Vector3D coverOscPAmp;
		CColor begColor;
		CColor endColor;
		CColor mouseOffColor;
		CColor shadowColorCenter;
		CColor shadowColorEnd;
		CColor shadowColorBeg;
		CColor shadowColorOff;
		Vector3D topSpacer;
		Vector3D bottomSpacer;
		Vector3D topAngle;
		Vector3D bottomAngle;
		Vector3D topDeltaAngle;
		Vector3D bottomDeltaAngle;
	};
	enum TexState { STATE_Loading, STATE_Ready, STATE_NoCover };
	struct CItem
	{
		dir_discHdr *hdr;
		string picPath;
		string boxPicPath;
		string blankBoxPicPath;
		int playcount;
		unsigned int lastPlayed;
		STexture texture;
		volatile bool boxTexture;
		volatile enum TexState state;
		// 
		CItem(dir_discHdr *itemHdr, const char *itemPic, const char *itemBoxPic, const char *itemBlankBoxPic, int playcount, unsigned int lastPlayed);
	};
	struct CCover
	{
		u32 index;
		Vector3D scale;
		Vector3D targetScale;
		Vector3D angle;
		Vector3D targetAngle;
		Vector3D pos;
		Vector3D targetPos;
		CColor color;
		CColor targetColor;
		float txtAngle;
		float txtTargetAngle;
		Vector3D txtPos;
		Vector3D txtTargetPos;
		u8 txtColor;
		u8 txtTargetColor;
		CText title;
		CColor shadowColor;
		CColor targetShadowColor;
		// 
		CCover(void);
	};
	enum CLRet { CL_OK, CL_ERROR, CL_NOMEM };
private:
	Mtx m_projMtx;
	Mtx m_viewMtx;
	Vector3D m_cameraPos;
	Vector3D m_cameraAim;
	Vector3D m_targetCameraPos;
	Vector3D m_targetCameraAim;
	vector<CItem> m_items;
	vector<CCover> m_covers;
	int m_delay;
	int m_minDelay;
	int m_jump;
	mutex_t m_mutex;
	volatile bool m_loadingCovers;
	volatile bool m_coverThrdBusy;
	volatile bool m_moved;
	//
	volatile bool m_renderTex;
	STexture *m_renderingTex;
	//
	volatile int m_hqCover;
	bool m_selected;
	int m_tickCount;
	STexture m_loadingTexture;
	STexture m_noCoverTexture;
	STexture m_dvdSkin;
	STexture m_dvdSkin_Red;
	STexture m_dvdSkin_Black;
	STexture m_dvdSkin_Yellow;
	STexture m_dvdSkin_GreenOne;
	STexture m_dvdSkin_GreenTwo;
	// Settings
	string m_pngLoadCover;
	string m_pngLoadCoverFlat;
	string m_pngNoCover;
	string m_pngNoCoverFlat;
	u32 m_numBufCovers;
	SFont m_font;
	CColor m_fontColor;
	CColor m_fanartFontColor;
	bool m_fanartPlaying;
	bool m_box;
	bool m_useHQcover;
	bool m_dvdskin_loaded;
	u32 m_range;
	u32 m_rows;
	u32 m_columns;
	SLayout m_loNormal;
	SLayout m_loSelected;
	int m_mouse[WPAD_MAX_WIIMOTES];
	bool m_hideCover;
	bool m_compressTextures;
	bool m_compressCache;
	string m_cachePath;
	bool m_deletePicsAfterCaching;
	bool m_mirrorBlur;
	float m_mirrorAlpha;
	float m_txtMirrorAlpha;
	float m_shadowScale;
	float m_shadowX;
	float m_shadowY;
	STexture m_effectTex;
	u32 m_blurRadius;
	float m_blurFactor;
	Vector3D m_flipCoverPos;
	Vector3D m_flipCoverAngle;
	Vector3D m_flipCoverScale;
	u8 sndCopyNum;
	GuiSound *m_flipSound;
	GuiSound *m_hoverSound;
	GuiSound *m_selectSound;
	GuiSound *m_cancelSound;
	u8 m_soundVolume;
	float m_lodBias;
	u8 m_aniso;
	bool m_edgeLOD;
	Sorting m_sorting;
private:
	void _draw(DrawMode dm = CFDR_NORMAL, bool mirror = false, bool blend = true);
	u32 _currentPos(void) const;
	void _effectBg(const STexture * &tex);
	void _effectBlur(bool vertical);
	bool _effectVisible(void);
	void _drawMirrorZ(void);
	void _drawTitle(int i, bool mirror, bool rectangle);
	void _drawCover(int i, bool mirror, CCoverFlow::DrawMode dm);
	void _drawCoverFlat(int i, bool mirror, CCoverFlow::DrawMode dm);
	void _drawCoverBox(int i, bool mirror, CCoverFlow::DrawMode dm);
	bool _checkCoverColor(char* gameID, const char* checkID[], int len);
	void _updateTarget(int i, bool instant = false);
	void _updateAllTargets(bool instant = false);
	void _loadCover(int i, int item);
	void _loadCoverTexture(int i);
	void _coverTick(int i);
	void _unselect(void);
	Vector3D _cameraMoves(void);
	Vector3D _coverMovesA(void);
	Vector3D _coverMovesP(void);
	STexture &_coverTexture(int i);
	void _left(int repeatDelay, u32 step);
	void _right(int repeatDelay, u32 step);
	void _jump(void);
	void _completeJump(void);
	void _setJump(int j);
	void _loadAllCovers(int i);
	static bool _calcTexLQLOD(STexture &tex);
	void _dropHQLOD(int i);
	bool _loadCoverTexPNG(u32 i, bool box, bool hq, bool blankBoxCover);
	CLRet _loadCoverTex(u32 i, bool box, bool hq, bool blankBoxCover);
	bool _invisibleCover(u32 x, u32 y);
	void _instantTarget(int i);
	void _transposeCover(vector<CCover> &dst, u32 rows, u32 columns, int pos);

	void _stopSound(GuiSound * &snd);
	void _playSound(GuiSound * &snd);

	static bool _sortByPlayCount(CItem item1, CItem item2);
	static bool _sortByLastPlayed(CItem item1, CItem item2);
	static bool _sortByGameID(CItem item1, CItem item2);
	static bool _sortByAlpha(CItem item1, CItem item2);
	static bool _sortByPlayers(CItem item1, CItem item2);
	static bool _sortByWifiPlayers(CItem item1, CItem item2);

private:
	static int _coverLoader(CCoverFlow *cf);
	static float _step(float cur, float tgt, float spd);
private:
	CCoverFlow(const CCoverFlow &);
	CCoverFlow &operator=(const CCoverFlow &);
};

extern CCoverFlow CoverFlow;

#endif // !defined(__COVERFLOW_HPP)
