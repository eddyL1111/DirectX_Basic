#include "basics.h"


Basics::Basics() {
	g_Frequency = 0;
	g_FrameRate = 0;
	g_FrameCount = 0;
}

/**
* Displays an error message when issues arises.
*/
void Basics::SetError(char* szFormat, ...) {
	char szBuffer[1024];
	va_list pArgList;

	va_start(pArgList, szFormat);
	_vsntprintf_s(szBuffer, sizeof(szBuffer) / sizeof(char), szFormat, pArgList);
	va_end(pArgList);

	OutputDebugString(szBuffer);
	OutputDebugString("\n");
}
/**
* Initiates the timing to calculate the frequency of frames 
* @return S_OK, if system supports high resolution timing
*/
HRESULT Basics::InitTiming() {
	QueryPerformanceFrequency((LARGE_INTEGER*)&g_Frequency);

	if (g_Frequency == 0) {
		SetError("The system does not support high resolution timing");
		return E_FAIL;
	}

	return S_OK;
}
/**
* Counts the frames per second.
*/
void Basics::FrameCount() {
	INT64 NewCount = 0;
	static INT64 LastCount = 0;
	INT64 Difference = 0;

	QueryPerformanceCounter((LARGE_INTEGER*)&NewCount);

	if (NewCount == 0)
		SetError("The system does not support high resolution timing");

	g_FrameCount++;
	Difference = NewCount - LastCount;

	if (Difference >= g_Frequency) {
		g_FrameRate = g_FrameCount;
		g_FrameCount = 0;
		LastCount = NewCount;
	}
}

/**
* Restores the graphics when issues were encountered while validating.
* @return S_OK
*/
HRESULT Basics::RestoreGraphics() {
	return S_OK;
}
/**
* Loads bitmap to surface.
* @return S_OK, if bitmap was loaded successfully
*/
int Basics::LoadBitmapToSurface(char* PathName, LPDIRECT3DSURFACE9* ppSurface, LPDIRECT3DDEVICE9 pDevice) {
	HRESULT r;
	HBITMAP hBitmap;
	BITMAP Bitmap;

	// Loads image according to the path name
	hBitmap = (HBITMAP)LoadImage(NULL, PathName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if (hBitmap == NULL) {
		SetError("Unable to load bitmap");
		return E_FAIL;
	}

	GetObject(hBitmap, sizeof(BITMAP), &Bitmap);
	DeleteObject(hBitmap);//we only needed it for the header info to create a D3D surface

	//create surface for bitmap
	r = pDevice->CreateOffscreenPlainSurface(Bitmap.bmWidth, Bitmap.bmHeight, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, ppSurface, NULL);
	if (FAILED(r)) {
		SetError("Unable to create surface for bitmap load");
		return E_FAIL;
	}

	//load bitmap onto surface
	r = D3DXLoadSurfaceFromFile(*ppSurface, NULL, NULL, PathName, NULL, D3DX_DEFAULT, 0, NULL);
	if (FAILED(r)) {
		SetError("Unable to load file to surface");
		return E_FAIL;
	}

	return S_OK;
}