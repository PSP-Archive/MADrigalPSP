#include "comun.h"

short shSalirDeAplicacion;
short ashBotones[NUM_BOTONES];
short ashPulsaciones[NUM_BOTONES];
SDL_Surface *screen;
SceCtrlData pad;
unsigned int ancientButtons;  //Only for detecting pulsations in comun.cpp
int done;
int phaserChannel;

//Inicia el SDL, el modo video e inicializa el array de botones
void vdIniciaTodo(void)
{
	int i;
	
	done = 0;
	
	//if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0)
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_TIMER) != 0)
	{
		/*if (SDL_Init(SDL_INIT_VIDEO) != 0)
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		sprintf(buffer, "Error: %s\n", SDL_GetError());*/
		//exit(1);
		/*debugPrint(buffer);*/
	}
	
    SDL_ShowCursor(0);
    
	TTF_Init();
	
	//screen = SDL_SetVideoMode(640, 480, 32, SDL_DOUBLEBUF | SDL_HWSURFACE);
	//screen = SDL_SetVideoMode(640, 480, 16, SDL_DOUBLEBUF | SDL_HWSURFACE);
	//screen = SDL_SetVideoMode(640, 480, 16, SDL_HWSURFACE);
	//screen = SDL_SetVideoMode(640, 480, 16, SDL_SWSURFACE | SDL_ANYFORMAT);
	//screen = SDL_SetVideoMode(640, 480, 0, SDL_SWSURFACE | SDL_ANYFORMAT);
	//screen = SDL_SetVideoMode(640, 480, 0, SDL_HWPALETTE);
	//screen = SDL_GetVideoSurface();
	//screen = SDL_SetVideoMode(640, 480, 16, 0);
	//screen = SDL_SetVideoMode(320, 240, 0, SDL_FULLSCREEN);
 	/////////////////screen = SDL_SetVideoMode(320, 240, 0, 0);
	//screen = SDL_SetVideoMode(320, 240, 0, SDL_HWPALETTE);
	/////////////////screen = SDL_SetVideoMode(640, 480, 0, SDL_HWPALETTE);
	//screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
	
	//ESTE ES EL QUE FUNCIONABA BIEN
	//screen = SDL_SetVideoMode(480, 272, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	
	//Esta es la prueba de hoy
	//screen = SDL_SetVideoMode(480, 272, 0, SDL_HWPALETTE);
	
	//screen = SDL_SetVideoMode(480, 272, 16, SDL_HWPALETTE | SDL_HWSURFACE);
	screen = SDL_SetVideoMode(480, 272, 16, SDL_HWPALETTE);
	//screen = SDL_SetVideoMode(480, 272, 16, SDL_SWSURFACE);
	
	if(screen == NULL)
	{
		printf("Error en screen\n");
		//exit(0);	
	}
	
	sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode( PSP_CTRL_MODE_ANALOG );
    
	for(i = 0; i < NUM_BOTONES; i++)
	{
		ashBotones[i] = 0;
	}
	
	 //Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,4096);
    
}

void vdTerminar(void)
{
	Mix_HaltMusic();
    
    //close sdl_mixer
    Mix_CloseAudio();
    
    //////NO ESTA DEFINIDO Mix_Quit();	
    SDL_Quit();
    return;    
}

void set_quit()
{
    done = 1;
}

/* Codigo para cerrar correctamente nuestro sistema */
int exit_callback ( int arg1 , int arg2 , void * common ) 
{
    //Anyado esto enero
    shSalirDeAplicacion = 1;
    vdTerminar();
    
    set_quit();
    return 0;
}

int CallbackThread ( SceSize args , void * argp ) 
{
    int cbid ;
    cbid = sceKernelCreateCallback (" Exit Callback ", exit_callback , NULL );
    sceKernelRegisterExitCallback ( cbid );
    sceKernelSleepThreadCB ();
    return 0;
}

int SetupCallbacks ( void ) 
{
    int thid = 0;
    thid = sceKernelCreateThread (" update_thread ", CallbackThread , 0x11 , 0xFA0 , 0, 0);
    if( thid >= 0) 
    {
        sceKernelStartThread (thid , 0, 0);
    }
    return thid ;
}

void vdCambiaValor(short* shValue)
{
	if(*shValue == 0)
		*shValue = 1;
	else
		*shValue = 0;	
}

void KeyboardUpdate()
{
   /* Ahora el teclado nuevo es el viejo */
   /* old_keyboard = actual_keyboard ;*/

    /* Actualizamos el estado de la entrada PSP */
    /*sceCtrlPeekBufferPositive (&buttonInput, 1);*/

    /* Actualizamos el estado del teclado */
    /*for (map < keys , PspCtrlButtons >:: iterator i = configured_keys.begin();
    i != configured_keys . end (); ++i)
    actual_keyboard [i-> first ] = ( buttonInput.Buttons & i-> second )?
    actual_keyboard [i-> first ] = true :
    actual_keyboard [i-> first ] = false;*/
}
void vdRellenaBotones(void)
{
 
    //Reseteo de pulsaciones
	for(int i = 0; i < NUM_BOTONES; i++)
	{
		ashPulsaciones[i] = 0;
	}
    
    sceCtrlReadBufferPositive(&pad, 1); 
    
    //Solo estan en la Xbox estos botones
    ashBotones[BOTON_BLANCO] = 0;
    ashBotones[BOTON_NEGRO] = 0;
    
    if (pad.Buttons & PSP_CTRL_UP)
    {
		ashBotones[BOTON_ARRIBA] = 1;
        
        if(!(ancientButtons & PSP_CTRL_UP))
            ashPulsaciones[BOTON_ARRIBA] = 1;
	}
	else
	{
	    ashBotones[BOTON_ARRIBA] = 0;
	} 
	if (pad.Buttons & PSP_CTRL_DOWN)
	{
	    ashBotones[BOTON_ABAJO] = 1;
        
        if(!(ancientButtons & PSP_CTRL_DOWN))
            ashPulsaciones[BOTON_ABAJO] = 1;
	} 
	else
	{
	    ashBotones[BOTON_ABAJO] = 0;
	}
    if (pad.Buttons & PSP_CTRL_LEFT)
    {
		ashBotones[BOTON_IZQUIERDA] = 1;
        
        if(!(ancientButtons & PSP_CTRL_LEFT))
            ashPulsaciones[BOTON_IZQUIERDA] = 1;
	} 
	else
	{
	    ashBotones[BOTON_IZQUIERDA] = 0;
	}
	if (pad.Buttons & PSP_CTRL_RIGHT)
	{
	    ashBotones[BOTON_DERECHA] = 1;
        
        if(!(ancientButtons & PSP_CTRL_RIGHT))
            ashPulsaciones[BOTON_DERECHA] = 1;
	}
	else
	{
	    ashBotones[BOTON_DERECHA] = 0;
	}   
	
	if (pad.Buttons & PSP_CTRL_START)
	{
	    ashBotones[BOTON_START] = 1;
        
        if(!(ancientButtons & PSP_CTRL_START))
            ashPulsaciones[BOTON_START] = 1;
	}
	else
	{
	    ashBotones[BOTON_START] = 0;
	}  
	
	if (pad.Buttons & PSP_CTRL_SELECT)
	{
	    ashBotones[BOTON_BACK] = 1;
        
        if(!(ancientButtons & PSP_CTRL_SELECT))
            ashPulsaciones[BOTON_BACK] = 1;
	}
	else
	{
	    ashBotones[BOTON_BACK] = 0;
	}  
	
	if (pad.Buttons & PSP_CTRL_CIRCLE)
	{
		ashBotones[BOTON_BB] = 1;
        
        if(!(ancientButtons & PSP_CTRL_CIRCLE))
            ashPulsaciones[BOTON_BB] = 1;
	}
	else
	{
		ashBotones[BOTON_BB] = 0;
	} 
	if (pad.Buttons & PSP_CTRL_CROSS)
	{
		ashBotones[BOTON_AA] = 1;
        
        if(!(ancientButtons & PSP_CTRL_CROSS))
            ashPulsaciones[BOTON_AA] = 1;
	}
	else
	{
		ashBotones[BOTON_AA] = 0;
	}
	if (pad.Buttons & PSP_CTRL_TRIANGLE)
	{
		ashBotones[BOTON_YY] = 1;
        
        if(!(ancientButtons & PSP_CTRL_TRIANGLE))
            ashPulsaciones[BOTON_YY] = 1;
	}
	else
	{
		ashBotones[BOTON_YY] = 0;
	}
	if (pad.Buttons & PSP_CTRL_SQUARE)
	{
		ashBotones[BOTON_XX] = 1;
        
        if(!(ancientButtons & PSP_CTRL_SQUARE))
            ashPulsaciones[BOTON_XX] = 1;
	}
	else
	{
		ashBotones[BOTON_XX] = 0;
	}
	
	if (pad.Buttons & PSP_CTRL_LTRIGGER)
	{
		ashBotones[BOTON_LTRIGGER] = 1;
        
        if(!(ancientButtons & PSP_CTRL_LTRIGGER))
            ashPulsaciones[BOTON_LTRIGGER] = 1;
	}
	else
	{
		ashBotones[BOTON_LTRIGGER] = 0;
	}
	
	if (pad.Buttons & PSP_CTRL_RTRIGGER)
	{
		ashBotones[BOTON_RTRIGGER] = 1;
        
        if(!(ancientButtons & PSP_CTRL_RTRIGGER))
            ashPulsaciones[BOTON_RTRIGGER] = 1;
	}
	else
	{
		ashBotones[BOTON_RTRIGGER] = 0;
	}
    
    ancientButtons = pad.Buttons;
}

