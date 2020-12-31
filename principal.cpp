#include "comun.h"

void vdCore();

extern "C" int SDL_main(int argc, char **argv)
//int main(int argc, char* argv[])
{
	vdIniciaTodo();
	
	vdCore();
	
	vdTerminar();
	
	return 0;
}

