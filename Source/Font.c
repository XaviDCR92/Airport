/* *************************************
 * 	Includes
 * *************************************/

#include "Font.h"

/* *************************************
 * 	Defines
 * *************************************/

#define FONT_INTERNAL_TEXT_BUFFER_MAX_SIZE 200

/* *************************************
 * 	Local Prototypes
 * *************************************/

/* *************************************
 * 	Local Variables
 * *************************************/

static char _internal_text[FONT_INTERNAL_TEXT_BUFFER_MAX_SIZE];
static unsigned char _blend_effect_lum;

bool FontLoadImage(char * strPath, TYPE_FONT * ptrFont)
{
	if(GfxSpriteFromFile(strPath, &ptrFont->spr) == false)
	{
		return false;
	}
	
	ptrFont->spr_w = ptrFont->spr.w;
	ptrFont->spr_h = ptrFont->spr.h;
	ptrFont->spr_u = ptrFont->spr.u;
	ptrFont->spr_v = ptrFont->spr.v;
	
	//Now set default values to font
	
	ptrFont->char_w = FONT_DEFAULT_CHAR_SIZE;
	ptrFont->char_h = FONT_DEFAULT_CHAR_SIZE;
	
	ptrFont->spr.attribute |= COLORMODE(COLORMODE_4BPP);
	ptrFont->spr.attribute &= COLORMODE(~(COLORMODE_8BPP | COLORMODE_16BPP | COLORMODE_24BPP));
	ptrFont->spr.r = NORMAL_LUMINANCE;
	ptrFont->spr.g = NORMAL_LUMINANCE;
	ptrFont->spr.b = NORMAL_LUMINANCE;
	
	//At this point, spr.w and spr.h = real w/h
	ptrFont->char_per_row = (uint8_t)(ptrFont->spr_w / ptrFont->char_w);
	ptrFont->max_ch_wrap = 0;
	
	ptrFont->spr.w = ptrFont->char_w;
	ptrFont->spr.h = ptrFont->char_h;
	
	ptrFont->flags = FONT_NOFLAGS;
	
	ptrFont->init_ch = FONT_DEFAULT_INIT_CHAR;
	
	dprintf("Sprite CX = %d, sprite CY = %d\n",ptrFont->spr.cx, ptrFont->spr.cy);
	
	return true;
}

void FontSetInitChar(TYPE_FONT * ptrFont, char c)
{
	ptrFont->init_ch = c;
}

void FontSetFlags(TYPE_FONT * ptrFont, FONT_FLAGS flags)
{
	ptrFont->flags = flags;
}

void FontSetSize(TYPE_FONT * ptrFont, short size)
{
	ptrFont->char_w = size;
	ptrFont->char_h = size;
	
	//At this point, spr.w and spr.h = real w/h
	ptrFont->char_per_row = (uint8_t)(ptrFont->spr_w / ptrFont->char_w);
	ptrFont->max_ch_wrap = 0;
	
	ptrFont->spr.w = ptrFont->char_w;
	ptrFont->spr.h = ptrFont->char_h;
}

void FontCyclic(void)
{
	_blend_effect_lum -= 8;
}

void FontPrintText(TYPE_FONT * ptrFont, short x, short y, char * str, ...)
{
	uint16_t i;
	uint16_t line_count = 0;
	int result;
	short orig_x = x;
	
	va_list ap;
	
	if(ptrFont->flags & FONT_1HZ_FLASH)
	{
		if(Gfx1HzFlash() == false)
		{
			return;
		}
	}
	else if(ptrFont->flags & FONT_2HZ_FLASH)
	{
		if(Gfx2HzFlash() == false)
		{
			return;
		}
	}
	
	va_start(ap, str);
	
	result = vsnprintf(	_internal_text,
						FONT_INTERNAL_TEXT_BUFFER_MAX_SIZE,
						str,
						ap	);
	
	for(i = 0; i < result ; i++)
	{
		char _ch = _internal_text[i];
		
		if(_ch == '\0')
		{
			// End of string
			break;
		}
		
		switch(_ch)
		{
			case ' ':
				x += ptrFont->char_w;
				continue;
			case '\n':
				x = orig_x;
				y += ptrFont->char_h;
			break;
			default:
				if(	(ptrFont->flags & FONT_WRAP_LINE) && (ptrFont->max_ch_wrap != 0) )
				{
					if(++line_count >= ptrFont->max_ch_wrap)
					{
						line_count = 0;
						x = orig_x;
						y += ptrFont->char_h;
					}
				}
	
				ptrFont->spr.x = x;
				ptrFont->spr.y = y;
				ptrFont->spr.w = ptrFont->char_w;
				ptrFont->spr.h = ptrFont->char_h;
				ptrFont->spr.u = (short)( (_ch - ptrFont->init_ch) % ptrFont->char_per_row) * ptrFont->char_w;
				ptrFont->spr.u += ptrFont->spr_u; // Add original offset for image
				ptrFont->spr.v = (short)( (_ch - ptrFont->init_ch) / ptrFont->char_per_row) * ptrFont->char_h;
				ptrFont->spr.v += ptrFont->spr_v; // Add original offset for image
				
				if(ptrFont->flags & FONT_BLEND_EFFECT)
				{
					ptrFont->spr.r += 8;
					ptrFont->spr.g += 8;
					ptrFont->spr.b += 8;
				}
				else
				{
					ptrFont->spr.r = NORMAL_LUMINANCE;
					ptrFont->spr.g = NORMAL_LUMINANCE;
					ptrFont->spr.b = NORMAL_LUMINANCE;
				}
				/*dprintf("char_w = %d, char_h = %d, char_per_row = %d, init_ch: %c\n",
						ptrFont->char_w,
						ptrFont->char_h,
						ptrFont->char_per_row,
						ptrFont->init_ch);
				dprintf("Char: %c, spr.u = %d, spr.v = %d\n",str[i],ptrFont->spr.u, ptrFont->spr.v);
				dprintf("Sprite CX = %d, sprite CY = %d\n",ptrFont->spr.cx, ptrFont->spr.cy);*/
				//dprintf("Sprite rgb={%d,%d,%d}\n",ptrFont->spr.r, ptrFont->spr.g, ptrFont->spr.b);
				
				GfxSortSprite(&ptrFont->spr);
				x += ptrFont->char_w;
			break;
		}
	}
	
	if(ptrFont->flags & FONT_BLEND_EFFECT)
	{
		ptrFont->spr.r = _blend_effect_lum;
		ptrFont->spr.g = _blend_effect_lum;
		ptrFont->spr.b = _blend_effect_lum;
	}

	va_end(ap);
}
