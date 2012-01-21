
#ifndef __BOXMESH_HPP
#define __BOXMESH_HPP

#include "video.hpp"

// Quick and dirty hardcoded DVD box mesh
// Should be replaced by a true mesh loader
// Lacks normals

struct SMeshVert
{
	guVector pos;
	CTexCoord texCoord;
};

// Flat cover
extern const SMeshVert g_flatCoverMesh[];
extern const u32 g_flatCoverMeshSize;
extern const CTexCoord g_flatCoverBoxTex[];

// Box
extern const SMeshVert g_boxMeshQ[];	// Quads
extern const u32 g_boxMeshQSize;
extern const SMeshVert g_boxMeshT[];	// Triangles
extern const u32 g_boxMeshTSize;
// Box cover
extern const SMeshVert g_boxBackCoverMesh[];
extern const u32 g_boxBackCoverMeshSize;
extern const SMeshVert g_boxCoverMesh[];
extern const u32 g_boxCoverMeshSize;
extern const CTexCoord g_boxCoverFlatTex[];
extern const CTexCoord g_boxCoverBackTex[];
// 
extern const float g_boxCoverYCenter;
extern const float g_coverYCenter;

// Bounding box size
extern const Vector3D g_boxSize;

#endif // !defined(__BOXMESH_HPP)
