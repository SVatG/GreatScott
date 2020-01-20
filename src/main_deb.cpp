//--------------------------------------------------------------------------//
// iq / rgba  .  tiny codes  .  2008                                        //
//--------------------------------------------------------------------------//

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <math.h>
#include "glext.h"

#define XRES    1280
#define YRES    720

#define TEXSIZE 128

#include "shader_code.h"

const static PIXELFORMATDESCRIPTOR pfd = {0,0,PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// ADDED: ADDITIONAL FUNCTION
/*GLAPI void APIENTRY glShaderSource (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);*/
typedef void (APIENTRYP PFNGLBINDIMAGETEXTUREEXTPROC) (GLuint index, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLint format);

const static char *funcNames[] = { "glCreateProgram",
                                   "glCreateShader",
                                   "glShaderSource",
                                   "glCompileShader",
                                   "glAttachShader",
                                   "glLinkProgram",
                                   "glUseProgram",
                                   "glBindImageTextureEXT",
                                   "glTexImage3D"
};

static void *funcPtrs[9];

#define oglCreateProgram          ((PFNGLCREATEPROGRAMPROC)funcPtrs[0])
#define oglCreateShader           ((PFNGLCREATESHADERPROC)funcPtrs[1])
#define oglShaderSource           ((PFNGLSHADERSOURCEPROC)funcPtrs[2])
#define oglCompileShader          ((PFNGLCOMPILESHADERPROC)funcPtrs[3])
#define oglAttachShader           ((PFNGLATTACHSHADERPROC)funcPtrs[4])
#define oglLinkProgram            ((PFNGLLINKPROGRAMPROC)funcPtrs[5])
#define oglUseProgram             ((PFNGLUSEPROGRAMPROC)funcPtrs[6])
#define oglBindImageTextureExt    ((PFNGLBINDIMAGETEXTUREEXTPROC)funcPtrs[7])
#define oglTexImage3D             ((PFNGLTEXIMAGE3DPROC)funcPtrs[8])
// #define oglTexParameterIiv          ((PFNGLTEXPARAMETERIIVPROC)funcPtrs[9]);

static RECT rec = { 0, 0, XRES, YRES };

//--------------------------------------------------------------------------//
static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// salvapantallas
	if( uMsg==WM_SYSCOMMAND && (wParam==SC_SCREENSAVE || wParam==SC_MONITORPOWER) )
		return( 0 );

	// boton x o pulsacion de escape
	if( uMsg==WM_CLOSE || uMsg==WM_DESTROY || (uMsg==WM_KEYDOWN && wParam==VK_ESCAPE) )
		{
		PostQuitMessage(0);
        return( 0 );
		}

    if( uMsg==WM_CHAR )
        {
        if( wParam==VK_ESCAPE )
            {
            PostQuitMessage(0);
            return( 0 );
            }
        }

    return( DefWindowProc(hWnd,uMsg,wParam,lParam) );
}


#define RANDTEX ((rand()%(TEXSIZE-2))+1)
#define CTEX(x) ((((x)<1?1:((x))%(TEXSIZE-1))))
extern double noise_at( float x, float y, float z );

char* texts[] = {
	"SVatG",
	"SVatG",
	"SVatG",
	"SVatG",
	"SVatG"
};
float textv[100][128*128];

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{ 
	int done = 0;

    WNDCLASS		wc;

    // create window
    ZeroMemory( &wc, sizeof(WNDCLASS) );
    wc.style         = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hPrevInstance;
    wc.lpszClassName = "iq";

    if( !RegisterClass(&wc) )
        return( 0 );

    const int dws = WS_VISIBLE | WS_CAPTION | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU;
    AdjustWindowRect( &rec, dws, 0 );
    HWND hWnd = CreateWindowEx( WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, wc.lpszClassName, "blah", dws,
                               (GetSystemMetrics(SM_CXSCREEN)-rec.right+rec.left)>>1,
                               (GetSystemMetrics(SM_CYSCREEN)-rec.bottom+rec.top)>>1,
                               rec.right-rec.left, rec.bottom-rec.top, 0, 0, hPrevInstance, 0 );
    if( !hWnd )
        return( 0 );

    HDC hDC=GetDC(hWnd);
    if( !hDC )
        return( 0 );

    // init opengl
    int pf = ChoosePixelFormat(hDC,&pfd);
    if( !pf )
        return( 0 );
    
    if( !SetPixelFormat(hDC,pf,&pfd) )
        return( 0 );

    HGLRC hRC = wglCreateContext(hDC);
    if( !hRC )
        return( 0 );

    if( !wglMakeCurrent(hDC,hRC) )
        return( 0 );

    const char *GLVersionString = (const char*)glGetString(GL_VERSION);
    printf(GLVersionString);

    // init extensions
    for( int i=0; i<(sizeof(funcPtrs)/sizeof(void*)); i++ )
    {
        funcPtrs[i] = wglGetProcAddress( funcNames[i] );
        if( !funcPtrs[i] )
            return 0;
    }

    // create shader
    int p = oglCreateProgram();
    int f = oglCreateShader(GL_FRAGMENT_SHADER);	
    oglShaderSource(f, 1, &shader_frag, 0);
    oglCompileShader(f);
    oglAttachShader(p,f);
    oglLinkProgram(p);
    oglUseProgram(p);
    GLuint buffertex;
    glGenTextures(1, &buffertex);
    glBindTexture(GL_TEXTURE_3D, buffertex);
    glEnable(GL_TEXTURE_3D);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    /*glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R, GL_REPEAT);*/
    //oglTexImage3D(GL_TEXTURE_3D,0,GL_RGBA32F,TEXSIZE,TEXSIZE,TEXSIZE,0,GL_RGBA, GL_FLOAT,malloc(TEXSIZE*TEXSIZE*TEXSIZE*32*4));


	// Set font.
	HFONT hFont; 
	hFont = (HFONT)CreateFont(0,0,0,0,700,FALSE,0,0,0,0,0,0,0,"Segoe UI"); 
	SelectObject(hDC, hFont);

	// Prepare text
	for(int i = 0; i < 5; i++) {
		// Display the text string.  
		TextOut(hDC, 0, 0, texts[i], 5);

		// Get pixels
		for(int x = 0; x < 128; x++) {
			for(int y = 0; y < 128; y++) {
				textv[i][x+128*y] = GetPixel(hDC, x/3, y/3) == 0 ? 1.0f : 0.0f;
			}
		}
	}

	// Init data
    float* testbuf = (float*)malloc(TEXSIZE*TEXSIZE*TEXSIZE*4*4*4);
    for(int i = 0; i < TEXSIZE*TEXSIZE*TEXSIZE*4; i++) {
		//testbuf[i] = ((float)rand())/((float)RAND_MAX);
		//testbuf[i] = 0.0f;
		testbuf[i*4] = 1.0f;
		testbuf[i*4+1] = 0.0f;
		testbuf[i*4+2] = 1.0f;
		testbuf[i*4+3] = 0.0f;
    }
	/*for(int i = 0; i < 10; i++) {
		int xp = rand() % 128;
		int yp = rand() % 128;
		int zp = rand() % 128;
		for(int x = 0; x < TEXSIZE; x++) {
		  for(int y = 0; y < TEXSIZE; y++) {
			for(int z = 0; z < TEXSIZE; z++) {
				int xd = x - xp;
				int yd = y - yp;
				int zd = z - zp;
				int dd = (xd*xd+yd*yd+zd*zd);
				if(dd < 5) {
					testbuf[(x+y*TEXSIZE+z*TEXSIZE*TEXSIZE)*4+1] = 1.0;
				}
			}
		  }
		}
	}*/

    oglTexImage3D(GL_TEXTURE_3D,0,GL_RGBA32F,TEXSIZE,TEXSIZE,TEXSIZE,0,GL_RGBA, GL_FLOAT,testbuf);
    oglBindImageTextureExt(0,buffertex,0,1,0,GL_READ_WRITE,GL_RGBA32F);
    ShowCursor(0); 

    int startTickCount = GetTickCount();
	int fc = 0;
	int tc = 0;
	int tp = 0;
    while( !done )
    {
        const float t = .0025f*(float)(GetTickCount()&65535);
		fc++;
		if(fc % 300 == 0) {
			glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, testbuf);
			for(int l = 10; l < 15; l++) {
				for(int x = 0; x < 128; x++) {
					for(int y = 0; y < 128; y++) {
						if(tc&1) {
							testbuf[(x+y*128+l*128*128)*4+1] = textv[tc][x+(128-((y+tp)%128))*128];
						}
						else {
							testbuf[(l+y*128+x*128*128)*4+3] = textv[tc][x+(128-((y+tp)%128))*128];
						}
					}
				}
			}
			oglTexImage3D(GL_TEXTURE_3D,0,GL_RGBA32F,TEXSIZE,TEXSIZE,TEXSIZE,0,GL_RGBA, GL_FLOAT,testbuf);
			tc++;
			tp += 64;
		}
        MSG msg;
        while( PeekMessage(&msg,0,0,0,PM_REMOVE) )
        {
            if( msg.message==WM_QUIT ) done=1;
		    TranslateMessage( &msg );
            DispatchMessage( &msg );
        }

        //glColor3f(t,sinf(.25f*t),0.0f);
        glColor3us((unsigned short)(GetTickCount() - startTickCount),0,0);
        glRects(-1,-1,1,1);
        SwapBuffers( hDC );
    }

    ExitProcess( 0 );

    return 0;
}
