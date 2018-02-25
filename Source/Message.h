#ifndef MESSAGE_HEADER__
#define MESSAGE_HEADER__

/* *************************************
 * 	Includes
 * *************************************/

#include "Global_Inc.h"

/* *************************************
 * 	Defines
 * *************************************/

#define MAX_MESSAGE_STR_SIZE 256

/* *************************************
 * 	Structs and enums
 * *************************************/

typedef struct t_messagedata
{
	bool used;
	uint32_t Timeout;
	char strMessage[MAX_MESSAGE_STR_SIZE];
}TYPE_MESSAGE_DATA;

/* *************************************
 * 	Global prototypes
 * *************************************/

void MessageInit(void);
bool MessageCreate(TYPE_MESSAGE_DATA* ptrMessage);
void MessageHandler(void);
void MessageRender(void);
char* MessageGetString(void);

#endif // MESSAGE_HEADER__
