#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <SDL/SDL.h>

static void vlog_citra(const char *format, va_list arg ) {
	char buf[2000];
//	static Handle mutex = 0;
//	if (!mutex) svcCreateMutex(&mutex, false);
	vsnprintf(buf, sizeof(buf), format, arg);
	int i=strlen(buf);
	while (i && buf[i-1]=='\n') buf[--i]=0; // strip trailing newlines
//	svcWaitSynchronization(mutex, U64_MAX);
	svcOutputDebugString(buf, i);
	printf("%s\n",buf);
//	svcReleaseMutex(mutex);
}

void log_citra(const char *format, ...)
{
    va_list argptr;
    va_start(argptr, format);
	vlog_citra(format, argptr);
    va_end(argptr);
}

int main(int argc, char* argv[])
{
	SDL_Event e;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
	SDL_Joystick *joy;
	joy=SDL_JoystickOpen(0);
	SDL_Surface *sdl=SDL_SetVideoMode(400,240,32, SDL_TOPSCR | SDL_CONSOLEBOTTOM);

	printf("Staring Event poll ...\n");
	for (;;) {
		SDL_Flip(sdl);
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_JOYBUTTONDOWN && e.jbutton.button == 0)
				goto quit;
			switch (e.type) {
			case SDL_JOYHATMOTION:
				printf("Joy hat %d set to %p\n",e.jhat.hat,e.jhat.value);
				break;
			case SDL_JOYBUTTONDOWN:
				printf("Joy button %d pressed\n",e.jbutton.button);
				break;			
			case SDL_JOYBUTTONUP:
				printf("Joy button %d released\n",e.jbutton.button);
				break;
			case SDL_JOYAXISMOTION:
				printf("Joy axis %d motion to %d\n",e.jaxis.axis, e.jaxis.value);
				break;
			default:
				printf("Got event type %d\n",e.type);
				break;
			}
		}
	}
quit:
	SDL_JoystickClose(joy);
	SDL_Quit();
}
