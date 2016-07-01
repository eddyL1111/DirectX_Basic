/*
*	Assignment 1
*	Eddy Lau
*	A00879725
*	Set 4B
*/
#ifndef GAME_H
#define GAME_H

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <mmsystem.h>
#include <tchar.h>
#include <strsafe.h>


#include "basics.h"
#include "fonts.h"
#include "utility.h"




extern LPDIRECT3DDEVICE9 g_pDevice;
extern LPDIRECT3D9 g_pD3D;
struct Ray
{
	D3DXVECTOR3 _origin;
	D3DXVECTOR3 _direction;
};

struct BoundingSphere
{
	BoundingSphere();
	D3DXVECTOR3 _center;
	float       _radius;
	
};
class Game {
public:
	Game(HINSTANCE);

	virtual ~Game();

	int Run();

	int Init();
	int Loop(float lastTime);
	int Render(float timeDelta);
	int ShutDown();
	VOID SetupMatrices();
	LRESULT MsgProc(HWND, UINT, WPARAM, LPARAM);
	Ray CalcPickingRay(int x, int y);
	void TransformRay(Ray* ray, D3DXMATRIX* T);
	bool RaySphereIntTest(Ray*, BoundingSphere*);
	HRESULT ValidateDevice();
	int InitDirect3D();
	int InitDirect3DDevice(HWND hWndTarget, int Width, int Height, BOOL bWindowed, D3DFORMAT FullScreenFormat, LPDIRECT3D9 pD3D, LPDIRECT3DDEVICE9* ppDevice);


private:
	LPDIRECT3DSURFACE9 g_pSurface; // Surface for bitmap
	PDIRECT3DSURFACE9 g_pBackSurface;//backbuffer
	D3DPRESENT_PARAMETERS g_SavedPresParams;

	char* _bitmapName = "NightLand.bmp";
	HWND _hGameWindow;
	HINSTANCE _hGameInstance;
	unsigned int _uiClientWidth;
	unsigned int _uiClientHeight;
	std::string _gameTitle;
	bool InitWindow();
						
	// Mouse Listener Attributes
	int xDown, yDown, xMove, yMove, xUp, yUp;
	bool isMoving;
	bool isMouseUpSet;

	
	

	bool autorotate;
	float eyeX; float eyeY; float eyeZ;
	float lookAtX; float lookAtY; float lookAtZ;
	float upVecX; float upVecY; float upVecZ;

	//int MESH_LEN;
	Fonts* _Fonts;
	Basics* _Basics;
	ID3DXMesh** _Meshes;
	//BoundingSphere BSphere;
	//ID3DXMesh* Sphere;
	//D3DXMATRIX* WorldMatrices;
};


#endif