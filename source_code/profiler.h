//--------------------------------------------------------------------------------------
// File: profiler.h
//
// A helper class for profiling by Directx11 Queries.
//--------------------------------------------------------------------------------------

#include "d3dutil.h"
#ifndef _Profiler_H_
#define _Profiler_H_
class Profiler
{
public:

	void Init(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
	void StartProfile();
	void EndProfile();
	float GetTime();

private:
	ID3D11Query* DisjointQuery;
	ID3D11Query* TimestampStartQuery;
	ID3D11Query* TimestampEndQuery;
	ID3D11Device* md3dDevice;
	ID3D11DeviceContext* mDeviceContext;
	bool m_bProfiling;
	float m_fTime;
};

#endif