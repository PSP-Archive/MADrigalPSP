#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdint.h>
#include <stddef.h>

/*
typedef signed short int16_t;
typedef signed long int int32_t;
typedef signed char int8_t;
typedef long long int64_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef long unsigned int uint32_t;
typedef unsigned long long uint64_t;
*/

/*
#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 800  //242
*/

/*
#define SCREEN_WIDTH  640   
#define SCREEN_HEIGHT 480
*/


#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272


/* internal flags (1 << 16 to 1 << 23) */
#define GWROM_FREE_DATA ( 1 << 16 )

/* use tar v7 archives */
#define GWROM_USE_TAR_V7

/* use zlib for decompression */
/*#define GWROM_USE_GZIP*/

/* use bzip2 for decompression */
#define GWROM_USE_BZIP2

/*
prevents memory reallocation during bzip2 decompression
note that in this case gwrom will do two decompression runs, one to evaluate
the decompressed size and another to decompress the data to the allocated
buffer, but memory will be entirely allocated in one go
*/
#define GWROM_NO_REALLOC 1

/* size of the buffer used during decompression */
#define GWROM_DECOMP_BUFFER 65536

/*---------------------------------------------------------------------------*/
/* flags to gwrom_init (1 << 0 to 1 << 15) */

/*
makes gwrom_init copy the rom into a new buffer, even if it's unecessary (i.e.
data wasn't compressed)
*/
#define GWROM_COPY_ALWAYS 1

/*---------------------------------------------------------------------------*/

/* everything went ok */
#define GWROM_OK 0

/* invalid rom format */
#define GWROM_INVALID_ROM -1

/* memory allocation error */
#define GWROM_NO_MEMORY -2

/* entry not found in the rom */
#define GWROM_ENTRY_NOT_FOUND -3

typedef struct
{
  /* entry name */
  const char* name;
  /* pointer to entry data */
  void*       data;
  /* entry data size */
  size_t      size;
  /* internal gwrom flags for the entry */
  uint32_t    flags;
  
  /* persistent (between gwrom_init and gwrom_destroy) user flags */
  uint32_t* user_flags;
  
  /* available to the user */
  void** user_data;
}
gwrom_entry_t;

typedef struct gwrom_t
{
  /* rom data */
  void*    data;
  /* rom data size */
  size_t   size;
  /* internal gwrom flags for the rom */
  uint32_t flags;
  
  /* persistent (between gwrom_init and gwrom_destroy) user flags */
  uint32_t user_flags;
  
  /* available to the user */
  void* user_data;
  
  /* frees all memory allocated to the rom */
  void (*destroy)( gwrom_t* );
  /* finds an entry in the rom */
  int  (*find)( gwrom_entry_t*, gwrom_t*, const char* );
  /* iterates over all rom entries */
  void (*iterate)( gwrom_t*, int (*)( gwrom_entry_t*, gwrom_t* ) );
}gwrom_t;

typedef struct
{
  /* returns GWROM_OK if the rom type is identified */
  int (*identify)( const void*, size_t );
  
  /* initializes the rom */
  int (*init)( gwrom_t* );
  
  /* frees all memory allocated to the rom */
  void (*destroy)( gwrom_t* );
  
  /* finds an entry in the rom */
  int (*find)( gwrom_entry_t*, gwrom_t*, const char* );
  
  /* iterates over all rom entries */
  void (*iterate)( gwrom_t*, int (*)( gwrom_entry_t*, gwrom_t* ) );
}
methods_t;

const char* gwrom_error_message( int error );

/* finds an entry in the rom */
#define gwrom_find( entry, gwrom, file_name ) ( ( gwrom )->find( entry, gwrom, file_name ) )

/* iterates over all rom entries */
#define gwrom_iterate( gwrom, callback ) ( ( gwrom )->iterate( gwrom, callback ) )


#define RETRO_DEVICE_ID_JOYPAD_B        0
#define RETRO_DEVICE_ID_JOYPAD_Y        1
#define RETRO_DEVICE_ID_JOYPAD_SELECT   2
#define RETRO_DEVICE_ID_JOYPAD_START    3
#define RETRO_DEVICE_ID_JOYPAD_UP       4
#define RETRO_DEVICE_ID_JOYPAD_DOWN     5
#define RETRO_DEVICE_ID_JOYPAD_LEFT     6
#define RETRO_DEVICE_ID_JOYPAD_RIGHT    7
#define RETRO_DEVICE_ID_JOYPAD_A        8
#define RETRO_DEVICE_ID_JOYPAD_X        9
#define RETRO_DEVICE_ID_JOYPAD_L       10
#define RETRO_DEVICE_ID_JOYPAD_R       11
#define RETRO_DEVICE_ID_JOYPAD_L2      12
#define RETRO_DEVICE_ID_JOYPAD_R2      13
#define RETRO_DEVICE_ID_JOYPAD_L3      14
#define RETRO_DEVICE_ID_JOYPAD_R3      15



#endif
