//--------------------------------------------------------------------------//
// iq / rgba  .  tiny codes  .  2008                                        //
//--------------------------------------------------------------------------//

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <GL/gl.h>
#include <math.h>
#include "glext.h"
#include <cstdlib>

#define XRES 1280
#define YRES 720
#define TEXSIZE 128

#include "shader_code.h"
#include "../4klanghalcy.h"

#include <MMSystem.h>
#include <MMReg.h>

#define USE_SOUND_THREAD
#define PERFECTSYNC 0

// MAX_SAMPLES gives you the number of samples for the whole song. we always produce stereo samples, so times 2 for the buffer
SAMPLE_TYPE	lpSoundBuffer[MAX_SAMPLES*2];  
HWAVEOUT	hWaveOut;

/////////////////////////////////////////////////////////////////////////////////
// initialized data
/////////////////////////////////////////////////////////////////////////////////

#pragma data_seg(".wavefmt")
WAVEFORMATEX WaveFMT =
{
#ifdef FLOAT_32BIT	
	WAVE_FORMAT_IEEE_FLOAT,
#else
	WAVE_FORMAT_PCM,
#endif		
    2, // channels
    SAMPLE_RATE, // samples per sec
    SAMPLE_RATE*sizeof(SAMPLE_TYPE)*2, // bytes per sec
    sizeof(SAMPLE_TYPE)*2, // block alignment;
    sizeof(SAMPLE_TYPE)*8, // bits per sample
    0 // extension not needed
};

#pragma data_seg(".wavehdr")
WAVEHDR WaveHDR = 
{
	(LPSTR)lpSoundBuffer, 
	MAX_SAMPLES*sizeof(SAMPLE_TYPE)*2,			// MAX_SAMPLES*sizeof(float)*2(stereo)
	0, 
	0, 
	0, 
	0, 
	0, 
	0
};

MMTIME MMTime = 
{ 
	TIME_SAMPLES,
	0
};

const static PIXELFORMATDESCRIPTOR pfd = {0,0,PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
typedef void (APIENTRYP PFNGLBINDIMAGETEXTUREEXTPROC) (GLuint index, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLint format);

static DEVMODE screenSettings = { 
    #if _MSC_VER < 1400
    {0},0,0,148,0,0x001c0000,{0},0,0,0,0,0,0,0,0,0,{0},0,32,XRES,YRES,0,0,      // Visual C++ 6.0
    #else
    {0},0,0,156,0,0x001c0000,{0},0,0,0,0,0,{0},0,32,XRES,YRES,{0}, 0,           // Visuatl Studio 2005
    #endif
    #if(WINVER >= 0x0400)
    0,0,0,0,0,0,
    #if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
    0,0
    #endif
    #endif
    };

//--------------------------------------------------------------------------//

//extern "C" int _fltused = 0;

float m[TEXSIZE*TEXSIZE*TEXSIZE*4*4];
char* texts[] = {
	"  hg ",
	"k2   ",
	" rno ",
	"m_s  ",
	" hjb ",
	"nce  ",
	"xayax",
	"SVatG",
	"SVatG",
	"SVatG",
	"SVatG"
};
float textv[128*128*128*4];

PFNGLTEXIMAGE3DPROC tex3d;

void entrypoint( void )
{ 
#if 1
    // full screen
    if( ChangeDisplaySettings(&screenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL) return; ShowCursor( 0 );

    // create windows
	HWND hWND = CreateWindow("edit", 0, WS_POPUP|WS_VISIBLE, 0, -100,1280,720,0,0,0,0);
    HDC hDC = GetDC( hWND );
#else
	HWND hWND = CreateWindow("edit", 0,  WS_CAPTION|WS_VISIBLE, 0, 0,1280,720,0,0,0,0);
    HDC hDC = GetDC( hWND );
#endif
    // init opengl
    SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
    wglMakeCurrent(hDC, wglCreateContext(hDC));

	// init data
	// init texture
	for(int i = 0; i < TEXSIZE*TEXSIZE*TEXSIZE*4; i++) {
		m[i*4] = 1.0;
		m[i*4+1] = i%44144 == 0 ? 1.0f : 0.0f;
		m[i*4+2] = 1.0f;
		m[i*4+3]  =i%44334 == 0 ? 1.0f : 0.0f;
    }

	// A function
	tex3d = (PFNGLTEXIMAGE3DPROC)wglGetProcAddress("glTexImage3D");

    // create shader
    const int p = ((PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram"))();
    const int s = ((PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader"))(GL_FRAGMENT_SHADER);	
    ((PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource"))(s, 1, &shader_frag, 0);
    ((PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader"))(s);
    ((PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader"))(p,s);
    ((PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram"))(p);
    ((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))(p);

	GLuint b[2];
    glGenTextures(2, b);
    glBindTexture(GL_TEXTURE_3D, b[0]);
    glEnable(GL_TEXTURE_3D);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    /*glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R, GL_REPEAT);*/
	(tex3d)(GL_TEXTURE_3D,0,GL_RGBA32F,TEXSIZE,TEXSIZE,TEXSIZE,0,GL_RGBA, GL_FLOAT,m);
    ((PFNGLBINDIMAGETEXTUREEXTPROC)wglGetProcAddress("glBindImageTextureEXT"))(0,b[0],0,1,0,GL_READ_WRITE,GL_RGBA32F);
	
	// Set font.
	HFONT hFont; 
	hFont = (HFONT)CreateFont(0,0,0,0,700,FALSE,0,0,0,0,0,0,0,"Segoe UI"); 
	SelectObject(hDC, hFont);

	// Prepare text
	for(int i = 0; i < 11; i++) {
		// Display the text string.
		for(int j = 0; j < 20; j++) {
			for(int k = 0; k < 20; k++) {
				TextOut(hDC, k*5, j*5, "     ", 5);
			}
		}
		TextOut(hDC, 0, 0, texts[i], 5);

		// Get pixels
		for(int x = 0; x < 127; x++) {
			for(int y = 0; y < 66; y++) {
				//for(int z = 0; z < 128; z++) {
						textv[(i*128*128+x+128*(128-y))*4+0] = GetPixel(hDC, x/3, y/3) == 0 ? 1.0f : 0.0f;
						//textv[(x+y*128+z*128*128)*4+i] = 0.0f;
					//}
				//}
			}
		}
	}

	// Texture data for text
	((PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture"))(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, b[1]);
	glEnable(GL_TEXTURE_3D);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_T, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_3D,GL_TEXTURE_WRAP_R, GL_REPEAT);
	(tex3d)(GL_TEXTURE_3D,0,GL_RGBA32F,128,128,128,0,GL_RGBA, GL_FLOAT,textv);

	MoveWindow(hWND, 0, 0, 1280, 720, 0);

	//ShowWindow(hWND, SW_MAXIMIZE);

	// Sound
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)_4klang_render, lpSoundBuffer, 0, 0);
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &WaveFMT, NULL, 0, CALLBACK_NULL );
	waveOutPrepareHeader(hWaveOut, &WaveHDR, sizeof(WaveHDR));
	waveOutWrite(hWaveOut, &WaveHDR, sizeof(WaveHDR));	

    // run
	unsigned int ls = 0;
	int last_switch = 0;
	unsigned int text_counter = 0;
	unsigned int text_counter_2 = 0;
	int switch_text = 0;
    do
    {
		// get sample position for timing
		waveOutGetPosition(hWaveOut, &MMTime, sizeof(MMTIME));
#if 0
		MSG msg;
        while( PeekMessage(&msg,0,0,0,PM_REMOVE) )
        {
		    TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
#endif

		//if(MMTime.u.sample - last_switch > 332688) {
		//if(MMTime.u.sample - last_switch > 339720) {
#if 0
		if(((long)MMTime.u.sample - 21168/2 - (long)text_counter_2 * (21168*2) >= 0)) {
#else
		if(((long)MMTime.u.sample - 21168 - (long)text_counter_2 * (21168*2) >= 0)) {
#endif
			switch_text = MAXUINT;
			text_counter_2++;
		}

#if 0
		if(((long)MMTime.u.sample + 21168/2 - (long)text_counter * (21168*2) >= 0)) {
#else
		if(((long)MMTime.u.sample - (long)text_counter * (21168*2) >= 0)) {
#endif
		//if((int)MMTime.u.sample/SAMPLES_PER_TICK - 32 * text_counter > 0) {
			/*glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, m);
			int py = (int)((sin((float)MMTime.u.sample)+1.0) * 999.0) % 128;
			int pl = (int)((cos((float)MMTime.u.sample)+1.0) * 566.0) % 110;
			for(int l = pl; l < pl+15; l++) {
				for(int x = 0; x < 128; x++) {
					for(int y = 0; y < 128; y++) {
						if(text_counter&1) {
							m[(x+y*128+l*128*128)*4+1] = textv[text_counter][x+(128-((y+py)%128))*128];
						}
						else {
							m[(l+y*128+x*128*128)*4+3] = textv[text_counter][x+(128-((y+py)%128))*128];
						}
					}
				}
			}
			(tex3d)(GL_TEXTURE_3D,0,GL_RGBA32F,TEXSIZE,TEXSIZE,TEXSIZE,0,GL_RGBA, GL_FLOAT,m);*/
			//last_switch = MMTime.u.sample;
			text_counter++;
		}
        glColor4ui(MMTime.u.sample, MMTime.u.sample - ls,(MAXUINT/2048) * text_counter, switch_text);
		if(switch_text <= MAXUINT/5) {
			switch_text = 0;
		}
		else {
			switch_text -= (MAXUINT/5) * (int)(1700.0f / (float)(MMTime.u.sample - ls));
		}
        glRects(-1,-1,1,1);
        SwapBuffers(hDC);
		ls = MMTime.u.sample;
		PeekMessageA(0, 0, 0, 0, PM_REMOVE); 
    } while (MMTime.u.sample < 21168*265 && !GetAsyncKeyState(VK_ESCAPE) );

    ExitProcess( 0 );
}
