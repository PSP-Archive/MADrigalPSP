#include "gwlua.h"

#include "../global.h"

#include <string.h>
#include <stdlib.h>

//#include <gwrom.h>
#include "bsreader.h"

#include "../lua/lua.h"
#include "../lua/lauxlib.h"
#include "../lua/lualib.h"

/*static*/ int     offset;
/*static*/ int     soft_width;
/*static*/ int     soft_height;

#define SRAM_MAX 8

typedef struct
{
  char types[ SRAM_MAX ];
  char keys[ SRAM_MAX ][ 32 ];
  char values[ SRAM_MAX ][ 64 ];
  char count;
}
sram_t;
static sram_t  sram;

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

#if 0
static void dump_stack( lua_State* L, const char* title )
{
  printf( "================================\n%s\n", title );
  int top = lua_gettop( L );
  int i;
  
  for ( i = 1; i <= top; i++ )
  {
    printf( "%2d %3d ", i, i - top - 1 );
    
    lua_pushvalue( L, i );
    
    switch ( lua_type( L, -1 ) )
    {
    case LUA_TNIL:
      printf( "nil\n" );
      break;
    case LUA_TNUMBER:
      printf( "%e\n", lua_tonumber( L, -1 ) );
      break;
    case LUA_TBOOLEAN:
      printf( "%s\n", lua_toboolean( L, -1 ) ? "true" : "false" );
      break;
    case LUA_TSTRING:
      printf( "\"%s\"\n", lua_tostring( L, -1 ) );
      break;
    case LUA_TTABLE:
      printf( "table\n" );
      break;
    case LUA_TFUNCTION:
      printf( "function\n" );
      break;
    case LUA_TUSERDATA:
      printf( "userdata\n" );
      break;
    case LUA_TTHREAD:
      printf( "thread\n" );
      break;
    case LUA_TLIGHTUSERDATA:
      printf( "light userdata\n" );
      break;
    default:
      printf( "?\n" );
      break;
    }
  }
  
  lua_settop( L, top );
}
#endif

static void* l_alloc( void* ud, void* ptr, size_t osize, size_t nsize )
{
  (void)ud;
  (void)osize;
  
  if ( nsize == 0 )
  {
    if ( ptr )
    {
      //gwlua_free( ptr );
        free( ptr );
    }
    
    return NULL;
  }
  
  return realloc( ptr, nsize );
  //return gwlua_realloc( ptr, nsize );
}

static int l_traceback( lua_State* L )
{
  luaL_traceback( L, L, lua_tostring( L, -1 ), 1 );
  return 1;
}

static int l_pcall( lua_State* L, int nargs, int nres )
{
  lua_pushcfunction( L, l_traceback );
  
  lua_insert( L, -nargs - 2 );
  
  if ( lua_pcall( L, nargs, nres, -nargs - 2 ) != LUA_OK )
  {
    gwlua_log( "\n==============================\n%s\n------------------------------\n", lua_tostring( L, -1 ) );
    return -1;
  }
  
  return 0;
}

void register_functions( lua_State* L, gwlua_t* state );

static int l_create( lua_State* L )
{
  gwlua_t* state = (gwlua_t*)lua_touserdata( L, 1 );
  
  register_functions( L, state );
  
  gwrom_entry_t entry;
  int error = gwrom_find( &entry, state->rom, "main.bs" );
  
  if ( error != GWROM_OK )
  {
    return luaL_error( L, "%s", gwrom_error_message( error ) );
  }
  
  void* bs = bsnew( entry.data );
  
  if ( !bs )
  {
    return luaL_error( L, "out of memory" );
  }
  
  if ( lua_load( L, bsread, bs, "main.lua", "t" ) != LUA_OK )
  {
    free( bs );
    return lua_error( L );
  }
  
  free( bs );
  
  lua_call( L, 0, 1 );
  gwlua_ref_create( L, -1, &state->tick_ref );
  return 0;
}



int gwlua_create( gwlua_t* state, gwrom_t* rom, int64_t now )
{
    static const luaL_Reg lualibs[] =
    {
        { "_G", luaopen_base },
        { LUA_LOADLIBNAME, luaopen_package },
        { LUA_COLIBNAME, luaopen_coroutine },
        { LUA_TABLIBNAME, luaopen_table },
        // { LUA_IOLIBNAME, luaopen_io }, // remove because of tmpfile
        // { LUA_OSLIBNAME, luaopen_os }, // remove because of system
        { LUA_STRLIBNAME, luaopen_string },
        { LUA_MATHLIBNAME, luaopen_math },
        { LUA_UTF8LIBNAME, luaopen_utf8 },
        { LUA_DBLIBNAME, luaopen_debug },
    };
  
	
  state->L = lua_newstate( l_alloc, NULL );
  
  if ( !state->L )
  {
    return -1;
  }
  
  
#ifndef NDEBUG
  lua_pushboolean( state->L, 1 );
  lua_setglobal( state->L, "_DEBUG" );
#endif
    
   int i;
  
   for ( i = 0; i < sizeof( lualibs ) / sizeof( lualibs[ 0 ] ); i++ )
   {
       luaL_requiref( state->L, lualibs[ i ].name, lualibs[ i ].func, 1 );
       lua_pop( state->L, 1 );
   }
  //luaL_openlibs( state->L );
  
  state->rom = rom;
  state->width = state->height = 0;
  state->screen = NULL;
  state->help = 0;
  state->now = now;
  memset( (void*)state->input, 0, sizeof( state->input ) );
  state->tick_ref = LUA_NOREF;
  
  lua_pushcfunction( state->L, l_create );
  
  lua_pushlightuserdata( state->L, (void*)state );
  
  //HCF: l_pcall is the memory leak
  if ( l_pcall( state->L, 1, 0 ) )
  {
    lua_close( state->L );
    state->L = NULL;
    return -1;
  }
  
  return 0;
}

void gwlua_destroy( gwlua_t* state )
{
  if(state->L)
  {
    lua_close( state->L );
    state->L = NULL;
  }

  //HCF failed test
  //free(state->rom->data);
  
  
}

int gwlua_reset( gwlua_t* state )
{
  gwrom_t* rom = state->rom;
  int64_t now = state->now;
  gwlua_destroy( state );
  return gwlua_create( state, rom, now );
}

/*---------------------------------------------------------------------------*/

bool bIsZoomEnabled = false;

void gwlua_set_button( gwlua_t* state, int button, int pressed )
{
  static bool selectPressed = false;

  if ( button != GWLUA_START )
  {
    state->input[ button ] = pressed;

	if(button == GWLUA_SELECT)
	{
		if(!selectPressed && pressed)
			bIsZoomEnabled = !bIsZoomEnabled;

		selectPressed = pressed;
	}
  }
  else
  {
	//HCF
	state->input[ GWLUA_START ] = pressed;
	if(pressed)
		state->help = !state->help;

	/*
    if ( pressed )
    {
      if ( !state->input[ GWLUA_START ] )
      {
        state->input[ GWLUA_START ] = 1;
        state->help = !state->help;
      }
    }
    else
    {
      state->input[ GWLUA_START ] = 0;
    }
	*/
  }
}

/*---------------------------------------------------------------------------*/

void gwlua_tick( gwlua_t* state, int64_t now )
{
  state->now = now;
  
  gwlua_ref_get( state->L, state->tick_ref );
  l_pcall( state->L, 0, 0 );
  lua_gc( state->L, LUA_GCSTEP, 0 );
}

/*---------------------------------------------------------------------------*/

uint32_t gwlua_djb2( const char* str )
{
  const uint8_t* aux = (const uint8_t*)str;
  uint32_t hash = 5381;

  while ( *aux )
  {
    hash = ( hash << 5 ) + hash + *aux++;
  }

  return hash;
}

/*---------------------------------------------------------------------------*/

void gwlua_log( const char* format, ... )
{
  /*
    va_list args;
  
  va_start( args, format );
  gwlua_vlog( format, args );
  va_end( args );
  */
}
//////////////////////////////

//ANDRE's PATCH
static int find_key( const char* key )
{
  for ( int i = 0; i < sram.count; i++ )
  {
    if ( !strcmp( sram.keys[ i ], key ) )
    {
      return i;
    }
  }
  
  return -1;
}

const char* gwlua_load_value( gwlua_t* state, const char* key, int* type )
{
  int i = find_key( key );

  if ( i != -1 )
  {
    *type = sram.types[ i ];
    return sram.values[ i ];
  }
  
  return NULL;
}

void gwlua_save_value( gwlua_t* state, const char* key, const char* value, int type )
{
  int i = find_key( key );

  if ( i == -1 )
  {
    if ( sram.count == SRAM_MAX )
    {
      /* TODO: return an error when SRAM is full */
      //log_cb( RETRO_LOG_ERROR, "Out of space writing <%s, %s> to SRAM\n", key, value );
      return;
    }

    i = sram.count++;
  }

  sram.types[ i ] = type;
  
  strncpy( sram.keys[ i ], key, sizeof( sram.keys[ i ] ) );
  sram.keys[ i ][ sizeof( sram.keys[ i ] ) - 1 ] = 0;
  
  strncpy( sram.values[ i ], value, sizeof( sram.values[ i ] ) );
  sram.values[ i ][ sizeof( sram.values[ i ] ) - 1 ] = 0;
}

//////////////

//Original methods
/***
const char* gwlua_load_value( gwlua_t* state, const char* key, int* type )
{
  for ( int i = 0; i < sram.count; i++ )
  {
    if ( !strcmp( sram.keys[ i ], key ) )
    {
      *type = sram.types[ i ];
      return sram.values[ i ];
    }
  }
  
  return NULL;
}

void gwlua_save_value( gwlua_t* state, const char* key, const char* value, int type )
{
  
  //HCF: If the key already existed, we don't need to create a new one. Thanks Andre Leiradella!

  //for ( int i = 0; i < sram.count; i++ )
  //{
   // if ( !strcmp( sram.keys[ i ], key ) )
    //{
     //   strncpy( sram.values[ i ], value, sizeof( sram.values[ i ] ) );
      //  sram.values[ i ][ sizeof( sram.values[ i ] ) - 1 ] = 0;
       // return;
    //}
  //}
  
  
  if ( sram.count < SRAM_MAX )
  {
    int ndx = sram.count++;
    
    sram.types[ ndx ] = type;
    
    strncpy( sram.keys[ ndx ], key, sizeof( sram.keys[ ndx ] ) );
    sram.keys[ ndx ][ sizeof( sram.keys[ ndx ] ) - 1 ] = 0;
    
    strncpy( sram.values[ ndx ], value, sizeof( sram.values[ ndx ] ) );
    sram.values[ ndx ][ sizeof( sram.values[ ndx ] ) - 1 ] = 0;
  }
  
  // TODO: return an error when SRAM is full 
}
*****/

int gwlua_set_fb( unsigned width, unsigned height )
{
  struct retro_game_geometry geometry;
  
  geometry.base_width = width;
  geometry.base_height = height;
  geometry.max_width = width;
  geometry.max_height = height;
  geometry.aspect_ratio = 0.0f;
  
  //HCF
  //env_cb( RETRO_ENVIRONMENT_SET_GEOMETRY, &geometry );
  
  offset = 0;
  soft_width = width;
  soft_height = height;
  
  return 0;
}

void gwlua_zoom( gwlua_t* state, int x0, int y0, int width, int height )
{
  struct retro_game_geometry geometry;
  
  if ( x0 >= 0 )
  {
    geometry.base_width = width;
    geometry.base_height = height;
    soft_width = width;
    soft_height = height;
    offset = y0 * state->width + x0;
  }
  else
  {
    geometry.base_width = state->width;
    geometry.base_height = state->height;
    soft_width = state->width;
    soft_height = state->height;
    offset = 0;
  }
  
  geometry.max_width = state->width;
  geometry.max_height = state->height;
  geometry.aspect_ratio = 0.0f;
  
  //HCF
  //env_cb( RETRO_ENVIRONMENT_SET_GEOMETRY, &geometry );
}

//HCF Functions for highscore management
void vdSaveHighscore(char *achFile)
{
    FILE *fdsco;
    fdsco = fopen(achFile, "wb+");
    fwrite (&sram , sizeof(sram), 1, fdsco);
    fclose(fdsco);
}

void vdLoadHighscore(char *achFile)
{
    FILE *fdsco = NULL;
    fdsco = fopen(achFile, "rb");
    if(fdsco != NULL)
    {
        fread (&sram , sizeof(sram), 1, fdsco);
        fclose(fdsco);
    }
	else
	{
		memset(&sram, 0x00, sizeof(sram));
	}
}

