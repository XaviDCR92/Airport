/* *************************************
 * 	Includes
 * *************************************/

#include "EndAnimation.h"

/* *************************************
 * 	Defines
 * *************************************/

/* *************************************
 * 	Structs and enums
 * *************************************/

enum
{
	END_ANIMATION_FADEOUT_STEP = 8,
	
	END_ANIMATION_LINE_STEP = 2,
	
	END_ANIMATION_SQUARES_SIZE_BITSHIFT = 5,
	END_ANIMATION_SQUARES_SIZE = 32,
	END_ANIMATION_SQUARES_PER_COLUMN = 8,
	END_ANIMATION_SQUARES_PER_ROW = 12,
	END_ANIMATION_SQUARES_TOTAL = 	END_ANIMATION_SQUARES_PER_COLUMN *
									END_ANIMATION_SQUARES_PER_ROW,
									
	END_ANIMATION_SQUARES_TOTAL_MAX_INDEX = END_ANIMATION_SQUARES_TOTAL - 1,
	
	END_ANIMATION_SQUARES = 0,
	END_ANIMATION_FADEOUT,
	END_ANIMATION_LINE,
	END_ANIMATION_MAX_RAND_VALUE = END_ANIMATION_LINE
};

/* *************************************
 * 	Local Prototypes
 * *************************************/

static void EndAnimationSquares(void);
static void EndAnimationFadeOut(void);
static void EndAnimationLine(void);

/* *************************************
 * 	Local Variables
 * *************************************/

static GsRectangle EndAnimationRect;
static GsSprite EndAnimationDisplay;

void EndAnimation(void)
{
	uint8_t randIndex = 0;
	
	GfxSaveDisplayData(&EndAnimationDisplay);
	
	GfxSetGlobalLuminance(NORMAL_LUMINANCE);
	
	if(SystemIsRandSeedSet() == false)
	{
		// Set default end animation
		EndAnimationFadeOut();
	}
	else
	{
		randIndex = rand() % (END_ANIMATION_MAX_RAND_VALUE + 1);
		
		switch(randIndex)
		{
			case END_ANIMATION_SQUARES:
				EndAnimationSquares();
			break;
			
			case END_ANIMATION_FADEOUT:
				EndAnimationFadeOut();
			break;
			
			case END_ANIMATION_LINE:
				EndAnimationLine();
			break;
			
			default:
			break;
		}
		
		SfxStopMusic();
	}
}

void EndAnimationFadeOut(void)
{
	uint8_t i;
	
	while(1)
	{
		if( GfxGetGlobalLuminance() > 0)
		{
			GfxSetGlobalLuminance(GfxGetGlobalLuminance() - END_ANIMATION_FADEOUT_STEP);
			
			GfxSortSprite(&EndAnimationDisplay);;
			GfxDrawScene_Slow();
		}
		else
		{
			GsSortCls(0,0,0);
			
			for(i = 0 ; i < 2 ; i++)
			{
				// Draw two frames to ensure black display
				GfxDrawScene_Slow();
			}
			
			break;
		}
	}
}

void EndAnimationLine(void)
{
	short rectIndex = 0;
	
	do
	{
		GfxSortSprite(&EndAnimationDisplay);
		
		// Draw upper half rectangle
		
		EndAnimationRect.x = 0;
		EndAnimationRect.y = 0;
		
		EndAnimationRect.w = X_SCREEN_RESOLUTION;
		EndAnimationRect.h = rectIndex;
		
		GsSortRectangle(&EndAnimationRect);
		
		EndAnimationRect.x = 0;
		EndAnimationRect.y = Y_SCREEN_RESOLUTION - rectIndex;
		
		EndAnimationRect.w = X_SCREEN_RESOLUTION;
		EndAnimationRect.h = rectIndex;
		
		GsSortRectangle(&EndAnimationRect);
		
		GfxDrawScene_Slow();
		
		rectIndex += END_ANIMATION_LINE_STEP;
		
	}while(rectIndex <= (X_SCREEN_RESOLUTION >> 1) );
	
}

void EndAnimationSquares(void)
{
	uint16_t i, j, k;
	uint16_t randInd = 0;
	bool sqPos[END_ANIMATION_SQUARES_TOTAL];
	uint16_t sqCounter = END_ANIMATION_SQUARES_TOTAL;
	uint16_t maxIndex = END_ANIMATION_SQUARES_TOTAL_MAX_INDEX;
	
	EndAnimationRect.w = END_ANIMATION_SQUARES_SIZE;
	EndAnimationRect.h = END_ANIMATION_SQUARES_SIZE;
	
	EndAnimationRect.r = 0;
	EndAnimationRect.g = 0;
	EndAnimationRect.b = 0;
	
	memset(sqPos, false , END_ANIMATION_SQUARES_TOTAL);
	
	for(i = 0; i < END_ANIMATION_SQUARES_TOTAL ; i++)
	{
		
		do
		{
			randInd = SystemRand(0,maxIndex);
			
			/*dprintf("randInd = %d\t",randInd);
			dprintf("sqPos[randInd] = %d\n", sqPos[randInd]);*/
		
			if(sqPos[randInd] == false)
			{
				sqPos[randInd] = true;
				sqCounter--;
				
				while(sqPos[maxIndex] == true)
				{
					// Lower maximum value for rand() so that it's
					// easier to spot new empty index on next iteration.
					maxIndex--;
				}
				
				break;
			}
			else
			{
				if(sqCounter == 0)
				{
					break;
				}
			}
						
		}while(1);
		
		GfxSortSprite(&EndAnimationDisplay);
		
		if(sqCounter != 0)
		{
			for(j = 0; j < END_ANIMATION_SQUARES_TOTAL ; j++)
			{
				if(sqPos[j] == true)
				{
					EndAnimationRect.x = ((j) << END_ANIMATION_SQUARES_SIZE_BITSHIFT) -
												(short)( (j / END_ANIMATION_SQUARES_PER_ROW) *
												X_SCREEN_RESOLUTION);
												
					EndAnimationRect.y =	(short) (j/ END_ANIMATION_SQUARES_PER_ROW) <<
												END_ANIMATION_SQUARES_SIZE_BITSHIFT;
												
					GsSortRectangle(&EndAnimationRect);
				}
			}
		}
		else
		{
			// Quick fix: draw a full black rectangle instead of multiple squares
			for(k = 0 ; k < 2 ; k++)
			{
				// Draw two frames to ensure black display
				GsSortCls(0,0,0);
				GfxDrawScene_Slow();
			}
		}

		GfxDrawScene_Slow();
	}
}
