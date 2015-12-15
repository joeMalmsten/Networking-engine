#include "WindowJunk.h"

WindowJunk::WindowJunk(void)
{
  width = 800;
  height = 600;

  HINSTANCE hInstance = GetModuleHandle(NULL);

  windowClass.style = CS_HREDRAW | CS_VREDRAW;
  windowClass.lpfnWndProc = WndProc;
  windowClass.cbClsExtra = 0;
  windowClass.cbWndExtra = sizeof(void*);
  windowClass.hIcon = (HICON) LoadImage((HINSTANCE) GetModuleHandle(NULL),0, IMAGE_ICON, 0, 0,  0 );
  windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);	
   // TODO: Don't always do this...
  //ShowCursor(false);
  windowClass.hbrBackground = (HBRUSH)( GetStockObject(BLACK_BRUSH) );
  windowClass.lpszMenuName  = NULL;
  windowClass.lpszClassName = "WAMP WAMP WAMP";
  windowClass.hInstance = hInstance;

  RegisterClass(&windowClass);

  RECT windowRectangle = {0,0,width,height};
  
  AdjustWindowRect(&windowRectangle, WS_OVERLAPPEDWINDOW, NULL);

  windowHandle = CreateWindow(windowClass.lpszClassName,
      "WAMP WAMP WAMP",
      WS_OVERLAPPEDWINDOW,//WS_MAXIMIZE | WS_POPUP,//WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      windowRectangle.right - windowRectangle.left,
      windowRectangle.bottom - windowRectangle.top,
      NULL,
      NULL,
      0,
      this);

  ShowWindow(windowHandle, SW_SHOW);
  UpdateWindow(windowHandle);

  RECT clientRect;
  GetClientRect(windowHandle, &clientRect);

  width  = clientRect.right - clientRect.left;
  height = clientRect.bottom - clientRect.top;
  CtorDirectXJunk();
}

WindowJunk::~WindowJunk(void)
{
  DtorDirectXJunk();
  UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

LRESULT CALLBACK WindowJunk::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        /* When it's time for the window to be closed and removed */
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);

    }
    return 0;
}
void WindowJunk::CtorDirectXJunk(void)
{
  //! Create the Direct3D interface.
  g_pDirect3D = Direct3DCreate9(D3D_SDK_VERSION);

  //Create a clean struct to hold various device information.
  ZeroMemory(&Direct3D_Params, sizeof(Direct3D_Params)); // Cleans out the struct, for use.

  //! Sets up the struct for the desired Direct X Window.
  Direct3D_Params.Windowed = TRUE;   // Set fullscreen.
  Direct3D_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;    // Discards old frames.
  Direct3D_Params.hDeviceWindow = windowHandle;         // Set the Window to be used by Direct3D.
  Direct3D_Params.BackBufferFormat = D3DFMT_X8R8G8B8;    // Set the back buffer format to 32-bit.
  Direct3D_Params.BackBufferWidth = width;        // Set the Width.
  Direct3D_Params.BackBufferHeight = height;      // Set the Height.
  Direct3D_Params.EnableAutoDepthStencil = TRUE;         // Enables the Depth Stencil.
  Direct3D_Params.AutoDepthStencilFormat = D3DFMT_D16;   // Formats the Depth Stencil.

  D3DCAPS9 caps;
  g_pDirect3D->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
  DWORD VertexProc = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

  if(caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
    VertexProc = D3DCREATE_HARDWARE_VERTEXPROCESSING;

  //! Creates a device class using this information and information from the d3dpp stuct.
  g_pDirect3D->CreateDevice(D3DADAPTER_DEFAULT, // Default setting.
    D3DDEVTYPE_HAL,     // Some setting.
    windowHandle,       // Handle for the Window.
    VertexProc,         // Software/Hardware setting.
    &Direct3D_Params,   // Direct3D Window struct.
    &g_pDirect3D_Device // Direct X Device.    
    );

  // Turn off the 3D lighting. *NOTE: handled in shaders.*
  g_pDirect3D_Device->SetRenderState(D3DRS_LIGHTING, FALSE);
  // Allows for two sided Tiles and flipping.
  g_pDirect3D_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
  // Turn on the z-buffer.
  g_pDirect3D_Device->SetRenderState(D3DRS_ZENABLE, TRUE);

  // Transparency Settings.
  g_pDirect3D_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
  g_pDirect3D_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
  g_pDirect3D_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

  // Create a D3DX font object //WITH SIZE 18 can fit 33 lines
  D3DXCreateFont(g_pDirect3D_Device, 
                  18, 0, FW_NORMAL, 0, 
                  FALSE, 
                  DEFAULT_CHARSET, 
                  OUT_DEFAULT_PRECIS, 
                  DEFAULT_QUALITY, 
                  DEFAULT_PITCH | FF_DONTCARE, 
                  TEXT("Trebuchet MS"), 
                  &m_font 
                );


}
void WindowJunk::addText(std::string text_)
{

  // This clears out the Window preparing it to be rebuffered.
  g_pDirect3D_Device->Clear(0,                                // Some setting.
    NULL,                             // Some setting.
    D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, // Clear the backbuffer and the zBuffer.
    D3DCOLOR_XRGB(0, 0, 0),           // Color set to black.
    1.0f,                             // Default Z value.
    0);                               // Some setting.



  //! Renders the actual scene if the Device Begins.
  if(SUCCEEDED(g_pDirect3D_Device->BeginScene()))
  {
    // Create a colour for the text - in this case blue
    D3DCOLOR fontColor = D3DCOLOR_ARGB(200,0,255,0);   

    // Create a rectangle to indicate where on the screen it should be drawn
    RECT rct;
    rct.left = 10;
    rct.right = 790;//width-3;
    rct.top = 0;
    rct.bottom = 600;//height-3;

    // Draw some text
	if(text_.size())
    text.push_back(text_);

	if(text.size() > 39)
		text.pop_front();

	LONG i = 0; 
	for(std::list<std::string>::iterator b = text.begin(); b != text.end(); ++b)
	{
		rct.top = 15 * i;
		rct.bottom = 15 * ( 1 + i );
		++i;

		m_font->DrawText(NULL, b->c_str(), -1, &rct, 0, fontColor);
	}

    // Do 3D rendering on the back buffer here.
    g_pDirect3D_Device->EndScene(); // Ends the 3D scene.

	g_pDirect3D_Device->Present(NULL, NULL, NULL, NULL);

  }
}
void WindowJunk::clearText(void)
{
  text.clear();
}
void WindowJunk::DtorDirectXJunk(void)
{
  if(g_pDirect3D_Device != NULL)
    g_pDirect3D_Device->Release();

  if(g_pDirect3D != NULL)
    g_pDirect3D->Release();
  
  if(m_font != NULL)
    m_font->Release();
}
