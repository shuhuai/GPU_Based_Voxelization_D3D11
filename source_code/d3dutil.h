//--------------------------------------------------------------------------------------
// File: d3dutil.h
//
// Helper functions for math and rendering.
//--------------------------------------------------------------------------------------
#include"Header.h"



#ifndef D3DUTIL_H
#define D3DUTIL_H

//*****************************************************************************
// Helper functions
//*****************************************************************************
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p) = nullptr; } }
typedef std::vector<D3DXVECTOR3> VertexList;
typedef std::vector<DWORD> IndexList;
void BuildGeoSphere(
	UINT numSubdivisions,
	float radius,
	std::vector<D3DXVECTOR3>& vertices,
	std::vector<DWORD>& indices);

float GetRandomFloat(float a, float b);

void GetRandomVec(D3DXVECTOR3& out);

struct AABB
{
	
	AABB()
		: minPt(INFINITY, INFINITY, INFINITY),
		maxPt(-INFINITY, -INFINITY, -INFINITY){}

	D3DXVECTOR3 center()const
	{
		return (minPt + maxPt)*0.5f;
	}

	D3DXVECTOR3 extent()const
	{
		return (maxPt - minPt)*0.5f;
	}

	void xform(const D3DXMATRIX& M, AABB& out)
	{
		// Convert to center/extent representation.
		D3DXVECTOR3 c = center();
		D3DXVECTOR3 e = extent();

		// Transform center in usual way.
		D3DXVec3TransformCoord(&c, &c, &M);

		// Transform extent.
		D3DXMATRIX absM;
		D3DXMatrixIdentity(&absM);
		absM(0, 0) = fabsf(M(0, 0)); absM(0, 1) = fabsf(M(0, 1)); absM(0, 2) = fabsf(M(0, 2));
		absM(1, 0) = fabsf(M(1, 0)); absM(1, 1) = fabsf(M(1, 1)); absM(1, 2) = fabsf(M(1, 2));
		absM(2, 0) = fabsf(M(2, 0)); absM(2, 1) = fabsf(M(2, 1)); absM(2, 2) = fabsf(M(2, 2));
		D3DXVec3TransformNormal(&e, &e, &absM);

		// Convert back to AABB representation.
		out.minPt = c - e;
		out.maxPt = c + e;
	}
	D3DXVECTOR3 minPt;
	D3DXVECTOR3 maxPt;
};

template<typename T>
D3DX11INLINE T Min(const T& a, const T& b)
{
	return a < b ? a : b;
}

template<typename T>
D3DX11INLINE T Max(const T& a, const T& b)
{
	return a > b ? a : b;
}

#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)                                      \
	{                                                  \
		HRESULT hr = x;                                \
		if(FAILED(hr))                                 \
								{                                              \
		DXTraceW(_T(__FILE__), __LINE__, hr, L#x, TRUE); \
								}                                              \
	}
#endif

#else
#ifndef HR
#define HR(x) x;
#endif
#endif 
//*****************************************************************************
// Constants
//*****************************************************************************
const float PI       = 3.14159265358979323f;
const float MATH_EPS = 0.0001f;

const D3DXCOLOR WHITE(1.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR BLACK(0.0f, 0.0f, 0.0f, 0.0f);
const D3DXCOLOR RED(1.0f, 0.0f, 0.0f, 1.0f);
const D3DXCOLOR GREEN(0.0f, 1.0f, 0.0f, 1.0f);
const D3DXCOLOR BLUE(0.0f, 0.0f, 1.0f, 1.0f);
const D3DXCOLOR YELLOW(1.0f, 1.0f, 0.0f, 1.0f);
const D3DXCOLOR CYAN(0.0f, 1.0f, 1.0f, 1.0f);
const D3DXCOLOR MAGENTA(1.0f, 0.0f, 1.0f, 1.0f);

const D3DXCOLOR BEACH_SAND(1.0f, 0.96f, 0.62f, 1.0f);
const D3DXCOLOR LIGHT_YELLOW_GREEN(0.48f, 0.77f, 0.46f, 1.0f);
const D3DXCOLOR DARK_YELLOW_GREEN(0.1f, 0.48f, 0.19f, 1.0f);
const D3DXCOLOR DARKBROWN(0.45f, 0.39f, 0.34f, 1.0f);

//*****************************************************************************
// Light
//*****************************************************************************

struct Light
{
	Light()
	{
		ZeroMemory(this, sizeof(Light));
	}

	D3DXVECTOR3 pos;
	float pad1;      // not used
	D3DXVECTOR3 dir;
	float pad2;      // not used
	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
	D3DXCOLOR specular;
	D3DXVECTOR3 att;
	float spotPow;
	float range;
};

struct Mtrl
{
      Mtrl()
            :ambient(WHITE), diffuse(WHITE),
            spec(WHITE), specPower(8.0f){}
      Mtrl(const D3DXCOLOR& a, const D3DXCOLOR& d,
            const D3DXCOLOR& s, float power)
            :ambient(a), diffuse(d), spec(s), specPower(power){}

      D3DXCOLOR ambient;
      D3DXCOLOR diffuse;
      D3DXCOLOR spec;
  
	  
	  float specPower;
};

struct DirLight
{
	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
	D3DXCOLOR spec;
	D3DXVECTOR3 dirW;
};

struct SpotLight
{
	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
	D3DXCOLOR spec;
	D3DXVECTOR3 posW;
	D3DXVECTOR3 dirW;  
	float  spotPower;
};

#endif D3DUTIL_H