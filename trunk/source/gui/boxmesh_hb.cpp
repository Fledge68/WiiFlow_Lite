#include "boxmesh_hb.hpp"

// Quick and dirty hardcoded DVD box mesh

static const guVector g_coverBL_HB = { -0.65f, -0.315f, 0.f };
static const guVector g_coverTR_HB = { 0.65f, 0.315f, 0.f };
static const float g_boxCoverY_HB = 0.1f;
static const float g_boxBorderWidth_HB = 0.025f;
static const guVector g_frontCoverBL_HB = { g_coverBL_HB.x, g_coverBL_HB.y + g_boxCoverY_HB, g_coverBL_HB.z };
static const guVector g_frontCoverTR_HB = { g_coverTR_HB.x, g_coverTR_HB.y + g_boxCoverY_HB, g_coverTR_HB.z };
static const guVector g_backCoverBL_HB = { g_frontCoverBL_HB.x, g_frontCoverBL_HB.y, g_frontCoverBL_HB.z - 0.16f };
static const guVector g_backCoverTR_HB = { g_frontCoverTR_HB.x, g_frontCoverTR_HB.y, g_frontCoverTR_HB.z - 0.16f };
const float g_boxCoverY_HBCenter_HB = (g_frontCoverTR_HB.y - g_frontCoverBL_HB.y) * 0.5f;
const float g_coverYCenter_HB = (g_coverTR_HB.y - g_coverBL_HB.y) * 0.5f;
const Vector3D g_boxSize_HB(
	g_coverTR_HB.x - g_coverBL_HB.x + 2 * g_boxBorderWidth_HB,
	g_coverTR_HB.y - g_coverBL_HB.y + 2 * g_boxBorderWidth_HB,
	g_coverTR_HB.z - g_coverBL_HB.z + 2 * g_boxBorderWidth_HB);

#define w(x)	((float)x / 64.0f)
#define h(y)	((float)y / 256.0f)


const SMeshVert g_boxMeshQ_HB[] = {	// Quads
	// Bordure du bas devant
	{ { g_frontCoverBL_HB.x, g_frontCoverBL_HB.y,						g_frontCoverBL_HB.z },						CTexCoord(w(0), h(256)) },
	{ { g_frontCoverBL_HB.x, g_frontCoverBL_HB.y - g_boxBorderWidth_HB,	g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(256)) },
	{ { g_frontCoverTR_HB.x, g_frontCoverBL_HB.y - g_boxBorderWidth_HB,	g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(224)) },
	{ { g_frontCoverTR_HB.x, g_frontCoverBL_HB.y,						g_frontCoverBL_HB.z },						CTexCoord(w(0), h(224)) },

	// Bordure du haut devant
	{ { g_frontCoverBL_HB.x, g_frontCoverTR_HB.y + g_boxBorderWidth_HB,	g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(0)) },
	{ { g_frontCoverBL_HB.x, g_frontCoverTR_HB.y, 					g_frontCoverBL_HB.z },						CTexCoord(w(0), h(0)) },
	{ { g_frontCoverTR_HB.x, g_frontCoverTR_HB.y,						g_frontCoverBL_HB.z },						CTexCoord(w(0), h(32)) },
	{ { g_frontCoverTR_HB.x, g_frontCoverTR_HB.y + g_boxBorderWidth_HB,	g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(32)) },

	// Bordure du bas derrière
	{ { g_backCoverBL_HB.x, g_backCoverBL_HB.y - g_boxBorderWidth_HB,	g_backCoverBL_HB.z + g_boxBorderWidth_HB },	CTexCoord(w(54), h(256)) },
	{ { g_backCoverBL_HB.x, g_backCoverBL_HB.y,						g_backCoverBL_HB.z },						CTexCoord(w(64), h(256)) },
	{ { g_backCoverTR_HB.x, g_backCoverBL_HB.y,						g_backCoverBL_HB.z },						CTexCoord(w(64), h(224)) },
	{ { g_backCoverTR_HB.x, g_backCoverBL_HB.y - g_boxBorderWidth_HB,	g_backCoverBL_HB.z + g_boxBorderWidth_HB },	CTexCoord(w(54), h(224)) },

	// Bordure du haut derrière
	{ { g_backCoverBL_HB.x, g_backCoverTR_HB.y,						g_backCoverBL_HB.z },						CTexCoord(w(64), h(0)) },
	{ { g_backCoverBL_HB.x, g_backCoverTR_HB.y + g_boxBorderWidth_HB,	g_backCoverBL_HB.z + g_boxBorderWidth_HB },	CTexCoord(w(54), h(0)) },
	{ { g_backCoverTR_HB.x, g_backCoverTR_HB.y + g_boxBorderWidth_HB,	g_backCoverBL_HB.z + g_boxBorderWidth_HB },	CTexCoord(w(54), h(32)) },
	{ { g_backCoverTR_HB.x, g_backCoverTR_HB.y,						g_backCoverBL_HB.z },						CTexCoord(w(64), h(32)) },

	// Bordure de droite devant
	{ { g_frontCoverTR_HB.x,						g_frontCoverBL_HB.y, g_frontCoverBL_HB.z },						CTexCoord(w(0), h(256)) },
	{ { g_frontCoverTR_HB.x + g_boxBorderWidth_HB,	g_frontCoverBL_HB.y, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(256)) },
	{ { g_frontCoverTR_HB.x + g_boxBorderWidth_HB,	g_frontCoverTR_HB.y, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(0)) },
	{ { g_frontCoverTR_HB.x,						g_frontCoverTR_HB.y, g_frontCoverBL_HB.z },						CTexCoord(w(0), h(0)) },

	// Bordure de droite derrière
	{ { g_backCoverTR_HB.x + g_boxBorderWidth_HB,	g_backCoverBL_HB.y, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(54), h(256)) },
	{ { g_backCoverTR_HB.x,					g_backCoverBL_HB.y, g_backCoverBL_HB.z },							CTexCoord(w(64), h(256)) },
	{ { g_backCoverTR_HB.x,					g_backCoverTR_HB.y, g_backCoverBL_HB.z },							CTexCoord(w(64), h(0)) },
	{ { g_backCoverTR_HB.x + g_boxBorderWidth_HB,	g_backCoverTR_HB.y, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(54), h(0)) },

	// Face du haut
	{ { g_frontCoverBL_HB.x, g_frontCoverTR_HB.y + g_boxBorderWidth_HB, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(0)) },
	{ { g_frontCoverTR_HB.x, g_frontCoverTR_HB.y + g_boxBorderWidth_HB, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(32)) },
	{ { g_backCoverTR_HB.x, g_backCoverTR_HB.y + g_boxBorderWidth_HB, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(54), h(32)) },
	{ { g_backCoverBL_HB.x, g_backCoverTR_HB.y + g_boxBorderWidth_HB, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(54), h(0)) },

	// Angle face du haut / face de droite
	{ { g_frontCoverTR_HB.x, g_frontCoverTR_HB.y + g_boxBorderWidth_HB, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(32)) },
	{ { g_frontCoverTR_HB.x + g_boxBorderWidth_HB, g_frontCoverTR_HB.y, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(0)) },
	{ { g_backCoverTR_HB.x + g_boxBorderWidth_HB, g_backCoverTR_HB.y, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(54), h(0)) },
	{ { g_backCoverTR_HB.x, g_backCoverTR_HB.y + g_boxBorderWidth_HB, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(54), h(32)) },

	// Face de droite
	{ { g_frontCoverTR_HB.x + g_boxBorderWidth_HB, g_frontCoverTR_HB.y, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(0)) },
	{ { g_frontCoverTR_HB.x + g_boxBorderWidth_HB, g_frontCoverBL_HB.y, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(256)) },
	{ { g_backCoverTR_HB.x + g_boxBorderWidth_HB, g_backCoverBL_HB.y, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(54), h(256)) },
	{ { g_backCoverTR_HB.x + g_boxBorderWidth_HB, g_backCoverTR_HB.y, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(54), h(0)) },

	// Angle face de droite / face du bas
	{ { g_frontCoverTR_HB.x + g_boxBorderWidth_HB, g_frontCoverBL_HB.y, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(256)) },
	{ { g_frontCoverTR_HB.x, g_frontCoverBL_HB.y - g_boxBorderWidth_HB, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(224)) },
	{ { g_backCoverTR_HB.x, g_backCoverBL_HB.y - g_boxBorderWidth_HB, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(54), h(224)) },
	{ { g_backCoverTR_HB.x + g_boxBorderWidth_HB, g_backCoverBL_HB.y, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(54), h(256)) },

	// Face du bas
	{ { g_frontCoverTR_HB.x, g_frontCoverBL_HB.y - g_boxBorderWidth_HB, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(224)) },
	{ { g_frontCoverBL_HB.x, g_frontCoverBL_HB.y - g_boxBorderWidth_HB, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(256)) },
	{ { g_backCoverBL_HB.x, g_backCoverBL_HB.y - g_boxBorderWidth_HB, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(54), h(256)) },
	{ { g_backCoverTR_HB.x, g_backCoverBL_HB.y - g_boxBorderWidth_HB, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(54), h(224)) },
	
	// Face de gauche en haut
	{ { g_frontCoverBL_HB.x, g_frontCoverTR_HB.y, g_frontCoverBL_HB.z },											CTexCoord(w(0), h(0)) },
	{ { g_frontCoverBL_HB.x, g_frontCoverTR_HB.y + g_boxBorderWidth_HB, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(0), h(0)) },
	{ { g_backCoverBL_HB.x, g_backCoverTR_HB.y + g_boxBorderWidth_HB, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(0), h(0)) },
	{ { g_backCoverBL_HB.x, g_backCoverTR_HB.y, g_backCoverBL_HB.z},												CTexCoord(w(0), h(0)) },

	// Face de gauche en bas
	{ { g_frontCoverBL_HB.x, g_frontCoverBL_HB.y - g_boxBorderWidth_HB, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(0), h(0)) },
	{ { g_frontCoverBL_HB.x, g_frontCoverBL_HB.y, g_frontCoverBL_HB.z },											CTexCoord(w(0), h(0)) },
	{ { g_backCoverBL_HB.x, g_backCoverBL_HB.y, g_backCoverBL_HB.z },											CTexCoord(w(0), h(0)) },
	{ { g_backCoverBL_HB.x, g_backCoverBL_HB.y - g_boxBorderWidth_HB, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(0), h(0)) }
};

const SMeshVert g_boxMeshT_HB[] = {	// Triangles
	// Haut devant
	{ { g_frontCoverTR_HB.x, g_frontCoverTR_HB.y, g_frontCoverBL_HB.z },											CTexCoord(w(0), h(16)) },
	{ { g_frontCoverTR_HB.x + g_boxBorderWidth_HB, g_frontCoverTR_HB.y, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(0)) },
	{ { g_frontCoverTR_HB.x, g_frontCoverTR_HB.y + g_boxBorderWidth_HB, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(32)) },

	// Haut derrière
	{ { g_backCoverTR_HB.x, g_backCoverTR_HB.y, g_backCoverBL_HB.z },											CTexCoord(w(64), h(16)) },
	{ { g_backCoverTR_HB.x, g_backCoverTR_HB.y + g_boxBorderWidth_HB, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(54), h(32)) },
	{ { g_backCoverTR_HB.x + g_boxBorderWidth_HB, g_backCoverTR_HB.y, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(54), h(0)) },

	// Bas devant
	{ { g_frontCoverTR_HB.x, g_frontCoverBL_HB.y, g_frontCoverBL_HB.z },											CTexCoord(w(0), h(240)) },
	{ { g_frontCoverTR_HB.x, g_frontCoverBL_HB.y - g_boxBorderWidth_HB, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(224)) },
	{ { g_frontCoverTR_HB.x + g_boxBorderWidth_HB, g_frontCoverBL_HB.y, g_frontCoverBL_HB.z - g_boxBorderWidth_HB },	CTexCoord(w(10), h(256)) },

	// Bas derrière
	{ { g_backCoverTR_HB.x, g_backCoverBL_HB.y, g_backCoverBL_HB.z },											CTexCoord(w(64), h(240)) },
	{ { g_backCoverTR_HB.x + g_boxBorderWidth_HB, g_backCoverBL_HB.y, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(54), h(256)) },
	{ { g_backCoverTR_HB.x, g_backCoverBL_HB.y - g_boxBorderWidth_HB, g_backCoverBL_HB.z + g_boxBorderWidth_HB },		CTexCoord(w(54), h(224)) }
};

#undef h
#undef w

const SMeshVert g_flatCoverMesh_HB[] = {
	{ { g_coverBL_HB.x, g_coverBL_HB.y, g_coverBL_HB.z }, CTexCoord(0.f, 1.f) },
	{ { g_coverTR_HB.x, g_coverBL_HB.y, g_coverBL_HB.z }, CTexCoord(1.f, 1.f) },
	{ { g_coverTR_HB.x, g_coverTR_HB.y, g_coverBL_HB.z }, CTexCoord(1.f, 0.f) },
	{ { g_coverBL_HB.x, g_coverTR_HB.y, g_coverBL_HB.z }, CTexCoord(0.f, 0.f) }
};

const CTexCoord g_flatCoverBoxTex_HB[sizeof g_flatCoverMesh_HB / sizeof g_flatCoverMesh_HB[0]] = {
	CTexCoord(1.46f / 2.76f, 1.f),
	CTexCoord(1.f, 1.f),
	CTexCoord(1.f, 0.f),
	CTexCoord(1.46f / 2.76f, 0.f)
};

const SMeshVert g_boxBackCoverMesh_HB[] = {
	{ { g_backCoverTR_HB.x, g_backCoverBL_HB.y, g_backCoverBL_HB.z }, CTexCoord(0.f, 1.f) },
	{ { g_backCoverBL_HB.x, g_backCoverBL_HB.y, g_backCoverBL_HB.z }, CTexCoord(1.3f / 2.76f, 1.f) },
	{ { g_backCoverBL_HB.x, g_backCoverTR_HB.y, g_backCoverBL_HB.z }, CTexCoord(1.3f / 2.76f, 0.f) },
	{ { g_backCoverTR_HB.x, g_backCoverTR_HB.y, g_backCoverBL_HB.z }, CTexCoord(0.f, 0.f) },

	{ { g_frontCoverBL_HB.x, g_frontCoverBL_HB.y, g_frontCoverBL_HB.z }, CTexCoord(1.46f / 2.76f, 1.f) },
	{ { g_frontCoverBL_HB.x, g_frontCoverTR_HB.y, g_frontCoverBL_HB.z }, CTexCoord(1.46f / 2.76f, 0.f) },
	{ { g_backCoverBL_HB.x, g_backCoverTR_HB.y, g_backCoverBL_HB.z }, CTexCoord(1.3f / 2.76f, 0.f) },
	{ { g_backCoverBL_HB.x, g_backCoverBL_HB.y, g_backCoverBL_HB.z }, CTexCoord(1.3f / 2.76f, 1.f) },
};

const SMeshVert g_boxCoverMesh_HB[] = {
	{ { g_frontCoverBL_HB.x, g_frontCoverBL_HB.y, g_frontCoverBL_HB.z }, CTexCoord(1.46f / 2.76f, 1.f) },
	{ { g_frontCoverTR_HB.x, g_frontCoverBL_HB.y, g_frontCoverBL_HB.z }, CTexCoord(1.f, 1.f) },
	{ { g_frontCoverTR_HB.x, g_frontCoverTR_HB.y, g_frontCoverBL_HB.z }, CTexCoord(1.f, 0.f) },
	{ { g_frontCoverBL_HB.x, g_frontCoverTR_HB.y, g_frontCoverBL_HB.z }, CTexCoord(1.46f / 2.76f, 0.f) }
};

const CTexCoord g_boxCoverFlatTex_HB[sizeof g_boxCoverMesh_HB / sizeof g_boxCoverMesh_HB[0]] = {
	CTexCoord(0.f, 1.f),
	CTexCoord(1.f, 1.f),
	CTexCoord(1.f, 0.f),
	CTexCoord(0.f, 0.f)
};

const CTexCoord g_boxCoverBackTex_HB[sizeof g_boxBackCoverMesh_HB / sizeof g_boxBackCoverMesh_HB[0]] =
{
	CTexCoord(0.f, 1.f),
	CTexCoord(1.3f / 1.46f, 1.f),
	CTexCoord(1.3f / 1.46f, 0.f),
	CTexCoord(0.f, 0.f),

	CTexCoord(1.f, 1.f),
	CTexCoord(1.f, 0.f),
	CTexCoord(1.3f / 1.46f, 0.f),
	CTexCoord(1.3f / 1.46f, 1.f)
};

const u32 g_flatCoverMesh_HBSize = sizeof g_flatCoverMesh_HB / sizeof g_flatCoverMesh_HB[0];
const u32 g_boxMeshQ_HBSize = sizeof g_boxMeshQ_HB / sizeof g_boxMeshQ_HB[0];
const u32 g_boxMeshT_HBSize = sizeof g_boxMeshT_HB / sizeof g_boxMeshT_HB[0];
const u32 g_boxCoverMesh_HBSize = sizeof g_boxCoverMesh_HB / sizeof g_boxCoverMesh_HB[0];
const u32 g_boxBackCoverMesh_HBSize = sizeof g_boxBackCoverMesh_HB / sizeof g_boxBackCoverMesh_HB[0];
