#include "game.h"


namespace {
	Game* _Game; // Global pointer to Game
	bool dirLight;
	bool spotLight;
	bool pointLight;
	const int MESH_LEN = 2;
}
static int selectedMesh = 0;
LPDIRECT3DDEVICE9 g_pDevice = NULL;//graphics device
LPDIRECT3D9 g_pD3D = NULL;

psys::PSystem* Snow = 0;
ID3DXMesh* Sphere = NULL;
BoundingSphere BSphere[MESH_LEN];
D3DXMATRIX WorldMatrices[MESH_LEN];
D3DMATERIAL9 Material[MESH_LEN];
BoundingSphere::BoundingSphere() {	_radius = 1.0f; }
D3DXVECTOR3 _Pos(0.0f, 0.0f, -10.0f);
D3DXVECTOR3 _Target(0.0f, 0.0f, 0.0f);
D3DXVECTOR3 _Up(0.0f, 1.0f, 0.0f);
/**
* C++ transformed code to handle message callbacks--forward messages.
* @return The window message from the MsgProc function
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam) {
	return _Game->MsgProc(hWnd, uMessage, wParam, lParam);
}
/**
* Game Constructor.
* @param hInstance Handle of the main window
*/
Game::Game(HINSTANCE hInstance) {
	_hGameInstance = hInstance; 
	_hGameWindow = NULL;
	_uiClientWidth = 512;
	_uiClientHeight = 512;
	_gameTitle = "Assignment 2";

	xDown, yDown, xMove, yMove, xUp, yUp;
	isMoving = false;
	isMouseUpSet = false;

	autorotate = false;
	eyeX = 0.0f; eyeY = 3.0f; eyeZ = -5.0f;
	lookAtX = 0.0f; lookAtY = 0.0f; lookAtZ = 0.0f;
	upVecX = 0.0f; upVecY = 1.0f; upVecZ = 0.0f;

	//g_pD3D = NULL;//COM object
	//g_pDevice = NULL;//graphics device
	g_pSurface = 0; // Surface for bitmap
	g_pBackSurface = 0;//backbuffer
	g_SavedPresParams = {};
	_Meshes = new ID3DXMesh*[MESH_LEN];
	_Fonts = new Fonts();
	_Basics = new Basics();
}
/**
* Game Deconstructor
*/
Game::~Game() {}
/**
* Handles the window's message loop and the game cycle.
* @return a windows message, more likely to be a process termination message
*/
int Game::Run() {
	MSG msg = {0};
	static float lastTime = (float)timeGetTime();
	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			Loop(lastTime);
		}
	}
	ShutDown();

	return static_cast<int>(msg.wParam);
}
/**
* Creates the main window and initializes the COM object.
* @return HRESULT value, S_OK, if operations were successfull
*		or E_FAIL, if the window or COM object could not create
*/
int Game::Init() {
	if (!InitWindow()) { return E_FAIL; }
	if (FAILED(InitDirect3D())) { return E_FAIL; }
	
	// Setting up snow particles
	util::BoundingBox boundingBox;
	boundingBox._min = D3DXVECTOR3(-10.0f, -10.0f, -10.0f);
	boundingBox._max = D3DXVECTOR3(10.0f, 10.0f, 10.0f);
	Snow = new psys::Snow(&boundingBox, 5000);
	Snow->init(g_pDevice, "snowflake.dds");
	
	BYTE* v = 0;
	// Index 0: Teapot
	D3DXCreateTeapot(g_pDevice, &_Meshes[0], 0);
	Material[0] = util::YELLOW_MTRL;
	
	D3DXComputeBoundingSphere(
		(D3DXVECTOR3*)v,
		_Meshes[0]->GetNumVertices(),
		D3DXGetFVFVertexSize(_Meshes[0]->GetFVF()),
		&BSphere[0]._center,
		&BSphere[0]._radius);
	_Meshes[0]->UnlockVertexBuffer();
	
	D3DXCreateSphere(g_pDevice, BSphere[0]._radius, 20, 20, &Sphere, 0); // Bounding sphere
	D3DXMatrixTranslation(&WorldMatrices[0], 0.0f, 0.0f, 0.0f); // Inital place of object

	// Index 1: Box
	D3DXCreateBox(g_pDevice, 2.0f, 2.0f, 2.0f, &_Meshes[1], 0);
	Material[1] = util::GREEN_MTRL;

	D3DXComputeBoundingSphere(
		(D3DXVECTOR3*)v,
		_Meshes[1]->GetNumVertices(),
		D3DXGetFVFVertexSize(_Meshes[0]->GetFVF()),
		&BSphere[1]._center,
		&BSphere[1]._radius);
	_Meshes[1]->UnlockVertexBuffer();

	D3DXCreateSphere(g_pDevice, BSphere[1]._radius, 20, 20, &Sphere, 0); // Bounding sphere
	D3DXMatrixTranslation(&WorldMatrices[1], 0.0f, 0.0f, 0.0f);


	// Set up the lights
	util::InitLights(g_pDevice);
	dirLight = true;
	spotLight = true;
	pointLight = true;

	// Set up the view matrix
	SetupMatrices();

	return S_OK;
}
/**
* Handles frame count, rendering, and game exiting by key.
* @return HRESULT, S_OK, if the operation succeeded
*/
int Game::Loop(float lastTime) {
	float currTime = (float)timeGetTime();
	float timeDelta = (currTime - lastTime)*0.000001f;
	lastTime = currTime;

	_Basics->FrameCount();
	Render(timeDelta);
	if (GetAsyncKeyState(VK_ESCAPE)) { PostQuitMessage(0); }

	return S_OK;
}
/**
* Handles the rendering of a bitmap, displaying the frame rate and drawing lines.
*/
int Game::Render(float timeDelta) {
	HRESULT r;

	if (!g_pDevice) {
		_Basics->SetError("Cannot render because there is no device");
		return E_FAIL;
	}

	//clear the display arera with colour black, ignore stencil buffer
	g_pDevice->Clear(0, 0, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 25), 1.0f, 0);
	
	r = ValidateDevice();
	if (FAILED(r)) { return E_FAIL; }
	
	// Display surface with bitmap
	g_pDevice->StretchRect(g_pSurface, NULL, g_pBackSurface, NULL, D3DTEXF_LINEAR);

	Snow->update(timeDelta);
	


	// Begin the scene
	if (SUCCEEDED(g_pDevice->BeginScene()))
	{
		for (int i = 0; i < MESH_LEN; i++)
		{
			// Render Meshes
			g_pDevice->SetTransform(D3DTS_WORLD, &WorldMatrices[i]);
			g_pDevice->SetMaterial(&Material[i]);
			_Meshes[i]->DrawSubset(0);

		
			/*// Render the bounding sphere with alpha blending so we can see 
			// through it.
			D3DMATERIAL9 blue = util::BLUE_MTRL;
			blue.Diffuse.a = 0.25f; // 25% opacity
			g_pDevice->SetMaterial(&blue);
			Sphere->DrawSubset(0);
			g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
			*/
		}
		
		Snow->render();
		// End the scene
		g_pDevice->EndScene();
	}
	

	
	// Frame Rate
	D3DLOCKED_RECT Locked;
	D3DSURFACE_DESC d3dsd;
	g_pBackSurface->GetDesc(&d3dsd);
	g_pBackSurface->LockRect(&Locked, 0, 0);

	_Fonts->PrintFrameRate(_Basics, d3dsd.Width - 45, 15, TRUE, // Display framerate
		D3DCOLOR_ARGB(255, 255, 0, 255), (DWORD*)Locked.pBits, Locked.Pitch);
	g_pBackSurface->UnlockRect();

	g_pDevice->Present(NULL, NULL, NULL, NULL);//swap over buffer to primary surface
	return S_OK;
}
/**
* Release all resources for the game cycle.
* @return HRESULT, S_OK, if the operation succeeded.
*/
int Game::ShutDown() {
	_Fonts->UnloadAlphabet();
	
	for (int i = 0; i < MESH_LEN; i++)
	{
		_Meshes[i]->Release();
	}

	if (g_pSurface) { g_pSurface->Release(); } //////
	if (g_pBackSurface) { g_pBackSurface->Release(); }
	if (g_pDevice) { g_pDevice->Release(); }
	if (g_pD3D) { g_pD3D->Release(); }
		
	return S_OK;
}
/**
* Creates the main window for displaying the game.
* @return TRUE, if the window was successfully created
*/
bool Game::InitWindow() {
	HWND hWnd;
	WNDCLASSEX wc;

	// Set every value in destination to NULL
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	static char strAppName[] = "Eddy Lau, Assignment 2";

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = _hGameInstance;
	wc.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_HAND);
	wc.hCursor = LoadCursor(NULL, IDC_CROSS);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = strAppName;

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Failed to register window class.", NULL, NULL);
		return false;
	}

	hWnd = CreateWindowEx(NULL,
		strAppName,
		strAppName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		_uiClientWidth, _uiClientHeight,
		NULL,
		NULL,
		_hGameInstance,
		NULL);

	_hGameWindow = hWnd;//set our global window handle
	if (!_hGameWindow) {
		MessageBox(NULL, "Failed to create game window.", NULL, NULL);
		return false;
	}
	ShowWindow(_hGameWindow, SW_SHOW);
	UpdateWindow(_hGameWindow);

	return true;
}
/**
* Initiates the COM oject and its components to handle 3D images.
* @return S_OK, if all components for handling 3D are sucessfully created
*/
int Game::InitDirect3D() { // GameInit();
	HRESULT r = 0;//return values

	g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);//COM object
	if (g_pD3D == NULL) {
		_Basics->SetError("Could not create IDirect3D9 object");
		return E_FAIL;
	}

	r = InitDirect3DDevice(_hGameWindow, 640, 480, FALSE, D3DFMT_X8R8G8B8, g_pD3D, &g_pDevice);
	if (FAILED(r)) {
		_Basics->SetError("Initialization of the device failed");
		return E_FAIL;
	}
	g_pDevice->Clear(0, 0, D3DCLEAR_TARGET, D3DCOLOR_ARGB(255, 0, 0, 55), 1.0f, 0);

	//get pointer to backbuffer
	r = g_pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &g_pBackSurface);
	if (FAILED(r)) {
		_Basics->SetError("Couldn't get backbuffer");
		return E_FAIL;
	}

	// Loading bitmap and put it to a surface
	r = _Basics->LoadBitmapToSurface(_bitmapName, &g_pSurface, g_pDevice);
	if (FAILED(r)) { _Basics->SetError("could not load bitmap surface"); }
	r = D3DXLoadSurfaceFromSurface(g_pBackSurface, NULL, NULL, g_pSurface, NULL, NULL,
		D3DX_FILTER_TRIANGLE, 0);
	if (FAILED(r)) { _Basics->SetError("did not copy surface"); }

	_Fonts->LoadAlphabet("Alphabet vSmall.bmp", 8, 16, g_pDevice);
	_Basics->InitTiming();

	return S_OK;
}

/**
* Initializes the Direct 3D device.
* @return S_OK, if it succeeded
*/
int Game::InitDirect3DDevice(HWND hWndTarget, int Width, int Height, BOOL bWindowed, D3DFORMAT FullScreenFormat, LPDIRECT3D9 pD3D, LPDIRECT3DDEVICE9* ppDevice) {
	D3DPRESENT_PARAMETERS d3dpp;//rendering info
	D3DDISPLAYMODE d3ddm;//current display mode info
	HRESULT r = 0;

	if (*ppDevice) { (*ppDevice)->Release(); }

	ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	r = pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);
	if (FAILED(r)) {
		_Basics->SetError("Could not get display adapter information");
		return E_FAIL;
	}

	d3dpp.BackBufferWidth = Width;
	d3dpp.BackBufferHeight = Height;
	d3dpp.BackBufferFormat = bWindowed ? d3ddm.Format : FullScreenFormat;
	d3dpp.BackBufferCount = 1;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;//D3DSWAPEFFECT_COPY;
	d3dpp.hDeviceWindow = hWndTarget;
	d3dpp.Windowed = bWindowed;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.FullScreen_RefreshRateInHz = 0;//default refresh rate
	d3dpp.PresentationInterval = bWindowed ? 0 : D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	r = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWndTarget, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, ppDevice);
	if (FAILED(r)) {
		_Basics->SetError("Could not create the render device");
		return E_FAIL;
	}

	return S_OK;
}
/**
* Handles all given Windows Messages.
*/
LRESULT Game::MsgProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam) {
	POINT m;
	D3DXMATRIXA16 matWorld, matTranslate, matRotation;

	switch (uMessage) {
	case WM_CREATE: {
		return 0;
	}
	case WM_LBUTTONDOWN: {

		return 0;
	}
	case WM_LBUTTONUP: {
		// compute the ray in view space given the clicked screen point
		Ray ray = CalcPickingRay(LOWORD(lParam), HIWORD(lParam));
		
		// transform the ray to world space
		D3DXMATRIX view;
		g_pDevice->GetTransform(D3DTS_VIEW, &view);

		D3DXMATRIX viewInverse;
		D3DXMatrixInverse(&viewInverse, 0, &view);

		TransformRay(&ray, &viewInverse);
		
		// test for a hit
		for (int i = 0; i < MESH_LEN; i++) {
			if (RaySphereIntTest(&ray, &BSphere[i])) {
				selectedMesh = i;
				if (dirLight) {
					dirLight = false;
					g_pDevice->LightEnable(0, false);
				}
				else {
					dirLight = true;
					g_pDevice->LightEnable(0, true);
				}
			}
		}
		
			
		return 0;
	}
	case WM_KEYDOWN: {
		switch (LOWORD(wParam)) {
		// Select Light
		case VK_F1: {	
			if (dirLight) {
				dirLight = false;
				g_pDevice->LightEnable(0, false);
			}
			else {
				dirLight = true;
				g_pDevice->LightEnable(0, true);
			}
			break;
		}
		case VK_F2: {
			if (pointLight) {
				pointLight = false;
				g_pDevice->LightEnable(1, false);
			}
			else {
				pointLight = true;
				g_pDevice->LightEnable(1, true);
			}
			break;
		}
		case VK_F3: {
			if (spotLight) {
				spotLight = false;
				g_pDevice->LightEnable(2, false);
			}
			else {
				spotLight = true;
				g_pDevice->LightEnable(2, true);
			}
			break;
		}
		case VK_F4: {
			break;
		}
		case VK_F5: {
			dirLight = false; g_pDevice->LightEnable(0, false);
			spotLight = false; g_pDevice->LightEnable(1, false);
			pointLight = false; g_pDevice->LightEnable(2, false);
			break;
		}
		// Select mesh
		case 0x31: { // Keycode for 1
			selectedMesh = 0; // Tiger

			break;
		}
		case 0x32: { // Keycode for 2
			selectedMesh = 1; // Airplane

			break;
		}
		case 0x33: { // Keycode for 2
			// no mesh selected
			eyeX = 0.0f;	eyeY = 3.0f;	eyeZ = -5.0f;
			lookAtX = 0.0f;	lookAtY = 0.0f; lookAtZ = 0.0f;
			upVecX = 0.0f;	upVecY = 1.0f;	upVecZ = 0.0f;
			break;
		}
		// Mesh Transformation
		case VK_LEFT: { // Keycode for left arrow
			D3DXMatrixTranslation(&matTranslate, -1.0f, 0.0f, 0.0f);
			WorldMatrices[selectedMesh] *= matTranslate;
			BSphere[selectedMesh]._center = D3DXVECTOR3(
				WorldMatrices[selectedMesh]._41, WorldMatrices[selectedMesh]._42, WorldMatrices[selectedMesh]._43);
			
			break;
		}
		case VK_DOWN: { // Keycode for down arrow
			D3DXMatrixTranslation(&matTranslate, 0.0f, -1.0f, 0.0f);
			WorldMatrices[selectedMesh] *= matTranslate;
			BSphere[selectedMesh]._center = D3DXVECTOR3(
				WorldMatrices[selectedMesh]._41, WorldMatrices[selectedMesh]._42, WorldMatrices[selectedMesh]._43);
			break;
		}
		case VK_RIGHT: { // Keycode for  right arrow
			D3DXMatrixTranslation(&matTranslate, 1.0f, 0.0f, 0.0f);
			WorldMatrices[selectedMesh] *= matTranslate;
			BSphere[selectedMesh]._center = D3DXVECTOR3(
				WorldMatrices[selectedMesh]._41, WorldMatrices[selectedMesh]._42, WorldMatrices[selectedMesh]._43);
			break;
		}
		case VK_UP: { // Keycode for up arrow
			D3DXMatrixTranslation(&matTranslate, 0.0f, 1.0f, 0.0f);
			WorldMatrices[selectedMesh] *= matTranslate;
			BSphere[selectedMesh]._center = D3DXVECTOR3(
				WorldMatrices[selectedMesh]._41, WorldMatrices[selectedMesh]._42, WorldMatrices[selectedMesh]._43);
			break;
		}
		case VK_NUMPAD8: { // Keycode for Numpad8
			D3DXMatrixTranslation(&matTranslate, 0.0f, 0.0f, 1.0f);
			WorldMatrices[selectedMesh] *= matTranslate;
			BSphere[selectedMesh]._center = D3DXVECTOR3(
				WorldMatrices[selectedMesh]._41, WorldMatrices[selectedMesh]._42, WorldMatrices[selectedMesh]._43);
			break;
		}
		case VK_NUMPAD2: { // Keycode for Numpad2
			D3DXMatrixTranslation(&matTranslate, 0.0f, 0.0f, -1.0f);
			WorldMatrices[selectedMesh] *= matTranslate;
			BSphere[selectedMesh]._center = D3DXVECTOR3(
				WorldMatrices[selectedMesh]._41, WorldMatrices[selectedMesh]._42, WorldMatrices[selectedMesh]._43);
			break;
		}
		case VK_ADD: { // Keycode for +
			//_Meshes[selectedMesh]->scaleByFactor(0.1f); // scale up

			break;
		}
		case VK_SUBTRACT: { // Keycode for -
			//_Meshes[selectedMesh]->scaleByFactor(-0.1f); // scale down

			break;
		}
		case VK_NUMPAD4: { // Keycode for NUMPAD4
			D3DXMatrixRotationYawPitchRoll(&matRotation, 0.0f, -1.0f, 0.0);
			WorldMatrices[selectedMesh] *= matRotation;
			BSphere[selectedMesh]._center = D3DXVECTOR3(
				WorldMatrices[selectedMesh]._41, WorldMatrices[selectedMesh]._42, WorldMatrices[selectedMesh]._43);
			break;
		}
		case VK_NUMPAD6: { // Keycode for NUMPAD4
			D3DXMatrixRotationYawPitchRoll(&matRotation, 1.0f, 0.0f, 0.0);
			WorldMatrices[selectedMesh] *= matRotation;
			BSphere[selectedMesh]._center = D3DXVECTOR3(
				WorldMatrices[selectedMesh]._41, WorldMatrices[selectedMesh]._42, WorldMatrices[selectedMesh]._43);

			break;
		}
		case VK_NUMPAD5: { 

			break;
		}
		case 0x57: { // Keycode for W
			_Pos.y += 0.3; _Target.y += 0.3;
			D3DXMATRIX V;
			D3DXMatrixLookAtLH(&V, &_Pos, &_Target, &_Up);
			g_pDevice->SetTransform(D3DTS_VIEW, &V);

			break;
		}
		case 0x41: { // Keycode for A
			_Pos.x -= 0.3; _Target.x -= 0.3;
			D3DXMATRIX V;
			D3DXMatrixLookAtLH(&V, &_Pos, &_Target, &_Up);
			g_pDevice->SetTransform(D3DTS_VIEW, &V);
			break;
		}
		case 0x53: { // Keycode for S
			_Pos.y -= 0.3; _Target.y -= 0.3;
			D3DXMATRIX V;
			D3DXMatrixLookAtLH(&V, &_Pos, &_Target, &_Up);
			g_pDevice->SetTransform(D3DTS_VIEW, &V);
			break;
		}
		case 0x44: { // Keycode for D
			_Pos.x += 0.3; _Target.x += 0.3;
			D3DXMATRIX V;
			D3DXMatrixLookAtLH(&V, &_Pos, &_Target, &_Up);
			g_pDevice->SetTransform(D3DTS_VIEW, &V);
			break;
		}

		case 0x49: { // Keycode for I
			_Pos.z += 0.3; _Target.z += 0.3;
			D3DXMATRIX V;
			D3DXMatrixLookAtLH(&V, &_Pos, &_Target, &_Up);
			g_pDevice->SetTransform(D3DTS_VIEW, &V);

			break;
		}
		case 0x4A: { // Keycode for J
			_Pos.x -= 0.3;
			D3DXMATRIX V;
			D3DXMatrixLookAtLH(&V, &_Pos, &_Target, &_Up);
			g_pDevice->SetTransform(D3DTS_VIEW, &V);
			break;
		}
		case 0x4B: { // Keycode for K
			_Pos.z -= 0.3; _Target.z -= 0.3;
			D3DXMATRIX V;
			D3DXMatrixLookAtLH(&V, &_Pos, &_Target, &_Up);
			g_pDevice->SetTransform(D3DTS_VIEW, &V);
			break;
		}
		case 0x4C: { // Keycode for L
			// rotate right
			_Pos.x += 0.3;
			D3DXMATRIX V;
			D3DXMatrixLookAtLH(&V, &_Pos, &_Target, &_Up);
			g_pDevice->SetTransform(D3DTS_VIEW, &V);

			break;
		}
		} // end keydown switch

		return 0;
	}
	case WM_PAINT: {
		ValidateRect(hWnd, NULL);//basically saying - yeah we took care of any paint msg without any overhead
		return 0;
	}
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	default: {
		return DefWindowProc(hWnd, uMessage, wParam, lParam);
	} 
	} // end of main switch
}
/* Transformed matrices to determine the position settings for the camera and the meshes*/

VOID Game::SetupMatrices()
{
	

	D3DXMATRIX V;
	D3DXMatrixLookAtLH(&V, &_Pos, &_Target, &_Up);
	g_pDevice->SetTransform(D3DTS_VIEW, &V);

	//
	// Set projection matrix.
	//

	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(
		&proj,
		D3DX_PI * 0.25f, // 45 - degree
		(float)800 / (float)600,
		1.0f,
		1000.0f);
	g_pDevice->SetTransform(D3DTS_PROJECTION, &proj);
}

/**
* Validates the device every time there's a render.
* @return S_OK, if device had no issues in its state
*/
HRESULT Game::ValidateDevice() {
	HRESULT r = 0;
	//Test current state of device
	r = g_pDevice->TestCooperativeLevel();
	if (FAILED(r)) {
		//if device is lost then return failure
		if (r == D3DERR_DEVICELOST)
			return E_FAIL;
		//if device is ready to be reset then try
		if (r == D3DERR_DEVICENOTRESET) {
			//release back surface
			g_pBackSurface->Release();
			//reset device
			r = g_pDevice->Reset(&g_SavedPresParams);
			if (FAILED(r)) {
				//device was not ready to be reset
				_Basics->SetError("Could not reset device");
				PostQuitMessage(E_FAIL);
				return E_FAIL;
			}
			//reacquire a pointer to new back buffer
			r = g_pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &g_pBackSurface);
			if (FAILED(r)) {
				_Basics->SetError("Unable to reacquire back buffer");
				PostQuitMessage(0);
				return E_FAIL;
			}
			g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 0.0f, 0);
			_Basics->RestoreGraphics();
		}
	}
	return S_OK;
}

Ray Game::CalcPickingRay(int x, int y)
{
	float px = 0.0f;
	float py = 0.0f;

	D3DVIEWPORT9 vp;
	g_pDevice->GetViewport(&vp);

	D3DXMATRIX proj;
	g_pDevice->GetTransform(D3DTS_PROJECTION, &proj);

	px = (((2.0f*x) / vp.Width) - 1.0f) / proj(0, 0);
	py = (((-2.0f*y) / vp.Height) + 1.0f) / proj(1, 1);

	Ray ray;
	ray._origin = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	ray._direction = D3DXVECTOR3(px, py, 1.0f);

	return ray;
}

void Game::TransformRay(Ray* ray, D3DXMATRIX* T)
{
	// transform the ray's origin, w = 1.
	D3DXVec3TransformCoord(
		&ray->_origin,
		&ray->_origin,
		T);

	// transform the ray's direction, w = 0.
	D3DXVec3TransformNormal(
		&ray->_direction,
		&ray->_direction,
		T);

	// normalize the direction
	D3DXVec3Normalize(&ray->_direction, &ray->_direction);
}

bool Game::RaySphereIntTest(Ray* ray, BoundingSphere* sphere)
{
	D3DXVECTOR3 v = ray->_origin - sphere->_center;

	float b = 2.0f * D3DXVec3Dot(&ray->_direction, &v);
	float c = D3DXVec3Dot(&v, &v) - (sphere->_radius * sphere->_radius);

	// find the discriminant
	float discriminant = (b * b) - (4.0f * c);

	// test for imaginary number
	if (discriminant < 0.0f)
		return false;

	discriminant = sqrtf(discriminant);

	float s0 = (-b + discriminant) / 2.0f;
	float s1 = (-b - discriminant) / 2.0f;

	// if a solution is >= 0, then we intersected the sphere
	if (s0 >= 0.0f || s1 >= 0.0f)
		return true;

	return false;
}