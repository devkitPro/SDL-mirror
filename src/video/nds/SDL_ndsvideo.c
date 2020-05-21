/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

#include <nds.h>

#include "SDL.h"
#include "SDL_error.h"
#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_ndsvideo.h"
#include "SDL_ndsevents_c.h"

#define NDSVID_DRIVER_NAME "nds"

/* Initialization/Query functions */
static int NDS_VideoInit(_THIS, SDL_PixelFormat *vformat);
static SDL_Rect **NDS_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags);
static SDL_Surface *NDS_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int NDS_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors);
static void NDS_VideoQuit(_THIS);

/* Hardware surface functions */
static int NDS_AllocHWSurface(_THIS, SDL_Surface *surface);
static int NDS_LockHWSurface(_THIS, SDL_Surface *surface);
static int NDS_FlipHWSurface(_THIS, SDL_Surface *surface);
static void NDS_UnlockHWSurface(_THIS, SDL_Surface *surface);
static void NDS_FreeHWSurface(_THIS, SDL_Surface *surface);

/* etc. */
static void NDS_UpdateRects(_THIS, int numrects, SDL_Rect *rects);

/* NDS driver bootstrap functions */

static int NDS_Available(void)
{
	return 1;
}

static void NDS_DeleteDevice(SDL_VideoDevice *device)
{
	SDL_free(device->hidden);
	SDL_free(device);
}

static int HWAccelBlit(SDL_Surface *src, SDL_Rect *srcrect,
                        SDL_Surface *dst, SDL_Rect *dstrect)
{
	return 0;
}
 
static int CheckHWBlit(_THIS, SDL_Surface *src, SDL_Surface *dst)
{
	if (src->flags & SDL_SRCALPHA) return false;
	if (src->flags & SDL_SRCCOLORKEY) return false;
	if (src->flags & SDL_HWPALETTE) return false;
	if (dst->flags & SDL_SRCALPHA) return false;
	if (dst->flags & SDL_SRCCOLORKEY) return false;
	if (dst->flags & SDL_HWPALETTE) return false;

	if (src->format->BitsPerPixel != dst->format->BitsPerPixel) return false;
	if (src->format->BytesPerPixel != dst->format->BytesPerPixel) return false;

	src->map->hw_blit = HWAccelBlit;
	return true;
}

static SDL_VideoDevice *NDS_CreateDevice(int devindex)
{
	SDL_VideoDevice *device = 0;

	/* Initialize all variables that we clean on shutdown */
	device = (SDL_VideoDevice *)SDL_malloc(sizeof(SDL_VideoDevice));
	if (device) {
		SDL_memset(device, 0, (sizeof *device));
		device->hidden = (struct SDL_PrivateVideoData *)
				SDL_malloc((sizeof *device->hidden));
	}
	if ((device == NULL) || (device->hidden == NULL)) {
		SDL_OutOfMemory();
		if (device) {
			SDL_free(device);
		}
		return(0);
	} 
	SDL_memset(device->hidden, 0, (sizeof *device->hidden));

	/* Set the function pointers */
	device->VideoInit = NDS_VideoInit;
	device->ListModes = NDS_ListModes;
	device->SetVideoMode = NDS_SetVideoMode;
	device->CreateYUVOverlay = NULL;
	device->SetColors = NDS_SetColors;
	device->UpdateRects = NDS_UpdateRects;
	device->VideoQuit = NDS_VideoQuit;
	device->AllocHWSurface = NDS_AllocHWSurface;
	device->CheckHWBlit = CheckHWBlit;
	device->FillHWRect = NULL;
	device->SetHWColorKey = NULL;
	device->SetHWAlpha = NULL;
	device->LockHWSurface = NDS_LockHWSurface;
	device->UnlockHWSurface = NDS_UnlockHWSurface;
	device->FlipHWSurface = NDS_FlipHWSurface;
	device->FreeHWSurface = NDS_FreeHWSurface;
	device->SetCaption = NULL;
	device->SetIcon = NULL;
	device->IconifyWindow = NULL;
	device->GrabInput = NULL;
	device->GetWMInfo = NULL;
	device->InitOSKeymap = NDS_InitOSKeymap;
	device->PumpEvents = NDS_PumpEvents;
	device->info.blit_hw=1;

	device->free = NDS_DeleteDevice;
	return device;
}

VideoBootStrap NDS_bootstrap = {
	NDSVID_DRIVER_NAME, "SDL NDS video driver",
	NDS_Available, NDS_CreateDevice
};

u16* frontBuffer;
u16* backBuffer;

int NDS_VideoInit(_THIS, SDL_PixelFormat *vformat)
{
	/* Determine the screen depth (use default 8-bit depth) */
	/* we change this during the SDL_SetVideoMode implementation... */
	vformat->BitsPerPixel = 16;	// mode 3
	vformat->BytesPerPixel = 2;
	vformat->Rmask = 0x0000f800;
	vformat->Gmask = 0x000007e0;
	vformat->Bmask = 0x0000001f; 
	powerOn(POWER_ALL);

	videoSetMode(MODE_6_2D | DISPLAY_BG2_ACTIVE);
	
	//set the sub background up for text display (we could just print to one
	//of the main display text backgrounds just as easily
	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE); //sub bg 0 will be used to print text

	vramSetPrimaryBanks(VRAM_A_MAIN_BG, VRAM_B_MAIN_BG, VRAM_C_MAIN_BG, VRAM_D_MAIN_BG);
	vramSetBankH(VRAM_H_SUB_BG);
	vramSetBankI(VRAM_I_LCD);

	consoleInit(NULL, 0, BgType_Text4bpp, BgSize_T_256x256, 15, 0, false, true);

	frontBuffer = (u16*)(0x06000000);

	this->hidden->touchscreen = ((REG_POWERCNT & POWER_SWAP_LCDS) == 0);

	/* We're done! */
	return 0;
}

SDL_Rect **NDS_ListModes(_THIS, SDL_PixelFormat *format, Uint32 flags)
{
	return (SDL_Rect **) -1;
}

SDL_Surface *NDS_SetVideoMode(_THIS, SDL_Surface *current,
				int width, int height, int bpp, Uint32 flags)
{
	Uint32 Rmask, Gmask, Bmask, Amask; 

	if (bpp > 8) {
		bpp=16;
 		Rmask = 0x0000001F;
		Gmask = 0x000003E0;
		Bmask = 0x00007C00;
		Amask = 0x00008000;

		videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE);

		vramSetPrimaryBanks(VRAM_A_MAIN_BG, VRAM_B_MAIN_BG, VRAM_C_MAIN_BG, VRAM_D_MAIN_BG);

		REG_BG2CNT = BG_BMP16_512x512;
		REG_BG2PA = ((width / 256) << 8) | (width % 256) ;
		REG_BG2PB = 0;
		REG_BG2PC = 0;
		REG_BG2PD = ((height / 192) << 8) | ((height % 192) + (height % 192) / 3) ;
		REG_BG2X = 0;
		REG_BG2Y = 0;
	} else if (bpp <= 8) {
		bpp=8;
		Rmask = 0x00000000;
		Gmask = 0x00000000;
		Bmask = 0x00000000;

		REG_BG2CNT = BG_BMP8_1024x512;
		REG_BG2PA = ((width / 256) << 8) | (width % 256);
		REG_BG2PB = 0;
		REG_BG2PC = 0;
		REG_BG2PD = ((height / 192) << 8) | ((height % 192) + (height % 192) / 3);
	}

	if(width<256) width=256;
	if(height<192) height=192;
	
	if(bpp==8)
	{
		this->hidden->ndsmode=4;
	}

	if(bpp == 15)
	{
		if(width<256) this->hidden->ndsmode = 5;
		else this->hidden->ndsmode = 3;
	}

	this->hidden->buffer = frontBuffer;

	SDL_memset(this->hidden->buffer, 0, 1024 * 512 * ((this->hidden->ndsmode == 4 || this->hidden->ndsmode == 5) ? 2 : 1 ) * ((bpp + 7) / 8));

	/* Allocate the new pixel format for the screen */
	if (!SDL_ReallocFormat(current, bpp, Rmask, Gmask, Bmask, Amask)) {
		this->hidden->buffer = NULL;
		SDL_SetError("Couldn't allocate new pixel format for requested mode");
		return (NULL);
	}

	/* Set up the new mode framebuffer */
	current->flags = flags | SDL_FULLSCREEN | SDL_HWSURFACE | (this->hidden->ndsmode > 0 ? SDL_DOUBLEBUF : 0);
	this->hidden->w = current->w = width;
	this->hidden->h = current->h = height;
	current->pixels = frontBuffer;
	current->pitch = 1024;

	if (flags & SDL_DOUBLEBUF) { 
		this->hidden->secondbufferallocd = 1;
		backBuffer=(u16*)SDL_malloc(1024 * 512 * 2);
		current->pixels = backBuffer; 
	}

	if (flags & SDL_BOTTOMSCR) {
		lcdMainOnBottom();
	} else if (flags & SDL_TOPSCR) {
		lcdMainOnTop();
	}
	this->hidden->touchscreen = ((REG_POWERCNT & POWER_SWAP_LCDS) == 0);

	/* We're done */
	return current;
}

static int NDS_AllocHWSurface(_THIS, SDL_Surface *surface)
{
	if(this->hidden->secondbufferallocd) {
		return -1;
	}

	return 0;
}
static void NDS_FreeHWSurface(_THIS, SDL_Surface *surface)
{
	this->hidden->secondbufferallocd = 0;
}

/* We need to wait for vertical retrace on page flipped displays */
static int NDS_LockHWSurface(_THIS, SDL_Surface *surface)
{
	return 0;
}

static void NDS_UnlockHWSurface(_THIS, SDL_Surface *surface)
{
}

static int NDS_FlipHWSurface(_THIS, SDL_Surface *surface)
{
	if(this->hidden->secondbufferallocd){
		while(REG_VCOUNT!=192);
		while(REG_VCOUNT==192);

		dmaCopyAsynch(backBuffer,frontBuffer,1024*512);
	}

	return 0;
}

static void NDS_UpdateRects(_THIS, int numrects, SDL_Rect *rects)
{
	/* do nothing. */
}

int NDS_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
	short r,g,b;
	
	if(this->hidden->ndsmode != 4)
	{
		printf("This is not a palettized mode\n");
		return -1;
	}

	int i, j = firstcolor + ncolors;
	for(i = firstcolor; i < j; i++)
	{
		r = colors[i].r>>3;
		g = colors[i].g>>3;
		b = colors[i].b>>3;
		BG_PALETTE[i] = RGB15(r, g, b);
	} 

	return 0;
}

/* Note:  If we are terminated, this could be called in the middle of
   another SDL video routine -- notably UpdateRects.
*/
void NDS_VideoQuit(_THIS)
{
}
