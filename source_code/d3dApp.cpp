//--------------------------------------------------------------------------------------
// File: D3DApp.cpp
//--------------------------------------------------------------------------------------
#include "d3dApp.h"
#include "resource.h"

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	static D3DApp* app = 0;

	switch (message)
	{
	case WM_CREATE:
	{

		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		app = (D3DApp*)cs->lpCreateParams;

		return 0;
	}
	}

	// Don't start processing messages until after WM_CREATE.
	if (app)
		return app->msgProc(message, wParam, lParam);
	else
		return DefWindowProc(hWnd, message, wParam, lParam);
}


// Set default parameters.
D3DApp::D3DApp(HINSTANCE hInstance)
{
	mhAppInst = hInstance;

	m_bAppPaused = false;
	m_bMinimized = false;
	m_bMaximized = false;
	m_bResizing = false;

	m_sFrameStats = L"";

	md3dDevice = 0;
	mSwapChain = 0;
	mDepthStencilBuffer = 0;
	mRenderTargetView = 0;
	mDepthStencilView = 0;

	m_sMainWndCaption = L"D3D11 Application";
	md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	m_ClearColor = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
	m_iClientWidth = WINDOW_WIDTH;
	m_iClientHeight = WINDOW_HEIGHT;
}

D3DApp::~D3DApp()
{
	SAFE_RELEASE(mRenderTargetView);
	SAFE_RELEASE(mDepthStencilView);
	SAFE_RELEASE(mSwapChain);
	SAFE_RELEASE(mDepthStencilBuffer);
	SAFE_RELEASE(md3dDevice);
}
HINSTANCE D3DApp::getAppInst()
{
	return mhAppInst;
}

HWND D3DApp::getMainWnd()
{
	return mhMainWnd;
}

void D3DApp::initApp()
{
	initMainWindow();

	initDirect3D();
}

// Initialize Direct3D hardware.
void D3DApp::initDirect3D()
{
	// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = m_iClientWidth;
	sd.BufferDesc.Height = m_iClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// No multisampling.
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = mhMainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;




	// Select D3DX device creation flag.
	// Debug->D3D11_CREATE_DEVICE_DEBUG.
	// Release->D3D11_CREATE_DEVICE_SINGLETHREADED.
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined( DEBUG ) || defined( _DEBUG )
	createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL  FeatureLevelsRequested = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL FeatureLevels[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3
	};
	UINT               numLevelsRequested = 1;


	HR(D3D11CreateDeviceAndSwapChain(
		0,                 //default adapter
		md3dDriverType,
		0,                 // no software device
		createDeviceFlags,
		&FeatureLevelsRequested,
		numLevelsRequested,
		D3D11_SDK_VERSION,
		&sd,
		&mSwapChain,
		&md3dDevice,
		&mFeatureLevelsSupported,
		&md3dImmediateContext));
	if (mFeatureLevelsSupported != D3D_FEATURE_LEVEL_11_0)
	{
		MessageBoxA(mhMainWnd, "This device is not supported", 0, 0);
	}



	// The remaining steps that need to be carried out for d3d creation
	// also need to be executed every time the window is resized.  So
	// just call the onResize method here to avoid code duplication.

	onResize();
}


void D3DApp::onResize()
{
	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.

	SAFE_RELEASE(mRenderTargetView);
	SAFE_RELEASE(mDepthStencilView);
	SAFE_RELEASE(mDepthStencilBuffer);


	// Resize the swap chain and recreate the render target view.

	HR(mSwapChain->ResizeBuffers(1, m_iClientWidth, m_iClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

	HR(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&mBackBuffer)));
	HR(md3dDevice->CreateRenderTargetView(mBackBuffer, 0, &mRenderTargetView));
	SAFE_RELEASE(mBackBuffer);


	// Create the depth/stencil buffer and view.

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = m_iClientWidth;
	depthStencilDesc.Height = m_iClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1; // multisampling must match
	depthStencilDesc.SampleDesc.Quality = 0; // swap chain values.
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer));
	HR(md3dDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView));


	// Bind the render target view and depth/stencil view to the pipeline.

	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);


	// Set the viewport transform.

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = (float)m_iClientWidth;
	vp.Height = (float)m_iClientHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	md3dImmediateContext->RSSetViewports(1, &vp);
}
void D3DApp::drawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, m_ClearColor);
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

}

// Handling windows events.
LRESULT D3DApp::msgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_bAppPaused = true;
			mTimer.stop();
		}
		else
		{
			m_bAppPaused = false;
			mTimer.start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		m_iClientWidth = LOWORD(lParam);
		m_iClientHeight = HIWORD(lParam);
		if (md3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_bAppPaused = true;
				m_bMinimized = true;
				m_bMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_bAppPaused = false;
				m_bMinimized = false;
				m_bMaximized = true;
				onResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (m_bMinimized)
				{
					m_bAppPaused = false;
					m_bMinimized = false;
					onResize();
				}

				// Restoring from maximized state?
				else if (m_bMaximized)
				{
					m_bAppPaused = false;
					m_bMaximized = false;
					onResize();
				}
				else if (m_bResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					onResize();
				}
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		m_bAppPaused = true;
		m_bResizing = true;
		mTimer.stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		m_bAppPaused = false;
		m_bResizing = false;
		mTimer.start();
		onResize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;
	}

	return DefWindowProc(mhMainWnd, msg, wParam, lParam);
}
void D3DApp::updateScene(float dt)
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.

	static int frameCnt = 0;
	static float t_base = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((mTimer.getGameTime() - t_base) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << L"FPS: " << fps << L"\n"
			<< "Milliseconds: Per Frame: " << mspf;
		m_sFrameStats = outs.str();

		// Reset for next average.
		frameCnt = 0;
		t_base += 1.0f;
	}
}


int D3DApp::run()
{
	MSG msg = { 0 };

	mTimer.reset();

	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			mTimer.tick();

			if (!m_bAppPaused)
				updateScene(mTimer.getDeltaTime());
			else
				Sleep(50);

			drawScene();
		}
	}

	CoUninitialize();

	// Sleep for a second before dismissing the console window.
	Sleep(1000);

	FreeConsole();
	return (int)msg.wParam;
}

void D3DApp::initMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mhAppInst;
	wc.hIcon = LoadIcon(mhAppInst, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = "D3DWndClassName";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, "RegisterClass FAILED", 0, 0);
		PostQuitMessage(0);
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, m_iClientWidth, m_iClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mhMainWnd = CreateWindow("D3DWndClassName", _T("Voxelization Previewer"),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, this);
	if (!mhMainWnd)
	{
		MessageBox(0, "CreateWindow FAILED", 0, 0);
		PostQuitMessage(0);
	}


	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);

}

