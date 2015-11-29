//--------------------------------------------------------------------------------------
// File: profiler.cpp
//--------------------------------------------------------------------------------------
#include "profiler.h"

void Profiler::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_bProfiling = false;
	md3dDevice = device;
	mDeviceContext = deviceContext;
}

void Profiler::StartProfile()
{
	m_bProfiling = true;
	D3D11_QUERY_DESC desc;
	desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	desc.MiscFlags = 0;
	md3dDevice->CreateQuery(&desc, &DisjointQuery);
	desc.Query = D3D11_QUERY_TIMESTAMP;
	md3dDevice->CreateQuery(&desc, &TimestampStartQuery);
	md3dDevice->CreateQuery(&desc, &TimestampEndQuery);
	mDeviceContext->Begin(DisjointQuery);
	mDeviceContext->End(TimestampStartQuery);
}

void Profiler::EndProfile()
{
	// Insert the end timestamp.
	mDeviceContext->End(TimestampEndQuery);

	// End the disjoint query.
	mDeviceContext->End(DisjointQuery);
}
float Profiler::GetTime()
{
	if (m_bProfiling)
	{
		m_bProfiling = false;
		// Get the query data.
		UINT64 startTime = 0;
		while (mDeviceContext->GetData(TimestampStartQuery, &startTime, sizeof(startTime), 0) != S_OK);

		UINT64 endTime = 0;
		while (mDeviceContext->GetData(TimestampEndQuery, &endTime, sizeof(endTime), 0) != S_OK);

		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
		while (mDeviceContext->GetData(DisjointQuery, &disjointData, sizeof(disjointData), 0) != S_OK);


		float time = 0.0f;
		if (disjointData.Disjoint == FALSE)
		{
			UINT64 delta = endTime - startTime;
			float frequency = static_cast<float>(disjointData.Frequency);
			time = (delta / frequency) * 1000.0f;
		}
		m_fTime = time;
	}
	return m_fTime;
}