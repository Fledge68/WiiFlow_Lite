
#ifndef __BOXMESH_HB_HPP
#define __BOXMESH_HB_HPP

#include "video.hpp"
#include "boxmesh.hpp"

// Quick and dirty hardcoded DVD box mesh
// Should be replaced by a true mesh loader
// Lacks normals

// Flat cover
extern const SMeshVert g_flatCoverMesh_HB[];
extern const u32 g_flatCoverMesh_HBSize;
extern const CTexCoord g_flatCoverBoxTex_HB[];

// Box
extern const SMeshVert g_boxMeshQ_HB[];	// Quads
extern const u32 g_boxMeshQ_HBSize;
extern const SMeshVert g_boxMeshT_HB[];	// Triangles
extern const u32 g_boxMeshT_HBSize;
// Box cover
extern const SMeshVert g_boxBackCoverMesh_HB[];
extern const u32 g_boxBackCoverMesh_HBSize;
extern const SMeshVert g_boxCoverMesh_HB[];
extern const u32 g_boxCoverMesh_HBSize;
extern const CTexCoord g_boxCoverFlatTex_HB[];
extern const CTexCoord g_boxCoverBackTex_HB[];
// 
extern const float g_boxCoverYCenter_HB;
extern const float g_coverYCenter_HB;

// Bounding box size
extern const Vector3D g_boxSize_HB;

#endif // !defined(__BOXMESH_HB_HPP)
