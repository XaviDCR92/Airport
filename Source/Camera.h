#ifndef CAM_HEADER__
#define CAM_HEADER__

/* *************************************
 * 	Includes
 * *************************************/
#include "Global_Inc.h"
#include "GameStructures.h"

/* *************************************
 * 	Defines
 * *************************************/
/* *************************************
 * 	Global prototypes
 * *************************************/
void CameraInit(TYPE_PLAYER* ptrPlayer);
void CameraHandler(TYPE_PLAYER* ptrPlayer);
void CameraApplyCoordinatesToSprite(TYPE_PLAYER* ptrPlayer, GsSprite * spr);
void CameraApplyCoordinatesToRectangle(TYPE_PLAYER* ptrPlayer, GsRectangle * rect);
TYPE_ISOMETRIC_POS CameraGetIsoPos(TYPE_PLAYER* ptrPlayer);
void CameraMoveToIsoPos(TYPE_PLAYER* ptrPlayer, TYPE_ISOMETRIC_POS IsoPos);

#endif //CAM_HEADER__
