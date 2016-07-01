/*
*	Assignment 1
*	Eddy Lau
*	A00879725
*	Set 4B
*/

#ifndef BASICS_H
#define BASICS_H

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <tchar.h>
#include <stdio.h>

class Basics {
public:
	Basics();
	virtual ~Basics() {}

	HRESULT InitTiming();
	void FrameCount();
	void SetError(char* szFormat, ...);
	//int InitDirect3DDevice(HWND hWndTarget, int Width, int Height, BOOL bWindowed, D3DFORMAT FullScreenFormat, LPDIRECT3D9 pD3D, LPDIRECT3DDEVICE9* ppDevice);
	HRESULT RestoreGraphics();
	int LoadBitmapToSurface(char*, LPDIRECT3DSURFACE9*, LPDIRECT3DDEVICE9);
	INT64 GetFrameRate() { return g_FrameRate; }

private:
	INT64 g_Frequency;
	INT64 g_FrameRate;
	INT64 g_FrameCount;
};


#endif