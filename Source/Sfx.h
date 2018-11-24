#ifndef SFX_HEADER__
#define SFX_HEADER__

/* *************************************
 * 	Includes
 * *************************************/
#include "Global_Inc.h"
#include "System.h"

/* *************************************
 * 	Defines
 * *************************************/
 /* *************************************
 * 	Structs and enums
 * *************************************/
typedef enum
{
	INTRO_TRACK = 2,
	GAMEPLAY_TRACK1,
	GAMEPLAY_TRACK2,
	GAMEPLAY_FIRST_TRACK = GAMEPLAY_TRACK1,
	GAMEPLAY_LAST_TRACK = GAMEPLAY_TRACK2
}MUSIC_TRACKS;

/* *************************************
 * 	Global prototypes
 * *************************************/
void SfxPlaySound(SsVag* sound);
bool SfxUploadSound(const char* file_path, SsVag* vag);
void SfxPlayTrack(MUSIC_TRACKS track);
void SfxStopMusic(void);

#endif //SFX_HEADER__
