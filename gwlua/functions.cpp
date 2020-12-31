#include "gwlua.h"

#include <stdlib.h>

#include "../retroluxury/rl_rand.h"

#include <string.h>
//#include <stdint.h>
#include <time.h>
#include <math.h>

#include "../lua/lua.h"
#include "../lua/lauxlib.h"
#include "../lua/lualib.h"

#include "bsreader.h"

#include "lua/class.h"
#include "lua/classes.h"
#include "lua/controls.h"
#include "lua/dialogs.h"
#include "lua/extctrls.h"
#include "lua/fmod.h"
#include "lua/fmodtypes.h"
#include "lua/forms.h"
#include "lua/graphics.h"
#include "lua/jpeg.h"
#include "lua/math.h"
#include "lua/messages.h"
#include "lua/registry.h"
#include "lua/stdctrls.h"
#include "lua/sysutils.h"
#include "lua/windows.h"

#include "lua/system.h"

#include "png/boxybold_20.h"
#include "png/boxybold_21.h"
#include "png/boxybold_22.h"
#include "png/boxybold_23.h"
#include "png/boxybold_24.h"
#include "png/boxybold_25.h"
#include "png/boxybold_26.h"
#include "png/boxybold_27.h"
#include "png/boxybold_28.h"
#include "png/boxybold_29.h"
#include "png/boxybold_2a.h"
#include "png/boxybold_2b.h"
#include "png/boxybold_2c.h"
#include "png/boxybold_2d.h"
#include "png/boxybold_2e.h"
#include "png/boxybold_2f.h"
#include "png/boxybold_30.h"
#include "png/boxybold_31.h"
#include "png/boxybold_32.h"
#include "png/boxybold_33.h"
#include "png/boxybold_34.h"
#include "png/boxybold_35.h"
#include "png/boxybold_36.h"
#include "png/boxybold_37.h"
#include "png/boxybold_38.h"
#include "png/boxybold_39.h"
#include "png/boxybold_3a.h"
#include "png/boxybold_3b.h"
#include "png/boxybold_3c.h"
#include "png/boxybold_3d.h"
#include "png/boxybold_3e.h"
#include "png/boxybold_3f.h"
#include "png/boxybold_40.h"
#include "png/boxybold_41.h"
#include "png/boxybold_42.h"
#include "png/boxybold_43.h"
#include "png/boxybold_44.h"
#include "png/boxybold_45.h"
#include "png/boxybold_46.h"
#include "png/boxybold_47.h"
#include "png/boxybold_48.h"
#include "png/boxybold_49.h"
#include "png/boxybold_4a.h"
#include "png/boxybold_4b.h"
#include "png/boxybold_4c.h"
#include "png/boxybold_4d.h"
#include "png/boxybold_4e.h"
#include "png/boxybold_4f.h"
#include "png/boxybold_50.h"
#include "png/boxybold_51.h"
#include "png/boxybold_52.h"
#include "png/boxybold_53.h"
#include "png/boxybold_54.h"
#include "png/boxybold_55.h"
#include "png/boxybold_56.h"
#include "png/boxybold_57.h"
#include "png/boxybold_58.h"
#include "png/boxybold_59.h"
#include "png/boxybold_5a.h"
#include "png/boxybold_5b.h"
#include "png/boxybold_5c.h"
#include "png/boxybold_5d.h"
#include "png/boxybold_5e.h"
#include "png/boxybold_5f.h"
#include "png/boxybold_60.h"
#include "png/boxybold_7b.h"
#include "png/boxybold_7c.h"
#include "png/boxybold_7d.h"
#include "png/boxybold_7e.h"
#include "png/snes.h"

#define __inline
#include "entries.cpp"
#undef __inline

#define get_state( L ) ( ( gwlua_t* )lua_touserdata( L, lua_upvalueindex( 1 ) ) )

static int l_playsound( lua_State* L )
{
  rl_sound_t* sound = *(rl_sound_t**)luaL_checkudata( L, 1, "sound" );
  rl_sound_stop_all();
  rl_sound_play( sound, lua_toboolean( L, 2 ), NULL );
  return 0;
}

static int l_stopsounds( lua_State* L )
{
  rl_sound_stop_all();
  return 0;
}

static int l_randomize( lua_State* L )
{
  rl_srand( time( NULL ) );
  return 0;
}

static int l_random( lua_State* L )
{
  if ( lua_isnumber( L, 1 ) )
  {
    lua_pushinteger( L, rl_random( 0, lua_tointeger( L, 1 ) - 1 ) );
  }
  else
  {
    lua_pushnumber( L, rl_rand() / 4294967296.0 );
  }
  
  return 1;
}

static int l_round( lua_State* L )
{
  double x = luaL_checknumber( L, 1 );
  double f = floor( x );
  double c = ceil( x );
  double d1 = x - f;
  double d2 = c - x;
  
  /* TODO: is this behavior present in a built-in function? */
  if ( d1 < d2 )
  {
    lua_pushnumber( L, f );
  }
  else if ( d1 > d2 )
  {
    lua_pushnumber( L, c );
  }
  else
  {
    if ( 1 & (int64_t)f )
    {
      lua_pushnumber( L, c );
    }
    else
    {
      lua_pushnumber( L, f );
    }
  }
  
  return 1;
}

static int l_now( lua_State* L )
{
  lua_pushinteger( L, time( NULL ) );
  return 1;
}

static int l_splittime( lua_State* L )
{
  time_t tt = (time_t)luaL_checkinteger( L, 1 );
  struct tm* tm = localtime( &tt );
  
  lua_pushinteger( L, tm->tm_hour );
  lua_pushinteger( L, tm->tm_min );
  lua_pushinteger( L, tm->tm_sec );
  lua_pushinteger( L, 0 );
  lua_pushinteger( L, tm->tm_mday );
  lua_pushinteger( L, tm->tm_mon + 1 );
  lua_pushinteger( L, tm->tm_year + 1900 );
  return 7;
}

static int l_inttostr( lua_State* L )
{
  lua_tostring( L, 1 );
  lua_pushvalue( L, 1 );
  return 1;
}

static int l_loadvalue( lua_State* L )
{
  gwlua_t* state = get_state( L );
  const char* key = luaL_checkstring( L, 1 );
  
  int type;
  const char* value = gwlua_load_value( state, key, &type );
  
  if ( value )
  {
    switch ( type )
    {
    case GWLUA_NULL:
    default:
      lua_pushnil( L );
      break;
      
    case GWLUA_BOOLEAN:
      lua_pushboolean( L, !strcmp( value, "true" ) );
      break;
      
    case GWLUA_NUMBER:
      if ( !lua_stringtonumber( L, value ) )
      {
        lua_pushinteger( L, 0 );
      }
      break;
      
    case GWLUA_STRING:
      lua_pushstring( L, value );
      break;
    }
  }
  else
  {
    lua_pushnil( L );
  }
  
  return 1;
}

static int l_savevalue( lua_State* L )
{
  gwlua_t* state = get_state( L );
  const char* key = luaL_checkstring( L, 1 );
  
  switch ( lua_type( L, 2 ) )
  {
  case LUA_TBOOLEAN:
    gwlua_save_value( state, key, lua_toboolean( L, 2 ) ? "true" : "false", GWLUA_BOOLEAN );
    break;
    
  case LUA_TNUMBER:
    gwlua_save_value( state, key, lua_tostring( L, 2 ), GWLUA_NUMBER );
    break;
    
  case LUA_TSTRING:
    gwlua_save_value( state, key, lua_tostring( L, 2 ), GWLUA_STRING );
    break;
    
  default:
    gwlua_save_value( state, key, NULL, GWLUA_NULL );
    break;
  }
  
  return 1;
}

#define MAX( a, b ) ( a > b ? a : b )

static int l_setbackground( lua_State* L )
{
  gwlua_t* state = get_state( L );
  rl_image_t*** picture = (rl_image_t***)luaL_checkudata( L, 1, "picture" );
  rl_image_t* bg = **picture;
  
  int width = MAX( bg->width, 480 );
  
  if ( rl_backgrnd_create( width, bg->height ) )
  {
    return luaL_error( L, "out of memory" );
  }
  
  int x0 = ( width - bg->width ) / 2;
  
  state->screen = rl_backgrnd_fb( &state->width, &state->height );
  rl_backgrnd_clear( 0 );
  rl_image_blit_nobg( bg, x0, 0 );
  rl_sprites_translate( x0, 0 );
  
  gwlua_set_fb( state->width, state->height );
  return 0;
}

static int l_setzoom( lua_State* L )
{
  gwlua_t* state = get_state( L );
  
  if ( lua_type( L, 1 ) == LUA_TTABLE )
  {
    lua_geti( L, 1, 1 );
    int x0 = luaL_checkinteger( L, -1 );
    lua_geti( L, 1, 2 );
    int y0 = luaL_checkinteger( L, -1 );
    lua_geti( L, 1, 3 );
    int width = luaL_checkinteger( L, -1 );
    lua_geti( L, 1, 4 );
    int height = luaL_checkinteger( L, -1 );
    
    gwlua_zoom( state, x0, y0, width, height );
  }
  else
  {
    gwlua_zoom( state, -1, -1, -1, -1 );
  }
  
  return 0;
}

static const char* button_name( int button )
{
  switch ( button )
  {
  case GWLUA_UP:     return "up";
  case GWLUA_DOWN:   return "down";
  case GWLUA_LEFT:   return "left";
  case GWLUA_RIGHT:  return "right";
  case GWLUA_A:      return "a";
  case GWLUA_B:      return "b";
  case GWLUA_X:      return "x";
  case GWLUA_Y:      return "y";
  case GWLUA_L1:     return "l1";
  case GWLUA_R1:     return "r1";
  case GWLUA_L2:     return "l2";
  case GWLUA_R2:     return "r2";
  case GWLUA_L3:     return "l3";
  case GWLUA_R3:     return "r3";
  case GWLUA_SELECT: return "select";
  case GWLUA_START:  return "start";
  default:           return "?";
  }
}

static int l_inputstate( lua_State* L )
{
  gwlua_t* state = get_state( L );
  int i;
  
  if ( lua_type( L, 1 ) == LUA_TTABLE )
  {
    lua_pushvalue( L, 1 );
  }
  else
  {
    lua_createtable( L, 0, 17 );
  }
  
  for ( i = 1; i < sizeof( state->input ) / sizeof( state->input[ 0 ] ); i++ )
  {
    lua_pushboolean( L, state->input[ i ] );
    lua_setfield( L, -2, button_name( i ) );
  }
  
  return 1;
}

static int l_loadbin( lua_State* L )
{
  gwlua_t* state = get_state( L );
  size_t len;
  const char* name = luaL_checklstring( L, 1, &len );
  const struct binary_t* found = in_word_set( name, len );
  gwrom_entry_t entry;
  
  if ( found )
  {
    entry.data = (void*)found->data;
    entry.size = found->size;
  }
  else
  {
    if ( gwrom_find( &entry, state->rom, name ) != GWROM_OK )
    {
      return 0;
    }
  }
  
  lua_pushlstring( L, (char*)entry.data, entry.size );
  return 1;
}

static int l_bsread( lua_State* L )
{
  void* bs = lua_touserdata( L, lua_upvalueindex( 1 ) );
  const char* string;
  size_t size;
  
  string = bsread( L, bs, &size );
  
  if ( string )
  {
    lua_pushlstring( L, string, size );
    return 1;
  }
  
  free( bs );
  return 0;
}

static int l_loadbs( lua_State* L )
{
  gwlua_t* state = get_state( L );
  const char* name = luaL_checkstring( L, 1 );
  gwrom_entry_t entry;
  
  if ( gwrom_find( &entry, state->rom, name ) == GWROM_OK )
  {
    void* bs = bsnew( entry.data );
    
    if ( bs )
    {
      lua_pushlightuserdata( L, bs );
      lua_pushcclosure( L, l_bsread, 1 );
      return 1;
    }
  }
  
  return 0;
}

void register_image( lua_State* L, gwlua_t* state );
void register_sound( lua_State* L, gwlua_t* state );
void register_timer( lua_State* L, gwlua_t* state );

void register_functions( lua_State* L, gwlua_t* state )
{
  static const luaL_Reg statics[] =
  {
    { "playsound",     l_playsound },
    { "stopsounds",    l_stopsounds },
    { "randomize",     l_randomize },
    { "random",        l_random },
    { "round",         l_round },
    { "now",           l_now },
    { "splittime",     l_splittime },
    { "inttostr",      l_inttostr },
    { "loadvalue",     l_loadvalue },
    { "savevalue",     l_savevalue },
    { "setbackground", l_setbackground },
    { "setzoom",       l_setzoom },
    { "inputstate",    l_inputstate },
    { "loadbin",       l_loadbin },
    { "loadbs",        l_loadbs },
    { NULL, NULL }
  };
  
  lua_newtable( L );
  
  register_image( L, state );
  register_sound( L, state );
  register_timer( L, state );
  
  lua_pushlightuserdata( L, (void*)state );
  luaL_setfuncs( L, statics, 1 );
  
  // module
  
  if ( luaL_loadbufferx( L, (const char*)gwlua_lua_system_lua, gwlua_lua_system_lua_len, "system.lua", "t" ) != LUA_OK )
  {
    lua_error( L );
    return;
  }
  
  // module chunk
  
  lua_call( L, 0, 1 );
  
  // module function
  
  lua_pushvalue( L, -2 );
  
  // module function module
  
  lua_call( L, 1, 0 );
  
  // module
  
  lua_setglobal( L, "system" );
  
  // --
}
