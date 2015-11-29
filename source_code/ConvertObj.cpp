//--------------------------------------------------------------------------------------
// File: ConvertObj.cpp
//--------------------------------------------------------------------------------------

#include "ConvertObj.h"
#pragma warning(disable: 4091)
Convert::Convert()
{
	mVB = 0;
	mIB = 0;
}
void Convert::reloadObject(char *filename, float scale)
{
	delete objData;
	SAFE_RELEASE(mVB);
	SAFE_RELEASE(mIB);
	m_fScale = scale;

	objData = new objLoader();
	m_sName = filename;
	objData->load(filename);
	m_dwNumVertices = objData->vertexCount;
	m_dwNumFaces = objData->faceCount;


	std::vector<DWORD> indices;

	buildStacks(indices);

	m_dwNumVertices = (UINT)mList.size();
	m_dwNumFaces = (UINT)indices.size() / 3;



	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(VertexPos) * m_dwNumVertices;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = sizeof(VertexPos);

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &mList[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));



	D3DXComputeBoundingBox((D3DXVECTOR3*)&mList[0], m_dwNumVertices,
		sizeof(VertexPos), &bndBox.minPt, &bndBox.maxPt);

	mList.clear();


	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(DWORD) * m_dwNumFaces * 3;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));
}
void Convert::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char *filename, float scale)
{
	md3dDevice = device;
	mDeviceContext = deviceContext;
	m_fScale = scale;

	objData = new objLoader();
	m_sName = filename;
	objData->load(filename);
	m_dwNumVertices = objData->vertexCount;
	m_dwNumFaces = objData->faceCount;


	std::vector<DWORD> indices;

	buildStacks(indices);


	m_dwNumVertices = (UINT)mList.size();
	m_dwNumFaces = (UINT)indices.size() / 3;



	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(VertexPos) * m_dwNumVertices;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = sizeof(VertexPos);

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &mList[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));



	D3DXComputeBoundingBox((D3DXVECTOR3*)&mList[0], m_dwNumVertices,
		sizeof(VertexPos), &bndBox.minPt, &bndBox.maxPt);

	mList.clear();


	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(DWORD) * m_dwNumFaces * 3;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));

}
void Convert::draw()
{
	UINT stride = sizeof(VertexPos);
	UINT offset = 0;


	mDeviceContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	mDeviceContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

	mDeviceContext->DrawIndexed(m_dwNumFaces * 3, 0, 0);
}
void Convert::drawIndex()
{

	mDeviceContext->Draw(m_dwNumFaces * 3, 0);

}
ID3D11Buffer* Convert::GetBufferh()
{
	return mVB;
}
Convert::~Convert()
{
	delete objData;
	SAFE_RELEASE(mVB);
	SAFE_RELEASE(mIB);
}
void Convert::buildStacks(IndexList& indices)
{
	std::vector<D3DXVECTOR3> vertex_temp;
	for (int i = 0; i < objData->vertexCount; i++)	{
		obj_vector *o = objData->vertexList[i];
		vertex_temp.push_back(D3DXVECTOR3(o->e[0], o->e[1], o->e[2])*m_fScale);
	}

	std::vector<D3DXVECTOR3> normal_temp;

	for (int i = 0; i < objData->normalCount; i++)	{

		obj_vector *o = objData->normalList[i];

		normal_temp.push_back(D3DXVECTOR3(o->e[0], o->e[1], o->e[2]));
	}




	DWORD index[3];

	for (int i = 0; i < objData->faceCount; i++)	{


		// Get a face from the faceList.
		obj_face *o = objData->faceList[i];
		index[0] = AddVertex(vertex_temp[o->vertex_index[0]], normal_temp[o->normal_index[0]]);
		index[1] = AddVertex(vertex_temp[o->vertex_index[1]], normal_temp[o->normal_index[1]]);
		index[2] = AddVertex(vertex_temp[o->vertex_index[2]], normal_temp[o->normal_index[2]]);


		indices.push_back(index[0]);
		indices.push_back(index[1]);
		indices.push_back(index[2]);




	}

}
DWORD Convert::AddVertex(D3DXVECTOR3& position, D3DXVECTOR3& normal)
{

	VertexPos v;
	v.position = position;
	v.normal = normal;

	mList.push_back(v);

	return mList.size() - 1;

}

