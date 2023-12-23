#include <stdio.h>
#include <3ds.h>
#include <3ds/svc.h>
#include <3ds/types.h>
#include <citro2d.h>
#include <citro3d.h>
#include <tex3ds.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <cwav.h>
#include <ncsnd.h>
#include <opusfile.h>

#include "math.h"
#include "sprite_animation_manager.h"
#include "ds_time.h"

#include "vshader_shbin.h"

#include "road_t3x.h"
#include "stop_t3x.h"
#include "people_t3x.h"
#include "tunnel_t3x.h"
#include "tvBase_t3x.h"
#include "tvDisplay_t3x.h"
#include "tvStand_t3x.h"

#include "all_vertex.h"
#include "tv.h"


#define CLEAR_COLOR 0x2c547cFF
#define SCREEN_WIDTH 400
#define SCREEN_BUTTOM 320
#define SCREEN_HEIGHT 240

#define DISPLAY_TRANSFER_FLAGS \
	(GX_TRANSFER_FLIP_VERT(0) | GX_TRANSFER_OUT_TILED(0) | GX_TRANSFER_RAW_COPY(0) | \
	GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8) | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8) | \
	GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO))


static DVLB_s* vshader_dvlb;
static shaderProgram_s program;
static int uLoc_projection, uLoc_modelView;
static int uLoc_lightVec, uLoc_lightHalfVec, uLoc_lightClr, uLoc_material;
static C3D_Mtx projection;
static C3D_Mtx material =
{
	{
	{ { 0.0f, 0.2f, 0.2f, 0.2f } }, 
	{ { 0.0f, 0.4f, 0.4f, 0.4f } }, 
	{ { 0.0f, 0.8f, 0.8f, 0.8f } }, 
	{ { 1.0f, 0.5f, 0.5f, 0.5f } }, 
	}
};

static void* vbo_data;
static C3D_Tex road_tex, stop_tex, people_tex;
static C3D_Tex tvBase_tex, tvDisplay_tex, tvStand_tex;
static C3D_FogLut fog_Lut;
static float angleY = 0.0, angleX =  C3D_AngleFromDegrees(190), jumpZ = 40.0f;
static float jumpY = 66.1, angleW = 16.0, angleH = 0.0;
static float vector = 0.0;
static float verticalSpeed = 0.0f;
float initVy, gravity, vy, limitPy; 
float density = 1.5f;
int jumpFlag;

static C2D_SpriteSheet spriteSheet;
C3D_RenderTarget* target;

static bool loadTextureFromMem(C3D_Tex* tex, C3D_TexCube* cube, const void* data, size_t size)
{
	Tex3DS_Texture t3x = Tex3DS_TextureImport(data, size, tex, cube, false);
	if (!t3x)
		return false;
	
	Tex3DS_TextureFree(t3x);
	return true;
}

static void sceneInit( void )
{
	vshader_dvlb = DVLB_ParseFile((u32 *)vshader_shbin, vshader_shbin_size);
	shaderProgramInit(&program);
	shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
	C3D_BindProgram(&program);

	uLoc_projection = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
	uLoc_modelView  = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");
	uLoc_lightVec   = shaderInstanceGetUniformLocation(program.vertexShader, "lightVec");
	uLoc_lightHalfVec = shaderInstanceGetUniformLocation(program.vertexShader, "lightHalfVec");
	uLoc_lightClr   = shaderInstanceGetUniformLocation(program.vertexShader, "lightClr");
	uLoc_material   = shaderInstanceGetUniformLocation(program.vertexShader, "material");

	C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
	AttrInfo_Init(attrInfo);
	AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // 位置情報（3つの浮動小数点数）の属性ローダーを追加します
	AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // テクスチャ座標（2つの浮動小数点数）の属性ローダーを追加します
	AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 3); // 法線（3つの浮動小数点数）の属性ローダーを追加します

	// レンダリングのための透視投影行列を設定します
	Mtx_PerspTilt(&projection, C3D_AngleFromDegrees(80.0f), C3D_AspectRatioTop, 0.01f, 1000.0f, false);

	vbo_data = linearAlloc(sizeof(road_vertex_list) + sizeof(stop_vertex_list) + sizeof(people_vertex_list) + sizeof(tvBase_vertex_list) + sizeof(tvDisplay_vertex_list) + sizeof(tvStand_vertex_list));

	typedef struct {
		size_t size;
		void* data;
	} VertexData;

	VertexData vertexLists[] = {
		{ sizeof(road_vertex_list), road_vertex_list },
		{ sizeof(stop_vertex_list), stop_vertex_list },
		{ sizeof(people_vertex_list), people_vertex_list },
		{ sizeof(tvBase_vertex_list), tvBase_vertex_list },
		{ sizeof(tvDisplay_vertex_list), tvDisplay_vertex_list },
		{ sizeof(tvStand_vertex_list), tvStand_vertex_list }
	};

	size_t totalSize = 0;
	for (size_t i = 0; i < sizeof(vertexLists) / sizeof(vertexLists[0]); ++i) {
		memcpy((void*)((u32)vbo_data + totalSize), vertexLists[i].data, vertexLists[i].size);
		totalSize += vertexLists[i].size;
	}
	
	C3D_BufInfo* bufInfo = C3D_GetBufInfo();
	BufInfo_Init(bufInfo);
	BufInfo_Add(bufInfo, vbo_data, sizeof(vertex), 3, 0x210);

	typedef struct {
		C3D_Tex* tex;
		const void* data;
		u32 size;
	} TextureData;

	void loadAndSetTexture(C3D_Tex* tex, const void* data, u32 size) {
		if (!loadTextureFromMem(tex, NULL, data, size))
			svcBreak(USERBREAK_PANIC);
		C3D_TexSetFilter(tex, GPU_LINEAR, GPU_NEAREST);
		C3D_TexSetFilterMipmap(tex, GPU_LINEAR);
	}

	TextureData textures[] = {
		{ &road_tex, road_t3x, road_t3x_size },
		{ &stop_tex, stop_t3x, stop_t3x_size },
		{ &people_tex, people_t3x, people_t3x_size },
		{ &tvBase_tex, tvBase_t3x, tvBase_t3x_size },
		{ &tvDisplay_tex, tvDisplay_t3x, tvDisplay_t3x_size },
		{ &tvStand_tex, tvStand_t3x, tvStand_t3x_size }
	};

	for (size_t i = 0; i < sizeof(textures) / sizeof(textures[0]); ++i) {
		loadAndSetTexture(textures[i].tex, textures[i].data, textures[i].size);
	}
}

void prepareToRender(void)
{
    C3D_BindProgram(&program);

    C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // 位置情報（3つの浮動小数点数）の属性ローダーを追加します
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // テクスチャ座標（2つの浮動小数点数）の属性ローダーを追加します
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 3); // 法線（3つの浮動小数点数）の属性ローダーを追加します
    
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, vbo_data, sizeof(vertex), 3, 0x210);
}


static void sceneRender(void)
{
	prepareToRender();
	C3D_Mtx modelView;
	Mtx_Identity(&modelView);
	Mtx_LookAt(&modelView, FVec3_New(angleW, jumpZ, jumpY), FVec3_New(angleW, jumpZ, angleH), FVec3_New(0, 1, 0), true);
	Mtx_Translate(&modelView, 0.0, 0.0, -2.0 + 0.5 * sinf(angleY), false);
	Mtx_RotateY(&modelView, angleX, false);
	Mtx_RotateX(&modelView, angleY, false);


	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView, &modelView);
	C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_material, &material);
	C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightVec,     0.0f, 0.0f, -1.0f, 0.0f);
	C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightHalfVec, 0.0f, 0.0f, -1.0f, 0.0f);
	C3D_FVUnifSet(GPU_VERTEX_SHADER, uLoc_lightClr,     1.0f, 1.0f,  1.0f, 1.0f);

	
	C3D_TexEnv* env = C3D_GetTexEnv(0);
	C3D_TexEnvInit(env);
	C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, 0);
	C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);
	

	typedef struct {
		C3D_Tex  tex;
		int count;
	} DrawData;

	DrawData drawData[] = {
		{ road_tex, road_vertex_list_count },
		{ stop_tex, stop_vertex_list_count },
		{ people_tex, people_vertex_list_count },
		{ tvBase_tex, tvBase_vertex_list_count },
		{ tvDisplay_tex, tvDisplay_vertex_list_count },
		{ tvStand_tex, tvStand_vertex_list_count }
	};

	int totalVertexCount = 0;
	for (int i = 0; i < sizeof(drawData) / sizeof(drawData[0]); ++i) {
		C3D_TexBind(0, &drawData[i].tex);
		C3D_DrawArrays(GPU_TRIANGLES, totalVertexCount, drawData[i].count);
		totalVertexCount += drawData[i].count;
	}

	
	FogLut_Exp(&fog_Lut, density, 1.5f, 0.01f, 20.0f);
	C3D_FogGasMode(GPU_FOG, GPU_DEPTH_DENSITY, false);
	C3D_FogColor(0x7C542C);//2c547c
	C3D_FogLutBind(&fog_Lut);
	
}


static void sceneExit(void)
{
	C3D_Tex textures[] = { road_tex, stop_tex, people_tex, tvBase_tex, tvDisplay_tex, tvStand_tex };

	for (int i = 0; i < sizeof(textures) / sizeof(textures[0]); ++i) {
		C3D_TexDelete(&textures[i]);
	}

	linearFree(vbo_data);

	shaderProgramFree(&program);
	DVLB_Free(vshader_dvlb);
}




// static void drawText(const char* text)
// {
//     C2D_TextBuf textBuf = C2D_TextBufNew(4096);
// 	C2D_Font font = C2D_FontLoad("romfs:/onryou.bcfnt");
//     C2D_Text textObj;
//     C2D_TextFontParse(&textObj, font, textBuf, text);
//     C2D_TextOptimize(&textObj);
//     C2D_DrawText(&textObj, C2D_AlignCenter, 0, 8.0f, 8.0f, 0.5f, 0, 0);
// }

void waitFor5Seconds() {
    time_t startTime = time(NULL);
    while (difftime(time(NULL), startTime) < 5) {
    }
}

bool flagExecuted = false;

void execureOnce()
{
	if (!flagExecuted) {
		sceneInit();
		C3D_RenderTargetSetOutput(target, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
		C3D_RenderTargetSetOutput(target, GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);
		flagExecuted = true;
	}
}

int main(void)
{
	romfsInit();
	gfxInitDefault();
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	consoleInit(GFX_BOTTOM, NULL);


	target = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

	spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/title.t3x");
	C2D_Sprite companySprite, collaboraterSprite, logoSprite;

	// C2D_Sprite spriteSheets[] = {companySprite, collaboraterSprite};

	// for (int i = 0; i < (sizeof(spriteSheets)/sizeof(spriteSheets[0])); i++)
	// {
	// 	C2D_SpriteFromSheet(&spriteSheets[i], spriteSheet, i);
	// 	C2D_SpriteSetCenter(&spriteSheets[i], 0.5, 0.5);
	// 	C2D_SpriteSetPos(&spriteSheets[i], 400 / 2, 240 / 2);
	// }

	C2D_SpriteFromSheet(&companySprite, spriteSheet, 1);
	C2D_SpriteSetCenter(&companySprite, 0.5, 0.5);
	C2D_SpriteSetPos(&companySprite, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	C2D_SpriteFromSheet(&collaboraterSprite, spriteSheet, 0);
	C2D_SpriteSetCenter(&collaboraterSprite, 0.5, 0.5);
	C2D_SpriteSetPos(&collaboraterSprite, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	C2D_SpriteFromSheet(&logoSprite, spriteSheet, 2);
	C2D_SpriteSetCenter(&logoSprite, 0.5, 0.5);
	C2D_SpriteSetPos(&logoSprite, SCREEN_WIDTH / 2,  SCREEN_HEIGHT / 2);

	time_t programStartTime = time(NULL);

	initVy = 20;
	gravity = -1.1f;
	jumpFlag = 0;
	vy = 1.5f;
	limitPy = jumpZ;

	while (aptMainLoop())
	{
		hidScanInput();

		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();
		u32 kUp = hidKeysUp();

		if (kDown & KEY_START)
			break;
		
	

		// if (kHeld & KEY_DOWN)
		// 	angleY += C3D_AngleFromDegrees(2.0f);
		// else if (kHeld & KEY_UP)
		// 	angleY -= C3D_AngleFromDegrees(2.0f);
		// else
		// {
		// 	if(angleY != 0)
		// 	{
		// 		if(angleY < C3D_AngleFromDegrees(0.0f))
		// 		{
		// 			angleY += C3D_AngleFromDegrees(3.0f);
		// 			if(angleY > C3D_AngleFromDegrees(1.00f))
		// 				angleY = C3D_AngleFromDegrees(0.0f);	
		// 		}
		// 		else
		// 		{
		// 			angleY -= C3D_AngleFromDegrees(3.0f);
		// 			if(angleY < C3D_AngleFromDegrees(1.0f))
		// 				angleY = C3D_AngleFromDegrees(0.0f);	
		// 		}
		// 	}
		// }

		// if (kHeld & KEY_RIGHT)
		// 	angleX += C3D_AngleFromDegrees(2.0f);
		// if (kHeld & KEY_LEFT)
		// 	angleX -= C3D_AngleFromDegrees(2.0f);


		// if (kHeld & KEY_B) {
		// 	vector = 2.0f;
		// }
		// else if (kHeld & KEY_X) {
			
		// 	vector = -2.0f;
		// }

		// if (/*jumpZ == 20 &&*/(kDown & KEY_A)) {
		// 	vy = initVy;
		// 	jumpFlag = 1;
			
		// }

		// if (jumpFlag == 1)
		// {
		// 	jumpZ +=  vy;
		// 	vy += gravity;
		// 	if (jumpZ < limitPy)
		// 	{
		// 		jumpZ = limitPy;
		// 		jumpFlag = 0;
		// 	}
		// }
			
		// if (kUp & KEY_B || kUp & KEY_X)
		// 	vector = 0.0;

		// if (angleY > C3D_AngleFromDegrees(30.0f)) {
		// 	angleY = C3D_AngleFromDegrees(30.0f);
		// } else if (angleY < C3D_AngleFromDegrees(-30.0f)) {
		// 	angleY = C3D_AngleFromDegrees(-30.0f);
		// }

		// if (angleX > C3D_AngleFromDegrees(190.0f)) {
		// 	angleX = C3D_AngleFromDegrees(190.0f);
		// } else if (angleX < C3D_AngleFromDegrees(160.0f)) {
		// 	angleX = C3D_AngleFromDegrees(160.0f);
		// }

		// //-z
		// if (jumpY >= 200) {
		// 	jumpY = 200;
			
		// 	//vector = 0.0;
		// }
		// //+z
		// if (jumpY < -60) {
		// 	// drawText("これ以上は進めないようだ");
		// 	jumpY = -60;
		// 	// angleX = C3D_AngleFromDegrees(180);
		// 	//vector = 0.0;
		// }

		// //-X
		// if (angleW < -30) {
		// 	angleW = -30;
		// 	vector = 0.0;
		// }
		// //+x
		// if (angleW > 30) {
		// 	angleW = 30;
		// 	vector = 0.0;
		// }

		
		
		jumpY += vector * cos(angleX);
		angleH += vector * cos(angleX);
		angleW -= vector * sin(angleX);
		angleH += verticalSpeed * cos(angleX); // Y軸の移動をcosによって角度に反映
    	jumpY -= verticalSpeed * sin(angleX); // Y軸の移動をsinによって角度に反映
		if (angleH < 0)
			angleH = -angleH;

		time_t currentTime = time(NULL);

		consoleClear();
		printf("X:%03f Y:%03f Z:%03f\n", angleW, jumpY, jumpZ);
		printf("angleX:%03f angleY:%03f angleH:%03f\n",angleX, angleY, angleH);
	
		
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		C2D_TargetClear(target, C2D_Color32f(0.0f, 0.0f, 0.0f, 1.0f));
		C2D_SceneBegin(target);
		if (difftime(currentTime, programStartTime) >= 1)
		{
			
			C2D_DrawSprite(&collaboraterSprite);
			if (difftime(currentTime, programStartTime) >= 5)
			{
				C2D_DrawSprite(&companySprite);
				if (difftime(currentTime, programStartTime) > 7)
				{
					//C2D_SpriteSheetFree(spriteSheet);
					execureOnce();
					C3D_RenderTargetClear(target, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
					C3D_FrameDrawOn(target);
					sceneRender();

					if (density > 0.05)
					{
						density -= 0.0047f;
					}

					C2D_Prepare();
					C2D_DrawSprite(&logoSprite);
					C2D_Flush();
				}
			}
		}
		C3D_FrameEnd(0);
	}
	sceneExit();
	C2D_SpriteSheetFree(spriteSheet);
	C3D_Fini();
	C2D_Fini();
	gfxExit();
	romfsExit();
	return 0;
}