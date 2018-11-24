/* *************************************
 *  Includes
 * *************************************/

#include "Aircraft.h"
#include "System.h"
#include "Game.h"
#include "Camera.h"
#include "LoadMenu.h"

/* *************************************
 *  Defines
 * *************************************/

#define AIRCRAFT_SIZE               16
#define AIRCRAFT_SIZE_FIX16         fix16_from_int(AIRCRAFT_SIZE)
#define AIRCRAFT_INVALID_IDX        0xFF

/* *************************************
 *  Structs and enums
 * *************************************/

enum
{
    AIRCRAFT_SPRITE_W = 24,
    AIRCRAFT_SPRITE_H = 16,
    AIRCRAFT_SPRITE_VRAM_X = 800,
    AIRCRAFT_SPRITE_VRAM_Y = 256,
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
 *  Local variables
 * *************************************/

static TYPE_AIRCRAFT_DATA AircraftData[GAME_MAX_AIRCRAFT];
static uint8_t aircraftIndex;
static GsSprite AircraftSpr;
static GsSprite UpDownArrowSpr;
static GsSprite LeftRightArrowSpr;
static TYPE_ISOMETRIC_POS AircraftCenterIsoPos;
static TYPE_CARTESIAN_POS AircraftCenterPos;
static char* AircraftLiveryNamesTable[] = {"PHX", NULL};
static AIRCRAFT_LIVERY AircraftLiveryTable[] = {AIRCRAFT_LIVERY_0, AIRCRAFT_LIVERY_UNKNOWN};

static const char* GameFileList[] = {   "DATA\\SPRITES\\UDNARROW.TIM",
                                        "DATA\\SPRITES\\LFRARROW.TIM"    };

static void* GameFileDest[] = { (GsSprite*)&UpDownArrowSpr,
                                (GsSprite*)&LeftRightArrowSpr       };

// Used to quickly link FlightData indexes against AircraftData indexes.
static uint8_t flightDataIdxTable[GAME_MAX_AIRCRAFT];

static const fix16_t AircraftSpeedsTable[] = {  [AIRCRAFT_SPEED_IDLE] = 0,
                                                [AIRCRAFT_SPEED_GROUND] = 0x9999,
                                                [AIRCRAFT_SPEED_TAKEOFF] = 0x20000,
                                                [AIRCRAFT_SPEED_FINAL] = 0x10000,
                                                [AIRCRAFT_SPEED_FINAL_Z] = 0x4000   };

/* *************************************
 *  Local prototypes
 * *************************************/

static void AircraftDirection(TYPE_AIRCRAFT_DATA* const ptrAircraft);
static AIRCRAFT_LIVERY AircraftLiveryFromFlightNumber(char* strFlightNumber);
static void AircraftAttitude(TYPE_AIRCRAFT_DATA* const ptrAircraft);
static void AircraftUpdateSpriteFromData(TYPE_AIRCRAFT_DATA* const ptrAircraft);
static void AircraftSpeed(TYPE_AIRCRAFT_DATA* const ptrAircraft);
static bool AircraftCheckCollision(const TYPE_AIRCRAFT_DATA* const ptrRefAircraft, const TYPE_AIRCRAFT_DATA* const ptrOtherAircraft);
static bool AircraftCheckPath(TYPE_AIRCRAFT_DATA* ptrAicraft, TYPE_AIRCRAFT_DATA* ptrOtherAircraft);

void AircraftInit(void)
{
    static bool initialised;

    bzero(AircraftData, GAME_MAX_AIRCRAFT * sizeof (TYPE_AIRCRAFT_DATA));
    aircraftIndex = 0;

    AircraftSpr.x = 0;
    AircraftSpr.y = 0;

    AircraftSpr.attribute = COLORMODE(COLORMODE_8BPP);

    AircraftSpr.cx = PHX_LIVERY_CLUT_X;
    AircraftSpr.cy = PHX_LIVERY_CLUT_Y;

    AircraftSpr.w = AIRCRAFT_SPRITE_W;
    AircraftSpr.h = AIRCRAFT_SPRITE_H;

    GfxTPageOffsetFromVRAMPosition(&AircraftSpr, AIRCRAFT_SPRITE_VRAM_X, AIRCRAFT_SPRITE_VRAM_Y);

    AircraftCenterIsoPos.x = AIRCRAFT_SIZE >> 1;
    AircraftCenterIsoPos.y = AIRCRAFT_SIZE >> 1;
    AircraftCenterIsoPos.z = 0;

    AircraftCenterPos = GfxIsometricToCartesian(&AircraftCenterIsoPos);

    memset(flightDataIdxTable, AIRCRAFT_INVALID_IDX, sizeof (flightDataIdxTable));

    if (initialised == false)
    {
        initialised = true;

        LoadMenu(   GameFileList,
                    GameFileDest,
                    sizeof (GameFileList) / sizeof (GameFileList[0]),
                    sizeof (GameFileDest) / sizeof (GameFileDest[0])  );
    }
}

bool AircraftAddNew(    TYPE_FLIGHT_DATA* const ptrFlightData,
                        uint8_t FlightDataIndex,
                        uint16_t* targets,
                        DIRECTION direction     )
{
    if (aircraftIndex < GAME_MAX_AIRCRAFT)
    {
        TYPE_AIRCRAFT_DATA* const ptrAircraft = &AircraftData[aircraftIndex];

        memcpy(ptrAircraft->Target, targets, sizeof (uint16_t) * AIRCRAFT_MAX_TARGETS);

        ptrAircraft->TargetIdx = 0;
        ptrAircraft->Livery = AircraftLiveryFromFlightNumber(ptrFlightData->strFlightNumber[FlightDataIndex]);

        ptrAircraft->FlightDataIdx = FlightDataIndex;

        Serial_printf("ptrAircraft->FlightDataIdx = %d, FlightDataIndex = %d\n", ptrAircraft->FlightDataIdx, FlightDataIndex);

        if (ptrFlightData->FlightDirection[FlightDataIndex] == ARRIVAL)
        {
            const uint8_t level_columns = GameGetLevelColumns();

            switch (direction)
            {
                case DIR_EAST:
                    ptrAircraft->IsoPos.x = 0;

                    ptrAircraft->IsoPos.y = targets[0] / level_columns;
                    ptrAircraft->IsoPos.y <<= TILE_SIZE_BIT_SHIFT;
                    ptrAircraft->IsoPos.y += TILE_SIZE >> 1; // Adjust to tile center
                    ptrAircraft->IsoPos.y = fix16_from_int(ptrAircraft->IsoPos.y);

                    ptrAircraft->IsoPos.z = targets[0] % level_columns;
                    ptrAircraft->IsoPos.z <<= TILE_SIZE_BIT_SHIFT - 1;
                    ptrAircraft->IsoPos.z = fix16_from_int(ptrAircraft->IsoPos.z);
                break;

                case DIR_SOUTH:
                    ptrAircraft->IsoPos.x = targets[0] % level_columns;
                    ptrAircraft->IsoPos.x <<= TILE_SIZE_BIT_SHIFT;
                    ptrAircraft->IsoPos.x += TILE_SIZE >> 1; // Adjust to tile center
                    ptrAircraft->IsoPos.x = fix16_from_int(ptrAircraft->IsoPos.x);

                    ptrAircraft->IsoPos.y = 0;

                    ptrAircraft->IsoPos.z = targets[0] / level_columns;
                    ptrAircraft->IsoPos.z <<= TILE_SIZE_BIT_SHIFT - 1;
                    ptrAircraft->IsoPos.z = fix16_from_int(ptrAircraft->IsoPos.z);
                break;

                case NO_DIRECTION:
                    // Fall through
                default:
                    Serial_printf("Invalid runway direction %d for inbound flight.\n", direction);
                return false;
            }
        }
        else if (ptrFlightData->FlightDirection[FlightDataIndex] == DEPARTURE)
        {
            if (direction == NO_DIRECTION)
            {
                Serial_printf("Invalid direction for outbound flight.\n");
                return false;
            }

            ptrAircraft->IsoPos.x = GameGetXFromTile(ptrFlightData->Parking[FlightDataIndex]);
            ptrAircraft->IsoPos.y = GameGetYFromTile(ptrFlightData->Parking[FlightDataIndex]);
            ptrAircraft->IsoPos.z = 0;
        }

        ptrAircraft->Direction = direction;

        ptrAircraft->State = ptrFlightData->State[FlightDataIndex];
        flightDataIdxTable[FlightDataIndex] = aircraftIndex;

        Serial_printf("\nAircraft Data:\n");
        Serial_printf("\tTargets:");

        {
            uint8_t i;

            for (i = 0; i < AIRCRAFT_MAX_TARGETS; i++)
            {
                if (ptrAircraft->Target[i] == 0)
                {
                    break;
                }

                Serial_printf(" %d", ptrAircraft->Target[i]);
            }
        }

        Serial_printf("\n\tDirection: %d\n", ptrAircraft->Direction);

        Serial_printf("\nLivery: %d\n", ptrAircraft->Livery );

        Serial_printf("Aircraft position: {%d, %d, %d}\n",
                fix16_to_int(ptrAircraft->IsoPos.x),
                fix16_to_int(ptrAircraft->IsoPos.y),
                fix16_to_int(ptrAircraft->IsoPos.z) );

        aircraftIndex++;

        return true;
    }
    else
    {
        Serial_printf("Exceeded maximum aircraft capacity!\n");
    }

    return false;
}

static AIRCRAFT_LIVERY AircraftLiveryFromFlightNumber(char* strFlightNumber)
{
    int32_t liveryIndex;
    char strLivery[4];

    memset(strLivery, 0, 4 * sizeof (char) );

    strncpy(strLivery, strFlightNumber, 3);

    liveryIndex = SystemIndexOfStringArray(strLivery, AircraftLiveryNamesTable);

    if (liveryIndex == -1)
    {
        return AIRCRAFT_LIVERY_UNKNOWN;
    }

    return AircraftLiveryTable[liveryIndex];
}

bool AircraftRemove(uint8_t aircraftIdx)
{
    if (aircraftIdx != AIRCRAFT_INVALID_IDX)
    {
        TYPE_AIRCRAFT_DATA* const ptrAircraft = AircraftFromFlightDataIndex(aircraftIdx);

        if (ptrAircraft->State != STATE_IDLE)
        {
            if (ptrAircraft->FlightDataIdx == aircraftIdx)
            {
                ptrAircraft->State = STATE_IDLE;
                Serial_printf("Flight %d removed\n", ptrAircraft->FlightDataIdx);
                return true;
            }
        }
    }

    return false;
}

void AircraftHandler(void)
{
    uint8_t i;

    for (i = 0; i < GAME_MAX_AIRCRAFT; i++)
    {
        TYPE_AIRCRAFT_DATA* const ptrAircraft = &AircraftData[i];
        bool collision_warning = false;

        if (ptrAircraft->State != STATE_IDLE)
        {
            uint8_t j;

            AircraftDirection(ptrAircraft);
            AircraftAttitude(ptrAircraft);
            AircraftSpeed(ptrAircraft);

            // Check collision against all other aircraft.
            for (j = 0; j < GAME_MAX_AIRCRAFT; j++)
            {
                TYPE_AIRCRAFT_DATA* ptrOtherAircraft = &AircraftData[j];

                if (ptrOtherAircraft->State == STATE_IDLE)
                {
                    continue;
                }

                if (i == j)
                {
                    continue;
                }
                else
                {
                    // Check whether aircraft should stop in order to avoid collision against
                    // other aircraft.
                    // WARNING: only STATE_TAXIING can be used to automatically stop an aircraft
                    // when calling GameStopFlight() or GameResumeFlightFromAutoStop().
                    collision_warning |= AircraftCheckPath(ptrAircraft, ptrOtherAircraft);
                }

                if (j > i)
                {
                    if (AircraftCheckCollision(ptrAircraft, ptrOtherAircraft))
                    {
                        GameAircraftCollision(ptrAircraft->FlightDataIdx);
                        break;
                    }
                }
                else
                {
                    // Collision already calculated.
                }

            }

            if (collision_warning)
            {
                GameStopFlight(ptrAircraft->FlightDataIdx);
            }
            else
            {
                GameResumeFlightFromAutoStop(ptrAircraft->FlightDataIdx);
            }

            ptrAircraft->State = GameGetFlightDataStateFromIdx(ptrAircraft->FlightDataIdx);
        }
    }
}

static bool AircraftCheckPath(TYPE_AIRCRAFT_DATA* const ptrAircraft, TYPE_AIRCRAFT_DATA* ptrOtherAircraft)
{
    const uint16_t currentTile = AircraftGetTileFromFlightDataIndex(ptrAircraft->FlightDataIdx);
    uint16_t nextTile; // Keep compiler happy

    switch (ptrAircraft->Direction)
    {
        case DIR_EAST:
            nextTile = currentTile + 1;
        break;

        case DIR_WEST:
            nextTile = currentTile - 1;
        break;

        case DIR_NORTH:
            nextTile = currentTile - GameGetLevelColumns();
        break;

        case DIR_SOUTH:
            nextTile = currentTile + GameGetLevelColumns();
        break;

        case NO_DIRECTION:
            // Fall through
        default:
            Serial_printf("AircraftCheckPath: Undefined direction\n");
        return false;
    }

    {
        const uint16_t otherAircraft_currentTile = AircraftGetTileFromFlightDataIndex(ptrOtherAircraft->FlightDataIdx);

        if (    (otherAircraft_currentTile == nextTile)
                            ||
                (otherAircraft_currentTile == currentTile)  )
        {
            if (ptrOtherAircraft->Speed == 0)
            {
                // Make ptrAircraft stop if ptrOtherAircraft is nearby and not moving!
                return true;
            }
        }
    }

    return false;
}

static void AircraftSpeed(TYPE_AIRCRAFT_DATA* const ptrAircraft)
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

        case STATE_USER_STOPPED:
            // Fall through
        case STATE_AUTO_STOPPED:
            // Fall through
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

void AircraftRender(TYPE_PLAYER* const ptrPlayer, uint8_t aircraftIdx)
{
    if (aircraftIdx != AIRCRAFT_INVALID_IDX)
    {
        TYPE_AIRCRAFT_DATA* const ptrAircraft = AircraftFromFlightDataIndex(aircraftIdx);

        if (ptrAircraft != NULL)
        {
            if (ptrAircraft->State != STATE_IDLE)
            {
                AircraftUpdateSpriteFromData(ptrAircraft);

                if (ptrAircraft->IsoPos.z > 0)
                {
                    // Draw aircraft shadow
                    TYPE_ISOMETRIC_FIX16_POS shadowIsoPos;

                    shadowIsoPos.x = ptrAircraft->IsoPos.x;
                    shadowIsoPos.y = ptrAircraft->IsoPos.y;
                    shadowIsoPos.z = 0;

                    const TYPE_CARTESIAN_POS shadowCartPos = GfxIsometricFix16ToCartesian(&shadowIsoPos);

                    // Aircraft position is referred to aircraft center
                    AircraftSpr.x = shadowCartPos.x - (AircraftSpr.w >> 1);
                    AircraftSpr.y = shadowCartPos.y - (AircraftSpr.h >> 1);

                    CameraApplyCoordinatesToSprite(ptrPlayer, &AircraftSpr);

                    AircraftSpr.r = 0;
                    AircraftSpr.g = 0;
                    AircraftSpr.b = 0;

                    GfxSortSprite(&AircraftSpr);
                }

                const TYPE_CARTESIAN_POS cartPos = GfxIsometricFix16ToCartesian(&ptrAircraft->IsoPos);

                // Aircraft position is referred to aircraft center
                AircraftSpr.x = cartPos.x - (AircraftSpr.w >> 1);
                AircraftSpr.y = cartPos.y - (AircraftSpr.h >> 1);

                CameraApplyCoordinatesToSprite(ptrPlayer, &AircraftSpr);

                if ((ptrPlayer->FlightDataSelectedAircraft == aircraftIdx)
                                &&
                    (ptrPlayer->ShowAircraftData))
                {
                    static uint8_t aircraft_sine;
                    static bool aircraft_sine_decrease;

                    if (aircraft_sine_decrease == false)
                    {
                        if (aircraft_sine < 240)
                        {
                            aircraft_sine += 24;
                        }
                        else
                        {
                            aircraft_sine_decrease = true;
                        }
                    }
                    else
                    {
                        if (aircraft_sine > 24)
                        {
                            aircraft_sine -= 24;
                        }
                        else
                        {
                            aircraft_sine_decrease = false;
                        }
                    }

                    AircraftSpr.r = NORMAL_LUMINANCE >> 2;
                    AircraftSpr.g = NORMAL_LUMINANCE >> 2;
                    AircraftSpr.b = aircraft_sine;

                    if (GfxIsSpriteInsideScreenArea(&AircraftSpr) == false)
                    {
                        bool showLRArrow = false;
                        bool showUPDNArrow = false;
                        // When aircraft can't be shown on screen,
                        // show an arrow indicating its position.

                        if (AircraftSpr.x < 0)
                        {
                            LeftRightArrowSpr.x = 0;
                            LeftRightArrowSpr.attribute |= H_FLIP;
                            showLRArrow = true;
                        }
                        else if (AircraftSpr.x > X_SCREEN_RESOLUTION)
                        {
                            LeftRightArrowSpr.x = X_SCREEN_RESOLUTION - (LeftRightArrowSpr.w << 1);
                            LeftRightArrowSpr.attribute &= ~(H_FLIP);
                            showLRArrow = true;
                        }
                        else if (AircraftSpr.y < 0)
                        {
                            UpDownArrowSpr.y = 0;
                            UpDownArrowSpr.attribute &= ~(V_FLIP);
                            showUPDNArrow = true;
                        }
                        else if (AircraftSpr.y > Y_SCREEN_RESOLUTION)
                        {
                            UpDownArrowSpr.y = Y_SCREEN_RESOLUTION - (UpDownArrowSpr.h);
                            UpDownArrowSpr.attribute |= V_FLIP;
                            showUPDNArrow = true;
                        }

                        if (showLRArrow)
                        {
                            LeftRightArrowSpr.y = AircraftSpr.y;

                            // First, saturate calculated Y values to {0, Y_SCREEN_RESOLUTION - LeftRightArrowSpr.h}.
                            if (LeftRightArrowSpr.y < 0)
                            {
                                LeftRightArrowSpr.y = 0;
                            }
                            else if (LeftRightArrowSpr.y > (Y_SCREEN_RESOLUTION - LeftRightArrowSpr.h) )
                            {
                                LeftRightArrowSpr.y = (Y_SCREEN_RESOLUTION - LeftRightArrowSpr.h);
                            }

                            GfxSortSprite(&LeftRightArrowSpr);
                        }
                        else if (showUPDNArrow)
                        {
                            UpDownArrowSpr.x = AircraftSpr.x;

                            // First, saturate calculated Y values to {0, Y_SCREEN_RESOLUTION - UpDownArrowSpr.h}.
                            if (UpDownArrowSpr.x < 0)
                            {
                                UpDownArrowSpr.x = 0;
                            }
                            else if (UpDownArrowSpr.x > (X_SCREEN_RESOLUTION - (UpDownArrowSpr.w << 1) ) )
                            {
                                UpDownArrowSpr.x = (UpDownArrowSpr.w << 1);
                            }

                            GfxSortSprite(&UpDownArrowSpr);
                        }
                    }

                }
                else
                {
                    AircraftSpr.r = NORMAL_LUMINANCE;
                    AircraftSpr.g = NORMAL_LUMINANCE;
                    AircraftSpr.b = NORMAL_LUMINANCE;
                }

                GfxSortSprite(&AircraftSpr);
            }
        }
    }
}

static void AircraftDirection(TYPE_AIRCRAFT_DATA* const ptrAircraft)
{
    TYPE_ISOMETRIC_FIX16_POS targetPos;

    if (ptrAircraft->State != STATE_CLIMBING)
    {
        if (ptrAircraft->Target[ptrAircraft->TargetIdx] == 0)
        {
            return;
        }

        targetPos.x = GameGetXFromTile(ptrAircraft->Target[ptrAircraft->TargetIdx]);
        targetPos.y = GameGetYFromTile(ptrAircraft->Target[ptrAircraft->TargetIdx]);
        targetPos.z = 0;

        ptrAircraft->TargetReached = false;

        if (targetPos.y == ptrAircraft->IsoPos.y)
        {
            if (targetPos.x > ptrAircraft->IsoPos.x)
            {
                if (targetPos.x <= (ptrAircraft->IsoPos.x + ptrAircraft->Speed) )
                {
                    ptrAircraft->TargetReached = true;
                }
                else
                {
                    ptrAircraft->Direction = DIR_EAST;
                    ptrAircraft->IsoPos.x += ptrAircraft->Speed;
                }
            }
            else if (targetPos.x < ptrAircraft->IsoPos.x)
            {
                if (targetPos.x >= (ptrAircraft->IsoPos.x - ptrAircraft->Speed) )
                {
                    ptrAircraft->TargetReached = true;
                }
                else
                {
                    ptrAircraft->Direction = DIR_WEST;
                    ptrAircraft->IsoPos.x -= ptrAircraft->Speed;
                }
            }
            else
            {
                ptrAircraft->TargetReached = true;
            }
        }
        else if (targetPos.x == ptrAircraft->IsoPos.x)
        {
            if (targetPos.y > ptrAircraft->IsoPos.y)
            {
                if (targetPos.y <= (ptrAircraft->IsoPos.y + ptrAircraft->Speed) )
                {
                    ptrAircraft->TargetReached = true;
                }
                else
                {
                    ptrAircraft->Direction = DIR_SOUTH;
                    ptrAircraft->IsoPos.y += ptrAircraft->Speed;
                }
            }
            else if (targetPos.y < ptrAircraft->IsoPos.y)
            {
                if (targetPos.y >= (ptrAircraft->IsoPos.y - ptrAircraft->Speed) )
                {
                    ptrAircraft->TargetReached = true;
                }
                else
                {
                    ptrAircraft->Direction = DIR_NORTH;
                    ptrAircraft->IsoPos.y -= ptrAircraft->Speed;
                }
            }
            else
            {
                ptrAircraft->TargetReached = true;
            }
        }

        if (ptrAircraft->TargetReached)
        {
            ptrAircraft->IsoPos.x = targetPos.x;
            ptrAircraft->IsoPos.y = targetPos.y;

            if (ptrAircraft->Target[++ptrAircraft->TargetIdx] == 0)
            {
                Serial_printf("All targets reached!\n");
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
            case DIR_EAST:
                ptrAircraft->IsoPos.x += ptrAircraft->Speed;
            break;

            case DIR_WEST:
                ptrAircraft->IsoPos.x -= ptrAircraft->Speed;
            break;

            case DIR_NORTH:
                ptrAircraft->IsoPos.y -= ptrAircraft->Speed;
            break;

            case DIR_SOUTH:
                ptrAircraft->IsoPos.y += ptrAircraft->Speed;
            break;

            case NO_DIRECTION:
                // Fall through
            default:
            return;
        }

        ptrAircraft->IsoPos.z += AircraftSpeedsTable[AIRCRAFT_SPEED_FINAL_Z];

        if (GameInsideLevelFromIsoPos(&ptrAircraft->IsoPos) == false)
        {
            GameRemoveFlight(ptrAircraft->FlightDataIdx, true);

            // Deactivate TYPE_AIRCRAFT instance.
            ptrAircraft->State = STATE_IDLE;
        }
    }
}

static void AircraftUpdateSpriteFromData(TYPE_AIRCRAFT_DATA* const ptrAircraft)
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
            Serial_printf("Unknown livery %d!\n", ptrAircraft->Livery);
        break;
    }

    // Reset TPAGE and {U, V} offset first.
    GfxTPageOffsetFromVRAMPosition(&AircraftSpr, AIRCRAFT_SPRITE_VRAM_X, AIRCRAFT_SPRITE_VRAM_Y);

    switch(ptrAircraft->Direction)
    {
        case DIR_NORTH:
            AircraftSpr.v += AircraftSpr.h;
            AircraftSpr.attribute |= H_FLIP;
        break;

        case DIR_SOUTH:
            AircraftSpr.v += 0;
            AircraftSpr.attribute |= H_FLIP;
        break;

        case DIR_EAST:
            AircraftSpr.v += 0;
            AircraftSpr.attribute &= ~(H_FLIP);
        break;

        case DIR_WEST:
            AircraftSpr.v += AircraftSpr.h;
            AircraftSpr.attribute &= ~(H_FLIP);
        break;

        case NO_DIRECTION:
            // Fall through
        default:
        break;
    }
}

static void AircraftAttitude(TYPE_AIRCRAFT_DATA* const ptrAircraft)
{
    if (ptrAircraft->State == STATE_FINAL)
    {
        if (ptrAircraft->IsoPos.z > 0)
        {
            ptrAircraft->IsoPos.z -= AircraftSpeedsTable[AIRCRAFT_SPEED_FINAL_Z];
        }
    }
}

TYPE_ISOMETRIC_POS AircraftGetIsoPos(const uint8_t FlightDataIdx)
{
    TYPE_ISOMETRIC_POS retIsoPos = {0};

    if (FlightDataIdx != AIRCRAFT_INVALID_IDX)
    {
        TYPE_AIRCRAFT_DATA* const ptrAircraft = AircraftFromFlightDataIndex(FlightDataIdx);

        if (ptrAircraft != NULL)
        {
            // Aircraft position data is stored in fix16_t data type instead of "short" data type.
            // So we must perform a conversion first for convenience.
            const TYPE_ISOMETRIC_FIX16_POS fix16IsoPos = ptrAircraft->IsoPos;

            retIsoPos.x = (short)fix16_to_int(fix16IsoPos.x);
            retIsoPos.y = (short)fix16_to_int(fix16IsoPos.y);
            retIsoPos.z = (short)fix16_to_int(fix16IsoPos.z);
        }
    }

    return retIsoPos;
}

void AircraftAddTargets(TYPE_AIRCRAFT_DATA* const ptrAircraft, uint16_t* targets)
{
    memcpy(ptrAircraft->Target, targets, sizeof (uint16_t) * AIRCRAFT_MAX_TARGETS);
    ptrAircraft->TargetIdx = 0;
}

uint16_t AircraftGetTileFromFlightDataIndex(const uint8_t index)
{
    TYPE_AIRCRAFT_DATA* const ptrAircraft = AircraftFromFlightDataIndex(index);

    if (ptrAircraft != NULL)
    {
        if (ptrAircraft->State != STATE_IDLE)
        {
            TYPE_ISOMETRIC_POS isoPos = AircraftGetIsoPos(index);

            return GameGetTileFromIsoPosition(&isoPos);
        }
    }

    return 0;
}

TYPE_AIRCRAFT_DATA* AircraftFromFlightDataIndex(const uint8_t index)
{
    if ((index != AIRCRAFT_INVALID_IDX)
                &&
        (index < GAME_MAX_AIRCRAFT))
    {
        const uint8_t idx = flightDataIdxTable[index];

        if (idx != AIRCRAFT_INVALID_IDX)
        {
            return &AircraftData[idx];
        }
    }

    return NULL;
}

void AircraftFromFlightDataIndexAddTargets(uint8_t index, uint16_t* targets)
{
    TYPE_AIRCRAFT_DATA* const ptrAircraft = AircraftFromFlightDataIndex(index);

    if (ptrAircraft != NULL)
    {
        AircraftAddTargets(ptrAircraft, targets);
    }
}

DIRECTION AircraftGetDirection(TYPE_AIRCRAFT_DATA* const ptrAircraft)
{
    return ptrAircraft->Direction;
}

const uint16_t* AircraftGetTargets(uint8_t index)
{
    TYPE_AIRCRAFT_DATA* const ptrAircraft = AircraftFromFlightDataIndex(index);

    if (ptrAircraft != NULL)
    {
        return ptrAircraft->Target;
    }

    return NULL;
}

uint8_t AircraftGetTargetIdx(uint8_t index)
{
    TYPE_AIRCRAFT_DATA* const ptrAircraft = AircraftFromFlightDataIndex(index);

    if (ptrAircraft != NULL)
    {
        return ptrAircraft->TargetIdx;
    }

    return 0;
}

bool AircraftMoving(uint8_t index)
{
    TYPE_AIRCRAFT_DATA* const ptrAircraft = AircraftFromFlightDataIndex(index);

    if (ptrAircraft != NULL)
    {
        return (bool)ptrAircraft->Speed;
    }

    return false;
}

static bool AircraftCheckCollision(const TYPE_AIRCRAFT_DATA* const ptrRefAircraft, const TYPE_AIRCRAFT_DATA* const ptrOtherAircraft)
{
// Here I have used an old macro that I found on nextvolume's source code for "A Small Journey", IIRC.
// Totally fool-proof, so I dint' want to complicate things!
#define check_bb_collision(x1,y1,w1,h1,x2,y2,w2,h2) (!( ((x1)>=(x2)+(w2)) || ((x2)>=(x1)+(w1)) || \
                                                        ((y1)>=(y2)+(h2)) || ((y2)>=(y1)+(h1)) ))

    if (check_bb_collision( ptrRefAircraft->IsoPos.x,
                            ptrRefAircraft->IsoPos.y,
                            AIRCRAFT_SIZE_FIX16,
                            AIRCRAFT_SIZE_FIX16,
                            ptrOtherAircraft->IsoPos.x,
                            ptrOtherAircraft->IsoPos.y,
                            AIRCRAFT_SIZE_FIX16,
                            AIRCRAFT_SIZE_FIX16         ) != 0)
    {
        if (ptrRefAircraft->IsoPos.z == ptrOtherAircraft->IsoPos.z)
        {
            return true;
        }
    }

    return false;
#undef check_bb_collision
}
