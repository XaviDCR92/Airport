#define TILE_WIDTH 64
#define TILE_WIDTH_HALF 32
#define TILE_HEIGHT 64
#define TILE_HEIGHT_HALF 32

struct t_CartToIso
{
	u8 u8TileX;
	u8 u8TileY;
	s16 s16IsoX;
	s16 s16IsoY;
}

/* ******************************************************
 * void GameCartesianToIsometricPosition(TYPE_CartToIso * CTI)
 * 
 * BRIEF:
 * 		* Transforms (X,Y,Z) position buffer to Isometric position
 *  
 * PARAMETERS:
 * 		* TYPE_GXYZTOISO * t_GXYZTOISO
 * 
 * RETURNS:
 * 
 * 		* void
 * 
 * DATE:
 * 		1/January/2016
 * 
 * ******************************************************* */

/*
 * 	screen.x = map.x * TILE_WIDTH_HALF - map.y * TILE_WIDTH_HALF;
	screen.y = map.x * TILE_HEIGHT_HALF + map.y * TILE_HEIGHT_HALF;
* */

void GameCartesianToIsometricPosition(TYPE_CartToIso * CTI)
{
	CTI->u16IsoX = (CTI->u8TileX * TILE_WIDTH_HALF) - (CTI->u8TileY * TILE_WIDTH_HALF);
	CTI->u16IsoY = (CTI->u8TileX * TILE_HEIGHT_HALF) - (CTI->u8TileY * TILE_HEIGHT_HALF)
}
