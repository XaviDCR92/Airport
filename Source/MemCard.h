#ifndef __MEMCARD_HEADER__
#define __MEMCARD_HEADER__

/* *************************************
 * 	Includes
 * *************************************/

#include "Global_Inc.h"
#include "System.h"
#include "Pad.h"
#include "Font.h"

/* *************************************
 * 	Defines
 * *************************************/
 
/*	The memory is split into 16 blocks (of 8 Kbytes each), and each block 
 *	is split into 64 sectors (of 128 bytes each). The first block is used
 *	as Directory, the remaining 15 blocks are containing Files, each 
 *	file can occupy one or more blocks.	*/

#define MEMCARD_BLOCKS_PER_CARD 15
#define MEMCARD_NUMBER_OF_SLOTS 2
#define MEMCARD_FILENAME_SIZE 21
#define MEMCARD_NUMBER_OF_ICONS 3
#define MEMCARD_ICON_SIZE 0x80
#define MEMCARD_DATA_SIZE 0x80
#define MEMCARD_CLUT_SIZE 32
#define MEMCARD_GAME_FILENAME (char*)"XAVI18-18215AIRPORT"
#define MEMCARD_FIRST_OR_LAST_DATA_SIZE 0x1E00
#define MEMCARD_INTERMEDIATE_OR_LAST_DATA_SIZE 0x2000

/* *************************************
 * 	Structs and enums
 * *************************************/
 
typedef enum t_CardBlock
{
	SLOT_ONE = 0,
	SLOT_TWO
}MEMCARD_SLOTS;

typedef enum t_MCBlocks
{
	DIRECTORY_BLOCK = 0,
	BLOCK_1,
	BLOCK_2,
	BLOCK_3,
	BLOCK_4,
	BLOCK_5,
	BLOCK_6,
	BLOCK_7,
	BLOCK_8,
	BLOCK_9,
	BLOCK_10,
	BLOCK_11,
	BLOCK_12,
	BLOCK_13,
	BLOCK_14,
	BLOCK_15,
}MEMCARD_BLOCKS;

typedef enum t_BlockCount
{
	EMPTY_BLOCK = 0,
	FIRST_OR_ONLY_BLOCK,
	INTERMEDIATE_BLOCK,
	LAST_BLOCK
}MEMCARD_BLOCK_COUNT;

typedef struct t_MemCard
{
	MEMCARD_SLOTS Slot;
	MEMCARD_BLOCKS Block;
	uint8_t IconNumber; // Possible values : 1 ... 3
	uint8_t FileName[MEMCARD_FILENAME_SIZE];
	MEMCARD_BLOCK_COUNT BlockCount; // Look at MEMCARD_BLOCK_COUNT enum
	
	/*
	*	The first some letters of the filename should indicate the game
	*	to which the file belongs, in case of commercial games this is
	*	conventionally done like so: Two character region code:
	*	"BI"=Japan, "BE"=Europe, "BA"=America						
	* 	followed by 10 character game code,
	*	in "AAAA-NNNNN" form
	* 
	* 	Where the "AAAA" part does imply the region too;
	* 	(SLPS/SCPS=Japan, SLUS/SCUS=America, SLES/SCES=Europe)
	* 	(SCxS=Made by Sony, SLxS=Licensed by Sony),
	* 	followed by up to 8 characters, "abcdefgh"
	* 
	*	(which may identify the file if the game uses multiple files;
	* 	this part often contains a random string which seems to be
	* 	allowed to contain any chars in range of 20h..7Fh, of course it
	* 	shouldn't contain "?" and "*" wildcards).*/
	
	uint8_t Icons[MEMCARD_NUMBER_OF_ICONS][MEMCARD_ICON_SIZE];
	uint8_t CLUT[MEMCARD_NUMBER_OF_ICONS][MEMCARD_CLUT_SIZE];
	uint8_t * Data; // Buffer pointed to by "Data" must be 128 KB or higher!
	GsTPoly4 IconTPoly;
}TYPE_BLOCK_DATA;

/* *************************************
 * 	Global prototypes
 * *************************************/

// Inits default values for memory card blocks 
void MemCardInit(void);
 
// Sets null values to structure pointed to by ptrBlockData.
void MemCardResetBlockData(TYPE_BLOCK_DATA * ptrBlockData);

// Loads data from all blocks for both slots. Internally, it just calls
// MemCardGetBlockInfo multiple times and returns its result.
// All data gets saved into MemCardData (defined below).
bool MemCardGetAllData(void);

// Fills TYPE_BLOCK_DATA structure with basic info and icons from specific block.
// Take into account MemCardResetBlockData is automatically called first
// to flush previous data.
// If ptrBlockData->IconTPoly != NULL, MemCardUploadToGPU() is called.
bool MemCardGetBlockInfo(	TYPE_BLOCK_DATA * ptrBlockData,
							MEMCARD_SLOTS slot,
							MEMCARD_BLOCKS blockNumber	);

// Uploads block graphical data to GPU.
bool MemCardUploadToGPU(TYPE_BLOCK_DATA * ptrBlockData);

// To be called on every frame update (it modifies internal data)
void MemCardHandler(void);

// Reportedly, it draws icon data from a specified memory card block.
// If current block is on the middle of a file, block containing icon data
// is automatically searched when MemCardGetBlockInfo() is called.
void MemCardDrawIcon(TYPE_BLOCK_DATA * ptrBlockData, short x, short y);

// Loads all memory card data and shows a dialog with all icons and
// file names.
TYPE_BLOCK_DATA * MemCardShowMap(void);

// Reportedly, saves data to memory card given input block data.
bool MemCardSaveData(TYPE_BLOCK_DATA * ptrBlockData);

/* *************************************
 * 	Global variables
 * *************************************/
 
extern TYPE_BLOCK_DATA MemCardData[MEMCARD_BLOCKS_PER_CARD][MEMCARD_NUMBER_OF_SLOTS];

#endif //__MEMCARD_HEADER__
