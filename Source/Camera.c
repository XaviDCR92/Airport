/* *************************************
 * 	Includes
 * *************************************/

#include "Camera.h"
#include "Game.h"
#include "System.h"
#include "Pad.h"

/* *************************************
 * 	Defines
 * *************************************/

#define SPEED_CALCULATION_TIME 3
#define MAX_CAMERA_SPEED 5
#define MIN_CAMERA_SPEED 1
#define CAMERA_INITIAL_X_OFFSET (X_SCREEN_RESOLUTION >> 1)
#define CAMERA_INITIAL_X_OFFSET_2PLAYER (X_SCREEN_RESOLUTION >> 2)

/* *************************************
 * 	Local Variables
 * *************************************/

static int32_t Camera_Max_X_Offset;
static int32_t Camera_Max_Y_Offset;

/* *************************************
 * 	Local Prototypes
 * *************************************/

static void CameraUpdateSpeed(TYPE_PLAYER* ptrPlayer);
static bool CameraSpecialConditions(TYPE_PLAYER* ptrPlayer);

void CameraInit(TYPE_PLAYER* ptrPlayer)
{
	// Center camera on screen
	ptrPlayer->Camera.X_Offset = CAMERA_INITIAL_X_OFFSET;
	ptrPlayer->Camera.Y_Offset = 0;
	ptrPlayer->Camera.X_Speed = 0;
	ptrPlayer->Camera.Y_Speed = 0;
	ptrPlayer->Camera.Speed_Timer = SPEED_CALCULATION_TIME;

    Camera_Max_X_Offset = GameGetLevelColumns() << TILE_SIZE_BIT_SHIFT;
    Camera_Max_Y_Offset = GameGetLevelColumns() * TILE_SIZE_H;
}

void CameraApplyCoordinatesToSprite(TYPE_PLAYER* ptrPlayer, GsSprite * spr)
{
	spr->x += (short)ptrPlayer->Camera.X_Offset;
	spr->y += (short)ptrPlayer->Camera.Y_Offset;
}

void CameraApplyCoordinatesToRectangle(TYPE_PLAYER* ptrPlayer, GsRectangle * rect)
{
	rect->x += (short)ptrPlayer->Camera.X_Offset;
	rect->y += (short)ptrPlayer->Camera.Y_Offset;
}

void CameraUpdateSpeed(TYPE_PLAYER* ptrPlayer)
{
	if (ptrPlayer->PadDirectionKeyPressed_Callback() == true)
	{
		if (ptrPlayer->PadKeyPressed_Callback(PAD_LEFT) == true)
		{
			if (ptrPlayer->Camera.X_Speed < 0)
			{
				ptrPlayer->Camera.X_Speed += 2;
			}
			else if (ptrPlayer->Camera.X_Speed < MAX_CAMERA_SPEED)
			{
				ptrPlayer->Camera.X_Speed++;
			}
		}

		if (ptrPlayer->PadKeyPressed_Callback(PAD_UP) == true)
		{
			if (ptrPlayer->Camera.Y_Speed < 0)
			{
				ptrPlayer->Camera.Y_Speed += 2;
			}
			else if (ptrPlayer->Camera.Y_Speed < MAX_CAMERA_SPEED)
			{
				ptrPlayer->Camera.Y_Speed++;
			}
		}

		if (ptrPlayer->PadKeyPressed_Callback(PAD_DOWN) == true)
		{
			if (ptrPlayer->Camera.Y_Speed > 0)
			{
				ptrPlayer->Camera.Y_Speed -= 2;
			}
			else if (ptrPlayer->Camera.Y_Speed > -MAX_CAMERA_SPEED)
			{
				ptrPlayer->Camera.Y_Speed--;
			}
		}

		if (ptrPlayer->PadKeyPressed_Callback(PAD_RIGHT) == true)
		{
			if (ptrPlayer->Camera.X_Speed > 0)
			{
				ptrPlayer->Camera.X_Speed -= 2;
			}
			else if (ptrPlayer->Camera.X_Speed > -MAX_CAMERA_SPEED)
			{
				ptrPlayer->Camera.X_Speed--;
			}
		}
	}

	if (	(ptrPlayer->PadKeyPressed_Callback(PAD_LEFT) == false)
			&&
		(ptrPlayer->PadKeyPressed_Callback(PAD_RIGHT) == false)	)
	{
		if (ptrPlayer->Camera.X_Speed > 0)
		{
			ptrPlayer->Camera.X_Speed--;
		}
		else if (ptrPlayer->Camera.X_Speed < 0)
		{
			ptrPlayer->Camera.X_Speed++;
		}
	}

	if (	(ptrPlayer->PadKeyPressed_Callback(PAD_UP) == false)
			&&
		(ptrPlayer->PadKeyPressed_Callback(PAD_DOWN) == false)	)
	{
		if (ptrPlayer->Camera.Y_Speed > 0)
		{
			ptrPlayer->Camera.Y_Speed--;
		}
		else if (ptrPlayer->Camera.Y_Speed < 0)
		{
			ptrPlayer->Camera.Y_Speed++;
		}
	}
}

void CameraHandler(TYPE_PLAYER* ptrPlayer)
{
	if (CameraSpecialConditions(ptrPlayer) == true)
	{
		ptrPlayer->Camera.X_Speed = 0;
		ptrPlayer->Camera.Y_Speed = 0;
		return;
	}

	if (ptrPlayer->Camera.Speed_Timer < SPEED_CALCULATION_TIME)
	{
		ptrPlayer->Camera.Speed_Timer++;
	}
	else
	{
		ptrPlayer->Camera.Speed_Timer = 0;
		CameraUpdateSpeed(ptrPlayer);
	}

	ptrPlayer->Camera.X_Offset += ptrPlayer->Camera.X_Speed;
	ptrPlayer->Camera.Y_Offset += ptrPlayer->Camera.Y_Speed;

    //DEBUG_PRINT_VAR(ptrPlayer->Camera.X_Offset);
    //DEBUG_PRINT_VAR(ptrPlayer->Camera.Y_Offset);
}

bool CameraSpecialConditions(TYPE_PLAYER* ptrPlayer)
{
	if (	(ptrPlayer->ShowAircraftData == true)
					||
		(ptrPlayer->SelectRunway == true)		)
	{
		// Camera cannot be handled when these states are activated

		return true;
	}

	return false;
}

TYPE_ISOMETRIC_POS CameraGetIsoPos(TYPE_PLAYER* ptrPlayer)
{
	TYPE_ISOMETRIC_POS IsoPos;
	TYPE_CARTESIAN_POS CartPos;

	if (GameTwoPlayersActive() == true)
	{
		CartPos.x = CAMERA_INITIAL_X_OFFSET_2PLAYER - ptrPlayer->Camera.X_Offset;
		CartPos.y = (Y_SCREEN_RESOLUTION >> 1) - ptrPlayer->Camera.Y_Offset;
	}
	else
	{
		CartPos.x = CAMERA_INITIAL_X_OFFSET - ptrPlayer->Camera.X_Offset;
		CartPos.y = (Y_SCREEN_RESOLUTION >> 1) - ptrPlayer->Camera.Y_Offset;
	}

	/*Serial_printf("CartPos = {%d, %d}\n", CartPos.x, CartPos.y);*/

	IsoPos = GfxCartesianToIsometric(&CartPos);

	return IsoPos;
}

void CameraMoveToIsoPos(TYPE_PLAYER* ptrPlayer, TYPE_ISOMETRIC_POS IsoPos)
{
	TYPE_CARTESIAN_POS CartPos = GfxIsometricToCartesian(&IsoPos);

	/*Serial_printf("Isometric pos = {%d, %d, %d}, "
			"Cartesian pos = {%d, %d}\n",
			IsoPos.x,
			IsoPos.y,
			IsoPos.z,
			CartPos.x,
			CartPos.y	);*/

	if (GameTwoPlayersActive() == true)
	{
		ptrPlayer->Camera.X_Offset = CAMERA_INITIAL_X_OFFSET_2PLAYER - CartPos.x;
		ptrPlayer->Camera.Y_Offset = (Y_SCREEN_RESOLUTION >> 1) - CartPos.y;
	}
	else
	{
		ptrPlayer->Camera.X_Offset = CAMERA_INITIAL_X_OFFSET - CartPos.x;
		ptrPlayer->Camera.Y_Offset = (Y_SCREEN_RESOLUTION >> 1) - CartPos.y;
	}

	/*Serial_printf("Moving camera to {%d, %d}\n",
			ptrPlayer->Camera.X_Offset,
			ptrPlayer->Camera.Y_Offset	);*/
}
