#include "Game.hpp"

#define CLEAR_COLOR 0x2c547cFF

#define DISPLAY_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))

int main(void)
{
	romfsInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	
   	C3D_RenderTarget *top = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTarget *bottom  = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);

	C3D_RenderTargetSetOutput(top, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    C3D_RenderTargetSetOutput(bottom, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);

	Game game;

	while (aptMainLoop())
	{
		hidScanInput();

		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();
		u32 kUp = hidKeysUp();
		if (kDown & KEY_START)
			break;

		game.control(kDown, kHeld, kUp);
		
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		{
			{// Top
				C3D_RenderTargetClear(top, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
				C3D_FrameDrawOn(top);
				C2D_SceneTarget(top);
				game.renderTop();
			}
			{// Bottom
				C3D_RenderTargetClear(bottom, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
                C3D_FrameDrawOn(bottom);
                C2D_SceneTarget(bottom);
                game.renderBottom();
			}
		}
		C3D_FrameEnd(0);
	}
	C3D_Fini();
	C2D_Fini();
	gfxExit();
	romfsExit();
	return 0;
}