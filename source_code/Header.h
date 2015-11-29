//--------------------------------------------------------------------------------------
// File: Header.h
//
// Include header files.
//--------------------------------------------------------------------------------------
#include "Carmera.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>
#include <string>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#include <tchar.h>

// Default Window size.
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

// Forward declaration.
class Camera;
extern Camera* g_Camera;