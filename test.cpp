#include <windows.h>
#include "game.h"


/**
* Main function that starts the program.
* @return wParam message from a function that handles message loop
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pstrCmdLine, int iCmdShow) {
	Game* game = new Game(hInstance);

	// Creates window and the directX COM object
	if (FAILED(game->Init())) { return E_FAIL; }
	return game->Run();
}