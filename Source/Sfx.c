/* *************************************
 * 	Includes
 * *************************************/

#include "Sfx.h"

/* *************************************
 * 	Defines
 * *************************************/

#define MAX_VOLUME SPU_MAXVOL
#define SILENT 0

#define NUMBER_OF_VOICES 24

/* *************************************
 * 	Local Prototypes
 * *************************************/

/* *************************************
 * 	Local Variables
 * *************************************/

static uint8_t voiceIndex;
static uint16_t SfxGlobalVolumeReduction;

#ifndef NO_CDDA
static uint16_t SfxCddaVolumeReduction;
#endif // NO_CDDA

void SfxPlaySound(SsVag * sound)
{
	SsPlayVag(sound, sound->cur_voice, MAX_VOLUME - SfxGlobalVolumeReduction, MAX_VOLUME - SfxGlobalVolumeReduction);
}

bool SfxUploadSound(char* file_path, SsVag * vag)
{
	if(SystemLoadFile(file_path) == false)
	{
		return false;
	}
	 
	if(voiceIndex < NUMBER_OF_VOICES)
	{
		SsReadVag(vag,SystemGetBufferAddress());
		
		SsUploadVag(vag);
		
		vag->cur_voice = voiceIndex;
        
		voiceIndex++;

        Serial_printf("SPU voices used = %d\n", voiceIndex);
	}
	else
	{
		Serial_printf("Maximum number of SPU voices exceeded!\n");
		return false; //Maximum voices exceeded
	}
	
	return true;
}

void SfxPlayTrack(MUSIC_TRACKS track)
{
#ifndef NO_CDDA
	SsCdVol(0x7FFF - SfxCddaVolumeReduction,0x7FFF - SfxCddaVolumeReduction);
	SsEnableCd();
	CdPlayTrack(track);
	Serial_printf("Track number %d playing...\n",track);
#endif // NO_CDDA
}

void SfxStopMusic(void)
{
#ifndef NO_CDDA
	uint64_t timer = SystemGetGlobalTimer();
	uint16_t CDVol = 0x7FFF;
	uint8_t time_step = 5;
		
	while (CDVol > 0x3F)
	{
		CDVol>>=1;
		SsCdVol(CDVol,CDVol);
		
		while(SystemGetGlobalTimer() < (timer + time_step) );
		
		timer = SystemGetGlobalTimer();
	}
	
	CdSendCommand(CdlMute,0);
#endif
}
