#ifndef GFX_HEADER__
#define GFX_HEADER__

/* *************************************
 * 	Includes
 * *************************************/

#include "Global_Inc.h"
#include "GameStructures.h"

/* *************************************
 * 	Defines
 * *************************************/

#define X_SCREEN_RESOLUTION 	368
#define Y_SCREEN_RESOLUTION 	240
#define VRAM_W					1024
#define VRAM_H					512
#define MAX_SIZE_FOR_GSSPRITE 	256
#define GFX_TPAGE_WIDTH 		64
#define GFX_TPAGE_HEIGHT 		256
#define GFX_1HZ_FLASH			(1<<7)
#define GFX_2HZ_FLASH			(1<<8)
#define FULL_LUMINANCE			0xFF
#define ROTATE_BIT_SHIFT        12 // 4096 = 2^12
#define BUTTON_SIZE				16

/* *************************************
 * 	Global prototypes
 * *************************************/

void GfxInitDrawEnv(void);
void GfxInitDispEnv(void);
void GfxSetDefaultPrimitiveList(void);

// Renders new scene. Use this function unless you know what you are doing!
void GfxDrawScene(void);

// Blocking version. Calls GfxDrawScene() and then adds a while (GfxIsBusy() )
// after it.
void GfxDrawScene_Slow(void);

void GfxDrawScene_NoSwap(void);

void GfxSwapBuffers(void);

// Only renders screen and does not update any pad data or timer data.
// To be used in ISR!
void GfxDrawScene_Fast(void);

// Repotedly, tells is GPU is ready for a DMA transfer.
bool GfxReadyForDMATransfer(void);

// Fills a GsSprite structure with information from a TIM file.
bool GfxSpriteFromFile(const char* fname, GsSprite* spr);

// Reportedly, loads CLUT data from a TIM image (image data is discarded)
bool GfxCLUTFromFile(const char* fname);

// Returns true if current object is within screen limits, false otherwise.
bool GfxIsInsideScreenArea(short x, short y, short w, short h);

// Function overload for GsSprite structures.
bool GfxIsSpriteInsideScreenArea(GsSprite* spr);

// Used to know whether GPU operation can be done.
bool GfxIsGPUBusy(void);

// Draws a sprite on screen. First, it checks whether sprite is inside
// screen limits.
void GfxSortSprite(GsSprite* spr);

uint8_t GfxGetGlobalLuminance(void);

void GfxSetGlobalLuminance(uint8_t value);

void GfxIncreaseGlobalLuminance(int8_t step);

void GfxButtonSetFlags(uint8_t flags);

void GfxButtonRemoveFlags(uint8_t flags);

int GfxRotateFromDegrees(int deg);

void GfxDrawButton(short x, short y, unsigned short btn);

// Sends current display data on a specific VRAM section and fills
// sprite structure pointed to by "spr".
void GfxSaveDisplayData(GsSprite *spr);

TYPE_CARTESIAN_POS GfxIsometricToCartesian(TYPE_ISOMETRIC_POS* ptrIsoPos);
// Function overload for fixed-point 16.16 data type.

TYPE_CARTESIAN_POS GfxIsometricFix16ToCartesian(TYPE_ISOMETRIC_FIX16_POS * ptrIso16Pos);

// Transforms cartesian position to isometric position. Z axis is assumed to be zero!
TYPE_ISOMETRIC_POS GfxCartesianToIsometric(TYPE_CARTESIAN_POS * ptrCartPos);

// Fills GsSprite structure pointed to by "spr" with texture page and U/V
// offset data given a position in VRAM.
bool GfxTPageOffsetFromVRAMPosition(GsSprite* spr, short x, short y);

void GfxSetSplitScreen(uint8_t playerIndex);

void GfxDisableSplitScreen(void);

// Switches between true and false every 1 exact second (used for flashing effects)
bool Gfx1HzFlash(void);

// Switches between true and false every 500 milliseconds (used for flashing effects)
bool Gfx2HzFlash(void);

void GfxDrawScene_NoSwap(void);

void GfxDevMenuEnable(void);

short GfxGetDrawEnvWidth(void);
short GfxGetDrawEnvHeight(void);

/* *************************************
 * 	Global variables
 * *************************************/

extern GsSprite PSXButtons;

#endif //GFX_HEADER__
