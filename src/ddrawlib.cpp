// INCLUDES ///////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN

#include <windows.h>   // include important windows stuff
#include <windowsx.h>
#include <math.h>

#include <ddraw.h>    // directX includes
#include "ddrawlib.h"
// EXTERNALS /////////////////////////////////////////////

extern HWND main_window_handle; // save the window handle
extern HINSTANCE main_instance; // save the instance

// GLOBALS ////////////////////////////////////////////////
DWORD                start_clock_count = 0;     // used for timing

// notice that interface 7.0 is used on a number of interfaces
LPDIRECTDRAW7        lpdd         = NULL;  // dd object
LPDIRECTDRAWSURFACE7 lpddsprimary = NULL;  // dd primary surface
LPDIRECTDRAWSURFACE7 lpddsback    = NULL;  // dd back surface
LPDIRECTDRAWSURFACE7 lpddsball    = NULL;  // общая внеэкр поверхность
LPDIRECTDRAWCLIPPER  lpddclipper  = NULL;   // dd clipper for back surface
LPDIRECTDRAWCLIPPER  lpddclipperwin = NULL; // dd clipper for window

DDSURFACEDESC2       ddsd;                 // a direct draw surface description struct
UCHAR                *back_buffer    = NULL; // secondary back buffer
int                  back_lpitch     = 0;    // memory line pitch for back buffer

int dd_pixel_format;  // default pixel format

// these are overwritten globally by DDraw_Init()
int screen_width,            // width of screen
    screen_height;           // height of screen

int window_client_x0   = 0;   // used to track the starting (x,y) client area for
int window_client_y0   = 0;   // for windowed mode directdraw operations

//////////////////////////////////////////////////////////
int Draw_Gradient_Circle (int xc, int yc, int radius, COLORREF fromColor, COLORREF toColor, UCHAR *dest_buffer,int lpitch){

	int xs, xe;
	int ymin = yc-radius;
	int ymax = yc;
	int delta;

	if(radius <0) return 1;

	if(radius == 0) {
		Draw_HLine (xc, yc, xc, yc, toColor,dest_buffer,lpitch);
		return 0;
	}

	int lpitch_4 = lpitch >> 2; // lpitch_4 - pixels per screen line

	DWORD *dest_buffer4 = (DWORD *)dest_buffer;

	int RfromColor = (fromColor >> 16) & 0x000000FF;
	int GfromColor = (fromColor >> 8) & 0x000000FF;
	int BfromColor = fromColor & 0x000000FF;

	int RtoColor = (toColor >> 16) & 0x000000FF;
	int GtoColor = (toColor >> 8) & 0x000000FF;
	int BtoColor = toColor & 0x000000FF;

	int deltaR = RtoColor - RfromColor;
	int deltaG = GtoColor - GfromColor;
	int deltaB = BtoColor - BfromColor;


	for (int yscan=ymin; yscan<=ymax; ++yscan){
		xs=int(xc-sqrt((radius*radius)-((yscan-(yc-0.5))*(yscan-(yc-0.5))))+0.5);
        xe=int(xc+sqrt((radius*radius)-((yscan-(yc-0.5))*(yscan-(yc-0.5))))+0.5);

        delta = (xs-xc)*(xs-xc) + (yscan-yc)*(yscan-yc) - (radius*radius);
        if(delta >=radius){--xe; ++xs;}

        for (int x = xs; x<=xe; ++x){
        	//расчитать расстояние до центра круга
        	float dist = sqrt((x-xc)*(x-xc) + (yscan-yc)*(yscan-yc));
        	int Rcolor = abs((int)(RfromColor + deltaR*dist/radius + 0.5));
        	int Gcolor = abs((int)(GfromColor + deltaG*dist/radius + 0.5));
        	int Bcolor = abs((int)(BfromColor + deltaB*dist/radius + 0.5));
        	int color = (Rcolor << 16) + (Gcolor << 8) + Bcolor;
        	dest_buffer4[x+yscan*lpitch_4] = color;
        }

        for (int x = xs; x<=xe; ++x){
        	//расчитать расстояние до центра круга
        	float dist = sqrt((x-xc)*(x-xc) + (yscan-yc)*(yscan-yc));
        	int Rcolor = abs((int)(RfromColor + deltaR*dist/radius + 0.5));
        	int Gcolor = abs((int)(GfromColor + deltaG*dist/radius + 0.5));
        	int Bcolor = abs((int)(BfromColor + deltaB*dist/radius + 0.5));
        	int color = (Rcolor << 16) + (Gcolor << 8) + Bcolor;
        	dest_buffer4[x+(yc+(yc-yscan))*lpitch_4] = color;
        }

		//Draw_HLine (xs, yscan, xe, yscan, fromColor,dest_buffer,lpitch);
		//Draw_HLine (xs, yc+(yc-yscan), xe, yc+(yc-yscan), fromColor,dest_buffer,lpitch);
	}

	return 0;
}
///////////////////////////////////////////////////////////
int Draw_Fill_Circle (int xc, int yc, int radius, COLORREF color,UCHAR *dest_buffer,int lpitch){

	int xs, xe;
	int ymin = yc-radius;
	int ymax = yc;
	int delta;

	if(radius <0) return 1;

	if(radius == 0) {
		Draw_HLine (xc, yc, xc, yc, color,dest_buffer,lpitch);
		return 0;
	}

	for (int yscan=ymin; yscan<=ymax; ++yscan){
		xs=int(xc-sqrt((radius*radius)-((yscan-(yc-0.5))*(yscan-(yc-0.5))))+0.5);
        xe=int(xc+sqrt((radius*radius)-((yscan-(yc-0.5))*(yscan-(yc-0.5))))+0.5);

        delta = (xs-xc)*(xs-xc) + (yscan-yc)*(yscan-yc) - (radius*radius);
        if(delta >=radius){--xe; ++xs;}

		Draw_HLine (xs, yscan, xe, yscan, color,dest_buffer,lpitch);

		//отразить симметрично гориз оси
		Draw_HLine (xs, yc+(yc-yscan), xe, yc+(yc-yscan), color,dest_buffer,lpitch);
	}

	return 0;
}
//////////////////////////////////////////////////////////
int Draw_Fill_Circle (float xcf, float ycf, float radiusf, COLORREF color,UCHAR *dest_buffer,int lpitch){
	int xc = (int)(xcf+0.5);
	int yc = (int)(ycf+0.5);
	int radius = (int)(radiusf+0.5);

	Draw_Fill_Circle (xc, yc, radius, color, dest_buffer, lpitch);

	return 0;
}

//////////////////////////////////////////////////////////
int Draw_Fill_Circle (double xcd, double ycd, double radiusd, COLORREF color,UCHAR *dest_buffer,int lpitch){
	int xc = (int)(xcd+0.5);
	int yc = (int)(ycd+0.5);
	int radius = (int)(radiusd+0.5);

	Draw_Fill_Circle (xc, yc, radius, color, dest_buffer, lpitch);

	return 0;
}

//////////////////////////////////////////////////////////
int Draw_Circle (int xc, int yc, int radius, COLORREF color,UCHAR *dest_buffer,int lpitch){

	if (radius < 0) return 0;

	int lpitch_4 = lpitch >> 2; // lpitch_4 - pixels per screen line

	DWORD *dest_buffer4 = (DWORD *)dest_buffer;

	int x = 0;
	int y = radius;
	int d = 1 - radius;
	int delta1 = 3;
	int delta2 = -2*radius+5;

	//начальные точки
	dest_buffer4[(xc + x)+(yc + y)*lpitch_4] = color;
	dest_buffer4[(xc + y)+(yc + x)*lpitch_4] = color;
	dest_buffer4[(xc + y)+(yc - x)*lpitch_4] = color;
	dest_buffer4[(xc + x)+(yc - y)*lpitch_4] = color;
	dest_buffer4[(xc - x)+(yc - y)*lpitch_4] = color;
	dest_buffer4[(xc - y)+(yc - x)*lpitch_4] = color;
	dest_buffer4[(xc - y)+(yc + x)*lpitch_4] = color;
	dest_buffer4[(xc - x)+(yc + y)*lpitch_4] = color;

	while(y>x){
		if(d<0){
			d+=delta1;
			delta1+=2;
			delta2+=2;
			++x;
		}else{
			d+=delta2;
			delta1 +=2;
			delta2+=4;
			++x;
			--y;
		}
		dest_buffer4[(xc + x)+(yc + y)*lpitch_4] = color;
		dest_buffer4[(xc + y)+(yc + x)*lpitch_4] = color;
		dest_buffer4[(xc + y)+(yc - x)*lpitch_4] = color;
		dest_buffer4[(xc + x)+(yc - y)*lpitch_4] = color;
		dest_buffer4[(xc - x)+(yc - y)*lpitch_4] = color;
		dest_buffer4[(xc - y)+(yc - x)*lpitch_4] = color;
		dest_buffer4[(xc - y)+(yc + x)*lpitch_4] = color;
		dest_buffer4[(xc - x)+(yc + y)*lpitch_4] = color;
	}
	return 0;
}
//////////////////////////////////////////////////////////
int Draw_Circle (float xcf, float ycf, float radiusf, COLORREF color,UCHAR *dest_buffer,int lpitch){

	int xc = (int)(xcf+0.5);
	int yc = (int)(ycf+0.5);
	int radius = (int)(radiusf+0.5);

	Draw_Circle (xc, yc, radius, color, dest_buffer, lpitch);

	return 0;
}
/////////////////////////////////////////////////////////
int Draw_Circle (double xcd, double ycd, double radiusd, COLORREF color,UCHAR *dest_buffer,int lpitch){
	int xc = (int)(xcd+0.5);
	int yc = (int)(ycd+0.5);
	int radius = (int)(radiusd+0.5);

	Draw_Circle (xc, yc, radius, color, dest_buffer, lpitch);

	return 0;
}

//////////////////////////////////////////////////////////
UCHAR *DDraw_Lock_Surface(LPDIRECTDRAWSURFACE7 lpdds, int *lpitch)
{
	// this function locks the sent surface and returns a pointer to it

	// is this surface valid
	if (!lpdds)
		return(NULL);

	// lock the surface
	DDRAW_INIT_STRUCT(ddsd);
	lpdds->Lock(NULL,&ddsd,DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,NULL);

	// set the memory pitch
	if (lpitch)
		*lpitch = ddsd.lPitch;

	// return pointer to surface
	return((UCHAR *)ddsd.lpSurface);
} // end DDraw_Lock_Surface

///////////////////////////////////////////////////////////

int DDraw_Unlock_Surface(LPDIRECTDRAWSURFACE7 lpdds)
{
// this unlocks a general surface

// is this surface valid
if (!lpdds)
   return(0);

// unlock the surface memory
lpdds->Unlock(NULL);

// return success
return(1);
} // end DDraw_Unlock_Surface

///////////////////////////////////////////////////////////
int Draw_HLine(int x0, int y0, // starting position
                int x1, int y1, // ending position
                COLORREF color,     // color index
                UCHAR *dest_buffer, int lpitch) // video buffer and memory pitch
{
// this function draws a horizontal line from xo,yo to x1,y1 using differential error
// terms (based on Bresenahams work)
	if(y1 != y0) return (1);

	int lpitch_4 = lpitch >> 2; // lpitch_4 - pixels per screen line

	// pre-compute first pixel address in video buffer based on 16bit data
	DWORD *dest_buffer4 = (DWORD *)dest_buffer + x0 + y0*lpitch_4;


	int dx,             // difference in x's
    	x_inc,          // amount in pixel space to move during drawing
    	index;          // used for looping



	// compute horizontal and vertical deltas
	dx = x1-x0;

	// test which direction the line is going in i.e. slope angle
	if (dx>=0)   {
		x_inc = 1;
	} // end if line is moving right
	else{
		x_inc = -1;
		dx    = -dx;  // need absolute value
	} // end else moving left


   // draw the line
   for (index=0; index <= dx; ++index)
   {
       // set the pixel
       *dest_buffer4 = (DWORD)color;

       // move to the next pixel
       dest_buffer4+=x_inc;

   } // end for

   // return success
   return(0);
} // end Draw_HLine

///////////////////////////////////////////////////////////
int Draw_HLine(	float x0f, float y0f, // starting position
				float x1f, float y1f, // ending position
                COLORREF color,     // color index
                UCHAR *dest_buffer, int lpitch) // video buffer and memory pitch
{
	int x0 = (int)(x0f+0.5);
	int y0 = (int)(y0f+0.5);
	int x1 = (int)(x1f+0.5);
	int y1 = (int)(y1f+0.5);

	Draw_HLine(x0,y0,x1,y1,color,dest_buffer, lpitch);

	// return success
	return(0);
} // end Draw_HLine
///////////////////////////////////////////////////////////
int Draw_HLine(	double x0d, double y0d, // starting position
				double x1d, double y1d, // ending position
                COLORREF color,     // color index
                UCHAR *dest_buffer, int lpitch) // video buffer and memory pitch
{
	int x0 = (int)(x0d+0.5);
	int y0 = (int)(y0d+0.5);
	int x1 = (int)(x1d+0.5);
	int y1 = (int)(y1d+0.5);

	Draw_HLine(x0,y0,x1,y1,color,dest_buffer, lpitch);
	// return success
	return(0);
} // end Draw_HLine
///////////////////////////////////////////////////////////
int Draw_Line(int x0, int y0, // starting position
                int x1, int y1, // ending position
                COLORREF color,     // color index
                UCHAR *dest_buffer, int lpitch) // video buffer and memory pitch
{
// this function draws a line from xo,yo to x1,y1 using differential error
// terms (based on Bresenahams work)

int dx,             // difference in x's
    dy,             // difference in y's
    dx2,            // dx,dy * 2
    dy2,
    x_inc,          // amount in pixel space to move during drawing
    y_inc,          // amount in pixel space to move during drawing
    error,          // the discriminant i.e. error i.e. decision variable
    index;          // used for looping

int lpitch_4 = lpitch >> 2; // lpitch_4 - pixels per screen line

// pre-compute first pixel address in video buffer based on 16bit data
DWORD *dest_buffer4 = (DWORD *)dest_buffer + x0 + y0*lpitch_4;

// compute horizontal and vertical deltas
dx = x1-x0;
dy = y1-y0;

// test which direction the line is going in i.e. slope angle
if (dx>=0)
   {
   x_inc = 1;

   } // end if line is moving right
else
   {
   x_inc = -1;
   dx    = -dx;  // need absolute value

   } // end else moving left

// test y component of slope

if (dy>=0)
   {
   y_inc = lpitch_4;
   } // end if line is moving down
else
   {
   y_inc = -lpitch_4;
   dy    = -dy;  // need absolute value

   } // end else moving up

// compute (dx,dy) * 2
dx2 = dx << 1;
dy2 = dy << 1;

// now based on which delta is greater we can draw the line
if (dx > dy)
   {
   // initialize error term
   error = dy2 - dx;

   // draw the line
   for (index=0; index <= dx; index++)
       {
       // set the pixel
       *dest_buffer4 = (DWORD)color;

       // test if error has overflowed
       if (error >= 0)
          {
          error-=dx2;

          // move to next line
          dest_buffer4+=y_inc;

	   } // end if error overflowed

       // adjust the error term
       error+=dy2;

       // move to the next pixel
       dest_buffer4+=x_inc;

       } // end for

   } // end if |slope| <= 1
else
   {
   // initialize error term
   error = dx2 - dy;

   // draw the line
   for (index=0; index <= dy; index++)
       {
       // set the pixel
       *dest_buffer4 = (DWORD)color;

       // test if error overflowed
       if (error >= 0)
          {
          error-=dy2;

          // move to next line
          dest_buffer4+=x_inc;

          } // end if error overflowed

       // adjust the error term
       error+=dx2;

       // move to the next pixel
       dest_buffer4+=y_inc;

       } // end for

   } // end else |slope| > 1

// return success
return(0);

} // end Draw_Line

int Draw_Line(	float x0f, float y0f, // starting position
				float x1f, float y1f, // ending position
                COLORREF color,     // color index
                UCHAR *dest_buffer, int lpitch) // video buffer and memory pitch
{
	int x0 = (int)(x0f+0.5);
	int y0 = (int)(y0f+0.5);
	int x1 = (int)(x1f+0.5);
	int y1 = (int)(y1f+0.5);

	Draw_Line(x0,y0,x1,y1,color,dest_buffer, lpitch);

	// return success
	return(0);
} // end Draw_Line

int Draw_Line(	double x0d, double y0d, // starting position
				double x1d, double y1d, // ending position
                COLORREF color,     // color index
                UCHAR *dest_buffer, int lpitch) // video buffer and memory pitch
{
	int x0 = (int)(x0d+0.5);
	int y0 = (int)(y0d+0.5);
	int x1 = (int)(x1d+0.5);
	int y1 = (int)(y1d+0.5);

	Draw_Line(x0,y0,x1,y1,color,dest_buffer, lpitch);
	// return success
	return(0);
} // end Draw_Line

int Draw_Text_GDI(char *text, int x,int y,COLORREF color, LPDIRECTDRAWSURFACE7 lpdds)
{
	// this function draws the sent text on the sent surface
	// using color index as the color in the palette

	HDC xdc; // the working dc

	// get the dc from surface
	if (FAILED(lpdds->GetDC(&xdc)))
		return(0);

	// set the colors for the text up
	SetTextColor(xdc,color);

	// set background mode to transparent so black isn't copied
	SetBkMode(xdc, TRANSPARENT);

	// draw the text a
	TextOut(xdc,x,y,text,strlen(text));

	// release the dc
	lpdds->ReleaseDC(xdc);

	// return success
	return(1);
} // end Draw_Text_GDI

///////////////////////////////////////////////////////////

int DDraw_Flip(void)
{
// this function flip the primary surface with the secondary surface

// test if either of the buffers are locked
//if (primary_buffer || back_buffer)
	if (back_buffer)
		return(0);

	// flip pages
	RECT    dest_rect;    // used to compute destination rectangle

	// get the window's rectangle in screen coordinates
	GetWindowRect(main_window_handle, &dest_rect);

	// compute the destination rectangle
	dest_rect.left   +=window_client_x0;
	dest_rect.top    +=window_client_y0;

	dest_rect.right  =dest_rect.left+screen_width;
	dest_rect.bottom =dest_rect.top +screen_height;

   // clip the screen coords

	// blit the entire back surface to the primary
	if (FAILED(lpddsprimary->Blt(&dest_rect, lpddsback,NULL,DDBLT_WAIT,NULL)))
		return(0);


	// return success
	return(1);

} // end DDraw_Flip

///////////////////////////////////////////////////////////

UCHAR *DDraw_Lock_Back_Surface(void)
{
// this function locks the secondary back surface and returns a pointer to it
// and updates the global variables secondary buffer, and back_lpitch

	// is this surface already locked
	if (back_buffer)
	{
		// return to current lock
		return(back_buffer);
	} // end if

	// lock the primary surface
	DDRAW_INIT_STRUCT(ddsd);
	lpddsback->Lock(NULL,&ddsd,DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,NULL);

	// set globals
	back_buffer = (UCHAR *)ddsd.lpSurface;
	back_lpitch = ddsd.lPitch;

	// return pointer to surface
	return(back_buffer);

} // end DDraw_Lock_Back_Surface

///////////////////////////////////////////////////////////

int DDraw_Unlock_Back_Surface(void)
{
// this unlocks the secondary

	// is this surface valid
	if (!back_buffer)
	return(0);

	// unlock the secondary surface
	lpddsback->Unlock(NULL);

	// reset the secondary surface
	back_buffer = NULL;
	back_lpitch = 0;

	// return success
	return(1);
} // end DDraw_Unlock_Back_Surface

///////////////////////////////////////////////////////////

LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds,
                                         int num_rects,
                                         LPRECT clip_list)

{
	// this function creates a clipper from the sent clip list and attaches
	// it to the sent surface

	int index;                         // looping var
	LPDIRECTDRAWCLIPPER lpddclipper2;   // pointer to the newly created dd clipper
	LPRGNDATA region_data;             // pointer to the region data that contains
		// the header and clip list

	// first create the direct draw clipper
	if (FAILED(lpdd->CreateClipper(0,&lpddclipper2,NULL)))
		return(NULL);

	// now create the clip list from the sent data

	// first allocate memory for region data
	region_data = (LPRGNDATA)malloc(sizeof(RGNDATAHEADER)+num_rects*sizeof(RECT));

	// now copy the rects into region data
	memcpy(region_data->Buffer, clip_list, sizeof(RECT)*num_rects);

	// set up fields of header
	region_data->rdh.dwSize          = sizeof(RGNDATAHEADER);
	region_data->rdh.iType           = RDH_RECTANGLES;
	region_data->rdh.nCount          = num_rects;
	region_data->rdh.nRgnSize        = num_rects*sizeof(RECT);

	region_data->rdh.rcBound.left    =  64000;
	region_data->rdh.rcBound.top     =  64000;
	region_data->rdh.rcBound.right   = -64000;
	region_data->rdh.rcBound.bottom  = -64000;

	// find bounds of all clipping regions
	for (index=0; index<num_rects; index++)
    {
		// test if the next rectangle unioned with the current bound is larger
		if (clip_list[index].left < region_data->rdh.rcBound.left)
			region_data->rdh.rcBound.left = clip_list[index].left;

		if (clip_list[index].right > region_data->rdh.rcBound.right)
			region_data->rdh.rcBound.right = clip_list[index].right;

		if (clip_list[index].top < region_data->rdh.rcBound.top)
			region_data->rdh.rcBound.top = clip_list[index].top;

		if (clip_list[index].bottom > region_data->rdh.rcBound.bottom)
			region_data->rdh.rcBound.bottom = clip_list[index].bottom;

    } // end for index

	// now we have computed the bounding rectangle region and set up the data
	// now let's set the clipping list

	if (FAILED(lpddclipper2->SetClipList(region_data, 0)))
	{
		// release memory and return error
		free(region_data);
		return(NULL);
	} // end if

	// now attach the clipper to the surface
	if (FAILED(lpdds->SetClipper(lpddclipper2)))
	{
		// release memory and return error
		free(region_data);
		return(NULL);
	} // end if

	// all is well, so release memory and send back the pointer to the new clipper
	free(region_data);

	//рекомендуют в Microsoft
	if(lpddclipper2)
		lpddclipper2->Release();

	return(lpddclipper2);

} // end DDraw_Attach_Clipper

///////////////////////////////////////////////////////////
int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds, DWORD color, RECT *client)
{
	DDBLTFX ddbltfx; // this contains the DDBLTFX structure

	// clear out the structure and set the size field
	DDRAW_INIT_STRUCT(ddbltfx);

	// set the dwfillcolor field to the desired color
	ddbltfx.dwFillColor = color;

	// ready to blt to surface
	lpdds->Blt(client,     // ptr to dest rectangle
           NULL,       // ptr to source surface, NA
           NULL,       // ptr to source rectangle, NA
           DDBLT_COLORFILL | DDBLT_WAIT,   // fill and wait
           &ddbltfx);  // ptr to DDBLTFX structure

	// return success
	return(1);
} // end DDraw_Fill_Surface

//////////////////////////////////////////////////////

LPDIRECTDRAWSURFACE7 DDraw_Create_Surface(int width,
                                          int height,
                                          int mem_flags,
                                          DWORD color_key_value)
{
	// this function creates an offscreen plain surface

	LPDIRECTDRAWSURFACE7 lpdds;  // temporary surface

	// set to access caps, width, and height
	DDRAW_INIT_STRUCT(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

	// set dimensions of the new bitmap surface
	ddsd.dwWidth  =  width;
	ddsd.dwHeight =  height;

	// set surface to offscreen plain
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | mem_flags;

	// create the surface
	if (FAILED(lpdd->CreateSurface(&ddsd,&lpdds,NULL)))
		return(NULL);

	// set color key to default color 000
	// note that if this is a 8bit bob then palette index 0 will be
	// transparent by default
	// note that if this is a 16bit bob then RGB value 000 will be
	// transparent
	DDCOLORKEY color_key; // used to set color key
	color_key.dwColorSpaceLowValue  = color_key_value;
	color_key.dwColorSpaceHighValue = color_key_value;

	// now set the color key for source blitting
	lpdds->SetColorKey(DDCKEY_SRCBLT, &color_key);

	// return surface
	return(lpdds);
} // end DDraw_Create_Surface


//////////////////////////////////////////////////////////

int DDraw_Init(int width, int height)
{
	// this function initializes directdraw
	//int index; // looping variable

	// create IDirectDraw interface 7.0 object and test for error
	if (FAILED(DirectDrawCreateEx(NULL, (void **)&lpdd, IID_IDirectDraw7, NULL)))
	return(0);

	// set cooperation level to windowed mode
	if (FAILED(lpdd->SetCooperativeLevel(main_window_handle,DDSCL_NORMAL)))
		return(0);

	// Create the primary surface
	//memset(&ddsd,0,sizeof(ddsd));
	//ddsd.dwSize = sizeof(ddsd);
	DDRAW_INIT_STRUCT(ddsd);

	// windowed mode
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	// set the backbuffer count to 0 for windowed mode
	// 1 for fullscreen mode, 2 for triple buffering
	//ddsd.dwBackBufferCount = 0;

	// create the primary surface
	lpdd->CreateSurface(&ddsd,&lpddsprimary,NULL);



	// get the pixel format of the primary surface
	DDPIXELFORMAT ddpf; // used to get pixel format

	// initialize structure
	DDRAW_INIT_STRUCT(ddpf);

	// query the format from primary surface
	lpddsprimary->GetPixelFormat(&ddpf);

	// use number of bits, better method
	dd_pixel_format = ddpf.dwRGBBitCount;




	// create a double buffer that will be blitted
	// rather than flipped as in full screen mode
	lpddsback = DDraw_Create_Surface(width, height, DDSCAPS_SYSTEMMEMORY); // int mem_flags, USHORT color_key_flag);

	// only clear backbuffer
	DDraw_Fill_Surface(lpddsback,0);


	// set globals
	screen_height   = height;
	screen_width    = width;
/*
	// set software algorithmic clipping region
	min_clip_x = 0;
	max_clip_x = screen_width - 1;
	min_clip_y = 0;
	max_clip_y = screen_height - 1;
*/

	// setup backbuffer clipper always
	RECT screen_rect = {0,0,screen_width,screen_height};
	lpddclipper = DDraw_Attach_Clipper(lpddsback,1,&screen_rect);



	// set windowed clipper
	if (FAILED(lpdd->CreateClipper(0,&lpddclipperwin,NULL)))
		return(0);

	if (FAILED(lpddclipperwin->SetHWnd(0, main_window_handle)))
		return(0);

	if (FAILED(lpddsprimary->SetClipper(lpddclipperwin)))
		return(0);

	//Рекомендуют в Microsoft
	if(lpddclipperwin)
		lpddclipperwin->Release();

	// return success
	return(1);

} // end DDraw_Init

////////////////////////////////////////////////////

int DDraw_Shutdown(void)
{
// this function release all the resources directdraw
// allocated, mainly to com objects

	if (lpddclipperwin)
		lpddclipperwin->Release();

	// release the clippers first
	if (lpddclipper)
		lpddclipper->Release();

	// release the secondary surface
	if (lpddsback)
		lpddsback->Release();

	// release the primary surface
	if (lpddsprimary)
		lpddsprimary->Release();

	// finally, the main dd object
	if (lpdd)
		lpdd->Release();

	// return success
	return(1);
} // end DDraw_Shutdown

///////////////////////////////////////////////////////////

DWORD Get_Clock(void){
	// this function returns the current tick count

	return(GetTickCount());

} // end Get_Clock

///////////////////////////////////////////////////////////

DWORD Start_Clock(void){
	// this function starts the clock, that is, saves the current
	// count, use in conjunction with Wait_Clock()

	return(start_clock_count = Get_Clock());
} // end Start_Clock

///////////////////////////////////////////////////////////
