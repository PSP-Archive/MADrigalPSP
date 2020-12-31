#include <pspkernel.h>
#include <pspctrl.h>
//#include "engine/application.h"
//#include "engine/keyboard.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>

#define PLATAFORMA_XBOX 0
#define PLATAFORMA_PC 1
#define PLATAFORMA_PSP 2

#define TIPO_PLATAFORMA PLATAFORMA_PSP

#define NUM_BOTONES 20
#define BOTON_ARRIBA 0
#define BOTON_ABAJO 1
#define BOTON_IZQUIERDA 2
#define BOTON_DERECHA 3
#define BOTON_AA 4
#define BOTON_BB 5
#define BOTON_XX 6
#define BOTON_YY 7
#define BOTON_BLANCO 8
#define BOTON_NEGRO 9
#define BOTON_START 10
#define BOTON_BACK 11
#define BOTON_LTRIGGER 12
#define BOTON_RTRIGGER 13

extern short shSalirDeAplicacion;
extern short ashBotones[NUM_BOTONES];
extern short ashPulsaciones[NUM_BOTONES];
extern SDL_Surface *screen;

/*std::map <keys, PspCtrlButtons > configured_keys;*/
//PspCtrlButtons configured_keys;
extern SceCtrlData pad;
extern int done;
//SceCtrlData buttonInput ;
//int n_keys;

extern int phaserChannel;

void vdIniciaTodo(void);
void vdCambiaValor(short*);
void vdRellenaBotones(void);
void vdTerminar(void);
