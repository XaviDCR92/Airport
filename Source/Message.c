/* **************************************
 * 	Includes							*
 * *************************************/

#include "Message.h"
#include "Gfx.h"
#include "Pad.h"

/* **************************************
 * 	Defines								*
 * *************************************/

#define NO_MESSAGE			((uint8_t)0xFF)
#define MESSAGE_FIFO_SIZE	16

/* **************************************
 * 	Structs and enums					*
 * *************************************/

/* **************************************
 * 	Local prototypes					*
 * *************************************/

/* **************************************
 * 	Local variables						*
 * *************************************/

static TYPE_MESSAGE_DATA tMessageFIFO[MESSAGE_FIFO_SIZE];
static uint8_t MessageIdx;

void MessageInit(void)
{
	bzero(tMessageFIFO, sizeof (tMessageFIFO));
	MessageIdx = NO_MESSAGE;
}

bool MessageCreate(TYPE_MESSAGE_DATA* ptrMessage)
{
	uint8_t i;

	for (i = 0; i < MESSAGE_FIFO_SIZE; i++)
	{
		TYPE_MESSAGE_DATA* m = &tMessageFIFO[i];

		if (m->used == false)
		{
			memmove(m, ptrMessage, sizeof (TYPE_MESSAGE_DATA));

			m->used = true;

			Serial_printf("Successfully allocated message into slot %d.\n", i);

			return true;
		}
	}

	Serial_printf("Could not allocate message resource to FIFO.\n");
	return false;
}

void MessageHandler(void)
{
	uint8_t i;

	if (System1SecondTick())
	{
		for (i = 0; i < MESSAGE_FIFO_SIZE; i++)
		{
			TYPE_MESSAGE_DATA* ptrMessage = &tMessageFIFO[i];

			if (ptrMessage->used)
			{
				if (ptrMessage->Timeout == 0)
				{
					ptrMessage->used = false;
					MessageIdx = i;
				}
				else
				{
					ptrMessage->Timeout--;
				}
			}
		}
	}
}

void MessageRender(void)
{
	if (MessageIdx != NO_MESSAGE)
	{
		enum
		{
			MESSAGE_RECT_W = 256,
			MESSAGE_RECT_H = 72,
			MESSAGE_RECT_X = (X_SCREEN_RESOLUTION - MESSAGE_RECT_W) >> 1,
			MESSAGE_RECT_Y = (Y_SCREEN_RESOLUTION - MESSAGE_RECT_H) >> 1,
		};

		GsGPoly4 messageRect = {	.x[0] = MESSAGE_RECT_X,
									.x[1] = MESSAGE_RECT_X + MESSAGE_RECT_W,
									.x[2] = messageRect.x[0],
									.x[3] = messageRect.x[1],

									.y[0] = MESSAGE_RECT_Y,
									.y[1] = messageRect.y[0],
									.y[2] = MESSAGE_RECT_Y + MESSAGE_RECT_H,
									.y[3] = messageRect.y[2]	,

									.r[0] = 0,
									.r[1] = 0,
									.r[2] = 0,
									.r[3] = 0,

									.g[0] = NORMAL_LUMINANCE,
									.g[1] = NORMAL_LUMINANCE,
									.g[2] = NORMAL_LUMINANCE >> 1,
									.g[3] = NORMAL_LUMINANCE >> 1,

									.b[0] = NORMAL_LUMINANCE >> 2,
									.b[1] = NORMAL_LUMINANCE >> 2,
									.b[2] = NORMAL_LUMINANCE >> 3,
									.b[3] = NORMAL_LUMINANCE >> 3,

									.attribute = 0	};

		GsSprite backgroundSpr = {0};

		GfxSaveDisplayData(&backgroundSpr);

		backgroundSpr.x = 0;
		backgroundSpr.y = 0;

		do
		{
			enum
			{
				MESSAGE_TEXT_X = MESSAGE_RECT_X + 8,
				MESSAGE_TEXT_Y = MESSAGE_RECT_Y + 8,

				CONTINUE_TEXT_X = MESSAGE_TEXT_X,
				CONTINUE_TEXT_Y = MESSAGE_RECT_Y + MESSAGE_RECT_H - 16,
			};

			char* strMessage = MessageGetString();

			GfxSortSprite(&backgroundSpr);

			GsSortGPoly4(&messageRect);

			if (strMessage != NULL)
			{
				enum
				{
					MAX_CH_PER_LINE = 32
				};

				FontSetFlags(&SmallFont, FONT_WRAP_LINE);
				FontSetMaxCharPerLine(&SmallFont, MAX_CH_PER_LINE);

				FontPrintText(&SmallFont, MESSAGE_TEXT_X, MESSAGE_TEXT_Y, strMessage);

				// Restore default values
				FontSetFlags(&SmallFont, FONT_NOFLAGS);
				FontSetMaxCharPerLine(&SmallFont, 0);
			}

			FontPrintText(&SmallFont, CONTINUE_TEXT_X, CONTINUE_TEXT_Y, "Press   to continue...");

			GfxDrawButton(CONTINUE_TEXT_X + (strlen("Press") << 3) - 4, CONTINUE_TEXT_Y - 4, PAD_CROSS);

			GfxDrawScene_Slow();

		} while (PadOneKeySinglePress(PAD_CROSS) == false);

		MessageIdx = NO_MESSAGE;
	}
}

char* MessageGetString(void)
{
	if (MessageIdx != NO_MESSAGE)
	{
		return tMessageFIFO[MessageIdx].strMessage;
	}

	return NULL;
}
