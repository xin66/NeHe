#include <windows.h>
#include <stdio.h>

#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glut.h>
#include <gl\glaux.h>

#pragma comment(linker, "/subsystem:windows /entry:WinMainCRTStartup")
#pragma comment(lib, "glaux.lib")

HGLRC hRC = NULL;
HDC hDC = NULL;
HWND hWnd = NULL;
HINSTANCE hInstance;

BOOL keys[256];
BOOL active = TRUE;
BOOL isfullscreen = FALSE;

BOOL twinkle;
BOOL tp;

const int STAR_NUM = 50;

typedef struct {
	int r, g, b;
	GLfloat dist;
	GLfloat angle;
} stars;

stars star[STAR_NUM];

GLfloat zoom = -15.0f;
GLfloat tilt = 90.0f;
GLfloat spin;
GLuint loop;
GLuint texture[1];

AUX_RGBImageRec * loadBMP(char * filename) {
	FILE * fp = NULL;
	if (fp = fopen(filename, "r")) {
		fclose(fp);
		return auxDIBImageLoad(filename);
	}
	return NULL;
}

int loadGLTextures() {
	int status = FALSE;

	AUX_RGBImageRec * textureImage[1];
	memset(textureImage, 0, sizeof(void *));

	if (textureImage[0] = loadBMP("data/star.bmp")) {
		status = TRUE;
		glGenTextures(1, &texture[0]);
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, textureImage[0]->sizeX, textureImage[0]->sizeY,
			0, GL_RGB, GL_UNSIGNED_BYTE, textureImage[0]->data);
	}

	if (textureImage[0]) {
		if (textureImage[0]->data)
			free(textureImage[0]->data);
		free(textureImage[0]);
	}

	return status;
}

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

GLvoid resizeGL(GLsizei width, GLsizei height) {
		if (height == 0)
			height = 1;

		glViewport(0, 0, width, height);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//
		gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
}

// all setup for OpenGL goes here
int initGL(GLvoid) {
	if (!loadGLTextures())
		return FALSE;

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_BLEND);

	for (loop = 0; loop < STAR_NUM; loop ++) {
		star[loop].angle = 0.0f;
		star[loop].dist = (float(loop) / STAR_NUM) * 5.0f;
		star[loop].r = rand() % 256;
		star[loop].g = rand() % 256;
		star[loop].b = rand() % 256;
	}

	return TRUE;
}

int drawGL(GLvoid) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	for (loop = 0; loop < STAR_NUM; loop ++) {
		glLoadIdentity();
		glTranslatef(0.0f, 0.0f, zoom);
		glRotatef(tilt, 1.0f, 0.0f, 0.0f);
		glRotatef(star[loop].angle, 0.0f, 1.0f, 0.0f);
		glTranslatef(star[loop].dist, 0.0f, 0.0f);
		glRotatef(-star[loop].angle, 0.0f, 1.0f, 0.0f);
		glRotatef(-tilt, 1.0f, 0.0f, 0.0f);

		if (twinkle) {
			glColor4ub(star[(STAR_NUM - loop) - 1].r, star[(STAR_NUM - loop) - 1].g, star[(STAR_NUM - loop) - 1].b, 255);
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);
				glVertex3f(-1.0f, -1.0f, 0.0f);
				glTexCoord2f(1.0f, 0.0f);
				glVertex3f(1.0f, -1.0f, 0.0f);
				glTexCoord2f(1.0f, 1.0f);
				glVertex3f(1.0f, 1.0f, 0.0f);
				glTexCoord2f(0.0f, 1.0f);
				glVertex3f(-1.0f, 1.0f, 0.0f);
			glEnd();
		}

		glRotatef(spin, 0.0f, 0.0f, 1.0f);
		glColor4ub(star[loop].r, star[loop].g, star[loop].b, 255);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(-1.0f, -1.0f, 0.0f);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(1.0f, -1.0f, 0.0f);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(1.0f, 1.0f, 0.0f);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(-1.0f, 1.0f, 0.0f);
		glEnd();

		spin += 0.01f;
		star[loop].angle += float(loop) / STAR_NUM;
		star[loop].dist -= 0.01f;

		if (star[loop].dist < 0.0f) {
			star[loop].dist += 5.0f;
			star[loop].r = rand() % 256;
			star[loop].g = rand() % 256;
			star[loop].b = rand() % 256;
		}
	}
	
	return TRUE;
}

GLvoid killGL(GLvoid) {
	if (isfullscreen) {
		ChangeDisplaySettings(NULL, 0);
		ShowCursor(TRUE);
	}
	if (hRC) {
		if (!wglMakeCurrent(NULL, NULL))
			MessageBox(NULL, "release of DC & RC failed", "shutdown error", MB_OK | MB_ICONINFORMATION);
		if (!wglDeleteContext(hRC))
			MessageBox(NULL, "release rendering context failed", "shutdown error", MB_OK | MB_ICONINFORMATION);
		hRC = NULL;
	}
	//
	if (hDC && !ReleaseDC(hWnd, hDC)) {
		MessageBox(NULL, "release device context failed", "shutdown error", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;
	}
	//
	if (hWnd && !DestroyWindow(hWnd)) {
		MessageBox(NULL, "could not release hWnd", "shutdown error", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;
	}
	//
	if (!UnregisterClass("londontown", hInstance)) {
		MessageBox(NULL, "could not unregister class", "shutdown error", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;
	}
}

int createGL(char * title, int width, int height, int bits, BOOL fullscreenflag) {
	GLuint PixelFormat;

	WNDCLASS wnd;

	DWORD dwExStyle;
	DWORD dwStyle;

	RECT windowRect;
	windowRect.left = (long)0;
	windowRect.right = (long)width;
	windowRect.top = (long)0;
	windowRect.bottom = (long)height;

	hInstance = GetModuleHandle(NULL);
	wnd.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wnd.lpfnWndProc = (WNDPROC) WndProc;
	wnd.cbClsExtra = 0;
	wnd.cbWndExtra = 0;
	wnd.hInstance = hInstance;
	wnd.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wnd.hbrBackground = NULL;
	wnd.lpszMenuName = NULL;
	wnd.lpszClassName = "londontown";

	if (!RegisterClass(&wnd)) {
		MessageBox(NULL, "failed to register the window class", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	isfullscreen = fullscreenflag;
	
	if (isfullscreen) {
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = width;
		dmScreenSettings.dmPelsHeight = height;
		dmScreenSettings.dmBitsPerPel = bits;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			if (MessageBox(NULL, "the requested fullscreen mode is not supported by\n your video card.\
use windowed mode instead?", "londontown", MB_YESNO | MB_ICONEXCLAMATION) == IDYES) {
				isfullscreen = FALSE;
			} else {
				MessageBox(NULL, "program will now close", "londontown", MB_OK | MB_ICONSTOP);
				return FALSE;
			}
		}
	}

	if (isfullscreen) {
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
		ShowCursor(FALSE);
	} else {
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;
	}

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	if (!(hWnd = CreateWindowEx(dwExStyle,
			"londontown",
			title,
			WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle,
			0, 0,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,
			NULL,
			NULL,
			hInstance,
			NULL))) {
		killGL();
		MessageBox(NULL, "window creation error.", "error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		bits,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		16,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	if (!(hDC = GetDC(hWnd))) {
		killGL();
		MessageBox(NULL, "cannot create a GL device context.", "error", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}
	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd))) {
		killGL();
		MessageBox(NULL, "cannot find a suitable PixelFormat.", "error", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}
	if (!SetPixelFormat(hDC, PixelFormat, &pfd)) {
		killGL();
		MessageBox(NULL, "cannot set the PixelFormat.", "error", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}
	if (!(hRC = wglCreateContext(hDC))) {
		killGL();
		MessageBox(NULL, "cannot create a GL rendering context.", "error", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}
	if (!wglMakeCurrent(hDC, hRC)) {
		killGL();
		MessageBox(NULL, "cannot activate the GL rendering context.", "error", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	resizeGL(width, height);

	if (!initGL()) {
		killGL();
		MessageBox(NULL, "Initialization failed.", "error", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_ACTIVATE : {
			if (!HIWORD(wParam))
				active = TRUE;
			else
				active = FALSE;
			return 0;
		}
		case WM_SYSCOMMAND : {
			switch (wParam) {
				case SC_SCREENSAVE :
				case SC_MONITORPOWER :
					return 0;
			}
			break;
		}
		case WM_CLOSE : {
			PostQuitMessage(0);
			return 0;
		}
		case WM_KEYDOWN : {
			keys[wParam] = TRUE;
			return 0;
		}
		case WM_KEYUP : {
			keys[wParam] = FALSE;
			return 0;
		}
		case WM_SIZE : {
			resizeGL(LOWORD(lParam), HIWORD(lParam));
			return 0;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPSTR lpCmdLine,
		int nCmdShow) {
	MSG msg;
	int done = FALSE;

	/*
		if (MessageBox (NULL, "Would you like to run in fullscreen mode?", "Start fullscreen",
				MB_YESNO | MB_ICONQUESTION) == IDNO) {
			isfullscreen = FALSE;	// windowed mode
		}
	*/

	if (!createGL("Lua's OpenGL Framework", 640, 480, 16, isfullscreen))
		return 0;

	while (!done) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				done = TRUE;
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} else {
			if (active) {
				if (keys[VK_ESCAPE])
					done = TRUE;
				else {
					drawGL();
					SwapBuffers(hDC);
					if (keys['T'] && !tp) {
						tp = TRUE;
						twinkle = !twinkle;
					}
					if (!keys['T'])
						tp = TRUE;
					if (keys[VK_UP])
						tilt -= 0.5f;
					if (keys[VK_DOWN])
						tilt += 0.5f;
					if (keys[VK_PRIOR])
						zoom -= 0.2f;
					if (keys[VK_NEXT])
						zoom += 0.2f;
				}
				if (keys[VK_F1]) {
					keys[VK_F1] = FALSE;
					killGL();
					isfullscreen = !isfullscreen;

					if (!createGL("Lua's OpenGL Framework", 640, 480, 16, isfullscreen))
						return 0;
				}
			}
		}
	}

	killGL();
	return msg.wParam;
}