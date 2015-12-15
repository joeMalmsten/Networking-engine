#pragma once
#include <windows.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <list>

class WindowJunk
{

  public:
    WindowJunk(void);
    void CtorDirectXJunk(void);
    void DtorDirectXJunk(void);
    void addText(std::string text_);
    void clearText(void);
    ~WindowJunk(void);
  
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


    LONG width;
    LONG height;

    HWND windowHandle;
    WNDCLASS windowClass;   
    
    // DirectX Data
    LPDIRECT3D9 g_pDirect3D;               // Direct X Object
    LPDIRECT3DDEVICE9 g_pDirect3D_Device;  // Direct X Device
    D3DPRESENT_PARAMETERS Direct3D_Params; // Direct X Params
    LPD3DXFONT m_font;
	std::list<std::string> text;
};