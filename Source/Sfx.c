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
    if (sound->data_size != 0)
    {
        SsPlayVag(sound, sound->cur_voice, MAX_VOLUME - SfxGlobalVolumeReduction, MAX_VOLUME - SfxGlobalVolumeReduction);
    }
}

bool SfxUploadSound(char* file_path, SsVag * vag)
{
    static size_t SPUBytesUsed;

	if (SystemLoadFile(file_path) == false)
	{
		return false;
	}

	if (voiceIndex < NUMBER_OF_VOICES)
	{
		SsReadVag(vag,SystemGetBufferAddress());

		SsUploadVag(vag);

		vag->cur_voice = voiceIndex;

		voiceIndex++;

        Serial_printf("SPU voices used = %d\n", voiceIndex);

        SPUBytesUsed += vag->data_size;

        if (SPUBytesUsed != 0)
        {
            enum
            {
                SPU_MAX_ALLOWED_BYTES = 512 * 1024 // 512 KBytes
            };

            uint16_t percentage = SPUBytesUsed * 100 / SPU_MAX_ALLOWED_BYTES;

            dprintf("SPU usage: %d%%\n", percentage);
        }
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
	enum
	{
		CD_MAX_VOLUME = (uint16_t)0x7FFF
	};

	SsCdVol(CD_MAX_VOLUME - SfxCddaVolumeReduction,
			CD_MAX_VOLUME - SfxCddaVolumeReduction);
	SsEnableCd();
	CdPlayTrack(track);
	Serial_printf("Track number %d playing...\n",track);
#endif // NO_CDDA
}

void SfxStopMusic(void)
{
#ifndef NO_CDDA
	uint16_t CDVol = 0x7FFF;

	while (CDVol > 0x003F)
	{
		CDVol >>= 1;
		SsCdVol(CDVol, CDVol);
	}

	CdSendCommand(CdlMute,0);
#endif // NO_CDDA
}
