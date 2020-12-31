#include <stdio.h>
#include <stdlib.h>
//#include <stdint.h>
#include <string.h>

//HCF: Comment this to disable frameskip
#define FRAMESKIP 1

//HCF: Comment this to disable framerate displaying
//#define SHOW_FRAMERATE 1

#define SPECIAL_ZOOM_NONE 0
#define SPECIAL_ZOOM_HORIZONTAL 1
#define SPECIAL_ZOOM_VERTICAL 2

#include "retroluxury/rl_sprite.h"

#include "global.h"
#include "gwlua/gwlua.h"

#include <SDL.h>
#include <SDL_ttf.h>

//#include "mine/SDL_rotozoom.h"
#include "comun.h"

//HCF
#include <dirent.h>	
#include <psppower.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspsuspend.h>

#include <pspgu.h>


#define MAX_LENGTH_SISTEMA 128
#define MAX_APPS 128
#define MAX_LENGTH_APP 128

TTF_Font *font;
char achAppSeleccionada[MAX_LENGTH_SISTEMA];
char achHighScoreFile[MAX_LENGTH_SISTEMA];
char achPath[MAX_LENGTH_SISTEMA * 2];
//unsigned int oldButtons = 0;

//HCF: To allocate volatile memory
void* HCF_RAM_ARRAY;

int salirderom = 0;

void gwrom_destroy( gwrom_t* gwrom );
void vdSaveHighscore(char *file);
void vdLoadHighscore(char *file);

extern bool bIsZoomEnabled;
int inNeedsSpecialZoom = SPECIAL_ZOOM_NONE;

void vdSelectApp()
{
    
    #define ALTO_OPCION 14
    #define MAX_ROMS_EN_PANTALLA 8
    #define DESPL_RAPIDO 1
    //8
    #define NUM_SCROLL  MAX_ROMS_EN_PANTALLA
    #define MAX_ANCHO_NOMBRE_FICHERO  265
    
    int salir, redibujar, i;
    SceCtrlData pad;
    char aachApps[MAX_APPS][MAX_LENGTH_APP];
    
    static int avanceCursorListado = 0; /*Lo que se ha avanzado*/
	static int posCursorEnPantallaListado = 0; /*Entre 0 y 11*/
    static int posCursor = 0;
    
    DIR           *d;
 	struct dirent *dir;
    int inSeleccionado = 0;
    int iNumeroFicheros = 0;
    int contCursor;
    SDL_Rect rect;
    SDL_Surface *picTexto = NULL;
    SDL_Surface *picCursor = NULL;
    SDL_Surface *picRectangle = NULL;
    SDL_Surface *picRectangleTemp;
	SDL_Surface *picCursorTemp;
    SDL_Surface *picMenu;
    SDL_Rect rectTituloSrc;
    FILE *fdScreenshot;
    SDL_Surface *picScreenshot;
    SDL_Rect rectScreenshot;
    
    rectTituloSrc.x = 0;
    rectTituloSrc.y = 0;
    
    SDL_Color textColor = { 255, 255, 255 }; 
    
	//SDL_Init(SDL_INIT_AUDIO);
	//Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,4096);
    
	Mix_OpenAudio(44100,AUDIO_S16SYS,2,4096);
    

    //Title music
    Mix_Music* MiMusica = Mix_LoadMUS("snd/music.mp3");
    Mix_PlayMusic(MiMusica,-1);  // -1 es el modo LOOP 
    
	//SDL_PauseAudio(false);

    //Menu
    SDL_Surface *picMenuTemp = IMG_Load("img/menu.png");
    //SDL_Surface *picMenuTemp = IMG_Load("img/menu.jpg");
    
    SDL_SetColorKey( picMenuTemp, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(picMenuTemp->format, 255, 0, 255) );
	picMenu = SDL_DisplayFormatAlpha(picMenuTemp);
	SDL_FreeSurface(picMenuTemp);
  
    //Cursor
	picCursorTemp = IMG_Load("img/cursor.png");
	SDL_SetColorKey( picCursorTemp, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(picCursorTemp->format, 255, 0, 255) );
	picCursor = SDL_DisplayFormatAlpha(picCursorTemp);
	SDL_FreeSurface(picCursorTemp);
    
    //Cursor
	picRectangleTemp = IMG_Load("img/rectangle.png");
	SDL_SetColorKey( picRectangleTemp, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(picRectangleTemp->format, 255, 0, 255) );
	picRectangle = SDL_DisplayFormatAlpha(picRectangleTemp);
	SDL_FreeSurface(picRectangleTemp);
	
    //Draw Header and Foot
    rect.x = 0;
    rect.y = 0;
    SDL_BlitSurface(picMenu, NULL, screen, NULL);
    
    
    memset(aachApps, 0x00, MAX_APPS * MAX_LENGTH_APP * sizeof(char));
	iNumeroFicheros = 0;
	
    strcpy(achPath, "roms/");
    d = opendir(achPath);    
    
    while((dir = readdir(d)) != NULL)
	{
		if( strcmp(".", dir->d_name) && strcmp("..", dir->d_name) )
		{
            //longi = strlen(dir->d_name);
    		strcpy(aachApps[iNumeroFicheros], dir->d_name);
    	    iNumeroFicheros++;		
	    }
    } 

    salir = 0;
    //redibujar = 1;

    //HCF: fast scroll
    int iMostrando = 0;
    int iPrimeraEjecucion = 1;
	int iDesplazamientoRapido;
    int iParado;
    while(!salir)
    {
        iDesplazamientoRapido = 0;
        redibujar = 0;
        iParado = 1;
        
        sceCtrlReadBufferPositive(&pad, 1);
        //if (oldButtons != pad.Buttons) 
        //{
            if (pad.Buttons & PSP_CTRL_CROSS) 
            {
                strcpy(achAppSeleccionada, aachApps[posCursor]);
				bIsZoomEnabled = false;
				salir = 1;
            }
            if ( (pad.Buttons & PSP_CTRL_DOWN)  || (pad.Buttons & PSP_CTRL_RTRIGGER) )
            {
                if(	posCursor < iNumeroFicheros - 1 )
                {
                    if( posCursorEnPantallaListado < MAX_ROMS_EN_PANTALLA ) //- 1)
                    {
                        posCursorEnPantallaListado++;
                    }
                    else
                    {
                        avanceCursorListado++;
                    }
                    posCursor++;	
                    redibujar = 1;	
                    iParado = 0;
                    
                    if( pad.Buttons & PSP_CTRL_RTRIGGER )
                        iDesplazamientoRapido = 1;
                }
		    }
            if ( (pad.Buttons & PSP_CTRL_UP) || (pad.Buttons & PSP_CTRL_LTRIGGER) )
            {
                if(	posCursor > 0 )
                {
                    if( posCursorEnPantallaListado > 0)
                    {
                        posCursorEnPantallaListado--;
                    }
                    else
                    {
                        avanceCursorListado--;
                    }
                    posCursor--;
                    redibujar = 1;
                    iParado = 0;
                    
                    if(pad.Buttons & PSP_CTRL_LTRIGGER)
                        iDesplazamientoRapido = 1;
                }
		    }
            
            //oldButtons = pad.Buttons;
        //}
        
        //if(redibujar)
        if( iParado == 1  && iMostrando == 0 )
		{
            
             //Screenshot
            #define MAX_NOMBRE_FICHERO 128
            char achScreenshot[MAX_NOMBRE_FICHERO];
            strcpy( achScreenshot, "img/" );
            strcat( achScreenshot, aachApps[posCursor] );
            achScreenshot[strlen(achScreenshot) - 3] = '\0';
            strcat( achScreenshot, "jpg" );
            //strcat( achScreenshot, "png" );
            
			picScreenshot = NULL;

            //fdScreenshot = fopen(achScreenshot,"rb");
            //if(fdScreenshot != NULL)
            //{
              //  fclose(fdScreenshot);
                picScreenshot = IMG_Load(achScreenshot);
                if( picScreenshot != NULL )
                {
                    rectScreenshot.x = 0;
                    rectScreenshot.y = 60;
					
					SDL_BlitSurface(picScreenshot, NULL, screen, &rectScreenshot);
                    SDL_FreeSurface(picScreenshot);
                }
            //}
            
            //Miniature
            achScreenshot[strlen(achScreenshot) - 4] = '\0';
            strcat( achScreenshot, "_mini.png" );
            
			picScreenshot = NULL;

			//fdScreenshot = fopen(achScreenshot,"rb");
            //if(fdScreenshot != NULL)
            //{
              //  fclose(fdScreenshot);
                picScreenshot = IMG_Load(achScreenshot);
                if( picScreenshot != NULL )
                {
                    rectScreenshot.x = 340;
                    rectScreenshot.y = 86;
                    SDL_BlitSurface(picScreenshot, NULL, screen, &rectScreenshot);
                    SDL_FreeSurface(picScreenshot);
                }
            //}
            
            iMostrando = 1;
            
			//HCF test
			redibujar = 1;
        }
        else
		{
			if( iParado == 0 )
			{
				iMostrando = 0;	
			}
		}
        
        if(redibujar || iPrimeraEjecucion)
        {
            SDL_Rect rectMenu2;
            rectMenu2.x = 20;
            rectMenu2.y = 80;
            SDL_BlitSurface(picRectangle, NULL, screen, &rectMenu2);

            //Select game:
            picTexto = TTF_RenderText_Solid( font, "Select game:", textColor ); 
            rect.x = 40;
            rect.y = 85;
            SDL_BlitSurface(picTexto, NULL, screen, &rect);
            SDL_FreeSurface(picTexto);
            
            contCursor = 0;
            for(i = avanceCursorListado; i < avanceCursorListado + MAX_ROMS_EN_PANTALLA+1; i++ )
            {
                rect.y += ALTO_OPCION;
                
                if(posCursor == i)
                {
                    rect.x = 40;
                    SDL_BlitSurface(picCursor, NULL, screen, &rect);
                }

                rect.x = 56;
                                
                picTexto = TTF_RenderText_Solid( font, aachApps[i], textColor ); 
                if(picTexto->w <= MAX_ANCHO_NOMBRE_FICHERO)
				{
					rectTituloSrc.w = picTexto->w;
				}
				else
				{
					rectTituloSrc.w = MAX_ANCHO_NOMBRE_FICHERO;
				}
				rectTituloSrc.h = picTexto->h;
                
                SDL_BlitSurface(picTexto, &rectTituloSrc, screen, &rect);
                SDL_FreeSurface(picTexto);
                    
                //pspDebugScreenPrintf(aachApps[i]);
                //pspDebugScreenPrintf("\n\n");

            }
            //sceDisplayWaitVblankStart();
                                   
            
            //SDL_BlitSurface(picMenu, NULL, screen, NULL);

            
        }

		SDL_Flip(screen);
                    
        if(!iDesplazamientoRapido)
			SDL_Delay(75); //50);

        iPrimeraEjecucion = 0;
    }
    
    //Libera background
    SDL_FreeSurface(picRectangle);
    SDL_FreeSurface(picMenu);
    SDL_FreeSurface(picCursor);
    
	closedir(d);

    //Para musica
    Mix_HaltMusic();
	Mix_FreeMusic(MiMusica);
	MiMusica = NULL;

    //Sonido boton
    Mix_Chunk *SndBoton = Mix_LoadWAV("snd/button.wav");
	if( SndBoton != NULL )
	{
		phaserChannel = Mix_PlayChannel(-1, SndBoton, 0);
        SDL_Delay(1000);				
		if( phaserChannel != -1 )
		{
			if( Mix_Playing(phaserChannel) )
			{
				Mix_HaltChannel(phaserChannel);
			}
		}
        
		Mix_FreeChunk(SndBoton);
		SndBoton = NULL;
	}

    //SDL_PauseAudio(true);
    Mix_CloseAudio();    
    
    //Clear screen to display machine
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_Flip(screen);
       
}

/******************************************************************************
tar archive, use --format=v7 with gnu tar
******************************************************************************/

//int empezadoAudio = 0;
#define FACTOR_ESCALA 1.0

#ifdef GWROM_USE_TAR_V7

int posAudio = 0;
int posTocado = 0;
int retrasoAudio = 0;
int audioListo = 0;

SDL_Surface *temp_screen = NULL;

typedef union
{
  struct
  {
    char name[ 100 ];
    char mode[ 8 ];
    char owner[ 8 ];
    char group[ 8 ];
    char size[ 12 ];
    char modification[ 12 ];
    char checksum[ 8 ];
    char type;
    char linked[ 100 ];
    
    /*
    a space for the user to store things related to the entry, i.e. data has
    been converted to little endian
    CAUTION: things stored in user_flags are *not* persistent!
    */
    uint32_t user_flags;
    
    /*
    a space for the user to store things related to the entry, i.e. a different
    representation of the data
    CAUTION: things stored in user_flags are *not* persistent!
    */
    void* user_data;
  };
  
  char fill[ 512 ];
}
entry_tar_v7;

static int identify_tar_v7( const void* data, size_t size )
{
  /* tar size is always a multiple of 512 */
  if ( size & 511 )
  {
    return GWROM_INVALID_ROM;
  }
  
  entry_tar_v7* entry = (entry_tar_v7*)data;
  char* end = (char*)data + size - 512;
  
  /* iterate over the entries and do a basic chack on each on of them */
  while ( (char*)entry <= end && entry->name[ 0 ] )
  {
    char* endptr;
    long entry_size = strtol( entry->size, &endptr, 8 );
    
    /* Check for a valid entry size */
    if ( *endptr != 0 ) //|| errno == ERANGE )
    {
      return GWROM_INVALID_ROM;
    }
    
    char* name = entry->name;
    char* endname = name + 100;
    
    /* Check for a valid entry name */
    do
    {
      if ( *name++ < 32 )
      {
        return GWROM_INVALID_ROM;
      }
    }
    while ( *name && name < endname );
    
    /* go to the next entry */
    entry_size = ( entry_size + 511 ) / 512 + 1;
    entry += entry_size;
  }
  
  /* the last entry must be followed by one or more empty entries */
  if ( (char*)entry >= end )
  {
    return GWROM_INVALID_ROM;
  }
  
  /* check for empty entries */
  do
  {
    int i;
    
    for ( i = 0; i < 512; i++ )
    {
      if ( ( (char*)entry )[ i ] != 0 )
      {
        return GWROM_INVALID_ROM;
      }
    }
    
    entry++;
  }
  while ( (char*)entry < end );
  
  return GWROM_OK;
}

static int init_tar_v7( gwrom_t* gwrom )
{
  entry_tar_v7* entry = (entry_tar_v7*)gwrom->data;
  
  while ( entry->name[ 0 ] )
  {
    long entry_size = strtol( entry->size, NULL, 8 );
    
    /* zero user space */
    entry->user_flags = 0;
    entry->user_data = NULL;
    
    /* go to the next entry */
    entry_size = ( entry_size + 511 ) / 512 + 1;
    entry += entry_size;
  }
  
  return GWROM_OK;
}

static int find_tar_v7( gwrom_entry_t* file, gwrom_t* gwrom, const char* file_name )
{
  entry_tar_v7* entry = (entry_tar_v7*)gwrom->data;
  
  while ( entry->name[ 0 ] )
  {
    long entry_size = strtol( entry->size, NULL, 8 );
    
    if ( !strcmp( entry->name, file_name ) )
    {
      /* found the entry, fill in gwrom_entry_t* */
      file->name = entry->name;
      file->data = (void*)( entry + 1 );
      file->size = entry_size;
      file->user_flags = &entry->user_flags;
      
      return GWROM_OK;
    }
    
    /* go to the next entry */
    entry_size = ( entry_size + 511 ) / 512 + 1;
    entry += entry_size;
  }
  
  return GWROM_ENTRY_NOT_FOUND;
}

static void iterate_tar_v7( gwrom_t* gwrom, int (*callback)( gwrom_entry_t*, gwrom_t* ) )
{
  entry_tar_v7* entry = (entry_tar_v7*)gwrom->data;
  gwrom_entry_t file;
  
  while ( entry->name[ 0 ] )
  {
    long entry_size = strtol( entry->size, NULL, 8 );
    
    file.name = entry->name;
    file.data = (void*)( entry + 1 );
    file.size = entry_size;
    file.user_flags = &entry->user_flags;
    file.user_data = &entry->user_data;
    
    if ( !callback( &file, gwrom ) )
    {
      return;
    }
    
    /* go to the next entry */
    entry_size = ( entry_size + 511 ) / 512 + 1;
    entry += entry_size;
  }
}

/* tar doesn't need destruction */
#define destroy_tar_v7 default_destroy

#endif /* GWROM_USE_TAR_V7 */

static void default_destroy( gwrom_t* gwrom )
{
  //(void)gwrom;
}

/* all supported rom types must have an entry here */
static const methods_t methods[] =
{
#ifdef GWROM_USE_TAR_V7
  { identify_tar_v7, init_tar_v7, destroy_tar_v7, find_tar_v7, iterate_tar_v7 },
#endif

  /* add new rom types here */
};


#include "bzip2/bzlib.h"

/* needed because of -DBZ_NO_STDIO, which should be defined for compilation */
void bz_internal_error( int errcode )
{
  (void)errcode;
}

typedef int Int32;

/* use gwrom allocation routines */
void* bzalloc( void* opaque, Int32 items, Int32 size )
{
  (void)opaque;
  return malloc( items * size );
  //return gwrom_malloc( items * size );
}

/* use gwrom allocation routines */
void bzfree( void* opaque, void* addr )
{
  (void)opaque;
  //gwrom_free( addr );
  free( addr );
}

static int identify_bzip2( const void* data, size_t size )
{
  /* basic header check */
  const char* magic = (const char*)data;
  
  if ( magic[ 0 ] != 'B' || magic[ 1 ] != 'Z' || magic[ 2 ] != 'h' ||
       magic[ 3 ] < '0' || magic[ 3 ] > '9' )
  {
    return GWROM_INVALID_ROM;
  }
  
  /* TODO check the signature at the end of the data */
  return GWROM_OK;
}

static int decompress_bzip2( void** new_data, size_t* new_size, void* data, size_t size )
{
  bz_stream stream;
  
  /* setup the decompression stream */
  stream.bzalloc = bzalloc;
  stream.bzfree = bzfree;
  
  int res = BZ2_bzDecompressInit( &stream, 0, 0 );
  
  if ( res != BZ_OK )
  {
    return GWROM_INVALID_ROM;
  }
  
  stream.next_in = (char*)data;
  stream.avail_in = (unsigned)size;
  
#ifdef GWROM_NO_REALLOC
  /* first decompression run: evaluate size of decompressed data */
  for ( ;; )
  {
    char buffer[ GWROM_DECOMP_BUFFER ];
    stream.next_out = buffer;
    stream.avail_out = sizeof( buffer );
    
    res = BZ2_bzDecompress( &stream );
    
    if ( res == BZ_STREAM_END )
    {
      break;
    }
    
    if ( res != BZ_OK )
    {
      BZ2_bzDecompressEnd( &stream );
      return GWROM_INVALID_ROM;
    }
  }
  
  /* basic check for when size_t can't hold 64-bit values */
  if ( sizeof( size_t ) > 4 )
  {
    *new_size = stream.total_out_hi32;
    *new_size = *new_size << 32 | stream.total_out_lo32;
  }
  else
  {
    if ( stream.total_out_hi32 != 0 )
    {
      BZ2_bzDecompressEnd( &stream );
      return GWROM_NO_MEMORY;
    }
    
    *new_size = stream.total_out_lo32;
  }
  
  BZ2_bzDecompressEnd( &stream );
  //*new_data = gwrom_malloc( *new_size );
  *new_data = malloc( *new_size );
  
  if ( !*new_data )
  {
    return GWROM_NO_MEMORY;
  }
  
  /* second decompression run: decompress data to the allocated buffer */
  unsigned dest_len = *new_size;
  res = BZ2_bzBuffToBuffDecompress( (char*)*new_data, &dest_len, (char*)data, size, 0, 0 );
  
  if ( res != BZ_OK )
  {
    return GWROM_INVALID_ROM;
  }
#else
  *new_data = NULL;
  *new_size = 0;
  
  /* decompress while reallocating the decompressed data as necessary */
  for ( ;; )
  {
    char buffer[ GWROM_DECOMP_BUFFER ];
    stream.next_out = buffer;
    stream.avail_out = sizeof( buffer );
    
    res = BZ2_bzDecompress( &stream );
    
    if ( res != BZ_OK && res != BZ_STREAM_END )
    {
      BZ2_bzDecompressEnd( &stream );
      gwrom_free( *new_data );
      return GWROM_INVALID_ROM;
    }
    
    size_t count = sizeof( buffer ) - stream.avail_out;
    
    if ( count )
    {
      char* realloc_data = gwrom_realloc( *new_data, *new_size + count );
      
      if ( realloc_data == NULL )
      {
        gwrom_free( *new_data );
        return GWROM_NO_MEMORY;
      }
      
      *new_data = realloc_data;
      memcpy( (void*)( (char*)*new_data + *new_size ), (void*)buffer, count );
      *new_size += count;
    }
    
    if ( res == BZ_STREAM_END )
    {
      break;
    }
  }
  
  BZ2_bzDecompressEnd( &stream );
#endif

  return GWROM_OK;
}

/* use the default destroy method */
#define destroy_bzip2 default_destroy

/* bzip2 roms don't have any entries in them */
#define find_bzip2 default_find

//#endif /* GWROM_USE_BZIP2 */

/******************************************************************************
uncompressed
******************************************************************************/

static int identify_uncompressed( const void* data, size_t size )
{
  /* uncompressed data is always identified */
  return GWROM_OK;
}

static int inflate_uncompressed( void** new_data, size_t* new_size, void* data, size_t size )
{
  /*
  returns the same data, the caller must check that it wasn't inflated to a new
  buffer if it wants to copy the data into a new buffer
  */
  *new_data = data;
  *new_size = size;
  return GWROM_OK;
}

/******************************************************************************
decompress methods
******************************************************************************/

typedef struct
{
  /* returns GWROM_OK if the decompression method is identified */
  int (*identify)( const void*, size_t );
  
  /* decompresses the rom into a new buffer */
  int (*decompress)( void**, size_t*, void*, size_t );
}
decompress_t;

/* all inflate algorithms must have an entry here */
static const decompress_t decompress[] =
{
#ifdef GWROM_USE_GZIP
  { identify_gzip, decompress_gzip },
#endif

#ifdef GWROM_USE_BZIP2
  { identify_bzip2, decompress_bzip2 },
#endif

  /* add new inflate methods here */
  { identify_uncompressed, inflate_uncompressed },
};

int gwrom_init( gwrom_t* gwrom, void* data, size_t size, uint32_t flags )
{
  void* new_data = data;
  size_t new_size = size;
  unsigned i;
  
  /* check for compressed roms first */
  const decompress_t* decomp = decompress;
  
  for ( i = 0; i < sizeof( decompress ) / sizeof( decompress[ 0 ] ); i++, decomp++ )
  {
    if ( decomp->identify( data, size ) == GWROM_OK )
    {
      if ( decomp->decompress( &new_data, &new_size, data, size ) != GWROM_OK )
      {
        return GWROM_INVALID_ROM;
      }
      
      /* check if data was copied into a new buffer */
      if ( new_data != data )
      {
        /* yes, set flags to free the data */
        flags |= GWROM_FREE_DATA;
      }
      else
      {
        /* no, check if the caller has asked to copy it */
        if ( flags & GWROM_COPY_ALWAYS )
        {
          /* yes, copy data into a new buffer */
          //new_data = gwrom_malloc( size );
            new_data = malloc( size );
          
          if ( !new_data )
          {
            return GWROM_NO_MEMORY;
          }
          
          memcpy( new_data, data, size );
          new_size = size;
          
          /* set flags to free the data */
          flags |= GWROM_FREE_DATA;
        }
      }
      
      break;
    }
  }
  
  /* iterates over the supported types and compress algorithms */
  const methods_t* meth = methods;
  
  for ( i = 0; i < sizeof( methods ) / sizeof( methods[ 0 ] ); i++, meth++ )
  {
    if ( meth->identify( new_data, new_size ) == GWROM_OK )
    {
      /* type was identified, fill in gwrom and call its init method */
      gwrom->data = new_data;
      gwrom->size = new_size;
      gwrom->flags = flags;
      gwrom->destroy = meth->destroy;
      gwrom->find = meth->find;
      gwrom->iterate = meth->iterate;
      return meth->init( gwrom );
    }
  }
  
  /* rom not identified */
  return GWROM_INVALID_ROM;
}

extern "C" void* constcast( const void* ptr )
{
  return const_cast< void* >( ptr );
}

//HCF THE ROM!
gwrom_t rom;
gwlua_t state;
char *data;
extern SDL_Surface* screen;

#include "bzip2/bzlib.h"

typedef struct retro_game_geometry
{
   unsigned base_width;    /* Nominal video width of game. */
   unsigned base_height;   /* Nominal video height of game. */
   unsigned max_width;     /* Maximum possible width of game. */
   unsigned max_height;    /* Maximum possible height of game. */

   float    aspect_ratio;  /* Nominal aspect ratio of game. If
                            * aspect_ratio is <= 0.0, an aspect ratio
                            * of base_width / base_height is assumed.
                            * A frontend could override this setting,
                            * if desired. */
}retro_game_geometry;

struct retro_system_timing
{
   double fps;             /* FPS of video content. */
   double sample_rate;     /* Sampling rate of audio. */
};

struct retro_system_av_info
{
   struct retro_game_geometry geometry;
   struct retro_system_timing timing;
};

void retro_get_system_av_info( struct retro_system_av_info* info )
{
  info->geometry.base_width = SCREEN_WIDTH; //state.width;
  info->geometry.base_height = SCREEN_HEIGHT; //state.height;
  info->geometry.max_width = SCREEN_WIDTH; //state.width;
  info->geometry.max_height = SCREEN_HEIGHT; //state.height;
  info->geometry.aspect_ratio = 0.0f;
  info->timing.fps = 60.0;
  info->timing.sample_rate = 44100.0;
}
const char* gwrom_error_message( int error );
/*
{
  switch ( error )
  {
  case GWROM_OK:              return "Ok";
  case GWROM_INVALID_ROM:     return "Invalid ROM (corrupted file?)";
  case GWROM_NO_MEMORY:       return "Out of memory";
  case GWROM_ENTRY_NOT_FOUND: return "Entry not found";
  }
  
  return "Unknown error";
}
*/

static const struct { unsigned retro; int gw; } map[] =
  {
    { RETRO_DEVICE_ID_JOYPAD_UP,     GWLUA_UP },
    { RETRO_DEVICE_ID_JOYPAD_DOWN,   GWLUA_DOWN },
    { RETRO_DEVICE_ID_JOYPAD_LEFT,   GWLUA_LEFT },
    { RETRO_DEVICE_ID_JOYPAD_RIGHT,  GWLUA_RIGHT },
    { RETRO_DEVICE_ID_JOYPAD_A,      GWLUA_A },
    { RETRO_DEVICE_ID_JOYPAD_B,      GWLUA_B },
    { RETRO_DEVICE_ID_JOYPAD_X,      GWLUA_X },
    { RETRO_DEVICE_ID_JOYPAD_Y,      GWLUA_Y },
    { RETRO_DEVICE_ID_JOYPAD_L,      GWLUA_L1 },
    { RETRO_DEVICE_ID_JOYPAD_R,      GWLUA_R1 },
    { RETRO_DEVICE_ID_JOYPAD_L2,     GWLUA_L2 },
    { RETRO_DEVICE_ID_JOYPAD_R2,     GWLUA_R2 },
    { RETRO_DEVICE_ID_JOYPAD_L3,     GWLUA_L3 },
    { RETRO_DEVICE_ID_JOYPAD_R3,     GWLUA_R3 },
    { RETRO_DEVICE_ID_JOYPAD_SELECT, GWLUA_SELECT },
    { RETRO_DEVICE_ID_JOYPAD_START,  GWLUA_START },
  };
  
void vdInput()
{
	vdRellenaBotones();
    //SDL_Event event;

    for(int i = 0; i <= GWLUA_START; i++)
        gwlua_set_button( &state, i, 0 );
						
	
	if(ashBotones[BOTON_ARRIBA])
			gwlua_set_button( &state, GWLUA_UP, 1 );

		if(ashBotones[BOTON_ABAJO])
			gwlua_set_button( &state, GWLUA_DOWN, 1 );
			
		if(ashBotones[BOTON_IZQUIERDA])
			gwlua_set_button( &state, GWLUA_LEFT, 1 );

		if(ashBotones[BOTON_DERECHA])
			gwlua_set_button( &state, GWLUA_RIGHT, 1 );
		
		if(ashBotones[BOTON_AA])
			gwlua_set_button( &state, GWLUA_B, 1 );

		if(ashBotones[BOTON_BB])
			gwlua_set_button( &state, GWLUA_A, 1 );

		if(ashBotones[BOTON_XX])
			gwlua_set_button( &state, GWLUA_Y, 1 );

		if(ashBotones[BOTON_YY])
			gwlua_set_button( &state, GWLUA_X, 1 );
		
		if(ashBotones[BOTON_LTRIGGER] && ashBotones[BOTON_RTRIGGER])  //EXIT
            salirderom = 1;
		
		if(ashBotones[BOTON_LTRIGGER])
			gwlua_set_button( &state, GWLUA_L1, 1 );  //GAME A
	    	
		if(ashBotones[BOTON_RTRIGGER])
			gwlua_set_button( &state, GWLUA_R1, 1 );  //GAME B
		                    
	    
		if(ashBotones[BOTON_NEGRO])
			gwlua_set_button( &state, GWLUA_L2, 1 );   //TIME

		if(ashBotones[BOTON_START])
			gwlua_set_button( &state, GWLUA_START, 1 );
				
		if(ashPulsaciones[BOTON_BACK])
		{
	        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
			gwlua_set_button( &state, GWLUA_SELECT, 1 );
		}
	//}
}


extern int     offset;
extern int     soft_width;
extern int     soft_height;

int
SDL_SoftStretchHCF(SDL_Surface * src, const SDL_Rect * srcrect,
                SDL_Surface * dst, const SDL_Rect * dstrect, int parche, int inAvoidBegin, int inAvoidEnd);

                
/* Render a frame. Pixel format is 15-bit 0RGB1555 native endian 
 * unless changed (see RETRO_ENVIRONMENT_SET_PIXEL_FORMAT).
 *
 * Width and height specify dimensions of buffer.
 * Pitch specifices length in bytes between two lines in buffer.
 */
void video_cb(void *data, unsigned width, unsigned height, size_t pitch)
{
    int i, j;
    char *runnerSrc;
    char *runnerDst;

	if(temp_screen == NULL)
	{
		temp_screen = SDL_CreateRGBSurface(screen->flags, width, height, 16, 0xF800, 0x07E0, 0x001F, 0); //screen->format->Amask);
	}
	else
	{
		if(width > temp_screen->w || height > temp_screen->h)
		{
			SDL_FreeSurface(temp_screen);
			temp_screen = SDL_CreateRGBSurface(screen->flags, width, height, 16, 0xF800, 0x07E0, 0x001F, 0);
		}
	}

    int acumulado = 0;
    int indexSrc, indexDst;
    
    runnerSrc = (char*)data;
    runnerDst = (char*)temp_screen->pixels;

    //SDL_LockSurface(temp_screen);
     
          
    indexSrc = indexDst = 0;
    for (i = 0; i < height; i++)
    {
            memcpy(&runnerDst[indexDst], &runnerSrc[indexSrc], pitch );
            runnerSrc += pitch;
            //runnerDst += pitch;
            runnerDst += temp_screen->pitch; //screen->w * 2;
            //screen->pitch;
    }
    

    //SDL_UnlockSurface(temp_screen);


	//Se calcula el factor de zoom para que quepa en pantalla
    float f1, f2;

    SDL_Rect recta;
    SDL_Rect rectorig;

#define NO_AVOID 9999

    //if(screen->w > picPeq->w)
    if(inNeedsSpecialZoom == SPECIAL_ZOOM_NONE || !bIsZoomEnabled)
	{
		//Se calcula el factor de zoom para que quepa en pantalla
	    f1 = ((1.0)*width) / screen->w;
	    f2 = ((1.0)*height) / screen->h;
		if(f1 < f2)
			f1 = f2;

		if(screen->w > (width / f1) )
			recta.x = (screen->w - width/f1) / 2;
		else
			recta.x = 0;

		//if(screen->h > picPeq->h)
		if(screen->h > (height / f1) )
			recta.y = (screen->h - height/f1) / 2;
		else
			recta.y = 0;
    
		rectorig.x = 0;
		rectorig.y = 0;
		rectorig.w = width;
		rectorig.h = height;

		recta.w = (width / f1); //screen->w;
		recta.h = (height / f1);  //screen->h;
	
		SDL_SoftStretchHCF(temp_screen, &rectorig, screen, &recta, 0, NO_AVOID, NO_AVOID);
    
	}
	else if(inNeedsSpecialZoom == SPECIAL_ZOOM_VERTICAL)
	{
		#define HIDE_VERTICAL_TOP 101
		#define HIDE_VERTICAL_LEFT 142   //137     //125
		#define AVOID_BEGIN 125  //130  //100
		#define AVOID_END   225  //230  //175  //150
		#define AVOID_SOURCE   125  //95  //90 

		//Se calcula el factor de zoom para que quepa en pantalla
	    f1 = ((1.0)*(width-2*HIDE_VERTICAL_LEFT)) / screen->w;
	    f2 = ((1.0)*( ( (height-2*HIDE_VERTICAL_TOP) - (AVOID_SOURCE)) )) / screen->h;
		
		if(f1 < f2)
			f1 = f2;

		if(screen->w > ((width-2*HIDE_VERTICAL_LEFT) / f1) )
			recta.x = (screen->w - (width-2*HIDE_VERTICAL_LEFT)/f1) / 2;
		else
			recta.x = 0;

		if(screen->h > ( ( (height-2*HIDE_VERTICAL_TOP) - (AVOID_SOURCE))/ f1) )
			recta.y = (screen->h - ( ( (height-2*HIDE_VERTICAL_TOP) - (AVOID_SOURCE))/ f1) ) / 2;
		else
			recta.y = 0;
    
		rectorig.x = HIDE_VERTICAL_LEFT;
		rectorig.y = HIDE_VERTICAL_TOP;
		rectorig.w = (width-2*HIDE_VERTICAL_LEFT);
		rectorig.h = ( ( (height-2*HIDE_VERTICAL_TOP) - (AVOID_SOURCE)) );  //screen->h;

		recta.w = ((width-2*HIDE_VERTICAL_LEFT) / f1); //screen->w;
		recta.h = ( ( (height-2*HIDE_VERTICAL_TOP) - (AVOID_END - AVOID_BEGIN))/ f1);  //screen->h;

		SDL_SoftStretchHCF(temp_screen, &rectorig, screen, &recta, 17, AVOID_BEGIN, AVOID_END);
     
	}
	else
	{
		//Horizontal
		#define HIDE_HORIZONTAL_TOP 65  //60  //85   //95  //101
		#define HIDE_HORIZONTAL_BOTTOM 265  //270 
		#define HIDE_HORIZONTAL_LEFT 98  // 100   //137     //125

		//Se calcula el factor de zoom para que quepa en pantalla
	    f1 = ((1.0)*(width-2*HIDE_HORIZONTAL_LEFT)) / screen->w;
	    f2 = ((1.0)*(height-5*HIDE_HORIZONTAL_TOP)) / screen->h;
		if(f1 < f2)
			f1 = f2;

		if(screen->w > ((width-2*HIDE_HORIZONTAL_LEFT) / f1) )
			recta.x = (screen->w - (width-2*HIDE_HORIZONTAL_LEFT)/f1) / 2;
		else
			recta.x = 0;

		if(screen->h > ((height-HIDE_HORIZONTAL_BOTTOM) / f1) )
			recta.y = (screen->h - (height-HIDE_HORIZONTAL_BOTTOM)/f1) / 2;
		else
			recta.y = 0;
    
		rectorig.x = HIDE_HORIZONTAL_LEFT;
		rectorig.y = HIDE_HORIZONTAL_TOP;
		rectorig.w = (width-2*HIDE_HORIZONTAL_LEFT);
		rectorig.h = (height-HIDE_HORIZONTAL_BOTTOM);

		recta.w = ((width-2*HIDE_HORIZONTAL_LEFT) / f1); //screen->w;
		recta.h = ((height-HIDE_HORIZONTAL_BOTTOM) / f1);  //screen->h;

		SDL_SoftStretchHCF(temp_screen, &rectorig, screen, &recta, 0, NO_AVOID, NO_AVOID);
    
	}
		
    SDL_Flip(screen);
}

//AUDIO
unsigned int LeftFIFOHeadPtr, LeftFIFOTailPtr, RightFIFOHeadPtr, RightFIFOTailPtr;
SDL_AudioSpec desired;
//#define BUFFER_SIZE 0x8000
#define BUFFER_SIZE 0x80000

//unsigned char DACBuffer[BUFFER_SIZE];
unsigned char *DACBuffer;
void SDLSoundCallback(void * userdata, Uint8 * buffer, int length)
{
	int iCopyAux, iCopyTotal;

	
    //if(/*audioListo == 0 ||*/ retrasoAudio <= 0)
    if(/*audioListo == 0 ||*/ retrasoAudio <= length)
    {
        //memset(buffer, desired.silence, length);
        return;
    }
    //Queremos tocar lo que llevamos de retraso, mas lo de este nuevo frame
    int atocar = retrasoAudio; // + (RL_SAMPLES_PER_FRAME * 2);
    if(length < atocar)
    {
        retrasoAudio = atocar - length;
        atocar = length;
    }
    else
    {
        retrasoAudio = 0;
    }

    //retrasoAudio = retrasoaudio((RL_SAMPLES_PER_FRAME * 2) - atocar);

	if(posTocado + atocar < BUFFER_SIZE)
    {
        memcpy(buffer, &DACBuffer[posTocado], atocar);
        posTocado += atocar; // %(BUFFER_SIZE);
    }
    else
    {
        memcpy(buffer, &DACBuffer[posTocado], BUFFER_SIZE - posTocado);
        memcpy(&buffer[BUFFER_SIZE - posTocado], &DACBuffer[0], atocar - (BUFFER_SIZE - posTocado));
        posTocado = atocar - (BUFFER_SIZE - posTocado);
    }
        
    return;
    
	// Clear the buffer to silence, in case the DAC buffer is empty (or short)
	//HCF v1.0
	//memset(buffer, desired.silence, length);

	
}

void vdCore()
{
    char fn_r[512];
    BZFILE *BZ2fp_r = NULL;
    int len;
    char *buff;  //[0x1000];
    int t1, t2, t3;
    int frameskip;
	int currentframe;
        
    scePowerSetClockFrequency(333, 333, 166);

    #define MAX_DISPLAY_FRAMERATE 16
    #ifdef SHOW_FRAMERATE
        int inTimeStartingSecond = 0;
        int inFramesCurrentSecond = 0;
        pspDebugScreenInit();
        char achDisplayFramerate[MAX_DISPLAY_FRAMERATE];
    #endif
    
    //HCF
    //Alloc the Volatile memory
    int HCF_RAM_SIZE;  
    //int reta = sceKernelVolatileMemLock(0,	&HCF_RAM_ARRAY, &HCF_RAM_SIZE); 
    HCF_RAM_ARRAY  = malloc(4 * 1024 * 1024);
	data = (char*)HCF_RAM_ARRAY;
    //Instead of malloc, we try with volatile memory
    
     //AUDIO
	desired.freq = 44100;  //22050; //44100;		// SDL will do conversion on the fly, if it can't get the exact rate. Nice!
	desired.format = AUDIO_S16SYS;					// This uses the native endian (for portability)...
    
    //desired.padding = ;;

	desired.channels = 2;
	desired.samples = 4096; //1024; //4096; //2048; //4096;							// Let's try a 4K buffer (can always go lower)
	desired.callback = SDLSoundCallback;
    
    DACBuffer = (unsigned char*)HCF_RAM_ARRAY;
 
    temp_screen = NULL;
    
    font = TTF_OpenFont( "tahoma.ttf", 12 ); 
        
	//MAIN LOOP
    while(1)
    {
        vdSelectApp();
                
        strcpy(fn_r, "roms/");
        strcat(fn_r, achAppSeleccionada);
      
        strcpy(achHighScoreFile, "high/");
        strcat(achHighScoreFile, achAppSeleccionada);
        achHighScoreFile[strlen(achHighScoreFile) - 3] = '\0';
        strcat( achHighScoreFile, "sco" );
        
        vdLoadHighscore(achHighScoreFile);
      
        if( !strcasecmp(achAppSeleccionada, "Donkey Kong (Nintendo, Multi Screen).mgw")
            || !strcasecmp(achAppSeleccionada, "Donkey Kong II (Nintendo, Multi Screen).mgw"))
        {
            inNeedsSpecialZoom = SPECIAL_ZOOM_VERTICAL;
        }
		else if( !strcasecmp(achAppSeleccionada, "Lifeboat (Nintendo, Multi Screen).mgw") )
        {
            inNeedsSpecialZoom = SPECIAL_ZOOM_HORIZONTAL;
        }
		else if( !strcasecmp(achAppSeleccionada, "Mario Bros. (Nintendo, Multi Screen).mgw") )
        {
            inNeedsSpecialZoom = SPECIAL_ZOOM_HORIZONTAL;
        }
		else if( !strcasecmp(achAppSeleccionada, "Penguin Land (Bandai, LSI Game Double Play).mgw") )
        {
            inNeedsSpecialZoom = SPECIAL_ZOOM_HORIZONTAL;
        }
		else
        {
            inNeedsSpecialZoom = SPECIAL_ZOOM_NONE;
        }
        
        int posi = 0;

		buff = (char*)malloc(0x1000);
       
         if((fn_r == NULL && (BZ2fp_r = BZ2_bzdopen(fileno(stdin),"rb"))==NULL)
                || (fn_r != NULL && (BZ2fp_r = BZ2_bzopen(fn_r,"rb"))==NULL)){
                printf("can't bz2openstream\n");
                exit(1);
             }
             while((len=BZ2_bzread(BZ2fp_r,buff,0x1000))>0)
             {
                memcpy(&data[posi], buff, len);
                posi += len;
             }
             BZ2_bzclose(BZ2fp_r);

		
        //HCF: "rom" is initialized inside of this call
        //gwrom_init( &rom, constcast( info->data ), info->size, GWROM_COPY_ALWAYS );
        
		///gwrom_init( &rom, constcast( data ), posi, GWROM_COPY_ALWAYS );
		gwrom_init( &rom, constcast( data ), posi, 0 );

		
        //HCF
        //Free the Volatile memory
        //NO! Use it for audio!
        //sceKernelVolatileMemUnlock(0);
        
		free(buff);
        //free(data);
        
        
        temp_screen = NULL;

		rl_sound_init();

		rl_sprite_init();
        
        gwlua_create( &state, &rom, SDL_GetTicks()*1000 );
        struct retro_system_av_info info;
        retro_get_system_av_info( &info );
        
		int primeravez = 1;

       frameskip = currentframe = 0;
       t1 = SDL_GetTicks();
       
		//Init audio after having played the background music for selection
       SDL_OpenAudio(&desired, NULL);
       LeftFIFOHeadPtr = LeftFIFOTailPtr = 0, RightFIFOHeadPtr = RightFIFOTailPtr = 1;
       SDL_PauseAudio(false);							// Start playback!
       
       #ifdef SHOW_FRAMERATE
            inTimeStartingSecond = 0;
            inFramesCurrentSecond = 0;
            memset(achDisplayFramerate, 0x00, MAX_DISPLAY_FRAMERATE);
        #endif
       
       salirderom = 0;
        while(!salirderom)
        {
            #ifdef FRAMESKIP
            if(currentframe == 0)
            {
                t2 = SDL_GetTicks();
                t3 = t2 - t1;

				//60 FPS
				
                #define MAX_EXPECTED_FRAME_TIME 17
                #define MIN_EXPECTED_FRAME_TIME 16
        		#define MAX_FRAMESKIP 5

                if( (t3 > ((frameskip+1) * MAX_EXPECTED_FRAME_TIME))  )
                {
                    if(frameskip < MAX_FRAMESKIP) 
                        frameskip++;
                }
                else if( (t3 < ((frameskip+1) * MIN_EXPECTED_FRAME_TIME)) )
                {	
                    if(frameskip > 0)
                        frameskip--;
                }
                
                t1 = t2;
                
                #ifdef SHOW_FRAMERATE
                    if(t2 >= inTimeStartingSecond + 1000)
                    {
                        sprintf(achDisplayFramerate, "%d/60 %d", inFramesCurrentSecond, frameskip);
                        inTimeStartingSecond = t2;
                        inFramesCurrentSecond = 0;
                    }
                    //inFramesCurrentSecond++;
                #endif

            }
            
			#ifdef SHOW_FRAMERATE
            inFramesCurrentSecond++;
			#endif
            
            #endif
        
            //FALTA INPUT
            vdInput();

            gwlua_tick( &state, SDL_GetTicks()*1000 );
      
			//Frameskip agresivo
              #ifdef FRAMESKIP
              if(currentframe == 0)
              {
              #endif
                
      
              rl_sprites_blit();

              /*
              #ifdef FRAMESKIP
              if(currentframe == 0)
              #endif
                */
              
                  video_cb( state.screen + offset, soft_width, soft_height, state.width * sizeof( uint16_t ) );
              
              rl_sprites_unblit();
      
      
                #ifdef SHOW_FRAMERATE
                    pspDebugScreenSetXY(10, 10);
                    pspDebugScreenPrintf(achDisplayFramerate);
                #endif
                
              #ifdef FRAMESKIP
              }
              #endif
              
              
              //if(empezadoAudio == 1)
              //{
              //FALTA AUDIO
              if(posAudio + RL_SAMPLES_PER_FRAME * 4 < BUFFER_SIZE)
              {
                  memcpy(&DACBuffer[posAudio], rl_sound_mix(), RL_SAMPLES_PER_FRAME * 4);
                  posAudio = (posAudio + (RL_SAMPLES_PER_FRAME * 4)); // % (BUFFER_SIZE * sizeof(unsigned char));
                  retrasoAudio += (RL_SAMPLES_PER_FRAME * 4);
              }
              else
              { 
                  //Si copiamos todo, nos saldriamos del array
                  char achAux[RL_SAMPLES_PER_FRAME * 4 * 2];
                  memcpy(achAux, rl_sound_mix(), RL_SAMPLES_PER_FRAME * 4);

                  memcpy(&DACBuffer[posAudio], achAux, BUFFER_SIZE - posAudio);
                  memcpy(&DACBuffer[0], &achAux[BUFFER_SIZE - posAudio], RL_SAMPLES_PER_FRAME * 4 - (BUFFER_SIZE - posAudio));
                  posAudio = RL_SAMPLES_PER_FRAME * 4 - (BUFFER_SIZE - posAudio);
                  retrasoAudio += (RL_SAMPLES_PER_FRAME * 4);
              }
              
              //}
              //audioListo = 1;

              //memcpy(DACBuffer, rl_sound_mix(), RL_SAMPLES_PER_FRAME * 2);
              //audio_cb( rl_sound_mix(), RL_SAMPLES_PER_FRAME );

              
              #ifdef FRAMESKIP
               //HCF for frameskip
               currentframe++;
               if(currentframe > frameskip)
                   currentframe = 0;
              #endif
        }

		SDL_FreeSurface(temp_screen);
        temp_screen = NULL;

		SDL_PauseAudio(true);
        SDL_CloseAudio();

        vdSaveHighscore(achHighScoreFile);
        
		gwlua_destroy(&state);

		gwrom_destroy(&rom);

		//free(rom.data);
		
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
         
    } //while(1)
}