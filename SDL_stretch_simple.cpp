#include <SDL.h>

static unsigned char copy_row[4096]; // PAGE_ALIGNED;

//RRRRGGGGBBBBAAAA
//RRRRRGGGGGGBBBBB
//endian??

//AAAABBBBGGGGRRRR
//BBBBBGGGGGGRRRRR

#define DEFINE_COPY_ROW(name, type)         \
static void name(type *src, int src_w, type *dst, int dst_w)    \
{                                           \
    int i;     \
	int pos, inc;                           \
    type pixel = 0;                         \
                                            \
    pos = 0x10000;                          \
    inc = (src_w << 16) / dst_w;            \
    for ( i=dst_w; i>0; --i ) {             \
        while ( pos >= 0x10000L ) {         \
            pixel = *src++;                 \
            pos -= 0x10000L;                \
        }                                   \
		*dst++ = ((pixel >> 11) ) | ((pixel & 0x07E0) ) | (((pixel & 0x001F) << 11) );       \
        pos += inc;                         \
    }                                       \
}

/*
static void copy_and_filter(Uint16 *src, int src_w, Uint16 *dst, int dst_w)    
{                                           
    int i;     
	int pos, inc;                           
    Uint16 pixel = 0;                         
    Uint16 prepixel = 0;                         
    Uint16 postpixel = 0;                         
                                            
    pos = 0x10000;                          
    inc = (src_w << 16) / dst_w;       

	i = dst_w;
    while ( pos >= 0x10000L ) 
	{         
        pixel = *src++;                 
        pos -= 0x10000L;                
    }                                   
	*dst++ = ((pixel >> 11) ) | ((pixel & 0x07E0) ) | (((pixel & 0x001F) << 11) );       
    pos += inc;                         

	--i;

    for ( ; i>1; --i ) {             
        while ( pos >= 0x10000L ) {         
            prepixel = pixel;
			pixel = *src++;                 
            postpixel = *src;
			
			pos -= 0x10000L;                
        }                                   
		
		
		*dst++ = ( (((pixel >> 11) + (prepixel >> 11) + (postpixel >> 11)) / 3)  & 0x001F   ) 
			| (  ( (((pixel & 0x07E0) >>5 ) + ((prepixel & 0x07E0) >>5 ) + ((postpixel & 0x07E0) >>5 )) / 3 ) << 5   ) 
			| (  ( (((pixel & 0x001F) + (prepixel & 0x001F) + (postpixel & 0x001F) ) / 3 ) << 11)     );
		
		//*dst++ = ((pixel >> 11) ) | ((pixel & 0x07E0) ) | (((pixel & 0x001F) << 11) );       
        pos += inc;                         
    }              

	while ( pos >= 0x10000L ) 
	{         
        pixel = *src++;                 
        pos -= 0x10000L;                
    }                                   
	*dst++ = ((pixel >> 11) ) | ((pixel & 0x07E0) ) | (((pixel & 0x001F) << 11) );       
    //pos += inc;

}
*/

//*dst++ = ((pixel >> 12) << 12) | (((pixel & 0x07E0) >> 7) << 8) | (((pixel & 0x001F) >> 1) << 4);       
        
//*dst++ = (((pixel & 0xF800) >> 12) << 12) | (((pixel & 0x07E0) >> 7) << 8) | (((pixel & 0x001F) >> 1) << 4);       
        

/* *INDENT-OFF* */
DEFINE_COPY_ROW(copy_row1, Uint8)
DEFINE_COPY_ROW(copy_row2, Uint16)
DEFINE_COPY_ROW(copy_row4, Uint32)
/* *INDENT-ON* */

/* The ASM code doesn't handle 24-bpp stretch blits */
static void
copy_row3(Uint8 * src, int src_w, Uint8 * dst, int dst_w)
{
    int i;
    int pos, inc;
    Uint8 pixel[3] = { 0, 0, 0 };

    pos = 0x10000;
    inc = (src_w << 16) / dst_w;
    for (i = dst_w; i > 0; --i) {
        while (pos >= 0x10000L) {
            pixel[0] = *src++;
            pixel[1] = *src++;
            pixel[2] = *src++;
            pos -= 0x10000L;
        }
        *dst++ = pixel[0];
        *dst++ = pixel[1];
        *dst++ = pixel[2];
        pos += inc;
    }
}

/* Perform a stretch blit between two surfaces of the same format.
   NOTE:  This function is not safe to call from multiple threads!
*/
int
SDL_SoftStretchHCF(SDL_Surface * src, const SDL_Rect * srcrect,
                SDL_Surface * dst, const SDL_Rect * dstrect, int parche, int inBeginAvoid, int inEndAvoid)
{

#define NO_AVOID 9999
	
	FILE *fd; 

    int src_locked;
    int dst_locked;
    int pos, inc;
    int dst_maxrow;
    int src_row, dst_row;
    Uint8 *srcp = NULL;
    Uint8 *dstp;
    SDL_Rect full_src;
    SDL_Rect full_dst;
#ifdef USE_ASM_STRETCH
    SDL_bool use_asm = SDL_TRUE;
#ifdef __GNUC__
    int u1, u2;
#endif
#endif /* USE_ASM_STRETCH */
    const int bpp = dst->format->BytesPerPixel;

    /*
    if (src->format->format != dst->format->format) {
        return SDL_SetError("Only works with same format surfaces");
    }
    */

    /* Verify the blit rectangles */
    if (srcrect) {
        if ((srcrect->x < 0) || (srcrect->y < 0) ||
            ((srcrect->x + srcrect->w) > src->w) ||
            ((srcrect->y + srcrect->h) > src->h)) {
            return 11;  //SDL_SetError("Invalid source blit rectangle");
        }
    } else {
        full_src.x = 0;
        full_src.y = 0;
        full_src.w = src->w;
        full_src.h = src->h;
        srcrect = &full_src;
    }
    if (dstrect) {
        if ((dstrect->x < 0) || (dstrect->y < 0) ||
            ((dstrect->x + dstrect->w) > dst->w) ||
            ((dstrect->y + dstrect->h) > dst->h + (inEndAvoid - inBeginAvoid))) {
            //((dstrect->y + dstrect->h) > dst->h)) {
            
				//LOGS
				fd = fopen("zzlog.txt", "a");
				fprintf(fd, "Invalid dest rectangle\n");
				fclose(fd);
				//LOGS
				return 12; //SDL_SetError("Invalid destination blit rectangle");
				

        }
    } else {
        full_dst.x = 0;
        full_dst.y = 0;
        full_dst.w = dst->w;
        full_dst.h = dst->h;
        dstrect = &full_dst;
    }

    /* Lock the destination if it's in hardware */
    dst_locked = 0;
    if (SDL_MUSTLOCK(dst)) {
        if (SDL_LockSurface(dst) < 0) {

			//LOGS
				fd = fopen("zzlog.txt", "a");
				fprintf(fd, "Unable to lock dest\n");
				fclose(fd);
				//LOGS

            return 13; //SDL_SetError("Unable to lock destination surface");
        }
        dst_locked = 1;
    }
    /* Lock the source if it's in hardware */
    src_locked = 0;
    if (SDL_MUSTLOCK(src)) {
        if (SDL_LockSurface(src) < 0) {
            if (dst_locked) {
                SDL_UnlockSurface(dst);
            }

			//LOGS
				fd = fopen("zzlog.txt", "a");
				fprintf(fd, "Unable to lock source surface\n");
				fclose(fd);
				//LOGS

            return 14; //SDL_SetError("Unable to lock source surface");
        }
        src_locked = 1;
    }

    /* Set up the data... */
    pos = 0x10000;
    inc = (srcrect->h << 16) / dstrect->h;
    src_row = srcrect->y;
    dst_row = dstrect->y;

	int dst_row_real = dst_row;

    /* Perform the stretch blit */
    for (dst_maxrow = dst_row + dstrect->h + (inEndAvoid-inBeginAvoid) - parche; dst_row < dst_maxrow; ++dst_row) {
    //for (dst_maxrow = dst_row + dstrect->h + (inEndAvoid-inBeginAvoid) - 20; dst_row < dst_maxrow; ++dst_row) {
    //for (dst_maxrow = dst_row + dstrect->h; dst_row < dst_maxrow; ++dst_row) {
        bool skipa = (dst_row > inBeginAvoid && dst_row < inEndAvoid);
		if(!skipa)
		{	
			//dstp = (Uint8 *) dst->pixels + (dst_row * dst->pitch)
			dstp = (Uint8 *) dst->pixels + (dst_row_real * dst->pitch)
				+ (dstrect->x * bpp);
		}
		while (pos >= 0x10000L) {
            srcp = (Uint8 *) src->pixels + (src_row * src->pitch)
                + (srcrect->x * bpp);
            ++src_row;
            pos -= 0x10000L;
        }

		if(!skipa)
		{
            switch (bpp) {
            case 1:
                copy_row1(srcp, srcrect->w, dstp, dstrect->w);
                break;
            case 2:
                //copy_and_filter((Uint16 *) srcp, srcrect->w,
				copy_row2((Uint16 *) srcp, srcrect->w, (Uint16 *) dstp, dstrect->w);
                break;
            case 3:
                copy_row3(srcp, srcrect->w, dstp, dstrect->w);
                break;
            case 4:
                copy_row4((Uint32 *) srcp, srcrect->w,
                          (Uint32 *) dstp, dstrect->w);
                break;
            }

			dst_row_real++;
		}

        pos += inc;
    }

    /* We need to unlock the surfaces if they're locked */
    if (dst_locked) {
        SDL_UnlockSurface(dst);
    }
    if (src_locked) {
        SDL_UnlockSurface(src);
    }

    return (0);
}

/* vi: set ts=4 sw=4 expandtab: */
