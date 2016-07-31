/*
*	Assignment 1
*	Eddy Lau
*	A00879725
*	Set 4B
*/

#ifndef FONTS_H
#define FONTS_H 

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "basics.h"

class Fonts {
public:
	Fonts();
	virtual ~Fonts() { }

	HRESULT LoadAlphabet(char* strPathName, int LetterWidth, int LetterHeight, LPDIRECT3DDEVICE9 pDevice);
	HRESULT UnloadAlphabet();
	void PrintChar(int x, int y, char Character, BOOL bTransparent, D3DCOLOR ColorKey, DWORD* pDestData, int DestPitch);
	void PrintString(int x, int y, char* String, BOOL bTransparent, D3DCOLOR ColorKey, DWORD* pDestData, int DestPitch);
	void PrintFrameRate(Basics*, int x, int y, BOOL bTransparent, D3DCOLOR ColorKey, DWORD* pDestData, int DestPitch);

private:
	int g_AlphabetWidth;			// The width of the Alphabet bitmap
	int g_AlphabetHeight;			// The height of the Alphabet bitmap
	int g_AlphabetLetterWidth;		// The width of a letter
	int g_AlphabetLetterHeight;		// The height of a letter
	int g_AlphabetLettersPerRow;	// The number of letters per row
	LPDIRECT3DSURFACE9 g_pAlphabetSurface; // The surface holding the alphabet bitmap										 
	BOOL g_bAlphabetLoaded; // Has the alphabet bitmap been loaded yet?

	Basics* _Basics;
};


#endif