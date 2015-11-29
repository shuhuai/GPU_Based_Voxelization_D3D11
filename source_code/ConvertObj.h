//--------------------------------------------------------------------------------------
// File: ConvertObj.h
//
// Convert vertex data in system memory to D3D Buffers(VB & IB).
//--------------------------------------------------------------------------------------

#ifndef _OBJ_H_
#define _OBJ_H_

#include "objTester\\objLoader.h"
#include "Vertex.h"
#include "d3dutil.h"

// The model loading class.
class Convert
{
public:
	Convert();
	~Convert();
	void Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char *sFilename, float fScale);
	void draw();
	DWORD AddVertex(D3DXVECTOR3& position, D3DXVECTOR3& normal);
	AABB bndBox;
	DWORD GetVertexNumber(void){ return m_dwNumVertices; }
	DWORD GetFaceNumber(void){ return m_dwNumFaces; }
	void drawIndex();
	ID3D11Buffer* GetBufferh();
	std::string getName(){ return m_sName; }
	void reloadObject(char *sFilename, float fScale);


private:
	typedef std::vector<VertexPos> VertexList;
	typedef std::vector<DWORD> IndexList;
	typedef std::vector<D3DXVECTOR3> NormalList;

	VertexList mList;

	void buildStacks(IndexList& indices);
	objLoader* objData;
	DWORD m_dwNumVertices;
	DWORD m_dwNumFaces;
	DWORD m_dwNumNormals;

	ID3D11Device* md3dDevice;
	ID3D11DeviceContext* mDeviceContext;

	ID3D11Buffer* mVB;
	ID3D11Buffer* mNB;
	ID3D11Buffer* mIB;

	float m_fScale;

	std::string m_sName;

};

// The class is for managing multiple models.
class MultiConverts
{

public:
	std::vector<Convert*> renderModels;
	std::vector<D3DXMATRIX> transforms;
	AABB bndBox;
	void clear()
	{

		renderModels.clear();
		transforms.clear();
		bndBox = AABB();
	}
	void Add(Convert& model)
	{
		AABB temp;
		D3DXMATRIX idenity;
		D3DXMatrixIdentity(&idenity);
		renderModels.push_back(&model);
		transforms.push_back(idenity);

		model.bndBox.xform(idenity, temp);
		if (bndBox.maxPt.x < temp.maxPt.x)
		{
			bndBox.maxPt.x = temp.maxPt.x;
		}
		if (bndBox.maxPt.y < temp.maxPt.y)
		{
			bndBox.maxPt.y = temp.maxPt.y;
		}
		if (bndBox.maxPt.z < temp.maxPt.z)
		{
			bndBox.maxPt.z = temp.maxPt.z;
		}


		if (bndBox.minPt.x > temp.minPt.x)
		{
			bndBox.minPt.x = temp.minPt.x;
		}
		if (bndBox.minPt.y > temp.minPt.y)
		{
			bndBox.minPt.y = temp.minPt.y;
		}
		if (bndBox.minPt.z > temp.minPt.z)
		{
			bndBox.minPt.z = temp.minPt.z;
		}
	}
	void Add(Convert& model, D3DXMATRIX* trans){
		AABB temp;
		renderModels.push_back(&model);
		transforms.push_back(*trans);
		model.bndBox.xform(*trans, temp);
		if (bndBox.maxPt.x < temp.maxPt.x)
		{
			bndBox.maxPt.x = temp.maxPt.x;
		}
		if (bndBox.maxPt.y < temp.maxPt.y)
		{
			bndBox.maxPt.y = temp.maxPt.y;
		}
		if (bndBox.maxPt.z < temp.maxPt.z)
		{
			bndBox.maxPt.z = temp.maxPt.z;
		}


		if (bndBox.minPt.x > temp.minPt.x)
		{
			bndBox.minPt.x = temp.minPt.x;
		}
		if (bndBox.minPt.y > temp.minPt.y)
		{
			bndBox.minPt.y = temp.minPt.y;
		}
		if (bndBox.minPt.z > temp.minPt.z)
		{
			bndBox.minPt.z = temp.minPt.z;
		}
	}


};

#endif _OBJ_H_