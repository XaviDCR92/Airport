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
void CameraInit(TYPE_PLAYER* const ptrPlayer);
void CameraHandler(TYPE_PLAYER* const ptrPlayer);
void CameraApplyCoordinatesToSprite(TYPE_PLAYER* const ptrPlayer, GsSprite* spr);
void CameraApplyCoordinatesToRectangle(TYPE_PLAYER* const ptrPlayer, GsRectangle* rect);
void CameraApplyCoordinatesToCartesianPos(TYPE_PLAYER* const ptrPlayer, TYPE_CARTESIAN_POS* pos);
TYPE_ISOMETRIC_POS CameraGetIsoPos(TYPE_PLAYER* const ptrPlayer);
void CameraMoveToIsoPos(TYPE_PLAYER* const ptrPlayer, TYPE_ISOMETRIC_POS IsoPos);

#endif //CAM_HEADER__
