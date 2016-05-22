/*
 * Draw device and the graphics library.
 */

typedef struct fz_glyphcache_s fz_glyphcache;

fz_glyphcache * fz_newglyphcache(void);
fz_pixmap * fz_renderftglyph(fz_font *font, int cid, fz_matrix trm);
fz_pixmap * fz_rendert3glyph(fz_font *font, int cid, fz_matrix trm);
fz_pixmap * fz_renderglyph(fz_glyphcache*, fz_font*, int, fz_matrix);
void fz_evictglyphcache(fz_glyphcache *);
void fz_freeglyphcache(fz_glyphcache *);

fz_device *fz_newdrawdevice(fz_glyphcache *cache, fz_pixmap *dest);

/*
 * Scan converter
 */

typedef struct fz_edge_s fz_edge;
typedef struct fz_gel_s fz_gel;
typedef struct fz_ael_s fz_ael;

struct fz_edge_s
{
	int x, e, h, y;
	int adjup, adjdown;
	int xmove;
	int xdir, ydir;     /* -1 or +1 */
};

struct fz_gel_s
{
	fz_bbox clip;
	fz_bbox bbox;
	int cap;
	int len;
	fz_edge *edges;
};

struct fz_ael_s
{
	int cap;
	int len;
	fz_edge **edges;
};

fz_gel * fz_newgel(void);
void fz_insertgel(fz_gel *gel, float x0, float y0, float x1, float y1);
fz_bbox fz_boundgel(fz_gel *gel);
void fz_resetgel(fz_gel *gel, fz_bbox clip);
void fz_sortgel(fz_gel *gel);
void fz_freegel(fz_gel *gel);

fz_ael * fz_newael(void);
void fz_freeael(fz_ael *ael);

fz_error fz_scanconvert(fz_gel *gel, fz_ael *ael, int eofill,
	fz_bbox clip, fz_pixmap *pix, unsigned char *argb, int over);

void fz_fillpath(fz_gel *gel, fz_path *path, fz_matrix ctm, float flatness);
void fz_strokepath(fz_gel *gel, fz_path *path, fz_strokestate *stroke, fz_matrix ctm, float flatness, float linewidth);
void fz_dashpath(fz_gel *gel, fz_path *path, fz_strokestate *stroke, fz_matrix ctm, float flatness, float linewidth);

/*
 * Function pointers -- they can be replaced by cpu-optimized versions
 */

#define FZ_PSRC \
	unsigned char *src, int srcw, int srch
#define FZ_PDST \
	unsigned char *dst0, int dstw
#define FZ_PCTM \
	int u0, int v0, int fa, int fb, int fc, int fd, int w0, int h

extern void fz_accelerate(void);

extern void (*fz_duff_non)(unsigned char*,int,int,unsigned char*,int,int,int);
extern void (*fz_duff_nimcn)(unsigned char*,int,int,unsigned char*,int,int,unsigned char*,int,int,int);
extern void (*fz_duff_nimon)(unsigned char*,int,int,unsigned char*,int,int,unsigned char*,int,int,int);
extern void (*fz_duff_1o1)(unsigned char*,int,unsigned char*,int,int,int);
extern void (*fz_duff_4o4)(unsigned char*,int,unsigned char*,int,int,int);
extern void (*fz_duff_1i1c1)(unsigned char*,int,unsigned char*,int,unsigned char*,int,int,int);
extern void (*fz_duff_4i1c4)(unsigned char*,int,unsigned char*,int,unsigned char*,int,int,int);
extern void (*fz_duff_1i1o1)(unsigned char*,int,unsigned char*,int,unsigned char*,int,int,int);
extern void (*fz_duff_4i1o4)(unsigned char*,int,unsigned char*,int,unsigned char*,int,int,int);

extern void (*fz_path_1c1)(unsigned char*,unsigned char,int,unsigned char*);
extern void (*fz_path_1o1)(unsigned char*,unsigned char,int,unsigned char*);
extern void (*fz_path_w4i1o4)(unsigned char*,unsigned char*,unsigned char,int,unsigned char*);

extern void (*fz_text_1c1)(unsigned char*,int,unsigned char*,int,int,int);
extern void (*fz_text_1o1)(unsigned char*,int,unsigned char*,int,int,int);
extern void (*fz_text_w4i1o4)(unsigned char*,unsigned char*,int,unsigned char*,int,int,int);

extern void (*fz_img_ncn)(FZ_PSRC, int sn, FZ_PDST, FZ_PCTM);
extern void (*fz_img_1c1)(FZ_PSRC, FZ_PDST, FZ_PCTM);
extern void (*fz_img_4c4)(FZ_PSRC, FZ_PDST, FZ_PCTM);
extern void (*fz_img_1o1)(FZ_PSRC, FZ_PDST, FZ_PCTM);
extern void (*fz_img_4o4)(FZ_PSRC, FZ_PDST, FZ_PCTM);
extern void (*fz_img_w4i1o4)(unsigned char*,FZ_PSRC,FZ_PDST,FZ_PCTM);

extern void (*fz_decodetile)(fz_pixmap *pix, int skip, float *decode);
extern void (*fz_loadtile1)(unsigned char*, int sw, unsigned char*, int dw, int w, int h, int pad);
extern void (*fz_loadtile2)(unsigned char*, int sw, unsigned char*, int dw, int w, int h, int pad);
extern void (*fz_loadtile4)(unsigned char*, int sw, unsigned char*, int dw, int w, int h, int pad);
extern void (*fz_loadtile8)(unsigned char*, int sw, unsigned char*, int dw, int w, int h, int pad);
extern void (*fz_loadtile16)(unsigned char*, int sw, unsigned char*, int dw, int w, int h, int pad);

extern void (*fz_srown)(unsigned char *src, unsigned char *dst, int w, int denom, int n);
extern void (*fz_srow1)(unsigned char *src, unsigned char *dst, int w, int denom);
extern void (*fz_srow2)(unsigned char *src, unsigned char *dst, int w, int denom);
extern void (*fz_srow4)(unsigned char *src, unsigned char *dst, int w, int denom);
extern void (*fz_srow5)(unsigned char *src, unsigned char *dst, int w, int denom);

extern void (*fz_scoln)(unsigned char *src, unsigned char *dst, int w, int denom, int n);
extern void (*fz_scol1)(unsigned char *src, unsigned char *dst, int w, int denom);
extern void (*fz_scol2)(unsigned char *src, unsigned char *dst, int w, int denom);
extern void (*fz_scol4)(unsigned char *src, unsigned char *dst, int w, int denom);
extern void (*fz_scol5)(unsigned char *src, unsigned char *dst, int w, int denom);

