//--------------------------------------------------------------------------------------
// File: Camera.h
//
// A camera class for camera-related helper functions.
// This code is from the book "Introduction To 3D Game Programming With DirectX 9.0C: A Shader Approach".
//--------------------------------------------------------------------------------------

#ifndef CAMERA_H
#define CAMERA_H
#include "d3dutil.h"

class Camera
{
public:
	// By default, the camera starts out with its basis vectors
	// aligned with the world space axes, and its origin positioned
	// at the world space origin.
	Camera();

	// Read only accessor functions.
	const D3DXMATRIX& view() const;
	const D3DXMATRIX& proj() const;
	const D3DXMATRIX& viewProj() const;

	D3DXVECTOR3& right();
	D3DXVECTOR3& up();
	D3DXVECTOR3& look();
	void SetlockPitch(bool);
	void SetlockMove(bool);
	void setOrthoLens(float width, float height, float aspect, float nearZ, float farZ);
	bool isVisible(const AABB& box)const;
	float clip();

	// Read/write access to the camera position.
	D3DXVECTOR3& pos();
	void PosOp();

	// Our implementation of D3DXMatrixLookAtLH
	void lookAt(D3DXVECTOR3& pos, D3DXVECTOR3& target, D3DXVECTOR3& up);

	// Perspective projection parameters.
	void setLens(float fov, float aspect, float nearZ, float farZ);

	// Sets the camera speed.
	void setSpeed(float s);

	// Updates the camera's basis vectors and origin, relative to
	// the world space, based on user input.

	void update(float dt, float offsetHeight);

	void injectMousePosition(int x,int y);
	void clearMousePosition();
protected:
	void buildView();
	void buildWorldFrustumPlanes();
	// Save camera-related matrices.
	D3DXMATRIX mView;
	D3DXMATRIX mProj;
	D3DXMATRIX mViewProj;

	// Camera coordinate system relative to world space.
	D3DXVECTOR3 mPosW;
	D3DXVECTOR3 mRightW;
	D3DXVECTOR3 mUpW;
	D3DXVECTOR3 mLookW;
	bool lockPitch;
	bool lockMove;
	// Camera speed.
	float mSpeed;
	float farclip;

	// Frustum Planes
	D3DXPLANE mFrustumPlanes[6]; 
	// [0] = near
	// [1] = far
	// [2] = left
	// [3] = right
	// [4] = top
	// [5] = bottom
	int mMousePosX;
	int mMousePosY;
	float mRotX;
	float mRotY;


};
#endif // CAMERA_H