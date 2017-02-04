/* *************************************
 * 	Includes
 * *************************************/

#include "PSXSDKIntro.h"

/* *************************************
 * 	Defines
 * *************************************/

/* **************************************
 * 	Structs and enums					*
 * *************************************/

enum
{
	PSX_W = 48,
	PSX_H = 32,
	PSX_U = 30,
	PSX_V = 0,
	PSX_X = (X_SCREEN_RESOLUTION >> 1) - (PSX_W >> 1),
	PSX_Y = (Y_SCREEN_RESOLUTION >> 1) - (PSX_H >> 1),
	
	DISK_X = PSX_X + 11,
	DISK_Y = PSX_Y + 2,
	DISK_W = 27,
	DISK_H = 27,
	DISK_U = 2,
	DISK_V = 2,
	MAX_DISK_SPIN = 5,
	
	READER_X = PSX_X + 9,
	READER_Y = PSX_Y + 1,
	
	CLOSED_READER_W = 30,
	CLOSED_READER_H = 30,
	CLOSED_READER_U = 0,
	CLOSED_READER_V = 30,
	
	OPEN_READER_W = CLOSED_READER_W,
	OPEN_READER_H = 15,
	OPEN_READER_U = 0,
	OPEN_READER_V = (CLOSED_READER_H << 1) + 1,
	
	TRANSPARENT_CLOSED_READER_W = CLOSED_READER_W,
	TRANSPARENT_CLOSED_READER_H = CLOSED_READER_H,
	TRANSPARENT_CLOSED_READER_U = CLOSED_READER_W,
	TRANSPARENT_CLOSED_READER_V = PSX_H,
	
	TRANSPARENT_OPEN_READER_W = OPEN_READER_W,
	TRANSPARENT_OPEN_READER_H = OPEN_READER_H,
	TRANSPARENT_OPEN_READER_U = OPEN_READER_W,
	TRANSPARENT_OPEN_READER_V = TRANSPARENT_CLOSED_READER_V + 
								TRANSPARENT_CLOSED_READER_H
};

enum
{
	CLOSE_SHELL_EV_TIM = 50,
	DISK_SPIN_EV_TIM = 30,
	TEXT_APPEAR_RANDOM_TIM = 100,
	TEXT_APPEAR_STRING_TIM = 100,
	INTRO_CLOSE_TIM = 100
};

enum
{
	FONT_COLUMNS = 5,
	FONT_SIZE_BITSHIFT = 4,
	FONT_SIZE = 16,
	FONT_TPAGE = 6,
	FONT_U = 0,
	FONT_V = 76,
	FONT_X = 64,
	FONT_Y = 144,
	FONT_X2 = 88,
	FONT_Y2 = 160
};

enum
{
	BG_LUMINANCE_TARGET = NORMAL_LUMINANCE,
	BG_LUMINANCE_STEP = 2,
};

enum
{
	GPL_LOGO_LUMINANCE_STEP = 1,
	GPL_LOGO_LUMINANCE_TARGET = NORMAL_LUMINANCE,
	GPL_LOGO_X = 16,
	GPL_LOGO_Y = 16
};

enum
{
	OPEN_SOURCE_LOGO_LUMINANCE_STEP = 1,
	OPEN_SOURCE_LOGO_LUMINANCE_TARGET = NORMAL_LUMINANCE,
	OPEN_SOURCE_LOGO_X = X_SCREEN_RESOLUTION - 64 - 16,
	OPEN_SOURCE_LOGO_Y = 16
};

/* *************************************
 * 	Local Prototypes
 * *************************************/

static void PSXSDKIntroDrawConsole(void);
static void PSXSDKIntroRunTimers(void);
static void PSXSDKIntroDrawDisk(void);
static void PSXSDKIntroDrawTransCase(void);
static void PSXSDKIntroDrawChar(short x, short y, char ch);

/* *************************************
 * 	Local variables
 * *************************************/

// Events
static bool PSXSDKIntroCloseShellEvent;
static bool PSXSDKIntroCloseShellEventReminder;
static bool PSXSDKIntroSpinDiskEvent;
static bool PSXSDKIntroSpinDiskEventReminder;
static bool PSXSDKIntroRandTextEvent;
static bool PSXSDKIntroRandTextEventReminder;
static bool PSXSDKIntroStringEvent;
static bool PSXSDKIntroClose;

// Text
static char * strPSXSDKIntro = {"MADE WITH PSXSDK"};
static char * strPSXSDKIntroAuthor = {"BY NEXTVOLUME"};

/* *************************************
 * 	Global variables
 * *************************************/

GsSprite PsxDisk;
GsSprite PSXSDKIntroFont;
GsSprite GPL_Logo;
GsSprite OpenSource_Logo;
SsVag TrayClSnd;
SsVag SpinDiskSnd;

void PSXSDKIntroDrawChar(short x, short y, char ch)
{
	PSXSDKIntroFont.w = FONT_SIZE;
	PSXSDKIntroFont.h = FONT_SIZE;
	PSXSDKIntroFont.tpage = FONT_TPAGE;
	
	if( (ch >= 'A') && (ch <= 'Z')	)
	{
		ch -= 'A'; // Reset offset
		
		PSXSDKIntroFont.x = x;
		PSXSDKIntroFont.y = y;
		
		PSXSDKIntroFont.u = FONT_U + ( (ch % FONT_COLUMNS) << FONT_SIZE_BITSHIFT);
		PSXSDKIntroFont.v = FONT_V + ((short)(ch / FONT_COLUMNS) << FONT_SIZE_BITSHIFT);
		
		GfxSortSprite(&PSXSDKIntroFont);
	}
	
	if(ch == ' ')
	{
		return;
	}
}

void PSXSDKIntro(void)
{
	int i;
	
	PSXSDKIntroCloseShellEvent = false;
	PSXSDKIntroCloseShellEventReminder = false;
	PSXSDKIntroSpinDiskEvent = false;
	PSXSDKIntroSpinDiskEventReminder = false;
	PSXSDKIntroRandTextEvent = false;
	PSXSDKIntroRandTextEventReminder = false;
	PSXSDKIntroStringEvent = false;
	PSXSDKIntroClose = false;
	
	GfxSetGlobalLuminance(NORMAL_LUMINANCE);
	
	while(1)
	{
		while(GfxIsGPUBusy() == true);
		
		if( (	(GfxGetGlobalLuminance() - BG_LUMINANCE_STEP) > 0)
						&&
					(PSXSDKIntroClose == true)		)
		{
			GfxIncreaseGlobalLuminance(-BG_LUMINANCE_STEP);
		}
		
		if(	(GfxGetGlobalLuminance() <= BG_LUMINANCE_STEP)
						&&
			(PSXSDKIntroClose == true)	)
		{
			break;
		}
		
		GsSortCls(0,0,0);
		
		for(i = 0; i < strlen(strPSXSDKIntro) ; i++)
		{
			if(	(PSXSDKIntroRandTextEvent == true)
					&&
				(PSXSDKIntroStringEvent == false) )
			{
				if(PSXSDKIntroRandTextEventReminder == false)
				{
					PSXSDKIntroRandTextEventReminder = true;
					SfxPlaySound(&SpinDiskSnd);
				}
				
				PSXSDKIntroDrawChar(FONT_X + (i << FONT_SIZE_BITSHIFT),FONT_Y,SystemRand('A','Z'));
				
				GPL_Logo.r = 0;
				GPL_Logo.g = 0;
				GPL_Logo.b = 0;
				
				OpenSource_Logo.r = 0;
				OpenSource_Logo.g = 0;
				OpenSource_Logo.b = 0;
			}
			else if(	(PSXSDKIntroRandTextEvent == true)
							&&
						(PSXSDKIntroStringEvent == true) )
			{				
				PSXSDKIntroDrawChar(FONT_X + (i << FONT_SIZE_BITSHIFT),FONT_Y,strPSXSDKIntro[i]);
				
				if(System100msTick() == true)
				{
					if(GPL_Logo.r < GPL_LOGO_LUMINANCE_TARGET)
					{
						GPL_Logo.r += GPL_LOGO_LUMINANCE_STEP;
						GPL_Logo.g += GPL_LOGO_LUMINANCE_STEP;
						GPL_Logo.b += GPL_LOGO_LUMINANCE_STEP;
					}
					
					GPL_Logo.x = GPL_LOGO_X;
					GPL_Logo.y = GPL_LOGO_Y;
					
					if(OpenSource_Logo.r < OPEN_SOURCE_LOGO_LUMINANCE_TARGET)
					{
						OpenSource_Logo.r += OPEN_SOURCE_LOGO_LUMINANCE_STEP;
						OpenSource_Logo.g += OPEN_SOURCE_LOGO_LUMINANCE_STEP;
						OpenSource_Logo.b += OPEN_SOURCE_LOGO_LUMINANCE_STEP;
					}
					
					OpenSource_Logo.x = OPEN_SOURCE_LOGO_X;
					OpenSource_Logo.y = OPEN_SOURCE_LOGO_Y;
				}
				
				GfxSortSprite(&GPL_Logo);
				GfxSortSprite(&OpenSource_Logo);
			}
		}
		
		for(i = 0; i < strlen(strPSXSDKIntroAuthor) ; i++)
		{
			if(	(PSXSDKIntroRandTextEvent == true)
					&&
				(PSXSDKIntroStringEvent == false) )
			{				
				PSXSDKIntroDrawChar(FONT_X2 + (i << FONT_SIZE_BITSHIFT),FONT_Y2,SystemRand('A','Z'));
			}
			else if(	(PSXSDKIntroRandTextEvent == true)
							&&
						(PSXSDKIntroStringEvent == true) )
			{
				PSXSDKIntroDrawChar(FONT_X2 + (i << FONT_SIZE_BITSHIFT),FONT_Y2,strPSXSDKIntroAuthor[i]);
			}
		}
		
		PSXSDKIntroDrawConsole();
		
		PSXSDKIntroDrawDisk();
		
		if(PSXSDKIntroCloseShellEvent == true)
		{
			if(PSXSDKIntroCloseShellEventReminder == false)
			{
				PSXSDKIntroCloseShellEventReminder = true;
				
				SfxPlaySound(&TrayClSnd);
			}
			
			PsxDisk.x = READER_X;
			PsxDisk.y = READER_Y;
			
			PsxDisk.w = CLOSED_READER_W;
			PsxDisk.h = CLOSED_READER_H;
			
			PsxDisk.u = CLOSED_READER_U;
			PsxDisk.v = CLOSED_READER_V;
			
			GfxSortSprite(&PsxDisk);
		}
		else
		{
			PsxDisk.x = READER_X;
			PsxDisk.y = READER_Y;
			
			PsxDisk.w = OPEN_READER_W;
			PsxDisk.h = OPEN_READER_H;
			
			PsxDisk.u = OPEN_READER_U;
			PsxDisk.v = OPEN_READER_V;
			
			GfxSortSprite(&PsxDisk);
		}
		
		PSXSDKIntroDrawTransCase();
		
		GfxDrawScene();
		
		PSXSDKIntroRunTimers();
	}
}

void PSXSDKIntroRunTimers(void)
{
	static uint16_t intro_timer = 0;
	
	intro_timer++;
	
	if(	(intro_timer >= CLOSE_SHELL_EV_TIM)
			&&
		(PSXSDKIntroCloseShellEvent == false)	)
	{
		PSXSDKIntroCloseShellEvent = true;
		intro_timer = 0;
	}
	
	if(	(intro_timer >= DISK_SPIN_EV_TIM)
			&&
		(PSXSDKIntroCloseShellEvent == true)
			&&
		(PSXSDKIntroSpinDiskEvent == false)	)
	{
		PSXSDKIntroSpinDiskEvent = true;
		intro_timer = 0;
	}
	
	if( (intro_timer >= TEXT_APPEAR_RANDOM_TIM)
			&&
		(PSXSDKIntroCloseShellEvent == true)
			&&
		(PSXSDKIntroSpinDiskEvent == true)
			&&
		(PSXSDKIntroRandTextEvent == false)	)
	{
		PSXSDKIntroRandTextEvent = true;
		intro_timer = 0;
	}
	
	if( (intro_timer >= TEXT_APPEAR_STRING_TIM)
			&&
		(PSXSDKIntroCloseShellEvent == true)
			&&
		(PSXSDKIntroSpinDiskEvent == true)
			&&
		(PSXSDKIntroRandTextEvent == true)
			&&
		(PSXSDKIntroStringEvent == false)	)
	{
		PSXSDKIntroStringEvent = true;
		SfxPlayTrack(INTRO_TRACK);
		intro_timer = 0;
	}
	
	if( (intro_timer >= INTRO_CLOSE_TIM)
			&&
		(PSXSDKIntroCloseShellEvent == true)
			&&
		(PSXSDKIntroSpinDiskEvent == true)
			&&
		(PSXSDKIntroRandTextEvent == true)
			&&
		(PSXSDKIntroStringEvent == true)	)
	{
		PSXSDKIntroClose = true;
		intro_timer = 0;
	}
	
	
		
}

void PSXSDKIntroDrawDisk(void)
{
	static int spin_rotate = 0;
	static int spin_speed = 0;
	
	PsxDisk.x = DISK_X;
	PsxDisk.y = DISK_Y;
	
	PsxDisk.w = DISK_W;
	PsxDisk.h = DISK_H;
	
	PsxDisk.u = DISK_U;
	PsxDisk.v = DISK_V;
	
	if(PSXSDKIntroSpinDiskEvent == true)
	{
		if(PSXSDKIntroSpinDiskEventReminder == false)
		{
			PSXSDKIntroSpinDiskEventReminder = true;
			
			//SfxPlaySound(&SpinDiskSnd);
		}
			
		if(spin_speed < MAX_DISK_SPIN)
		{
			spin_speed++;
		}
		
		if(spin_rotate < GfxRotateFromDegrees(360) )
		{
			spin_rotate += GfxRotateFromDegrees(spin_speed++);
		}
		else
		{
			spin_rotate = SystemRand(0,GfxRotateFromDegrees(360));
		}
				
		PsxDisk.rotate = spin_rotate;
		PsxDisk.mx = DISK_W >> 1;
		PsxDisk.my = DISK_H >> 1;
	}
	
	GfxSortSprite(&PsxDisk);
	
	PsxDisk.rotate = 0;
}

void PSXSDKIntroDrawTransCase(void)
{
	/*if(PSXSDKIntroCloseShellEvent == false)
	{
		PsxDisk.x = READER_X;
		PsxDisk.y = READER_Y;
		
		PsxDisk.w = TRANSPARENT_OPEN_READER_W;
		PsxDisk.h = TRANSPARENT_OPEN_READER_H;
		
		PsxDisk.u = TRANSPARENT_OPEN_READER_U;
		PsxDisk.v = TRANSPARENT_OPEN_READER_V;
		
		PsxDisk.attribute |= ENABLE_TRANS | TRANS_MODE(0);
		
		GfxSortSprite(&PsxDisk);
		
		PsxDisk.attribute &= ~(ENABLE_TRANS | TRANS_MODE(0) );
	}
	else
	{
		PsxDisk.x = READER_X;
		PsxDisk.y = READER_Y;
		
		PsxDisk.w = TRANSPARENT_CLOSED_READER_W;
		PsxDisk.h = TRANSPARENT_CLOSED_READER_H;
		
		PsxDisk.u = TRANSPARENT_CLOSED_READER_U;
		PsxDisk.v = TRANSPARENT_CLOSED_READER_V;
		
		PsxDisk.attribute |= ENABLE_TRANS | TRANS_MODE(0);
		
		GfxSortSprite(&PsxDisk);
		
		PsxDisk.attribute &= ~(ENABLE_TRANS | TRANS_MODE(0) );
	}*/
	
}

void PSXSDKIntroDrawConsole(void)
{
	PsxDisk.x = PSX_X;
	PsxDisk.y = PSX_Y;
	
	PsxDisk.w = PSX_W;
	PsxDisk.h = PSX_H;
	
	PsxDisk.u = PSX_U;
	PsxDisk.v = PSX_V;
	
	GfxSortSprite(&PsxDisk);
}
