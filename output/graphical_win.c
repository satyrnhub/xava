#include "output/graphical_win.h"
#include "output/graphical.h"

const char szAppName[] = "CAVA";
const char wcWndName[] = "CAVA";

HWND cavaWinWindow;
MSG cavaWinEvent;
HMODULE cavaWinModule;
WNDCLASSEX cavaWinClass;	// same thing as window classes in Xlib
HDC cavaWinFrame;
HGLRC cavaWinGLFrame;
int fgcolor, bgcolor;
int gradientColor[2], grad = 0;
int shadowColor, shadow = 0;
double opacity[2] = {1.0, 1.0};
int trans;

LRESULT CALLBACK WindowFunc(HWND hWnd,UINT msg, WPARAM wParam, LPARAM lParam) {
    PAINTSTRUCT ps;

    switch(msg) {
        case WM_CREATE: break;
	case WM_DESTROY: 
 		// Perform cleanup tasks.
		PostQuitMessage(0); 
    break; 
        default:
            return DefWindowProc(hWnd,msg,wParam,lParam);
    }

    return 0;
}

unsigned char register_window_win(HINSTANCE HIn) {
    cavaWinClass.cbSize=sizeof(WNDCLASSEX);
    cavaWinClass.style=CS_HREDRAW | CS_VREDRAW;
    cavaWinClass.lpfnWndProc=WindowFunc;
    cavaWinClass.cbClsExtra=0;
    cavaWinClass.cbWndExtra=0;
    cavaWinClass.hInstance=HIn;
    cavaWinClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    cavaWinClass.hIcon=LoadIcon(NULL,IDI_APPLICATION);
    cavaWinClass.hCursor=LoadCursor(NULL,IDC_ARROW);
    cavaWinClass.hbrBackground=(HBRUSH)CreateSolidBrush(0x00000000);
    cavaWinClass.lpszMenuName=NULL;
    cavaWinClass.lpszClassName=szAppName;
    cavaWinClass.hIconSm=LoadIcon(NULL,IDI_APPLICATION);

    return RegisterClassEx(&cavaWinClass);
}

void GetDesktopResolution(int *horizontal, int *vertical) {
   RECT desktop;

   // Get a handle to the desktop window
   const HWND hDesktop = GetDesktopWindow();
   // Get the size of screen to the variable desktop
   GetWindowRect(hDesktop, &desktop);
   
   // return dimensions
   (*horizontal) = desktop.right;
   (*vertical) = desktop.bottom;

   return;
}

void init_opengl_win() {
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0, 0, 0, 0);
	return;
}

unsigned char CreateHGLRC(HWND hWnd) {
    PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR),
      1,                                // Version Number
      PFD_DRAW_TO_WINDOW      |         // Format Must Support Window
      PFD_SUPPORT_OPENGL      |         // Format Must Support OpenGL
      PFD_SUPPORT_COMPOSITION |         // Format Must Support Composition
      PFD_DOUBLEBUFFER,                 // Must Support Double Buffering
      PFD_TYPE_RGBA,                    // Request An RGBA Format
      32,                               // Select Our Color Depth
      0, 0, 0, 0, 0, 0,                 // Color Bits Ignored
      8,                                // An Alpha Buffer
      0,                                // Shift Bit Ignored
      0,                                // No Accumulation Buffer
      0, 0, 0, 0,                       // Accumulation Bits Ignored
      24,                               // 16Bit Z-Buffer (Depth Buffer)
      8,                                // Some Stencil Buffer
      0,                                // No Auxiliary Buffer
      PFD_MAIN_PLANE,                   // Main Drawing Layer
      0,                                // Reserved
      0, 0, 0                           // Layer Masks Ignored
   };     

   HDC hdc = GetDC(hWnd);
   int PixelFormat = ChoosePixelFormat(hdc, &pfd);
   if (PixelFormat == 0) {
      assert(0);
      return FALSE ;
   }

   BOOL bResult = SetPixelFormat(hdc, PixelFormat, &pfd);
   if (bResult==FALSE) {
      assert(0);
      return FALSE ;
   }

   cavaWinGLFrame = wglCreateContext(hdc);
   if (!cavaWinGLFrame){
      assert(0);
      return FALSE;
   }

   ReleaseDC(hWnd, hdc);

   return TRUE;
}

void resize_framebuffer(int width,int height) {
	glViewport(0,0, width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW );
	glLoadIdentity();
}

int init_window_win(char *color, char *bcolor, double foreground_opacity, int col, int bgcol, int gradient, char *gradient_color_1, char *gradient_color_2, unsigned int shdw, unsigned int shdw_col, int w, int h) {

	// get handle
	cavaWinModule = GetModuleHandle(NULL);
	FreeConsole();
	
	// register window class
	if(!register_window_win(cavaWinModule)) {
		MessageBox(NULL, "RegisterClassEx - failed", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}
	
	// get window size etc..
	int screenWidth, screenHeight;
	GetDesktopResolution(&screenWidth, &screenHeight);
	// adjust window position etc...
	if(!strcmp(windowAlignment, "top")){
		windowX = (screenWidth - w) / 2 + windowX;
	}else if(!strcmp(windowAlignment, "bottom")){
		windowX = (screenWidth - w) / 2 + windowX;
		windowY = (screenHeight - h) + (-1*windowY);
	}else if(!strcmp(windowAlignment, "top_left")){
		// Nothing to do here :P
	}else if(!strcmp(windowAlignment, "top_right")){
		windowX = (screenHeight - w) + (-1*windowX);
	}else if(!strcmp(windowAlignment, "left")){
		windowY = (screenHeight - h) / 2;
	}else if(!strcmp(windowAlignment, "right")){
		windowX = (screenWidth - w) + (-1*windowX);
		windowY = (screenHeight - h) / 2 + windowY;
	}else if(!strcmp(windowAlignment, "bottom_left")){
		windowY = (screenHeight - h) + (-1*windowY);
	}else if(!strcmp(windowAlignment, "bottom_right")){
		windowX = (screenWidth - w) + (-1*windowX);
		windowY = (screenHeight - h) + (-1*windowY);
	}else if(!strcmp(windowAlignment, "center")){
		windowX = (screenWidth - w) / 2 + windowX;
		windowY = (screenHeight - h) / 2 + windowY;
	}
	// Some error checking
	#ifdef DEBUG
		if(windowX > screenWidth - w) printf("Warning: Screen out of bounds (X axis)!");
		if(windowY > screenHeight - h) printf("Warning: Screen out of bounds (Y axis)!");
	#endif
	
	// create window
	cavaWinWindow = CreateWindowEx(WS_EX_APPWINDOW, szAppName, wcWndName, WS_VISIBLE | WS_POPUP, windowX, windowY, w, h, NULL, NULL, cavaWinModule, NULL);
	if(cavaWinWindow == NULL) {
		MessageBox(NULL, "CreateWindowEx - failed", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}
	
	// enable blur?
	DWM_BLURBEHIND bb = {0};
	HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
	bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
	bb.hRgnBlur = hRgn;
	bb.fEnable = TRUE;
	DwmEnableBlurBehindWindow(cavaWinWindow, &bb);
	
	CreateHGLRC(cavaWinWindow);
	cavaWinFrame = GetDC(cavaWinWindow);
	wglMakeCurrent(cavaWinFrame, cavaWinGLFrame);	
	
	// process colors
	if(!strcmp(color, "default")) {
		// instead of messing around with average colors like on Xlib
		// we'll just get the accent color (which is way easier and an better thing to do)

		unsigned char opaque = 1;
		HRESULT error = DwmGetColorizationColor(&fgcolor, &opaque);
		if(!SUCCEEDED(error)) {
			MessageBox(NULL, "DwmGetColorizationColor - failed", "Error", MB_OK | MB_ICONERROR);
			return 1;
		}
	} else if(color[0] != '#') {
		switch(col)
		{
			case 0: fgcolor = 0x000000; break;
			case 1: fgcolor = 0xFF0000; break;
			case 2: fgcolor = 0x00FF00; break;
			case 3: fgcolor = 0xFFFF00; break;
			case 4: fgcolor = 0x0000FF; break;
			case 5: fgcolor = 0xFF00FF; break;
			case 6: fgcolor = 0x00FFFF; break;
			case 7: fgcolor = 0xFFFFFF; break;
			default: MessageBox(NULL, "Missing color.\nPlease send a bug report!", "Error", MB_OK | MB_ICONERROR); return 1;
		}
	} else if(color[0] == '#') {
		unsigned char red, green, blue;
		sscanf(color, "#%hhx%hhx%hhx", &red, &green, &blue);
		fgcolor = (red << 16) | (green << 8) + blue + (unsigned char)(foreground_opacity*255)<<24;
	}

	if(!strcmp(bcolor, "default")) {
		// instead of messing around with average colors like on Xlib
		// we'll just get the accent color (which is way easier and an better thing to do)

		unsigned char opaque = 1;
		HRESULT error = DwmGetColorizationColor(&bgcolor, &opaque);
		if(!SUCCEEDED(error)) {
			MessageBox(NULL, "DwmGetColorizationColor - failed", "Error", MB_OK | MB_ICONERROR);
			return 1;
		}
	} else if(bcolor[0] != '#') {
		switch(col)
		{
			case 0: bgcolor = 0x000000; break;
			case 1: bgcolor = 0xFF0000; break;
			case 2: bgcolor = 0x00FF00; break;
			case 3: bgcolor = 0xFFFF00; break;
			case 4: bgcolor = 0x0000FF; break;
			case 5: bgcolor = 0xFF00FF; break;
			case 6: bgcolor = 0x00FFFF; break;
			case 7: bgcolor = 0xFFFFFF; break;
			default: MessageBox(NULL, "Missing color.\nPlease send a bug report!", "Error", MB_OK | MB_ICONERROR); return 1;
		}
	} else if(bcolor[0] == '#') {
		unsigned char red, green, blue;
		sscanf(bcolor, "#%hhx%hhx%hhx", &red, &green, &blue);
		bgcolor = (red << 16) | (green << 8) | blue | (transparentFlag*255<<24);
	}

	// parse all the color values
	grad = gradient;
	gradientColor[0] = gradient_color_1;
	gradientColor[1] = gradient_color_2;
	shadow = shdw;
	shadowColor = shdw_col;
	opacity[0] = foreground_opacity;
	trans = transparentFlag;

	// set up opengl and stuff
	init_opengl_win();
	return 0;
}

void apply_win_settings(int w, int h) {
	resize_framebuffer(w, h);
	ReleaseDC(cavaWinWindow, cavaWinFrame);
	return;
}


int get_window_input_win(int *should_reload, int *bs, double *sens, int *bw, int *w, int *h, char *color, char *bcolor, int gradient) {
	while(!*should_reload && PeekMessage(&cavaWinEvent, NULL, WM_KEYFIRST, WM_MOUSELAST, PM_REMOVE)) {	
		TranslateMessage(&cavaWinEvent);
		DispatchMessage(&cavaWinEvent);	// windows handles the rest
		switch(cavaWinEvent.message) {
			case WM_KEYDOWN:
			       switch(cavaWinEvent.wParam) {
			       		// should_reload = 1
					// resizeTerminal = 2
					// bail = -1
				        case 'A':
						(*bs)++;
						return 2;
					case 'S':
						if((*bs) > 0) (*bs)--;
						return 2;
					case 'F': // fullscreen
						//fs = !fs;
						return 2;
					case VK_UP:
						(*sens) *= 1.05;
						break;
					case VK_DOWN:
						(*sens) *= 0.95;
						break;
					case VK_LEFT:
						(*bw)++;
						return 2;
					case VK_RIGHT:
						if ((*bw) > 1) (*bw)--;
						return 2;
					case 'R': //reload config
						(*should_reload) = 1;
						return 1;
					case 'Q':
						return -1;
					case VK_ESCAPE:
						return -1;
					case 'B':
						if(trans) break;
						srand(time(NULL));
						bgcolor = (rand() << 16) + rand();
						return 2;
					case 'C':
						if(grad) break;
						srand(time(NULL));
						fgcolor = (rand() << 16) + rand();
						return 2;
			       		default: break;
			       }
			       break;
			case WM_CLOSE:
			       	return -1;
			case WM_DESTROY:
				return -1;
			case WM_QUIT:
				return -1;
			case WM_SIZE:
			{
				RECT rect;
				if(GetWindowRect(cavaWinWindow, &rect)) {
					w = rect.right - rect.left;
					h = rect.bottom - rect.top;
				}
				return 2;
			}
		}
	}
	return 0;
}

void draw_graphical_win(int window_height, int bars_count, int bar_width, int bar_spacing, int rest, int gradient, int f[200], int flastd[200], double foreground_opacity) {
	HDC hdc = GetDC(cavaWinWindow);
        wglMakeCurrent(hdc, cavaWinGLFrame);

	// clear color and calculate pixel witdh in double
	double pixelWidthGL = 2.0/(double)(bars_count*(bar_width+bar_spacing)+rest*2.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		
	for(int i = 0; i < bars_count; i++) {
		double point[4];
		point[0] = (double)pixelWidthGL*(rest+(bar_width+bar_spacing)*i)-1.0;
		point[1] = (double)pixelWidthGL*(rest+(bar_width+bar_spacing)*i+bar_width)-1.0;
		point[2] = (double)f[i]/window_height*2.0-1.0;
		point[3] = (double)-1.0;
		if(shadow) {
			point[2] += 2.0/window_height*shadow;
			point[3] += 2.0/window_height*shadow;
		}
			
		if(grad)
		{
			glBegin(GL_POLYGON);
					glColor4ub(
			(gradientColor[0]>>16%256)+((gradientColor[1]>>16%256)-(gradientColor[0]>>16%256))*f[i]/window_height,
			(gradientColor[0]>>8%256)+((gradientColor[1]>>8%256)-(gradientColor[0]>>8%256))*f[i]/window_height,
			(gradientColor[0]%256)+((gradientColor[1]%256)-(gradientColor[0]%256))*f[i]/window_height,
			foreground_opacity*0xFF);
				glVertex2d(point[0], point[2]);
				glVertex2d(point[1], point[2]);
					
				glColor4ub(gradientColor[0]>>16%256, gradientColor[0]>>8%256,
					       	gradientColor[0]%256, foreground_opacity*0xFF);
				glVertex2d(point[1], point[3]);
				glVertex2d(point[0], point[3]);
			glEnd();
		} else {
			glColor4ub(fgcolor>>16%256, fgcolor>>8%256, fgcolor%256, (unsigned char)(foreground_opacity*255));
			glBegin(GL_POLYGON);
				glVertex2f(point[0], point[2]);
				glVertex2f(point[0], point[3]);
				glVertex2f(point[1], point[3]);
				glVertex2d(point[1], point[2]);
			glEnd();
		}
		if(shadow) {
			glBegin(GL_POLYGON);
				glColor4ub((unsigned char)(shadowColor >> 16 % 256), (unsigned char)(shadowColor >> 8 % 256), (unsigned char)(shadowColor % 256),(unsigned char)(shadowColor >> 24 % 256));
				glVertex2d(point[1], point[2]);
				glVertex2d(point[1], point[3]);
				
				glColor4ub(0, 0, 0, 0);
				glVertex2d(point[1]+2.0/window_height*shadow/4.0, point[3]-2.0/window_height*shadow);
				glVertex2d(point[1]+2.0/window_height*shadow/4.0, point[2]-2.0/window_height*shadow);
			glEnd();
				
			glBegin(GL_POLYGON);
				glColor4ub((unsigned char)(shadowColor >> 16 % 256), (unsigned char)(shadowColor >> 8 % 256), (unsigned char)(shadowColor % 256), (unsigned char)(shadowColor >> 24 % 256));
				glVertex2d(point[0],point[3]);
				glVertex2d(point[1],point[3]);
		
				glColor4ub(0, 0, 0, 0);
				glVertex2d(point[1]+2.0/window_height*shadow/4.0, point[3]-2.0/window_height*shadow);
				glVertex2d(point[0]+2.0/window_height*shadow/4.0, point[3]-2.0/window_height*shadow);
			glEnd();
		}
	}
	
	glFlush();

	// swap buffers	
	SwapBuffers(hdc);
	ReleaseDC(cavaWinWindow, hdc);
}

void cleanup_graphical_win(void) {
	wglMakeCurrent(NULL, NULL);
        wglDeleteContext(cavaWinGLFrame);
	ReleaseDC(cavaWinWindow, cavaWinFrame);
	DestroyWindow(cavaWinWindow);
	UnregisterClass(szAppName, cavaWinModule);	
	CloseHandle(cavaWinModule);
}