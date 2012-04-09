#ifndef DDRAWLIB_H_
#define DDRAWLIB_H_

// MACROS /////////////////////////////////////////////////
// these read the keyboard asynchronously
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// this builds a 32 bit color value in A.8.8.8 format (8-bit alpha mode)
#define _RGB32BIT(a,r,g,b) ((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))

//использу€ макрос ниже получаетс€ красивый эффект
//#define _RGB32BITGRAD(from,to,koef) (from + (to-from)*koef)
//#define _RGB32BITGRAD(from,to,koef) ((from & 0x000000FF) + ((to  & 0x000000FF) - (from  & 0x000000FF))*koef)

// initializes a direct draw struct, basically zeros it and sets the dwSize field
#define DDRAW_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

// TYPES //////////////////////////////////////////////////

// PROTOTYPES /////////////////////////////////////////////


// DirectDraw functions
int DDraw_Init(int width, int height);
int DDraw_Shutdown(void);

LPDIRECTDRAWSURFACE7 DDraw_Create_Surface(int width, int height, int mem_flags=0, DWORD color_key_value=0);
int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds, DWORD color, RECT *client=NULL);
LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds, int num_rects, LPRECT clip_list);

UCHAR *DDraw_Lock_Back_Surface(void);
int DDraw_Unlock_Back_Surface(void);
UCHAR *DDraw_Lock_Surface(LPDIRECTDRAWSURFACE7 lpdds,int *lpitch);
int DDraw_Unlock_Surface(LPDIRECTDRAWSURFACE7 lpdds);

int DDraw_Flip(void);

// gdi functions
int Draw_Text_GDI(char *text, int x,int y,COLORREF color, LPDIRECTDRAWSURFACE7 lpdds);

// graphics functions
int Draw_HLine(int x0, int y0, int x1, int y1, DWORD color,UCHAR *dest_buffer,int lpitch);
int Draw_HLine(float x0f, float y0f, float x1f, float y1f, DWORD color,UCHAR *dest_buffer,int lpitch);
int Draw_HLine(double x0d, double y0d, double x1d, double y1d, DWORD color,UCHAR *dest_buffer,int lpitch);

int Draw_Line(int x0, int y0, int x1, int y1, DWORD color,UCHAR *dest_buffer,int lpitch);
int Draw_Line(float x0f, float y0f, float x1f, float y1f, DWORD color,UCHAR *dest_buffer,int lpitch);
int Draw_Line(double x0d, double y0d, double x1d, double y1d, DWORD color,UCHAR *dest_buffer,int lpitch);

int Draw_WuLine(int x0, int y0, int x1, int y1, DWORD color,UCHAR *dest_buffer,int lpitch);
int Draw_WuLine(float x0f, float y0f, float x1f, float y1f, DWORD color,UCHAR *dest_buffer,int lpitch);
int Draw_WuLine(double x0d, double y0d, double x1d, double y1d, DWORD color,UCHAR *dest_buffer,int lpitch);

int Draw_Circle (int xc, int yc, int radius, DWORD color,UCHAR *dest_buffer,int lpitch);
int Draw_Circle (float xcf, float ycf, float radiusf, DWORD color,UCHAR *dest_buffer,int lpitch);
int Draw_Circle (double xcd, double ycd, double radiusd, DWORD color,UCHAR *dest_buffer,int lpitch);

int Draw_Fill_Circle (int xc, int yc, int radius, DWORD color,UCHAR *dest_buffer,int lpitch);
int Draw_Fill_Circle (float xcf, float ycf, float radiusf, DWORD color,UCHAR *dest_buffer,int lpitch);
int Draw_Fill_Circle (double xcd, double ycd, double radiusd, DWORD color,UCHAR *dest_buffer,int lpitch);

int Draw_Gradient_Circle (int xc, int yc, int radius, DWORD fromColor, DWORD toColor, UCHAR *dest_buffer,int lpitch);
int Draw_Gradient_Circle (float xcf, float ycf, float radiusf, DWORD fromColor, DWORD toColor, UCHAR *dest_buffer,int lpitch);
int Draw_Gradient_Circle (double xcd, double ycd, double radiusd, DWORD fromColor, DWORD toColor, UCHAR *dest_buffer,int lpitch);

// general utility functions
DWORD Get_Clock(void);
DWORD Start_Clock(void);
DWORD GetGradientColor(DWORD fromColor, DWORD toColor, float koef);

// GLOBALS ////////////////////////////////////////////////

// notice that interface 4.0 is used on a number of interfaces
extern LPDIRECTDRAW7        lpdd;
extern LPDIRECTDRAWSURFACE7 lpddsback;            // dd back surface
extern LPDIRECTDRAWSURFACE7 lpddsball;
extern DDSURFACEDESC2       ddsd;

extern UCHAR                *back_buffer;         // secondary back buffer
extern int                  back_lpitch;          // memory line pitch

extern int defBckColor;

extern int window_client_x0;   // used to track the starting (x,y) client area for
extern int window_client_y0;   // for windowed mode directdraw operations

#endif /* DDRAWLIB_H_ */
