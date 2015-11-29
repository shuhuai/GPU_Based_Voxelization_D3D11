//--------------------------------------------------------------------------------------
// File: Vertex.h
//
// Declare vertex structures.
//--------------------------------------------------------------------------------------

#ifndef VERTEX_H
#define VERTEX_H

#include "d3dutil.h"

struct VertexBox
{
	VertexBox(){}
	VertexBox(float x, float y, float z):position(x,y,z){}
    D3DXVECTOR3 position;
};

struct VERTEX
{
    D3DXVECTOR3 position;
    D3DXVECTOR3 normal;
    D3DXVECTOR2 texcoord;
};
struct VERTEX3D
{
	VERTEX3D(){}
	VERTEX3D(float x, float y, float z, 
		float nx, float ny, float nz, 
		float u, float v,float w)
		: position(x,y,z), normal(nx,ny,nz), texcoord(u,v,w){}
    D3DXVECTOR3 position;
    D3DXVECTOR3 normal;
    D3DXVECTOR3 texcoord;
};
struct VertexPos
{
    D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
};

struct VertexL
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 normal;
	D3DXCOLOR   diffuse;
	D3DXCOLOR   spec; // (r, g, b, specPower);
};
struct VertexTex
{
	VertexTex(){}
	VertexTex(float x, float y, float z, 
		float nx, float ny, float nz, 
		float u, float v)
		: pos(x,y,z), normal(nx,ny,nz), texC(u,v){}

	D3DXVECTOR3 pos;
	D3DXVECTOR3 normal;
	D3DXVECTOR2 texC;
};
struct VertexSlip
{
	VertexSlip(){}
	VertexSlip(float x, float y, float z,
		float u, float v)
		: pos(x,y,z), texC(u,v){}

	D3DXVECTOR3 pos;

	D3DXVECTOR2 texC;
};
#endif // VERTEX_H