//--------------------------------------------------------------------------------------
// File: main.cpp
//
// The GPU-based voxelization implementation to convert triangles to voxels.
//--------------------------------------------------------------------------------------

// GUI headers.
#include  <CEGUI/RendererModules/Direct3D11/Renderer.h>
#include <CEGUI/CEGUI.h>

// D3D headers.
#include "d3dApp.h"
#include "RawDataViewer.h"
#include "Voxelizer.h"
#include "ConvertObj.h"
#include "voxelViewer.h"
#include "profiler.h"

//--------------------------------------------------------------------------------------
// A class for windows event handles, rendering and update loop.
//-------------------------------------------------------------------------------------
class InitDirect3DApp : public D3DApp
{
public: 
	InitDirect3DApp(HINSTANCE hInstance);
	~InitDirect3DApp();
	void initApp();		// Initialize all classes and parameters.
	void onResize();
	LRESULT msgProc(UINT msg, WPARAM wParam, LPARAM lParam);

	// Update method.
	void updateScene(float dt);
	// Rendering method.
	void drawScene();


private:
	void resetOMTargetsAndViewport();
	void loadingThread(std::string sName);

	// GUI helper methods.
	void initGUI();
	UINT virtualkey2scancode(WPARAM wParam, LPARAM lParam);
	void addModelItem(std::string sName);

	// GUI events.
	bool reVoxelization(const CEGUI::EventArgs& args)
	{
		m_bVoxelizationFlag = true;
		return true;
	}
	bool outputData(const CEGUI::EventArgs& args)
	{
		mVoxel.WriteToFile((char*)mFilenameBox->getText().c_str());
		return true;
	}


	bool showRaw(const CEGUI::EventArgs& args)
	{
		m_bRaw = !m_bRaw;
		return true;
	}

	bool modelChanged(const CEGUI::EventArgs& args)
	{

			std::string modelName(mModelSelector->getText().c_str());


	
			loadingThread(modelName);
		

		return true;
	}
	bool resolutionChanged(const CEGUI::EventArgs& args)
	{
		if (mRes64->isSelected())
		{
			m_iRes = 64;
			mVoxel.ResetVoxelRes(m_iRes, DXGI_FORMAT_R8G8B8A8_UNORM);

		}
		if (mRes128->isSelected())
		{
			m_iRes = 128;
			mVoxel.ResetVoxelRes(m_iRes, DXGI_FORMAT_R8G8B8A8_UNORM);
		}
		if (mRes256->isSelected())
		{
			m_iRes = 256;
			mVoxel.ResetVoxelRes(m_iRes, DXGI_FORMAT_R8G8B8A8_UNORM);
		}
		mSliceSpin->setMaximumValue(m_iRes);
		m_bVoxelizationFlag = true;
		return true;
	}

	bool dataChanged(const CEGUI::EventArgs& args)
	{
		if (mBinaryButton->isSelected())
		{
			m_iShowData = 1;
		}
		if (mNormalmButton->isSelected())
		{
			m_iShowData = 0;
		}
		if (mObjectButton->isSelected())
		{
			m_iShowData = 2;
		}


		return true;
	}



private:


	MultiConverts* mScene;


	
	bool m_bVoxelizationFlag;

	D3DXMATRIX mLandWorld;
	D3DXMATRIX mWVP;
	int m_iShowData;
	float m_fVoxelSize;
	Convert mObj;
	Convert mObj2;

	RawDataViewer mRawDataViewer;
	Voxelizer mVoxel;
	voxelViewer mViewer;
	int m_iRes;
	bool m_bRaw;
	Profiler mProfiler;

	// GUI objects.
	CEGUI::Combobox* mModelSelector;
	CEGUI::RadioButton* mRes64;
	CEGUI::RadioButton* mRes128;
	CEGUI::RadioButton* mRes256;
	CEGUI::RadioButton* mBinaryButton;
	CEGUI::RadioButton* mNormalmButton;
	CEGUI::RadioButton* mObjectButton;
	CEGUI::Spinner* mSliceSpin;
	CEGUI::Editbox* mFilenameBox;
	CEGUI::DefaultWindow* mVoxelTime;
	CEGUI::DefaultWindow* mLoadTime;
	CEGUI::DefaultWindow* mRenderTime;
	

};


//--------------------------------------------------------------------------------------
// Entry point to the program.
//--------------------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{


	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


	Camera camera;
	g_Camera = &camera;

	InitDirect3DApp theApp(hInstance);

	theApp.initApp();



	return theApp.run();

}
InitDirect3DApp::InitDirect3DApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	D3DXMatrixIdentity(&mLandWorld);
}
InitDirect3DApp::~InitDirect3DApp()
{
	if (md3dImmediateContext)
		md3dImmediateContext->ClearState();
}

//--------------------------------------------------------------------------------------
// Map VirtualKey.
//--------------------------------------------------------------------------------------
UINT InitDirect3DApp::virtualkey2scancode(WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(lParam) & 0x0F00)
	{
		UINT scancode = MapVirtualKey(wParam, 0);
		return scancode | 0x80;
	}
	else
	{
		return HIWORD(lParam) & 0x00FF;
	}
}

//--------------------------------------------------------------------------------------
// Model loading method.
//--------------------------------------------------------------------------------------
void InitDirect3DApp::loadingThread(std::string sName)
{

	GameTimer counter;
	counter.tick();

	mScene->clear();

	int index = sName.find("Scene");
	if (index >= 0)
	{
		if (sName.find("teapotx2") >= 0)
		{
			mObj.reloadObject((char*)"..//media//teapot.obj", 1.0f);
			mObj2.reloadObject((char*)"..//media//teapot.obj", 1.0f);
			mScene->Add(mObj);
			D3DXMATRIX trans;
			D3DXMatrixTranslation(&trans, 0, 0, 15);
			mScene->Add(mObj2, &trans);
		}

	}
	else
	{
		sName = "..//media//" + sName + ".obj";

		mObj.reloadObject((char*)sName.c_str(), 1.0f);
		mScene->Add(mObj);

	}
	m_bVoxelizationFlag = true;
	
	// Show loading time.
	counter.tick();
	float loadTime = counter.getDeltaTime();
	mLoadTime->setText("Load:" + std::to_string(loadTime) + "s");


}

LRESULT InitDirect3DApp::msgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{

	// Inject windows mouse and keyboard events to GUI.
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(CEGUI::MouseButton::LeftButton);
		break;
	case WM_LBUTTONUP:
		CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(CEGUI::MouseButton::LeftButton);
		break;
	case WM_KEYUP:
		CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyUp((CEGUI::Key::Scan)virtualkey2scancode(wParam, lParam));
		break;
	case WM_KEYDOWN:
		CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyDown((CEGUI::Key::Scan)virtualkey2scancode(wParam, lParam));
		break;
	case WM_CHAR:
		CEGUI::System::getSingleton().getDefaultGUIContext().injectChar(static_cast<CEGUI::utf32>(wParam));
		break;
	case WM_MOUSEMOVE:
	{
		CEGUI::System::getSingleton().getDefaultGUIContext().injectMousePosition((float)LOWORD(lParam), (float)HIWORD(lParam));

		if (wParam == MK_LBUTTON)
		{
			g_Camera->injectMousePosition((float)LOWORD(lParam), (float)HIWORD(lParam));
		}
		else
		{
			g_Camera->clearMousePosition();
		}
		break;
	}
		


	}
	return D3DApp::msgProc(msg, wParam, lParam);

}
//--------------------------------------------------------------------------------------
// Helper method for GUI to add a new item.
//--------------------------------------------------------------------------------------
void InitDirect3DApp::addModelItem(std::string sName)
{
	CEGUI::ListboxTextItem *item = new CEGUI::ListboxTextItem(sName, mModelSelector->getSelectionLength() + 1);
	item->setSelectionBrushImage("Vanilla-Images/GenericBrush");
	mModelSelector->addItem(item);
}
// --------------------------------------------------------------------------------------
// Initialize everything for voxelization.
//--------------------------------------------------------------------------------------
void InitDirect3DApp::initApp()
{

	// Initial parameters.
	m_iRes = 128;
	m_bRaw = false;
	m_fVoxelSize = 0.05f;
	m_ClearColor = D3DXCOLOR(1, 1, 1, 0);
	m_bVoxelizationFlag = true;
	m_iShowData = 0;
	// Initialize program.
	D3DApp::initApp();

	// Initialize voxel classes for voxelization and previewing.
	mViewer.Init(md3dDevice, md3dImmediateContext);
	mRawDataViewer.Init(md3dDevice, md3dImmediateContext);
	mVoxel.Init(md3dDevice, md3dImmediateContext, m_iRes, DXGI_FORMAT_R8G8B8A8_UNORM);

	// Initialize 2 model loaders.
	mObj.Init(md3dDevice, md3dImmediateContext, "..//media//venusm.obj", 0.005f);
	mObj2.Init(md3dDevice, md3dImmediateContext, "..//media//sphere.obj", 0.1f);

	// Add a multi-model loader.
	mScene = new MultiConverts();
	mScene->Add(mObj);

	// Initialize a profiler.
	mProfiler.Init(md3dDevice, md3dImmediateContext);

	// Initialize a camera.
	g_Camera->setLens(D3DX_PI * 0.25f, (float)m_iClientWidth / m_iClientHeight, 1.0f, 5000.0f);
	g_Camera->pos() = D3DXVECTOR3(0, 0.0f, 0.0f);
	g_Camera->SetlockMove(false);
	g_Camera->SetlockPitch(false);
	g_Camera->setSpeed(2.0f);
	g_Camera->pos() = D3DXVECTOR3(15, 26, 7);
	g_Camera->lookAt(g_Camera->pos(), mObj.bndBox.center(), D3DXVECTOR3(0.0f, 1.0f, 0.0f));

	// Initialize GUIs.
	initGUI();
}
// --------------------------------------------------------------------------------------
// Initialize GUIs.
//---------------------------------------------------------------------------------------
void InitDirect3DApp::initGUI()
{

	CEGUI::Direct3D11Renderer& myRenderer = CEGUI::Direct3D11Renderer::bootstrapSystem(md3dDevice, md3dImmediateContext);
	using namespace CEGUI;
	DefaultResourceProvider* rp = static_cast<DefaultResourceProvider*>(CEGUI::System::getSingleton().getResourceProvider());
	rp->setResourceGroupDirectory("schemes", "GUI/schemes/");
	rp->setResourceGroupDirectory("imagesets", "GUI/imagesets/");
	rp->setResourceGroupDirectory("fonts", "GUI/fonts/");
	rp->setResourceGroupDirectory("layouts", "GUI/layouts/");
	rp->setResourceGroupDirectory("looknfeels", "GUI/looknfeel/");
	rp->setResourceGroupDirectory("lua_scripts", "GUI/lua_scripts/");
	// Set the default resource groups.
	CEGUI::ImageManager::setImagesetDefaultResourceGroup("imagesets");
	CEGUI::Font::setDefaultResourceGroup("fonts");
	CEGUI::Scheme::setDefaultResourceGroup("schemes");
	CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
	CEGUI::WindowManager::setDefaultResourceGroup("layouts");
	CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");

	std::string s_skinName = "WindowsLook";
	SchemeManager::getSingleton().createFromFile(s_skinName + ".scheme");
	SchemeManager::getSingleton().createFromFile("VanillaSkin.scheme");
	SchemeManager::getSingleton().createFromFile("VanillaCommonDialogs.scheme");

	Font& defaultFont = FontManager::getSingleton().createFromFile("DejaVuSans-10.font");
	WindowManager& wmgr = WindowManager::getSingleton();
	Window* myRoot = wmgr.createWindow("DefaultWindow", "root");
	System::getSingleton().getDefaultGUIContext().setRootWindow(myRoot);

	// Load GUI layout.
	CEGUI::Window *newWindow = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("voxelWindow.layout");
	newWindow->setAlpha(0.7f);

	// Get performance labels.
	mVoxelTime = (CEGUI::DefaultWindow*)newWindow->getChild("Performance")->getChild("Voxelize");
	mLoadTime = (CEGUI::DefaultWindow*)newWindow->getChild("Performance")->getChild("Load");
	mRenderTime = (CEGUI::DefaultWindow*)newWindow->getChild("Performance")->getChild("Render");

	// Append all model lists.
	mModelSelector = (CEGUI::Combobox*)newWindow->getChild("ModelSelector");
	mModelSelector->setInheritsAlpha(false);
	mModelSelector->setAlpha(0.9);

	
	addModelItem("teapot");
	addModelItem("venusm");
	addModelItem("sphere");
	addModelItem("subbox");
	addModelItem("angle");
	addModelItem("dragon");
	addModelItem("hand");
	addModelItem("ming");
	addModelItem("sphere2");
	addModelItem("Scene1:teapotx2");

	// Add events for model loading.
	CEGUI::System::getSingleton().getDefaultGUIContext().getRootWindow()->addChild(newWindow);
	mModelSelector->subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&InitDirect3DApp::modelChanged, this));
	mModelSelector->setReadOnly(true);
	mModelSelector->setItemSelectState(size_t(0), true);
	CEGUI::EventArgs arg;
	modelChanged(arg);

	// Add events while changing the resolution of voxel data.
	mRes64 = (CEGUI::RadioButton*)newWindow->getChild("Res")->getChild("Res64");
	mRes128 = (CEGUI::RadioButton*)newWindow->getChild("Res")->getChild("Res128");
	mRes256 = (CEGUI::RadioButton*)newWindow->getChild("Res")->getChild("Res256");
	mRes128->setSelected(true);
	mRes64->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged, CEGUI::Event::Subscriber(&InitDirect3DApp::resolutionChanged, this));
	mRes128->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged, CEGUI::Event::Subscriber(&InitDirect3DApp::resolutionChanged, this));
	mRes256->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged, CEGUI::Event::Subscriber(&InitDirect3DApp::resolutionChanged, this));

	// Other GUIs.
	CEGUI::ToggleButton* rawToggle = (CEGUI::ToggleButton*)newWindow->getChild("Raw")->getChild("Display");
	rawToggle->setSelected(m_bRaw);
	rawToggle->subscribeEvent(CEGUI::ToggleButton::EventSelectStateChanged, CEGUI::Event::Subscriber(&InitDirect3DApp::showRaw, this));

	mBinaryButton = (CEGUI::RadioButton*)newWindow->getChild("Info")->getChild("Binary");
	mNormalmButton = (CEGUI::RadioButton*)newWindow->getChild("Info")->getChild("Normal");
	mObjectButton = (CEGUI::RadioButton*)newWindow->getChild("Info")->getChild("Object");

	mNormalmButton->setSelected(true);
	mBinaryButton->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged, CEGUI::Event::Subscriber(&InitDirect3DApp::dataChanged, this));
	mNormalmButton->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged, CEGUI::Event::Subscriber(&InitDirect3DApp::dataChanged, this));
	mObjectButton->subscribeEvent(CEGUI::RadioButton::EventSelectStateChanged, CEGUI::Event::Subscriber(&InitDirect3DApp::dataChanged, this));

	mSliceSpin = (CEGUI::Spinner*)newWindow->getChild("Raw")->getChild("Slice");
	mSliceSpin->setTextInputMode(Spinner::Integer);
	mSliceSpin->setMaximumValue(m_iRes);
	CEGUI::PushButton* voxelButton = (CEGUI::PushButton*)newWindow->getChild("Process")->getChild("Voxelization");
	voxelButton->subscribeEvent(CEGUI::RadioButton::EventMouseClick, CEGUI::Event::Subscriber(&InitDirect3DApp::reVoxelization, this));
	CEGUI::PushButton* outputButton = (CEGUI::PushButton*)newWindow->getChild("Process")->getChild("Output");
	outputButton->subscribeEvent(CEGUI::RadioButton::EventMouseClick, CEGUI::Event::Subscriber(&InitDirect3DApp::outputData, this));

	mFilenameBox = (CEGUI::Editbox*)newWindow->getChild("Process")->getChild("Filename");
	mFilenameBox->setText("Voxels.txt");
	

}
void InitDirect3DApp::onResize()
{
	D3DApp::onResize();
}

// --------------------------------------------------------------------------------------
// Update method.
//---------------------------------------------------------------------------------------
void InitDirect3DApp::updateScene(float dt)
{
	D3DApp::updateScene(dt);
	CEGUI::System::getSingleton().getDefaultGUIContext().injectTimePulse(dt);
	g_Camera->update(dt, 0.0f);
}

// --------------------------------------------------------------------------------------
// Main rendering method.
//---------------------------------------------------------------------------------------
void InitDirect3DApp::drawScene()
{
	// Clear back buffer.
	D3DApp::drawScene();

	// Restore default states, input layout and primitive topology .
	md3dImmediateContext->OMSetDepthStencilState(0, 0);
	float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	md3dImmediateContext->OMSetBlendState(0, blendFactors, 0xffffffff);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Setup for voxelization.
	if (m_bVoxelizationFlag)
	{

		// Start a profiling.
		mProfiler.StartProfile();

		Camera voxelView;
		AABB voxelRegion = mScene->bndBox;
		// Optimize voxel size to fit the model's bounding box.
		D3DXVECTOR3 voxelCamCenter = voxelRegion.center();

		D3DXVECTOR3 voxelRealSize = (2 * voxelRegion.extent()) / (mVoxel.GetResoultion() - 1);

		voxelCamCenter.z = -voxelRealSize.z + voxelRegion.minPt.z;

		// Find the maximum component of a voxel.
		m_fVoxelSize = max(voxelRealSize.z, max(voxelRealSize.x, voxelRealSize.y));

		// Clear binding.
		ID3D11ShaderResourceView *const pSRV[2] = { NULL, NULL };
		md3dImmediateContext->PSSetShaderResources(0, 2, pSRV);
		md3dImmediateContext->VSSetShaderResources(0, 2, pSRV);

		// Set the camera parameters to this voxelizer.
		voxelView.lookAt(voxelCamCenter, voxelRegion.center(), D3DXVECTOR3(0.0f, 1.0f, 0.0f));
		mVoxel.SetViewMatrix((voxelView.view()), true);
		mVoxel.SetVoxelSize((float*)(D3DXVECTOR3(m_fVoxelSize, m_fVoxelSize, m_fVoxelSize)));

		// Apply voxelization shaders and set the viewport size (32*32).
		mVoxel.BeginVoxelize(32);

		// Draw every model.
		for (int i = 0; i < mScene->renderModels.size(); i++)
		{
			mVoxel.SetObjID(i + 1);
			mVoxel.SetWorldMatrix(&mScene->transforms[i]);
			mScene->renderModels[i]->draw();
		}
		resetOMTargetsAndViewport();

		// Set voxel resolution for preview.
		mViewer.SetRes(mVoxel.GetResoultion());

		// Disable voxelization.
		m_bVoxelizationFlag = false;

		
		// End profiling.
		mProfiler.EndProfile();

	} 

	GameTimer counter;
	counter.tick();

	// Draw cubes for preview.
	mViewer.ShowVoxels(mVoxel.SRV(), mVoxel.GetResoultion()*mVoxel.GetResoultion()*mVoxel.GetResoultion(), 10, m_iShowData);

	// Draw a full-screen quad to show raw data in 3D texture.
	if (m_bRaw)
		mRawDataViewer.ShowRawVoxeldata(mVoxel.SRV(), (float)mSliceSpin->getCurrentValue() / (float)m_iRes);

	// Render GUI interfaces.
	CEGUI::System::getSingleton().renderAllGUIContexts();

	mSwapChain->Present(0, 0);
	mVoxelTime->setText("voxelization:" + std::to_string(mProfiler.GetTime()) + "ms");
	counter.tick();

	// Show rendering time.
	float renderingTime = counter.getDeltaTime();
	mRenderTime->setText("Render:" + std::to_string(renderingTime*1000) + "ms");
}


void InitDirect3DApp::resetOMTargetsAndViewport()
{
	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = (float)m_iClientWidth;
	vp.Height = (float)m_iClientHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	md3dImmediateContext->RSSetViewports(1, &vp);
}


