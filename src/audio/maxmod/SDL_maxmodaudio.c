/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"

#include <nds.h>
#include <maxmod9.h>

#include "SDL.h"
#include "SDL_endian.h"
#include "SDL_timer.h"
#include "SDL_audio.h"
#include "../SDL_audiomem.h"
#include "../SDL_audio_c.h"
#include "SDL_maxmodaudio.h"


/* Audio driver functions */
static int MaxMod_OpenAudio(_THIS, SDL_AudioSpec *spec);
static void MaxMod_WaitAudio(_THIS);
static void MaxMod_PlayAudio(_THIS);
static Uint8 *MaxMod_GetAudioBuf(_THIS);
static void MaxMod_CloseAudio(_THIS);

/* Audio driver bootstrap functions */

static int Audio_Available(void)
{
	return(1);
}

static void Audio_DeleteDevice(SDL_AudioDevice *device)
{
	SDL_free(device->hidden);
	SDL_free(device);
}

static SDL_AudioDevice *Audio_CreateDevice(int devindex)
{
	SDL_AudioDevice *this;

	/* Initialize all variables that we clean on shutdown */
	this = (SDL_AudioDevice *)malloc(sizeof(SDL_AudioDevice));
	if ( this ) {
		SDL_memset(this, 0, (sizeof *this));
		this->hidden = (struct SDL_PrivateAudioData *)
				SDL_malloc((sizeof *this->hidden));
	}
	if ( (this == NULL) || (this->hidden == NULL) ) {
		SDL_OutOfMemory();
		if ( this ) {
			SDL_free(this);
		}
		return(0);
	}
	SDL_memset(this->hidden, 0, (sizeof *this->hidden));

	/* Set the function pointers */
	this->OpenAudio = MaxMod_OpenAudio;
	this->WaitAudio = MaxMod_WaitAudio;
	this->PlayAudio = MaxMod_PlayAudio;
	this->GetAudioBuf = MaxMod_GetAudioBuf;
	this->CloseAudio = MaxMod_CloseAudio;

	this->free = Audio_DeleteDevice;

	return this;
}

AudioBootStrap MAXMOD_bootstrap = {
	"maxmod", "MaxMod",
	Audio_Available, Audio_CreateDevice
};

static size_t get_sample_size(mm_stream_formats format) {
	switch (format) {
	case MM_STREAM_8BIT_MONO: return 1;
	case MM_STREAM_8BIT_STEREO: return 2;
	case MM_STREAM_16BIT_MONO: return 2;
	case MM_STREAM_16BIT_STEREO: return 4;
	default: return 1;
	}
}

mm_word on_stream_request( mm_word length, mm_addr dest, mm_stream_formats format ) {
	size_t sample_size = get_sample_size(format);
	if (!current_audio)
		return 0;

	SDL_memset(dest, current_audio->spec.silence, length * sample_size);
	if (current_audio->enabled && !current_audio->paused)
		(*current_audio->spec.callback)(current_audio->spec.userdata, dest, length * sample_size);

	return length;
}

/* Dummy functions -- we don't use thread-based audio */
static void MaxMod_WaitAudio(_THIS)
{
	return;
}

static void MaxMod_PlayAudio(_THIS)
{
	return;
}

static Uint8 *MaxMod_GetAudioBuf(_THIS)
{
	return(NULL);
}

static void MaxMod_CloseAudio(_THIS)
{
	mmStreamClose();
}

static int MaxMod_OpenAudio(_THIS, SDL_AudioSpec *spec)
{
	Uint16 test_format = SDL_FirstAudioFormat(spec->format);
	int valid_datatype = 0;
	mm_word format = MM_STREAM_8BIT_MONO;

	while ((!valid_datatype) && (test_format)) {
		spec->format = test_format;
		switch (test_format) {
			case AUDIO_S8:
				format = (spec->channels >= 2) ? MM_STREAM_8BIT_STEREO : MM_STREAM_8BIT_MONO;
				valid_datatype = 1;
				break;

			case AUDIO_S16LSB:
				format = (spec->channels >= 2) ? MM_STREAM_16BIT_STEREO : MM_STREAM_16BIT_MONO;
				valid_datatype = 1;
				break;

			default:
				test_format = SDL_NextAudioFormat();
				break;
		}
	}

	if (!valid_datatype) {
		SDL_SetError("Unsupported audio format");
		return (-1);
	}

	if (spec->channels > 2)
		spec->channels = 2;

	/* Update the fragment size as size in bytes */
	SDL_CalculateAudioSpec(spec);

	mm_ds_system sys;
	sys.mod_count 			= 0;
	sys.samp_count			= 0;
	sys.mem_bank			= 0;
	sys.fifo_channel		= FIFO_MAXMOD;
	mmInit( &sys );

	mm_stream mystream;
	mystream.sampling_rate = spec->freq;
	mystream.buffer_length = spec->size / get_sample_size(format);
	mystream.callback = on_stream_request;
	mystream.format = format;
	mystream.timer = MM_TIMER2;
	mystream.manual = 0;
	mmStreamOpen( &mystream );

	return(1);
}
