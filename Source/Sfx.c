/* *************************************
 * 	Includes
 * *************************************/
#include "Sfx.h"

/* *************************************
 * 	Defines
 * *************************************/
#define MAX_VOLUME SPU_MAXVOL
#define NUMBER_OF_VOICES 24

/* *************************************
 * 	Local Prototypes
 * *************************************/

/* *************************************
 * 	Local Variables
 * *************************************/
static uint16_t SfxGlobalVolumeReduction;
static bool usedVoices[NUMBER_OF_VOICES];

#ifndef NO_CDDA
static uint16_t SfxCddaVolumeReduction;
#endif // NO_CDDA

void SfxPlaySound(SsVag* sound)
{
    if (sound->data_size != 0)
    {
        SsPlayVag(sound, sound->cur_voice, MAX_VOLUME - SfxGlobalVolumeReduction, MAX_VOLUME - SfxGlobalVolumeReduction);
    }
}

bool SfxUploadSound_Ex(const char* file_path, SsVag* vag, uint8_t voiceIndex)
{
#ifdef PSXSDK_DEBUG
	static size_t SPUBytesUsed;
#endif // PSXSDK_DEBUG

	if (voiceIndex >= NUMBER_OF_VOICES)
	{
		Serial_printf(	"Invalid input voice index %d. Only indexes [%d-%d] are allowed.\n",
						voiceIndex,
						0,
						NUMBER_OF_VOICES - 1	);
		return false;
	}

	if (usedVoices[voiceIndex])
	{
		Serial_printf("Voice number %d is already being used.\n", voiceIndex);
		return false;
	}

	if (SystemLoadFile(file_path) == false)
	{
		return false;
	}

	SsReadVag(vag, SystemGetBufferAddress());

	SsUploadVag(vag);

	vag->cur_voice = voiceIndex;

	usedVoices[voiceIndex] = true;

#ifdef PSXSDK_DEBUG

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

#endif // PSXSDK_DEBUG

	return true;
}

bool SfxUploadSound(const char* file_path, SsVag* vag)
{
	bool success = false;
	uint8_t i;

    for (i = 0; i < NUMBER_OF_VOICES; i++)
    {
		if (usedVoices[i] == false)
		{
			success = true;
			break;
		}
	}

	if (success == false)
	{
		Serial_printf("Could not find any free SPU slot.\n");
		return false;
	}

	return SfxUploadSound_Ex(file_path, vag, i);
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
