#ifndef INCLUDED_WINDOW_MANAGER_DIRECTX_H
#define INCLUDED_WINDOW_MANAGER_DIRECTX_H

#include "window_manager.h"

class WindowManagerDirectX : public WindowManager
{
protected:
	bool		m_waitVRT;

	int			m_borderWidth;
	int			m_titleHeight;
	
protected:
	HWND		m_hWnd;
	HDC			m_hDC;
	HGLRC		m_hRC;

public:
	WindowManagerDirectX();
	~WindowManagerDirectX() override;
	void SaveDesktop();
	void RestoreDesktop();
	bool EnableOpenGL(int _colourDepth, int _zDepth);
	void DisableOpenGL();
	void ListAllDisplayModes();
	bool CreateWin(int _width, int _height, bool _windowed,
			   int _colourDepth, int _refreshRate, int _zDepth, bool _waitVRT, bool _antiAlias,
                 const wchar_t* _title);
	void DestroyWin();
	PlatformWindow *Window();
	void Flip();
	void NastyPollForMessages();
	void NastySetMousePos(int x, int y);
	void NastyMoveMouse(int x, int y);
  void CaptureMouse();
	void UncaptureMouse();
	void EnsureMouseCaptured();
	void EnsureMouseUncaptured();
	void HideMousePointer();
  void WindowMoved();
	void OpenWebsite( const char *_url );
};

void AppMain();

#endif
