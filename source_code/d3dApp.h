//--------------------------------------------------------------------------------------
// File: D3DApp.h
//
// A helper base class for setting a window with D3D11.
// I modified the code from the book "Introduction To 3D Game Programming With DirectX 9.0C: A Shader Approach".
//--------------------------------------------------------------------------------------

#ifndef D3DAPP_H
#define D3DAPP_H

#include "d3dutil.h"
#include <string>
#include <dwrite.h>
#include "Timer.h"

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();

	HINSTANCE getAppInst();
	HWND      getMainWnd();

	int run();

	// Framework methods.  Derived client class overrides these methods to 
	// implement specific application requirements.
	virtual void initApp();
	virtual void onResize();
	virtual void updateScene(float dt);
	virtual void drawScene();
	virtual LRESULT msgProc(UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	void initMainWindow();
	void initDirect3D();
	void initMessageWindow();

protected:

	HINSTANCE mhAppInst;

	HWND      mhMainWnd;
	bool      m_bAppPaused;
	bool      m_bMinimized;
	bool      m_bMaximized;
	bool      m_bResizing;

	GameTimer mTimer;

	std::wstring m_sFrameStats;

	ID3D11Device*    md3dDevice;
	IDXGISwapChain*  mSwapChain;
	ID3D11Texture2D* mDepthStencilBuffer;
	ID3D11RenderTargetView* mRenderTargetView;
	ID3D11DepthStencilView* mDepthStencilView;
	D3D_FEATURE_LEVEL  mFeatureLevelsSupported;
	ID3D11DeviceContext* md3dImmediateContext;


	// Derived class should set these in derived constructor to customize starting values.
	std::wstring m_sMainWndCaption;
	D3D_DRIVER_TYPE md3dDriverType;
	D3DXCOLOR m_ClearColor;
	int m_iClientWidth;
	int m_iClientHeight;
	ID3D11Texture2D* mBackBuffer;


};




#endif // D3DAPP_H