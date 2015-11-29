//--------------------------------------------------------------------------------------
// File: Camera.cpp
//--------------------------------------------------------------------------------------
#include"Carmera.h"


Camera* g_Camera = 0;
Camera::Camera()
{

	D3DXMatrixIdentity(&mView);
	D3DXMatrixIdentity(&mProj);
	D3DXMatrixIdentity(&mViewProj);

	mPosW = D3DXVECTOR3(0.0f, 0.0f, .0f);
	mRightW = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	mUpW = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	mLookW = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	mMousePosX = 0;
	mMousePosY = 0;
	mRotX = 0;
	mRotY = 0;

	// Client should adjust to a value that makes sense for application's
	// unit scale, and the object the camera is attached to--e.g., car, jet,
	// human walking, etc.
	mSpeed = 100.0f;
	lockPitch = false;
}

const D3DXMATRIX& Camera::view() const
{
	return mView;
}

const D3DXMATRIX& Camera::proj() const
{
	return mProj;
}

const D3DXMATRIX& Camera::viewProj() const
{
	return mViewProj;
}

D3DXVECTOR3& Camera::right()
{
	return mRightW;
}

D3DXVECTOR3& Camera::up()
{
	return mUpW;
}

D3DXVECTOR3& Camera::look()
{
	return mLookW;
}

D3DXVECTOR3& Camera::pos()
{

	return mPosW;
}
void Camera::SetlockPitch(bool set)
{
	lockPitch = set;
}
void Camera::SetlockMove(bool set)
{
	lockMove = set;
}
void Camera::clearMousePosition()
{
	mMousePosX = 0;
	mMousePosY = 0;
	mRotX = 0;
	mRotY = 0;
}
void Camera::injectMousePosition(int x, int y)
{
	
	if (mMousePosX == 0 && mMousePosY == 0)
	{
		mMousePosX = x;
		mMousePosY = y;
	}
	mRotX = x - mMousePosX;
	mRotY = y - mMousePosY;

	mMousePosX = x;
	mMousePosY = y;
}
void Camera::update(float dt, float offsetHeight)
{
	// Find the net direction the camera is traveling (since the
	// camera could be running and strafing).
	D3DXVECTOR3 dir(0.0f, 0.0f, 0.0f);



	if (GetAsyncKeyState('W') & 0x8000)
		dir += mLookW;
	if (GetAsyncKeyState('S') & 0x8000)
		dir -= mLookW;
	if (GetAsyncKeyState('D') & 0x8000)
		dir += mRightW;
	if (GetAsyncKeyState('A') & 0x8000)
		dir -= mRightW;

	if (lockMove)
	{
		dir = D3DXVECTOR3(0, 0, 0);
	}

	// Move at mSpeed along net direction.
	D3DXVec3Normalize(&dir, &dir);
	D3DXVECTOR3 newPos = mPosW + dir*mSpeed*dt;
	D3DXVECTOR3 tempPos = mPosW;

	mPosW = newPos;


	

	// Angle to rotate around right vector.
	float pitch = dt*mRotY*0.5f;

	// Angle to rotate around world y-axis.
	//float yAngle=DirectInput->mouseDX()/150;
	float yAngle = dt*mRotX*0.5f;

	mRotX = 0;
	mRotY = 0;
	D3DXMATRIX R;
	//lock Pitch
	if (lockPitch){
		pitch = 0;
		yAngle = 0;
	}

	D3DXMatrixRotationAxis(&R, &mRightW, pitch);
	D3DXVec3TransformCoord(&mLookW, &mLookW, &R);
	D3DXVec3TransformCoord(&mUpW, &mUpW, &R);

	// Rotate camera axes about the world's y-axis.

	D3DXMatrixRotationY(&R, yAngle);
	D3DXVec3TransformCoord(&mRightW, &mRightW, &R);
	D3DXVec3TransformCoord(&mUpW, &mUpW, &R);
	D3DXVec3TransformCoord(&mLookW, &mLookW, &R);

	// Rebuild the view matrix to reflect changes.
	buildView();

	buildWorldFrustumPlanes();

	mViewProj = mView * mProj;




}
void Camera::buildView()
{

	// Keep camera's axes orthogonal to each other and
	// of unit length.
	D3DXVec3Normalize(&mLookW, &mLookW);
	D3DXVec3Cross(&mRightW, &mUpW, &mLookW);
	D3DXVec3Normalize(&mRightW, &mRightW);
	D3DXVec3Cross(&mUpW, &mLookW, &mRightW);
	D3DXVec3Normalize(&mUpW, &mUpW);


	// Fill in the view matrix entries.

	float x = -D3DXVec3Dot(&mPosW, &mRightW);
	float y = -D3DXVec3Dot(&mPosW, &mUpW);
	float z = -D3DXVec3Dot(&mPosW, &mLookW);

	mView(0, 0) = mRightW.x;
	mView(1, 0) = mRightW.y;
	mView(2, 0) = mRightW.z;
	mView(3, 0) = x;

	mView(0, 1) = mUpW.x;
	mView(1, 1) = mUpW.y;
	mView(2, 1) = mUpW.z;
	mView(3, 1) = y;

	mView(0, 2) = mLookW.x;
	mView(1, 2) = mLookW.y;
	mView(2, 2) = mLookW.z;
	mView(3, 2) = z;

	mView(0, 3) = 0.0f;
	mView(1, 3) = 0.0f;
	mView(2, 3) = 0.0f;
	mView(3, 3) = 1.0f;


}

void Camera::lookAt(D3DXVECTOR3& pos, D3DXVECTOR3& target, D3DXVECTOR3& up)
{
	D3DXVECTOR3 L = target - pos;
	D3DXVec3Normalize(&L, &L);

	D3DXVECTOR3 R;
	D3DXVec3Cross(&R, &up, &L);
	D3DXVec3Normalize(&R, &R);

	D3DXVECTOR3 U;
	D3DXVec3Cross(&U, &L, &R);
	D3DXVec3Normalize(&U, &U);

	mPosW = pos;
	mRightW = R;
	mUpW = U;
	mLookW = L;

	buildView();
	buildWorldFrustumPlanes();

	mViewProj = mView * mProj;
}

void Camera::setLens(float fov, float aspect, float nearZ, float farZ)
{
	farclip = farZ;
	D3DXMatrixPerspectiveFovLH(&mProj, fov, aspect, nearZ, farZ);
	//D3DXMatrixOrthoOffCenterLH(&mProj,-5.5,5.5, -5.5*fov, 5.5*fov,nearZ, farZ);
	buildWorldFrustumPlanes();
	mViewProj = mView * mProj;
}
void Camera::setOrthoLens(float width, float height, float aspect, float nearZ, float farZ)
{
	farclip = farZ;

	//D3DXMatrixPerspectiveFovLH(&mProj, fov, aspect, nearZ, farZ);
	// D3DXMatrixOrthoOffCenterLH(&mProj,-100.5*aspect,100.5*aspect, -100.5, 100.5,nearZ, farZ);
	//D3DXMatrixOrthoOffCenterLH(&mProj,-width/2*aspect,width/2*aspect,-height/2, height/2,nearZ, farZ);
	D3DXMatrixOrthoLH(&mProj, width, height, nearZ, farZ);
	buildWorldFrustumPlanes();
	mViewProj = mView * mProj;
}
float Camera::clip()
{
	return farclip;
}
void Camera::setSpeed(float s)
{
	mSpeed = s;
}
void Camera::buildWorldFrustumPlanes()
{
	// Note: Extract the frustum planes in world space.

	D3DXMATRIX VP = mView * mProj;

	D3DXVECTOR4 col0(VP(0, 0), VP(1, 0), VP(2, 0), VP(3, 0));
	D3DXVECTOR4 col1(VP(0, 1), VP(1, 1), VP(2, 1), VP(3, 1));
	D3DXVECTOR4 col2(VP(0, 2), VP(1, 2), VP(2, 2), VP(3, 2));
	D3DXVECTOR4 col3(VP(0, 3), VP(1, 3), VP(2, 3), VP(3, 3));

	// Planes face inward.
	mFrustumPlanes[0] = (D3DXPLANE)(col2);        // near
	mFrustumPlanes[1] = (D3DXPLANE)(col3 - col2); // far
	mFrustumPlanes[2] = (D3DXPLANE)(col3 + col0); // left

	mFrustumPlanes[3] = (D3DXPLANE)(col3 - col0); // right
	mFrustumPlanes[4] = (D3DXPLANE)(col3 - col1); // top
	mFrustumPlanes[5] = (D3DXPLANE)(col3 + col1); // bottom

	for (int i = 0; i < 6; i++)
		D3DXPlaneNormalize(&mFrustumPlanes[i], &mFrustumPlanes[i]);

}
bool Camera::isVisible(const AABB& box)const
{
	D3DXVECTOR3 P;
	D3DXVECTOR3 Q;

	//      N  *Q                    *P
	//      | /                     /
	//      |/                     /
	// -----/----- Plane     -----/----- Plane
	//     /                     / |
	//    /                     /  |
	//   *P                    *Q  N
	//
	// PQ forms diagonal most closely aligned with plane normal.
	// For each frustum plane, find the box diagonal (there are
	// four main diagonals that intersect the box center point)
	// that points in the same direction as the normal along each
	// axis (i.e., the diagonal that is most aligned with the
	// plane normal).  Then test if the box is in front of the
	// plane or not.
	for (int i = 0; i < 6; ++i)
	{
		// For each coordinate axis x, y, z...
		for (int j = 0; j < 3; ++j)
		{
			// Make PQ point in the same direction as
			// the plane normal on this axis.
			if (mFrustumPlanes[i][j] >= 0.0f)
			{
				P[j] = box.minPt[j];
				Q[j] = box.maxPt[j];
			}
			else
			{
				P[j] = box.maxPt[j];
				Q[j] = box.minPt[j];
			}
		}
		// If box is in negative half-space, it is behind
		// the plane, and thus, completely outside the
		// frustum. Note that because PQ points roughly in
		// the direction of the plane normal, we can deduce
		// that if Q is outside, then P is also outside--thus we

		// only need to test Q.

		// outside
		if (D3DXPlaneDotCoord(&mFrustumPlanes[i], &Q) < 0.0f)
			return false;
	}
	// If we got here, then the AABB is not in the negative
	// space of any of the six frustums; therefore, it must
	// intersect the frustum.
	return true;


}
