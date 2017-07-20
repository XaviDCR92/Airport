/* *************************************
 * 	Includes
 * *************************************/

#include "Serial.h"

/* *************************************
 * 	Defines
 * *************************************/

#define SERIAL_BAUDRATE 115200
#define SERIAL_TX_RX_TIMEOUT 20000
#define SERIAL_RX_FIFO_EMPTY 0
#define SERIAL_TX_NOT_READY 0
#define SERIAL_PRINTF_INTERNAL_BUFFER_SIZE 256

/* **************************************
 * 	Structs and enums					*
 * *************************************/

typedef enum
{
    SERIAL_STATE_INIT = 0,
    SERIAL_STATE_STANDBY,
    SERIAL_STATE_WRITING_INITIAL
}SERIAL_STATE;

/* *************************************
 * 	Local Variables
 * *************************************/

static volatile SERIAL_STATE SerialState;

/* *************************************
 * 	Local Prototypes
 * *************************************/

void SerialInit(void)
{
    SIOStart(115200);
}

bool SerialRead(uint8_t* ptrArray, size_t nBytes)
{
    if(nBytes == 0)
    {
        dprintf("SerialRead: invalid size %d\n", nBytes);
        return false;
    }

    do
    {
        //uint16_t timeout = SERIAL_TX_RX_TIMEOUT;
        
        while( (SIOCheckInBuffer() == SERIAL_RX_FIFO_EMPTY)); // Wait for RX FIFO not empty

        *(ptrArray++) = SIOReadByte();
    }while(--nBytes);

    return true;
}

bool SerialWrite(void* ptrArray, size_t nBytes)
{

    if(nBytes == 0)
    {
        dprintf("SerialWrite: invalid size %d\n", nBytes);
        return false;
    }

    do
    {
        //uint16_t timeout = SERIAL_TX_RX_TIMEOUT;

        while( (SIOCheckOutBuffer() == SERIAL_TX_NOT_READY)); // Wait for TX FIFO empty.

        SIOSendByte(*(uint8_t*)ptrArray++);

    }while(--nBytes);

    return true;
}

#ifdef SERIAL_INTERFACE
void Serial_printf(const char* str, ...)
{
    va_list ap;
    int result;
    static char internal_buffer[SERIAL_PRINTF_INTERNAL_BUFFER_SIZE];

    va_start(ap, str);

    result = vsnprintf(	internal_buffer,
						SERIAL_PRINTF_INTERNAL_BUFFER_SIZE,
						str,
						ap	);

    SerialWrite(internal_buffer, result);
}
#endif // SERIAL_INTERFACE

