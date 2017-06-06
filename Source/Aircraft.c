/* *************************************
 * 	Includes
 * *************************************/

#include "Aircraft.h"

/* *************************************
 * 	Defines
 * *************************************/

#define AIRCRAFT_SIZE				16
#define AIRCRAFT_SIZE_FIX16			fix16_from_int(AIRCRAFT_SIZE)

/* *************************************
 * 	Structs and enums
 * *************************************/

enum
{
	AIRCRAFT_SPRITE_SIZE = 24,
	AIRCRAFT_SPRITE_VRAM_X = 800,
	AIRCRAFT_SPRITE_VRAM_Y = 304,
};

enum
{
	PHX_LIVERY_CLUT_X = 384,
	PHX_LIVERY_CLUT_Y = 497,
};

typedef enum t_aircraftSpeeds
{
	AIRCRAFT_SPEED_IDLE = 0,
	AIRCRAFT_SPEED_GROUND,
	AIRCRAFT_SPEED_APPROACH,
	AIRCRAFT_SPEED_TAKEOFF,
	AIRCRAFT_SPEED_FINAL,
	AIRCRAFT_SPEED_FINAL_Z,
}AIRCRAFT_SPEEDS;

/* *************************************
 * 	Local variables
 * *************************************/

static TYPE_AIRCRAFT_DATA AircraftData[GAME_MAX_AIRCRAFT];
static uint8_t AircraftIndex;
static GsSprite AircraftSpr;
static TYPE_ISOMETRIC_POS AircraftCenterIsoPos;
static TYPE_CARTESIAN_POS AircraftCenterPos;
static char* AircraftLiveryNamesTable[] = {"PHX", NULL};
static AIRCRAFT_LIVERY AircraftLiveryTable[] = {AIRCRAFT_LIVERY_0, AIRCRAFT_LIVERY_UNKNOWN};
static const fix16_t AircraftSpeedsTable[] = {	[AIRCRAFT_SPEED_IDLE] = 0,
												[AIRCRAFT_SPEED_GROUND] = 0x6666,
												[AIRCRAFT_SPEED_TAKEOFF] = 0x20000,
												[AIRCRAFT_SPEED_FINAL] = 0x10000,
												[AIRCRAFT_SPEED_FINAL_Z] = 0x4000	};

/* *************************************
 * 	Local prototypes
 * *************************************/

static void AircraftDirection(TYPE_AIRCRAFT_DATA* ptrAircraft);
static AIRCRAFT_LIVERY AircraftLiveryFromFlightNumber(char* strFlightNumber);
static void AircraftAttitude(TYPE_AIRCRAFT_DATA* ptrAircraft);
static void AircraftUpdateSpriteFromData(TYPE_AIRCRAFT_DATA* ptrAircraft);
static void AircraftSpeed(TYPE_AIRCRAFT_DATA* ptrAircraft);
static bool AircraftCheckCollision(TYPE_AIRCRAFT_DATA* ptrRefAircraft, TYPE_AIRCRAFT_DATA* ptrOtherAircraft);

void AircraftInit(void)
{
	bzero(AircraftData, GAME_MAX_AIRCRAFT * sizeof(TYPE_AIRCRAFT_DATA));
	AircraftIndex = 0;

	AircraftSpr.x = 0;
	AircraftSpr.y = 0;

	AircraftSpr.attribute = COLORMODE(COLORMODE_8BPP);

	AircraftSpr.cx = PHX_LIVERY_CLUT_X;
	AircraftSpr.cy = PHX_LIVERY_CLUT_Y;

	AircraftSpr.w = AIRCRAFT_SPRITE_SIZE;
	AircraftSpr.h = AIRCRAFT_SPRITE_SIZE;

	/*AircraftSpr.tpage = 28;
	AircraftSpr.u = 64;
	AircraftSpr.v = 48;*/

	GfxTPageOffsetFromVRAMPosition(&AircraftSpr, AIRCRAFT_SPRITE_VRAM_X, AIRCRAFT_SPRITE_VRAM_Y);

	AircraftCenterIsoPos.x = AIRCRAFT_SIZE >> 1;
	AircraftCenterIsoPos.y = AIRCRAFT_SIZE >> 1;
	AircraftCenterIsoPos.z = 0;

	AircraftCenterPos = GfxIsometricToCartesian(&AircraftCenterIsoPos);
}

bool AircraftAddNew(	TYPE_FLIGHT_DATA* ptrFlightData,
						uint8_t FlightDataIndex,
						uint16_t* targets		)
{
	TYPE_AIRCRAFT_DATA* ptrAircraft = &AircraftData[AircraftIndex];
	uint8_t level_columns = GameGetLevelColumns();
	uint8_t i;

	if(AircraftIndex >= GAME_MAX_AIRCRAFT)
	{
		dprintf("Exceeded maximum aircraft capacity!\n");
		return false;
	}

	memcpy(ptrAircraft->Target, targets, sizeof(uint16_t) * AIRCRAFT_MAX_TARGETS);

	ptrAircraft->TargetIdx = 0;
	ptrAircraft->Livery = AircraftLiveryFromFlightNumber(ptrFlightData->strFlightNumber[FlightDataIndex]);

	ptrAircraft->FlightDataIdx = FlightDataIndex;

	dprintf("ptrAircraft->FlightDataIdx = %d, FlightDataIndex = %d\n", ptrAircraft->FlightDataIdx, FlightDataIndex);

	if(ptrFlightData->FlightDirection[FlightDataIndex] == ARRIVAL)
	{
		ptrAircraft->IsoPos.x = 0;

		ptrAircraft->IsoPos.y = targets[0] / level_columns;
		ptrAircraft->IsoPos.y <<= TILE_SIZE_BIT_SHIFT;
		ptrAircraft->IsoPos.y += TILE_SIZE >> 1; // Adjust to tile center
		ptrAircraft->IsoPos.y = fix16_from_int(ptrAircraft->IsoPos.y);

		ptrAircraft->IsoPos.z = targets[0] % level_columns;
		ptrAircraft->IsoPos.z <<= TILE_SIZE_BIT_SHIFT - 1;
		ptrAircraft->IsoPos.z = fix16_from_int(ptrAircraft->IsoPos.z);
	}
	else if(ptrFlightData->FlightDirection[FlightDataIndex] == DEPARTURE)
	{
		ptrAircraft->IsoPos.x = GameGetXFromTile(ptrFlightData->Parking[FlightDataIndex]);
		ptrAircraft->IsoPos.y = GameGetYFromTile(ptrFlightData->Parking[FlightDataIndex]);
		ptrAircraft->IsoPos.z = 0;
	}

	ptrAircraft->State = ptrFlightData->State[FlightDataIndex];

	ptrAircraft->Direction = AIRCRAFT_DIR_NORTH; // Default to north direction

	dprintf("\nAircraft Data:\n");
	dprintf("\tTargets:");

	for(i = 0; i < AIRCRAFT_MAX_TARGETS; i++)
	{
		if(ptrAircraft->Target[i] == 0)
		{
			break;
		}

		dprintf(" %d", ptrAircraft->Target[i]);
	}

	dprintf("\nLivery: %d\n",	ptrAircraft->Livery	);

	dprintf("Aircraft position: {%d, %d, %d}\n",
			fix16_to_int(ptrAircraft->IsoPos.x),
			fix16_to_int(ptrAircraft->IsoPos.y),
			fix16_to_int(ptrAircraft->IsoPos.z)	);

	AircraftIndex++;

	return true;
}

AIRCRAFT_LIVERY AircraftLiveryFromFlightNumber(char* strFlightNumber)
{
	int32_t liveryIndex;
	char strLivery[4];

	memset(strLivery, 0, 4 * sizeof(char) );

	strncpy(strLivery, strFlightNumber, 3);

	liveryIndex = SystemIndexOfStringArray(strLivery, AircraftLiveryNamesTable);

	if(liveryIndex == -1)
	{
		return AIRCRAFT_LIVERY_UNKNOWN;
	}

	return AircraftLiveryTable[liveryIndex];
}

bool AircraftRemove(uint8_t aircraftIdx)
{
	uint8_t i;

	for(i = 0; i < GAME_MAX_AIRCRAFT; i++)
	{
		TYPE_AIRCRAFT_DATA* ptrAircraft = &AircraftData[i];

		if(ptrAircraft->State != STATE_IDLE)
		{
			if(ptrAircraft->FlightDataIdx == aircraftIdx)
			{
				DEBUG_PRINT_VAR(ptrAircraft->FlightDataIdx);
				DEBUG_PRINT_VAR(aircraftIdx);
				ptrAircraft->State = STATE_IDLE;
				dprintf("Flight %d removed\n", ptrAircraft->FlightDataIdx);
				return true;
			}
		}
	}

	return false;
}

void AircraftHandler(void)
{
	uint8_t i;

	for(i = 0; i < GAME_MAX_AIRCRAFT; i++)
	{
		TYPE_AIRCRAFT_DATA* ptrAircraft = &AircraftData[i];
		uint8_t j;

		if(ptrAircraft->State == STATE_IDLE)
		{
			continue;
		}
		
		AircraftDirection(ptrAircraft);
		AircraftAttitude(ptrAircraft);
		AircraftSpeed(ptrAircraft);

		for(j = 0; j < GAME_MAX_AIRCRAFT; j++)
		{
			TYPE_AIRCRAFT_DATA* ptrOtherAircraft = &AircraftData[j];
			
			if(i == j)
			{
				continue;
			}

			if(AircraftData[j].State == STATE_IDLE)
			{
				continue;
			}
			
			if(AircraftCheckCollision(ptrAircraft, ptrOtherAircraft) == true)
			{
				GameAircraftCollision(ptrAircraft->FlightDataIdx);
				break;
			}
		}

		ptrAircraft->State = GameGetFlightDataStateFromIdx(ptrAircraft->FlightDataIdx);
	}
}

void AircraftSpeed(TYPE_AIRCRAFT_DATA* ptrAircraft)
{
	switch(ptrAircraft->State)
	{
		case STATE_FINAL:
			ptrAircraft->Speed = AircraftSpeedsTable[AIRCRAFT_SPEED_FINAL];
		break;

		case STATE_TAKEOFF:
			// Fall through
		case STATE_CLIMBING:
			ptrAircraft->Speed = AircraftSpeedsTable[AIRCRAFT_SPEED_TAKEOFF];
		break;

		case STATE_TAXIING:
			// Fall through
		case STATE_ENTERING_RWY:
			ptrAircraft->Speed = AircraftSpeedsTable[AIRCRAFT_SPEED_GROUND];
		break;

		case STATE_READY_FOR_TAKEOFF:
			// Fall through
		case STATE_UNBOARDING:
			// Fall through
		case STATE_IDLE:
			// Fall through
		case STATE_LANDED:
			// Fall through
		case STATE_HOLDING_RWY:
			// Fall through
		default:
			ptrAircraft->Speed = 0;
		break;
	}
}

void AircraftRender(TYPE_PLAYER* ptrPlayer)
{
	uint8_t i;

	for(i = 0; i < GAME_MAX_AIRCRAFT; i++)
	{
		TYPE_AIRCRAFT_DATA* ptrAircraft = &AircraftData[i];
		TYPE_CARTESIAN_POS cartPos;
		TYPE_ISOMETRIC_FIX16_POS shadowIsoPos = {	.x = ptrAircraft->IsoPos.x,
													.y = ptrAircraft->IsoPos.y,
													.z = 0	};
		TYPE_CARTESIAN_POS shadowCartPos;
		
		if(ptrAircraft->State == STATE_IDLE)
		{
			continue;
		}

		AircraftUpdateSpriteFromData(ptrAircraft);

		if(ptrAircraft->IsoPos.z > 0)
		{
			// Draw aircraft shadow

			shadowCartPos = GfxIsometricFix16ToCartesian(&shadowIsoPos);

			// Aircraft position is referred to aircraft center
			AircraftSpr.x = shadowCartPos.x - (AircraftSpr.w >> 1);
			AircraftSpr.y = shadowCartPos.y - (AircraftSpr.h >> 1);

			CameraApplyCoordinatesToSprite(ptrPlayer, &AircraftSpr);

			AircraftSpr.r = 0;
			AircraftSpr.g = 0;
			AircraftSpr.b = 0;

			AircraftSpr.attribute |= ENABLE_TRANS | TRANS_MODE(0);

			GfxSortSprite(&AircraftSpr);
		}

		cartPos = GfxIsometricFix16ToCartesian(&ptrAircraft->IsoPos);

		// Aircraft position is referred to aircraft center
		AircraftSpr.x = cartPos.x - (AircraftSpr.w >> 1);
		AircraftSpr.y = cartPos.y - (AircraftSpr.h >> 1);

		AircraftSpr.attribute &= ~(ENABLE_TRANS | TRANS_MODE(0));

		CameraApplyCoordinatesToSprite(ptrPlayer, &AircraftSpr);

		AircraftSpr.r = NORMAL_LUMINANCE;
		AircraftSpr.g = NORMAL_LUMINANCE;
		AircraftSpr.b = NORMAL_LUMINANCE;

		GfxSortSprite(&AircraftSpr);
	}
}

void AircraftDirection(TYPE_AIRCRAFT_DATA* ptrAircraft)
{
	TYPE_ISOMETRIC_FIX16_POS targetPos;

	if(ptrAircraft->State != STATE_CLIMBING)
	{
		if(ptrAircraft->Target[ptrAircraft->TargetIdx] == 0)
		{
			return;
		}

		targetPos.x = GameGetXFromTile(ptrAircraft->Target[ptrAircraft->TargetIdx]);
		targetPos.y = GameGetYFromTile(ptrAircraft->Target[ptrAircraft->TargetIdx]);
		targetPos.z = 0;

		ptrAircraft->TargetReached = false;

		if(targetPos.y == ptrAircraft->IsoPos.y)
		{
			if(targetPos.x > ptrAircraft->IsoPos.x)
			{
				if(targetPos.x <= (ptrAircraft->IsoPos.x + ptrAircraft->Speed) )
				{
					ptrAircraft->TargetReached = true;
				}
				else
				{
					ptrAircraft->Direction = AIRCRAFT_DIR_EAST;
					ptrAircraft->IsoPos.x += ptrAircraft->Speed;
				}
			}
			else if(targetPos.x < ptrAircraft->IsoPos.x)
			{
				if(targetPos.x >= (ptrAircraft->IsoPos.x - ptrAircraft->Speed) )
				{
					ptrAircraft->TargetReached = true;
				}
				else
				{
					ptrAircraft->Direction = AIRCRAFT_DIR_WEST;
					ptrAircraft->IsoPos.x -= ptrAircraft->Speed;
				}
			}
			else
			{
				ptrAircraft->TargetReached = true;
			}
		}
		else if(targetPos.x == ptrAircraft->IsoPos.x)
		{
			if(targetPos.y > ptrAircraft->IsoPos.y)
			{
				if(targetPos.y <= (ptrAircraft->IsoPos.y + ptrAircraft->Speed) )
				{
					ptrAircraft->TargetReached = true;
				}
				else
				{
					ptrAircraft->Direction = AIRCRAFT_DIR_SOUTH;
					ptrAircraft->IsoPos.y += ptrAircraft->Speed;
				}
			}
			else if(targetPos.y < ptrAircraft->IsoPos.y)
			{
				if(targetPos.y >= (ptrAircraft->IsoPos.y - ptrAircraft->Speed) )
				{
					ptrAircraft->TargetReached = true;
				}
				else
				{
					ptrAircraft->Direction = AIRCRAFT_DIR_NORTH;
					ptrAircraft->IsoPos.y -= ptrAircraft->Speed;
				}
			}
			else
			{
				ptrAircraft->TargetReached = true;
			}
		}

		if(ptrAircraft->TargetReached == true)
		{
			ptrAircraft->IsoPos.x = targetPos.x;
			ptrAircraft->IsoPos.y = targetPos.y;
			
			if(ptrAircraft->Target[++ptrAircraft->TargetIdx] == 0)
			{
				dprintf("All targets reached!\n");
				ptrAircraft->State = GameTargetsReached(ptrAircraft->Target[0], ptrAircraft->FlightDataIdx);
				memset(ptrAircraft->Target, 0, AIRCRAFT_MAX_TARGETS);
			}
		}
	}
	else
	{
		// STATE_CLIMBING
		switch(ptrAircraft->Direction)
		{
			case AIRCRAFT_DIR_EAST:
				ptrAircraft->IsoPos.x += ptrAircraft->Speed;
			break;

			case AIRCRAFT_DIR_WEST:
				ptrAircraft->IsoPos.x -= ptrAircraft->Speed;
			break;

			case AIRCRAFT_DIR_NORTH:
				ptrAircraft->IsoPos.y -= ptrAircraft->Speed;
			break;

			case AIRCRAFT_DIR_SOUTH:
				ptrAircraft->IsoPos.y += ptrAircraft->Speed;
			break;

			case AIRCRAFT_DIR_NO_DIRECTION:
				// Fall through
			default:
			return;
		}

		ptrAircraft->IsoPos.z += AircraftSpeedsTable[AIRCRAFT_SPEED_FINAL_Z];

		if(GameInsideLevelFromIsoPos(&ptrAircraft->IsoPos) == true)
		{
			GameRemoveFlight(ptrAircraft->FlightDataIdx, true);
		}
	}
}

void AircraftUpdateSpriteFromData(TYPE_AIRCRAFT_DATA* ptrAircraft)
{
	switch(ptrAircraft->Livery)
	{
		case AIRCRAFT_LIVERY_0:
			AircraftSpr.cx = PHX_LIVERY_CLUT_X;
			AircraftSpr.cy = PHX_LIVERY_CLUT_Y;
		break;

		case AIRCRAFT_LIVERY_UNKNOWN:
			// Fall through
		default:
			dprintf("Unknown livery %d!\n", ptrAircraft->Livery);
		break;
	}

	// Reset TPAGE and {U, V} offset first.
	GfxTPageOffsetFromVRAMPosition(&AircraftSpr, AIRCRAFT_SPRITE_VRAM_X, AIRCRAFT_SPRITE_VRAM_Y);

	switch(ptrAircraft->Direction)
	{
		case AIRCRAFT_DIR_NORTH:
			AircraftSpr.v += AircraftSpr.w;
			AircraftSpr.attribute |= H_FLIP;
		break;

		case AIRCRAFT_DIR_SOUTH:
			AircraftSpr.v += 0;
			AircraftSpr.attribute |= H_FLIP;
		break;

		case AIRCRAFT_DIR_EAST:
			AircraftSpr.v += 0;
			AircraftSpr.attribute &= ~(H_FLIP);
		break;

		case AIRCRAFT_DIR_WEST:
			AircraftSpr.v += AircraftSpr.w;
			AircraftSpr.attribute &= ~(H_FLIP);
		break;

		case AIRCRAFT_DIR_NO_DIRECTION:
			// Fall through
		default:
		break;
	}
}

void AircraftAttitude(TYPE_AIRCRAFT_DATA* ptrAircraft)
{
	if(ptrAircraft->State == STATE_FINAL)
	{
		if(ptrAircraft->IsoPos.z > 0)
		{
			ptrAircraft->IsoPos.z -= AircraftSpeedsTable[AIRCRAFT_SPEED_FINAL_Z];
		}
	}
}

TYPE_ISOMETRIC_POS AircraftGetIsoPos(uint8_t FlightDataIdx)
{
	// Aircraft position data is stored in fix16_t data type instead of "short" data type.
	// So we must perform a conversion first for convenience.
	TYPE_ISOMETRIC_POS retIsoPos;
	TYPE_ISOMETRIC_FIX16_POS fix16IsoPos = AircraftFromFlightDataIndex(FlightDataIdx)->IsoPos;

	retIsoPos.x = (short)fix16_to_int(fix16IsoPos.x);
	retIsoPos.y = (short)fix16_to_int(fix16IsoPos.y);
	retIsoPos.z = (short)fix16_to_int(fix16IsoPos.z);

	return retIsoPos;
}

void AircraftAddTargets(TYPE_AIRCRAFT_DATA* ptrAircraft, uint16_t* targets)
{
	memcpy(ptrAircraft->Target, targets, sizeof(uint16_t) * AIRCRAFT_MAX_TARGETS);
	ptrAircraft->TargetIdx = 0;
}

uint16_t AircraftGetTileFromFlightDataIndex(uint8_t index)
{
	TYPE_ISOMETRIC_POS isoPos = AircraftGetIsoPos(index);

	if(AircraftFromFlightDataIndex(index)->State != STATE_IDLE)
	{
		return GameGetTileFromIsoPosition(&isoPos);
	}
	else
	{
		return 0;
	}
}

TYPE_AIRCRAFT_DATA* AircraftFromFlightDataIndex(uint8_t index)
{
	uint8_t i;
	TYPE_AIRCRAFT_DATA* ptrAircraft;

	for(i = 0; i < GAME_MAX_AIRCRAFT; i++)
	{
		ptrAircraft = &AircraftData[i];

		if(ptrAircraft->FlightDataIdx == index)
		{
			return ptrAircraft;
		}
	}

	return NULL;
}

void AircraftFromFlightDataIndexAddTargets(uint8_t index, uint16_t* targets)
{
	AircraftAddTargets(AircraftFromFlightDataIndex(index), targets);
}

AIRCRAFT_DIRECTION AircraftGetDirection(TYPE_AIRCRAFT_DATA* ptrAircraft)
{
	return ptrAircraft->Direction;
}

uint16_t* AircraftGetTargets(uint8_t index)
{
	TYPE_AIRCRAFT_DATA* ptrAircraft = AircraftFromFlightDataIndex(index);
	
	return ptrAircraft->Target;
}

uint8_t AircraftGetTargetIdx(uint8_t index)
{
	TYPE_AIRCRAFT_DATA* ptrAircraft = AircraftFromFlightDataIndex(index);
	
	return ptrAircraft->TargetIdx;
}

bool AircraftMoving(uint8_t index)
{
	TYPE_AIRCRAFT_DATA* ptrAircraft = AircraftFromFlightDataIndex(index);
	
	return (bool)ptrAircraft->Speed;
}

bool AircraftCheckCollision(TYPE_AIRCRAFT_DATA* ptrRefAircraft, TYPE_AIRCRAFT_DATA* ptrOtherAircraft)
{
// Here I have used an old macro that I found on nextvolume's source code for "A Small Journey", IIRC.
// Totally fool-proof, so I dint' want to complicate things!
#define check_bb_collision(x1,y1,w1,h1,x2,y2,w2,h2) (!( ((x1)>=(x2)+(w2)) || ((x2)>=(x1)+(w1)) || \
														((y1)>=(y2)+(h2)) || ((y2)>=(y1)+(h1)) ))

	if(check_bb_collision(	ptrRefAircraft->IsoPos.x,
							ptrRefAircraft->IsoPos.y,
							AIRCRAFT_SIZE_FIX16,
							AIRCRAFT_SIZE_FIX16,
							ptrOtherAircraft->IsoPos.x,
							ptrOtherAircraft->IsoPos.y,
							AIRCRAFT_SIZE_FIX16,
							AIRCRAFT_SIZE_FIX16			) != 0)
	{
		if(ptrRefAircraft->IsoPos.z == ptrOtherAircraft->IsoPos.z)
		{
			return true;
		}
	}

	return false;
}
