/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of the Wolfenstein: Enemy Territory GPL Source Code (Wolf ET Source Code).

Wolf ET Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Wolf ET Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Wolf ET Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Wolf: ET Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Wolf ET Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

/*****************************************************************************
 * name:		snd_dma.c
 *
 * desc:		main control for any streaming sound output device
 *
 * $Archive: /MissionPack/code/client/snd_dma.c $
 *
 *****************************************************************************/

#include "snd_local.h"
#include "snd_codec.h"
#include "client.h"

void S_Update_( void );
void S_Base_StopAllSounds( void );
void S_StopStreamingSound( int stream );
void S_FreeStreamingSound( int stream );
void S_UpdateStreamingSounds( void );

streamingSound_t streamingSounds[MAX_STREAMING_SOUNDS];

// =======================================================================
// Internal sound data & structures
// =======================================================================

// only begin attenuating sound volumes when outside the FULLVOLUME range
#define     SOUND_FULLVOLUME    80
#define     SOUND_ATTENUATE     0.0008f
#define     SOUND_RANGE_DEFAULT 1250

channel_t s_channels[MAX_CHANNELS];
channel_t loop_channels[MAX_CHANNELS];
int numLoopChannels;

static int s_soundStarted;
static qboolean s_soundMuted;

// sound fading
static float s_volStart, s_volTarget;
static int s_volTime1, s_volTime2;
static float s_volFadeFrac;
static qboolean s_stopSounds;
float s_volCurrent;

dma_t dma;

static int listener_number;
static vec3_t listener_origin;
static vec3_t listener_axis[3];

int s_soundtime;                // sample PAIRS
int s_paintedtime;              // sample PAIRS

// MAX_SFX may be larger than MAX_SOUNDS because
// of custom player sounds
#define     MAX_SFX         4096
static sfx_t knownSfx[MAX_SFX];
static int numSfx = 0;

#define     LOOP_HASH       128
static sfx_t       *sfxHash[LOOP_HASH];

cvar_t      *s_testsound;
cvar_t      *s_khz;
cvar_t      *s_show;
cvar_t      *s_mixahead;
cvar_t      *s_mixPreStep;
cvar_t      *s_debugStreams;

static loopSound_t loopSounds[MAX_LOOP_SOUNDS];
static vec3_t      entityPositions[MAX_GENTITIES];
static channel_t   *freelist = NULL;
static int         numLoopSounds;

int s_rawend[MAX_RAW_STREAMS];
portable_samplepair_t s_rawsamples[MAX_RAW_STREAMS][MAX_RAW_SAMPLES];


// ====================================================================
// User-setable variables
// ====================================================================


void S_Base_SoundInfo( void ) {
	// TODO: do more info here
	Com_Printf( "----- Sound Info -----\n" );
	if ( !s_soundStarted ) {
		Com_Printf( "sound system not started\n" );
	} else {
		Com_Printf( "%5d stereo\n", dma.channels - 1 );
		Com_Printf( "%5d samples\n", dma.samples );
		Com_Printf( "%5d samplebits\n", dma.samplebits );
		Com_Printf( "%5d submission_chunk\n", dma.submission_chunk );
		Com_Printf( "%5d speed\n", dma.speed );
		Com_Printf( "%p dma buffer\n", dma.buffer );
		if ( streamingSounds[0].stream ) {
			Com_Printf( "Background file: %s\n", streamingSounds[0].loopStream );
		} else {
			Com_Printf( "No background file.\n" );
		}

	}
	Com_Printf( "----------------------\n" );
}


#ifdef USE_VOIP
static
void S_Base_StartCapture( void ) {
	// !!! FIXME: write me.
}

static
int S_Base_AvailableCaptureSamples( void ) {
	// !!! FIXME: write me.
	return 0;
}

static
void S_Base_Capture( int samples, byte *data ) {
	// !!! FIXME: write me.
}

static
void S_Base_StopCapture( void ) {
	// !!! FIXME: write me.
}

static
void S_Base_MasterGain( float val ) {
	// !!! FIXME: write me.
}
#endif



/*
=================
S_Base_SoundList
=================
*/
void S_Base_SoundList( void ) {
	int i;
	sfx_t   *sfx;
	int size, total;
	char type[4][16];
	char mem[2][16];

	strcpy( type[0], "16bit" );
	strcpy( type[1], "adpcm" );
	strcpy( type[2], "daub4" );
	strcpy( type[3], "mulaw" );
	strcpy( mem[0], "paged out" );
	strcpy( mem[1], "resident " );
	total = 0;
	for ( sfx = knownSfx, i = 0 ; i < numSfx ; i++, sfx++ ) {
		size = sfx->soundLength;
		total += size;
		Com_Printf( "%6i[%s] : %s[%s]\n", size, type[sfx->soundCompressionMethod],
					sfx->soundName, mem[sfx->inMemory] );
	}
	Com_Printf( "Total resident: %i\n", total );
	S_DisplayFreeMemory();
}



void S_ChannelFree( channel_t *v ) {
	v->thesfx = NULL;
	*(channel_t **)v = freelist;
	freelist = (channel_t*)v;
}

channel_t*  S_ChannelMalloc( void ) {
	channel_t *v;
	if ( freelist == NULL ) {
		return NULL;
	}
	v = freelist;
	freelist = *(channel_t **)freelist;
	v->allocTime = Com_Milliseconds();
	return v;
}

void S_ChannelSetup( void ) {
	channel_t *p, *q;

	// clear all the sounds so they don't
	Com_Memset( s_channels, 0, sizeof( s_channels ) );

	p = s_channels;;
	q = p + MAX_CHANNELS;
	while ( --q > p ) {
		*(channel_t **)q = q - 1;
	}

	*(channel_t **)q = NULL;
	freelist = p + MAX_CHANNELS - 1;
	Com_DPrintf( "Channel memory manager started\n" );
}



// =======================================================================
// Load a sound
// =======================================================================

/*
================
return a hash value for the sfx name
================
*/
static long S_HashSFXName( const char *name ) {
	int i;
	long hash;
	char letter;

	hash = 0;
	i = 0;
	while ( name[i] != '\0' ) {
		letter = tolower( name[i] );
		if ( letter == '.' ) {
			break;                              // don't include extension
		}
		if ( letter == '\\' ) {
			letter = '/';                       // damn path names
		}
		hash += (long)( letter ) * ( i + 119 );
		i++;
	}
	hash &= ( LOOP_HASH - 1 );
	return hash;
}

/*
==================
S_FindName

Will allocate a new sfx if it isn't found
==================
*/
static sfx_t *S_FindName( const char *name ) {
	int i;
	int hash;

	sfx_t   *sfx;

	if ( !name ) {
		Com_Error( ERR_FATAL, "S_FindName: NULL\n" );
	}
	if ( !name[0] ) {
		Com_Error( ERR_FATAL, "S_FindName: empty name\n" );
	}

	if ( strlen( name ) >= MAX_QPATH ) {
		Com_Error( ERR_FATAL, "Sound name too long: %s", name );
	}

	hash = S_HashSFXName( name );

	sfx = sfxHash[hash];
	// see if already loaded
	while ( sfx ) {
		if ( !Q_stricmp( sfx->soundName, name ) ) {
			return sfx;
		}
		sfx = sfx->next;
	}

	// find a free sfx
	for ( i = 0 ; i < numSfx ; i++ ) {
		if ( !knownSfx[i].soundName[0] ) {
			break;
		}
	}

	if ( i == numSfx ) {
		if ( numSfx == MAX_SFX ) {
			Com_Error( ERR_FATAL, "S_FindName: out of sfx_t" );
		}
		numSfx++;
	}

	sfx = &knownSfx[i];
	Com_Memset( sfx, 0, sizeof( *sfx ) );
	strcpy( sfx->soundName, name );

	sfx->next = sfxHash[hash];
	sfxHash[hash] = sfx;

	return sfx;
}

/*
=================
S_DefaultSound
=================
*/
void S_DefaultSound( sfx_t *sfx ) {

	int i;

	sfx->soundLength = 512;
	sfx->soundData = SND_malloc();
	sfx->soundData->next = NULL;


	for ( i = 0 ; i < sfx->soundLength ; i++ ) {
		sfx->soundData->sndChunk[i] = i;
	}
}

/*
================
S_Reload
================
*/
void S_Base_Reload( void ) {
	sfx_t *sfx;
	int i;

	if ( !s_soundStarted ) {
		return;
	}

	Com_Printf( "reloading sounds...\n" );

	S_Base_StopAllSounds();

	for ( sfx = knownSfx, i = 0; i < numSfx; i++, sfx++ ) {
		sfx->inMemory = qfalse;
		S_memoryLoad( sfx );
	}
}

/*
===================
S_DisableSounds

Disables sounds until the next S_BeginRegistration.
This is called when the hunk is cleared and the sounds
are no longer valid.
===================
*/
void S_Base_DisableSounds( void ) {
	S_Base_StopAllSounds();
	s_soundMuted = qtrue;
}

/*
==================
S_RegisterSound

Creates a default buzz sound if the file can't be loaded
==================
*/
sfxHandle_t S_Base_RegisterSound( const char *name, qboolean compressed ) {
	sfx_t   *sfx;

	compressed = qfalse;
	if ( !s_soundStarted ) {
		return 0;
	}

	if ( strlen( name ) >= MAX_QPATH ) {
		Com_Printf( "Sound name exceeds MAX_QPATH\n" );
		return 0;
	}

	sfx = S_FindName( name );
	if ( sfx->soundData ) {
		if ( sfx->defaultSound ) {
			Com_DPrintf( S_COLOR_YELLOW "WARNING: could not find %s - using default\n", sfx->soundName );
			return 0;
		}
		return sfx - knownSfx;
	}

	sfx->inMemory = qfalse;
	sfx->soundCompressed = compressed;

	S_memoryLoad( sfx );

	if ( sfx->defaultSound ) {
		Com_DPrintf( S_COLOR_YELLOW "WARNING: could not find %s - using default\n", sfx->soundName );
		return 0;
	}

	return sfx - knownSfx;
}

/*
=====================
S_BeginRegistration

=====================
*/
void S_Base_BeginRegistration( void ) {
	s_soundMuted = qfalse;      // we can play again

	if ( numSfx == 0 ) {
		SND_setup();

		numSfx = 0;
		Com_Memset( knownSfx, 0, sizeof( knownSfx ) );
		Com_Memset( sfxHash, 0, sizeof( sfx_t * ) * LOOP_HASH );

		S_Base_RegisterSound( "sound/feedback/hit.wav", qfalse );     // changed to a sound in baseq3
	}
}

void S_memoryLoad( sfx_t *sfx ) {
	// load the sound file
	if ( !S_LoadSound( sfx ) ) {
//		Com_Printf( S_COLOR_YELLOW "WARNING: couldn't load sound: %s\n", sfx->soundName );
		sfx->defaultSound = qtrue;
	}
	sfx->inMemory = qtrue;
}

//=============================================================================

/*
=================
S_SpatializeOrigin

Used for spatializing s_channels
=================
*/
void S_SpatializeOrigin( vec3_t origin, int master_vol, int *left_vol, int *right_vol, float range, int no_attenuation ) {
	vec_t lscale, rscale;

	float dist_fullvol;
	dist_fullvol = range * 0.064f;

	if ( dma.channels == 1 || no_attenuation ) { // no attenuation = no spatialization
		rscale = 1.0;
		lscale = 1.0;
	} else {
		vec_t dist;
		vec3_t source_vec;
		vec3_t vec;
		
		// calculate stereo seperation and distance attenuation
		VectorSubtract( origin, listener_origin, source_vec );

		dist = VectorNormalize( source_vec );
		dist -= dist_fullvol;
		if ( dist < 0.0f || no_attenuation )
			dist = 0.0f;           // close enough to be at full volume
		else
			dist /= range;
	
		VectorRotate( source_vec, listener_axis, vec );

		rscale = sqrt( 1.0 - vec[1] );
		lscale = sqrt( 1.0 + vec[1] );
		if ( rscale < 0 )
			rscale = 0;
		else
			rscale *= 1.0f - dist;
		if ( lscale < 0 )
			lscale = 0;
		else
			lscale *= 1.0f - dist;
	}

	// add in distance effect
	*right_vol = ( master_vol * rscale );
	if ( *right_vol < 0 )
		*right_vol = 0;

	*left_vol = ( master_vol * lscale );
	if ( *left_vol < 0 )
		*left_vol = 0;
}

// =======================================================================
// Start a sound effect
// =======================================================================

/*
====================
S_StartSoundEx

Validates the parms and ques the sound up
if pos is NULL, the sound will be dynamically sourced from the entity
Entchannel 0 will never override a playing sound
====================
*/
void S_Base_StartSoundEx( vec3_t origin, int entnum, int entchannel, sfxHandle_t sfxHandle, int flags, int volume ) {
	channel_t   *ch;
	sfx_t       *sfx;
	int i, oldest, chosen, time;
	int inplay, allowed;
	
	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if ( !origin && ( entnum < 0 || entnum > MAX_GENTITIES ) ) {
		Com_Error( ERR_DROP, "S_StartSound: bad entitynum %i", entnum );
	}

	if ( sfxHandle < 0 || sfxHandle >= numSfx ) {
		Com_Printf( S_COLOR_YELLOW "S_StartSound: handle %i out of range\n", sfxHandle );
		return;
	}
	
	sfx = &knownSfx[ sfxHandle ];

	if ( sfx->inMemory == qfalse ) {
		S_memoryLoad( sfx );
	}

	if ( s_show->integer == 1 ) {
		Com_Printf( "%i : %s\n", s_paintedtime, sfx->soundName );
	}

	time = Com_Milliseconds();

	// pick a channel to play on
	allowed = 4;
	if ( entnum == listener_number ) {
		allowed = 8;
	}

	ch = s_channels;
	inplay = 0;
	for ( i = 0; i < MAX_CHANNELS ; i++, ch++ ) {
		if ( ch->entnum == entnum && ch->thesfx == sfx ) {
			if ( time - ch->allocTime < 50 ) {
				return;
			}
			inplay++;
		}
	}

	if ( inplay > allowed ) {
		return;
	}

	sfx->lastTimeUsed = time;

	ch = S_ChannelMalloc();
	if ( !ch ) {
		ch = s_channels;

		oldest = sfx->lastTimeUsed;
		chosen = -1;
		for ( i = 0 ; i < MAX_CHANNELS ; i++, ch++ ) {
			if ( ch->entnum != listener_number && ch->entnum == entnum && ch->allocTime < oldest && ch->entchannel != CHAN_ANNOUNCER ) {
				oldest = ch->allocTime;
				chosen = i;
			}
		}
		if ( chosen == -1 ) {
			ch = s_channels;
			for ( i = 0 ; i < MAX_CHANNELS ; i++, ch++ ) {
				if ( ch->entnum != listener_number && ch->allocTime < oldest && ch->entchannel != CHAN_ANNOUNCER ) {
					oldest = ch->allocTime;
					chosen = i;
				}
			}
			if ( chosen == -1 ) {
				ch = s_channels;
				if ( ch->entnum == listener_number ) {
					for ( i = 0 ; i < MAX_CHANNELS ; i++, ch++ ) {
						if ( ch->allocTime < oldest ) {
							oldest = ch->allocTime;
							chosen = i;
						}
					}
				}
				if ( chosen == -1 ) {
					Com_Printf( "dropping sound\n" );
					return;
				}
			}
		}
		ch = &s_channels[chosen];
		ch->allocTime = sfx->lastTimeUsed;
	}

	if ( origin ) {
		VectorCopy( origin, ch->origin );
		ch->fixed_origin = qtrue;
	} else {
		ch->fixed_origin = qfalse;
	}

	ch->flags = flags;
	ch->master_vol = volume;
	ch->entnum = entnum;
	ch->thesfx = sfx;
	ch->startSample = START_SAMPLE_IMMEDIATE;
	ch->entchannel = entchannel;
	ch->leftvol = ch->master_vol;       // these will get calced at next spatialize
	ch->rightvol = ch->master_vol;      // unless the game isn't running
	ch->doppler = qfalse;
}

/*
====================
S_StartSound
====================
*/
void S_Base_StartSound( vec3_t origin, int entnum, int entchannel, sfxHandle_t sfxHandle, int volume ) {
	S_Base_StartSoundEx( origin, entnum, entchannel, sfxHandle, 0, volume );
}

/*
==================
S_StartLocalSound
==================
*/
void S_Base_StartLocalSound( sfxHandle_t sfxHandle, int channelNum, int volume ) {
	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	if ( sfxHandle < 0 || sfxHandle >= numSfx ) {
		Com_Printf( S_COLOR_YELLOW "S_StartLocalSound: handle %i out of range\n", sfxHandle );
		return;
	}

	S_Base_StartSound( NULL, listener_number, channelNum, sfxHandle, volume );
}


/*
=================
S_ClearSounds
=================
*/
void S_Base_ClearSounds( qboolean clearStreaming, qboolean clearMusic ) {
	int i;
	streamingSound_t *ss;
	channel_t *ch;
	int clear;
	
	if ( !s_soundStarted ) {
		return;
	}
	
	// stop looping sounds
	Com_Memset( loopSounds, 0, MAX_LOOP_SOUNDS * sizeof( loopSound_t ) );
	Com_Memset( loop_channels, 0, MAX_CHANNELS * sizeof( channel_t ) );
	numLoopChannels = 0;
	numLoopSounds = 0;

	// RF, moved this up so streaming sounds dont get updated with the music, below, 
	// and leave us with a snippet off streaming sounds after we reload
	if ( clearStreaming ) {
		for ( i = 0, ss = streamingSounds; i < MAX_STREAMING_SOUNDS; i++, ss++ ) {
			if ( i > 0 || clearMusic ) {
				S_StopStreamingSound( i );
			}
		}

		// RF, we should also kill all channels, since we are killing streaming sounds anyway 
		// (fixes siren in forest playing after a map_restart/loadgame
		ch = s_channels;
		for ( i = 0; i < MAX_CHANNELS; i++, ch++ ) {
			if ( ch->thesfx ) {
				S_ChannelFree( ch );
			}
		}
	}
	
	if ( !clearMusic ) {
		S_UpdateStreamingSounds();
	}
	
	if ( clearStreaming && clearMusic ) {
		if ( dma.samplebits == 8 ) {
			clear = 0x80;
		} else {
			clear = 0;
		}

		SNDDMA_BeginPainting();
		if ( dma.buffer ) {
			Com_Memset( dma.buffer, clear, dma.samples * dma.samplebits / 8 );
		}
		SNDDMA_Submit();

		// NERVE - SMF - clear out channels so they don't finish playing when audio restarts
		S_ChannelSetup();
	}
}


/*
==================
S_ClearSoundBuffer

If we are about to perform file access, clear the buffer
so sound doesn't stutter.
==================
*/
void S_Base_ClearSoundBuffer( qboolean killStreaming ) {
	if ( !s_soundStarted ) {
		return;
	}

	S_Base_ClearSounds( killStreaming, qtrue );
}

/*
==================
S_StopAllSounds
==================
*/
void S_Base_StopAllSounds( void ) {
	int i;
	
	if ( !s_soundStarted ) {
		return;
	}

	S_Base_ClearSoundBuffer( qtrue );
}

/*
==================
S_ClearLoopingSounds

==================
*/
void S_Base_ClearLoopingSounds( void ) {
	int i;
	for ( i = 0 ; i < numLoopSounds ; i++ ) {
		loopSounds[i].active = qfalse;
		loopSounds[i].kill = qfalse;
	}
	numLoopSounds = 0;
	numLoopChannels = 0;
}

/*
==================
S_AddLoopingSound

Called during entity generation for a frame
Include velocity in case I get around to doing doppler...
==================
*/

#define UNDERWATER_BIT 16

void S_Base_AddLoopingSound( const vec3_t origin, const vec3_t velocity, int range, sfxHandle_t sfxHandle, int volume, int soundTime ) {
	sfx_t *sfx;

	if ( !s_soundStarted || s_soundMuted || !volume ) {
		return;
	}

	if ( numLoopSounds >= MAX_LOOP_SOUNDS ) {
		return;
	}

	if ( sfxHandle < 0 || sfxHandle >= numSfx ) {
		Com_Printf( S_COLOR_YELLOW "S_AddLoopingSound: handle %i out of range\n", sfxHandle );
		return;
	}

	sfx = &knownSfx[ sfxHandle ];

	if ( sfx->inMemory == qfalse ) {
		S_memoryLoad( sfx );
	}

	if ( !sfx->soundLength ) {
		Com_Error( ERR_DROP, "%s has length 0", sfx->soundName );
	}

	VectorCopy( origin, loopSounds[numLoopSounds].origin );
	VectorCopy( velocity, loopSounds[numLoopSounds].velocity );
	loopSounds[numLoopSounds].startSample = soundTime % sfx->soundLength;
	loopSounds[numLoopSounds].active = qtrue;
	loopSounds[numLoopSounds].kill = qtrue;
	loopSounds[numLoopSounds].doppler = qfalse;
	loopSounds[numLoopSounds].oldDopplerScale = 1.0;
	loopSounds[numLoopSounds].dopplerScale = 1.0;
	loopSounds[numLoopSounds].sfx = sfx;
	loopSounds[numLoopSounds].range = range ? range : SOUND_RANGE_DEFAULT;
	loopSounds[numLoopSounds].loudUnderWater = ( volume & 1 << UNDERWATER_BIT ) != 0;
	if ( volume > 65535 ) {
		volume = 65535;
	} else if ( volume < 0 ) {
		volume = 0;
	}
	loopSounds[numLoopSounds].volume = (int)( (float)volume * s_volCurrent );

	if ( s_doppler->integer && VectorLengthSquared( velocity ) > 0.0 ) {
		vec3_t out;
		float lena, lenb;

		loopSounds[numLoopSounds].doppler = qtrue;
		lena = DistanceSquared( entityPositions[listener_number], entityPositions[numLoopSounds] );
		VectorAdd( entityPositions[numLoopSounds], loopSounds[numLoopSounds].velocity, out );
		lenb = DistanceSquared( entityPositions[listener_number], out );
		if ( ( loopSounds[numLoopSounds].framenum + 1 ) != cls.framecount ) {
			loopSounds[numLoopSounds].oldDopplerScale = 1.0;
		} else {
			loopSounds[numLoopSounds].oldDopplerScale = loopSounds[numLoopSounds].dopplerScale;
		}
		loopSounds[numLoopSounds].dopplerScale = lenb / ( lena * 100 );
		if ( loopSounds[numLoopSounds].dopplerScale <= 1.0 ) {
			loopSounds[numLoopSounds].doppler = qfalse;         // don't bother doing the math
		} else if ( loopSounds[numLoopSounds].dopplerScale > MAX_DOPPLER_SCALE ) {
			loopSounds[numLoopSounds].dopplerScale = MAX_DOPPLER_SCALE;
		}
	}

	loopSounds[numLoopSounds].framenum = cls.framecount;
	numLoopSounds++;
}

/*
==================
S_AddLoopingSound

Called during entity generation for a frame
Include velocity in case I get around to doing doppler...
==================
*/
void S_Base_AddRealLoopingSound( const vec3_t origin, const vec3_t velocity, int range, sfxHandle_t sfxHandle, int volume, int soundTime ) {
	sfx_t *sfx;

	if ( !s_soundStarted || s_soundMuted || !volume ) {
		return;
	}

	if ( numLoopSounds >= MAX_LOOP_SOUNDS ) {
		return;
	}

	if ( sfxHandle < 0 || sfxHandle >= numSfx ) {
		Com_Printf( S_COLOR_YELLOW "S_AddRealLoopingSound: handle %i out of range\n", sfxHandle );
		return;
	}
	
	sfx = &knownSfx[ sfxHandle ];

	if ( sfx->inMemory == qfalse ) {
		S_memoryLoad( sfx );
	}

	if ( !sfx->soundLength ) {
		Com_Error( ERR_DROP, "%s has length 0", sfx->soundName );
	}
		
	VectorCopy( origin, loopSounds[numLoopSounds].origin );
	VectorCopy( velocity, loopSounds[numLoopSounds].velocity );
	loopSounds[numLoopSounds].sfx = sfx;
	loopSounds[numLoopSounds].active = qtrue;
	loopSounds[numLoopSounds].kill = qfalse;
	loopSounds[numLoopSounds].doppler = qfalse;
	loopSounds[numLoopSounds].range = range ? range : SOUND_RANGE_DEFAULT;
	loopSounds[numLoopSounds].loudUnderWater = ( volume & 1 << UNDERWATER_BIT ) != 0;
	if ( volume > 65535 ) {
		volume = 65535;
	} else if ( volume < 0 ) {
		volume = 0;
	}
	loopSounds[numLoopSounds].volume = (int)( (float)volume * s_volCurrent );
	numLoopSounds++;
}



/*
==================
S_AddLoopSounds

Spatialize all of the looping sounds.
All sounds are on the same cycle, so any duplicates can just
sum up the channel multipliers.
==================
*/
void S_AddLoopSounds( void ) {
	int i, j, time;
	int left_total, right_total, left, right;
	channel_t   *ch;
	loopSound_t *loop, *loop2;
	static int loopFrame;

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	numLoopChannels = 0;

	time = Com_Milliseconds();

	loopFrame++;
	for ( i = 0 ; i < numLoopSounds; i++ ) {
		loop = &loopSounds[i];
		if ( !loop->active || loop->mergeFrame == loopFrame ) {
			continue;   // already merged into an earlier sound
		}

		if ( loop->kill ) {
			S_SpatializeOrigin( loop->origin, 127, &left_total, &right_total, loop->range, qfalse );          // 3d
		} else {
			S_SpatializeOrigin( loop->origin, 90,  &left_total, &right_total, loop->range, qfalse );          // sphere
		}

		// adjust according to volume
		left_total = (int)( (float)loop->volume * (float)left_total / 256.0 );
		right_total = (int)( (float)loop->volume * (float)right_total / 256.0 );
		
		loop->sfx->lastTimeUsed = time;

		for ( j = ( i + 1 ); j < MAX_GENTITIES ; j++ ) {
			loop2 = &loopSounds[j];
			if ( !loop2->active || loop2->doppler || loop2->sfx != loop->sfx ) {
				continue;
			}
			loop2->mergeFrame = loopFrame;

			if ( loop2->kill ) {
				S_SpatializeOrigin( loop2->origin, 127, &left, &right, loop->range, qfalse );             // 3d
			} else {
				S_SpatializeOrigin( loop2->origin, 90,  &left, &right, loop->range, qfalse );             // sphere
			}

			// adjust according to volume
			left = (int)( (float)loop2->volume * (float)left / 256.0 );
			right = (int)( (float)loop2->volume * (float)right / 256.0 );
			
			loop2->sfx->lastTimeUsed = time;
			left_total += left;
			right_total += right;
		}
		if ( left_total == 0 && right_total == 0 ) {
			continue;       // not audible
		}

		// allocate a channel
		ch = &loop_channels[numLoopChannels];

		if ( left_total > 255 ) {
			left_total = 255;
		}
		if ( right_total > 255 ) {
			right_total = 255;
		}

		ch->master_vol = 127;
		ch->leftvol = left_total;
		ch->rightvol = right_total;
		ch->thesfx = loop->sfx;
		ch->doppler = loop->doppler;
		ch->dopplerScale = loop->dopplerScale;
		ch->oldDopplerScale = loop->oldDopplerScale;
		ch->startSample = loop->startSample; // allow offsetting of sound samples
		numLoopChannels++;
		if ( numLoopChannels == MAX_CHANNELS ) {
			i = numLoopSounds + 1;
		}
	}
}

//=============================================================================

/*
=================
S_ByteSwapRawSamples

If raw data has been loaded in little endien binary form, this must be done.
If raw data was calculated, as with ADPCM, this should not be called.
=================
*/
void S_ByteSwapRawSamples( int samples, int width, int s_channels, const byte *data ) {
	int i;

	if ( width != 2 ) {
		return;
	}
	if ( LittleShort( 256 ) == 256 ) {
		return;
	}

	if ( s_channels == 2 ) {
		samples <<= 1;
	}
	for ( i = 0 ; i < samples ; i++ ) {
		( (short *)data )[i] = LittleShort( ( (short *)data )[i] );
	}
}

/*
============
S_Base_RawSamples

Music streaming
============
*/
void S_Base_RawSamples( int stream, int samples, int rate, int width, int s_channels, const byte *data, float lvol, float rvol ) {
	int i;
	int src, dst;
	float scale;
	int lintVolume, rintVolume = 0;
	portable_samplepair_t *rawsamples;

	if ( !s_soundStarted || s_soundMuted )
		return;

	if ( ( stream < 0 ) || ( stream >= MAX_RAW_STREAMS ) )
		return;
		
	rawsamples = s_rawsamples[stream];

	if ( !s_muted->integer ) {
		lintVolume = 256 * lvol * s_volume->value;
		rintVolume = 256 * rvol * s_volume->value;
	}

	if ( s_rawend[stream] < s_soundtime ) {
		Com_DPrintf( "S_Base_RawSamples: resetting minimum: %i < %i\n", s_rawend[stream], s_soundtime );
		s_rawend[stream] = s_soundtime;
	}

	scale = (float)rate / dma.speed;

	if ( s_channels == 2 && width == 2 ) {
		if ( scale == 1.0 ) { // optimized case
			for ( i = 0 ; i < samples ; i++ )
			{
				dst = s_rawend[stream] & ( MAX_RAW_SAMPLES - 1 );
				s_rawend[stream]++;
				rawsamples[dst].left = ( (short *)data )[i * 2] * lintVolume;
				rawsamples[dst].right = ( (short *)data )[i * 2 + 1] * rintVolume;
			}
		} else
		{
			for ( i = 0 ; ; i++ )
			{
				src = i * scale;
				if ( src >= samples ) {
					break;
				}
				dst = s_rawend[stream] & ( MAX_RAW_SAMPLES - 1 );
				s_rawend[stream]++;
				rawsamples[dst].left = ( (short *)data )[src * 2] * lintVolume;
				rawsamples[dst].right = ( (short *)data )[src * 2 + 1] * rintVolume;
			}
		}
	} else if ( s_channels == 1 && width == 2 ) {
		for ( i = 0 ; ; i++ )
		{
			src = i * scale;
			if ( src >= samples ) {
				break;
			}
			dst = s_rawend[stream] & ( MAX_RAW_SAMPLES - 1 );
			s_rawend[stream]++;
			rawsamples[dst].left = ( (short *)data )[src] * lintVolume;
			rawsamples[dst].right = ( (short *)data )[src] * rintVolume;
		}
	} else if ( s_channels == 2 && width == 1 ) {
		lintVolume *= 256;
		rintVolume *= 256;

		for ( i = 0 ; ; i++ )
		{
			src = i * scale;
			if ( src >= samples ) {
				break;
			}
			dst = s_rawend[stream] & ( MAX_RAW_SAMPLES - 1 );
			s_rawend[stream]++;
			rawsamples[dst].left = ( (char *)data )[src * 2] * lintVolume;
			rawsamples[dst].right = ( (char *)data )[src * 2 + 1] * rintVolume;
		}
	} else if ( s_channels == 1 && width == 1 ) {
		lintVolume *= 256;
		rintVolume *= 256;

		for ( i = 0 ; ; i++ )
		{
			src = i * scale;
			if ( src >= samples ) {
				break;
			}
			dst = s_rawend[stream] & ( MAX_RAW_SAMPLES - 1 );
			s_rawend[stream]++;
			rawsamples[dst].left = ( ( (byte *)data )[src] - 128 ) * lintVolume;
			rawsamples[dst].right = ( ( (byte *)data )[src] - 128 ) * rintVolume;
		}
	}

	if ( s_rawend[stream] > s_soundtime + MAX_RAW_SAMPLES ) {
		Com_DPrintf( "S_Base_RawSamples: overflowed %i > %i\n", s_rawend[stream], s_soundtime );
	}
}

//=============================================================================

/*
=====================
S_UpdateEntityPosition

let the sound system know where an entity currently is
======================
*/
void S_Base_UpdateEntityPosition( int entnum, const vec3_t origin ) {
	if ( entnum < 0 || entnum > MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "S_UpdateEntityPosition: bad entitynum %i", entnum );
	}
	VectorCopy( origin, entityPositions[entnum] );
}


/*
============
S_Respatialize

Change the volumes of all the playing sounds for changes in their positions
============
*/
void S_Base_Respatialize( int entnum, const vec3_t head, vec3_t axis[3], int inwater ) {
	int i;
	channel_t   *ch;
	vec3_t origin;

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	listener_number = entnum;
	VectorCopy( head, listener_origin );
	VectorCopy( axis[0], listener_axis[0] );
	VectorCopy( axis[1], listener_axis[1] );
	VectorCopy( axis[2], listener_axis[2] );

	// update spatialization for dynamic sounds
	ch = s_channels;
	for ( i = 0 ; i < MAX_CHANNELS ; i++, ch++ ) {
		if ( !ch->thesfx ) {
			continue;
		}
		// anything coming from the view entity will always be full volume
		if ( ch->entnum == listener_number ) {
			ch->leftvol = ch->master_vol;
			ch->rightvol = ch->master_vol;
		} else {
			if ( ch->fixed_origin ) {
				VectorCopy( ch->origin, origin );
			} else {
				VectorCopy( entityPositions[ch->entnum], origin );
			}

			S_SpatializeOrigin( origin, ch->master_vol, &ch->leftvol, &ch->rightvol, SOUND_RANGE_DEFAULT, ch->flags & SND_NO_ATTENUATION );
		}
	}

	// add loopsounds
	S_AddLoopSounds();
}


/*
========================
S_ScanChannelStarts

Returns qtrue if any new sounds were started since the last mix
========================
*/
static qboolean S_ScanChannelStarts( void ) {
	channel_t       *ch;
	int i;
	qboolean newSamples;

	newSamples = qfalse;
	ch = s_channels;

	for ( i = 0; i < MAX_CHANNELS ; i++, ch++ ) {
		if ( !ch->thesfx ) {
			continue;
		}
		// if this channel was just started this frame,
		// set the sample count to it begins mixing
		// into the very first sample
		if ( ch->startSample == START_SAMPLE_IMMEDIATE ) {
			ch->startSample = s_paintedtime;
			newSamples = qtrue;
			continue;
		}

		// if it is completely finished by now, clear it
		if ( ch->startSample + ( ch->thesfx->soundLength ) <= s_paintedtime ) {
			S_ChannelFree( ch );
		}
	}

	return newSamples;
}

/*
============
S_Update

Called once each time through the main loop
============
*/
void S_Base_Update( void ) {
	int i;
	int total;
	channel_t   *ch;

	if ( !s_soundStarted || s_soundMuted ) {
//		Com_DPrintf ("not started or muted\n");
		return;
	}

	//
	// debugging output
	//
	if ( s_show->integer == 2 ) {
		total = 0;
		ch = s_channels;
		for ( i = 0 ; i < MAX_CHANNELS; i++, ch++ ) {
			if ( ch->thesfx && ( ch->leftvol || ch->rightvol ) ) {
				Com_Printf( "%d %d %s\n", ch->leftvol, ch->rightvol, ch->thesfx->soundName );
				total++;
			}
		}

		Com_Printf( "----(%i)---- painted: %i\n", total, s_paintedtime );
	}

	// add raw data from streamed samples
	S_UpdateStreamingSounds();

	// mix some sound
	S_Update_();
}

void S_GetSoundtime( void ) {
	int samplepos;
	static int buffers;
	static int oldsamplepos;
	int fullsamples;

	fullsamples = dma.samples / dma.channels;

/* TODO: Avi recording
	if ( CL_VideoRecording() ) {
		s_soundtime += (int)ceil( dma.speed / cl_aviFrameRate->value );
		return;
	}
*/

	// it is possible to miscount buffers if it has wrapped twice between
	// calls to S_Update.  Oh well.
	samplepos = SNDDMA_GetDMAPos();
	if ( samplepos < oldsamplepos ) {
		buffers++;                  // buffer wrapped

		if ( s_paintedtime > 0x40000000 ) { // time to chop things off to avoid 32 bit limits
			buffers = 0;
			s_paintedtime = fullsamples;
			S_Base_StopAllSounds();
		}
	}
	oldsamplepos = samplepos;

	s_soundtime = buffers * fullsamples + samplepos / dma.channels;

#if 0
// check to make sure that we haven't overshot
	if ( s_paintedtime < s_soundtime ) {
		Com_DPrintf( "S_Update_ : overflow\n" );
		s_paintedtime = s_soundtime;
	}
#endif

	if ( dma.submission_chunk < 256 ) {
		s_paintedtime = s_soundtime + s_mixPreStep->value * dma.speed;
	} else {
		s_paintedtime = s_soundtime + dma.submission_chunk;
	}
}


void S_Update_( void ) {
	unsigned endtime;
	int samps;
	static float lastTime = 0.0f;
	float ma, op;
	float thisTime, sane;
	static int ot = -1;

	if ( !s_soundStarted || s_soundMuted ) {
		return;
	}

	thisTime = Com_Milliseconds();

	// Updates s_soundtime
	S_GetSoundtime();

	if ( s_soundtime == ot ) {
		return;
	}
	ot = s_soundtime;

	// clear any sound effects that end before the current time,
	// and start any new sounds
	S_ScanChannelStarts();

	sane = thisTime - lastTime;
	if ( sane < 11 ) {
		sane = 11;          // 85hz
	}

	ma = s_mixahead->value * dma.speed;
	op = s_mixPreStep->value + sane * dma.speed * 0.01;

	if ( op < ma ) {
		ma = op;
	}

	// mix ahead of current position
	endtime = s_soundtime + ma;

	// mix to an even submission block size
	endtime = ( endtime + dma.submission_chunk - 1 )
			  & ~( dma.submission_chunk - 1 );

	// never mix more than the complete buffer
	samps = dma.samples >> ( dma.channels - 1 );
	if ( endtime - s_soundtime > samps ) {
		endtime = s_soundtime + samps;
	}

	// global volume fading
	
	// endtime or s_paintedtime or s_soundtime...
	if ( s_soundtime < s_volTime2 ) { // still has fading to do
		if ( s_soundtime > s_volTime1 ) { // has started fading
			s_volFadeFrac = ( (float)( s_soundtime - s_volTime1 ) / (float)( s_volTime2 - s_volTime1 ) );
			s_volCurrent = ( ( 1.0f - s_volFadeFrac ) * s_volStart + s_volFadeFrac * s_volTarget );
		} else {
			s_volCurrent = s_volStart;
		}
	} else {
		s_volCurrent = s_volTarget;
		if ( s_stopSounds ) {
			// stop playing any sounds if they are all faded out
			S_StopAllSounds();
			s_stopSounds = qfalse;
		}
	}

	SNDDMA_BeginPainting();

	S_PaintChannels( endtime );

	SNDDMA_Submit();

	lastTime = thisTime;
}



/*
===============================================================================

streamed sound functions

===============================================================================
*/

float S_StartStreamingSoundEx( const char *intro, const char *loop, int entnum, int channel,
                                   qboolean music, int param ) {
	streamingSound_t *ss = 0;
	int freeTrack = 0;
	int i;
	
	if ( music ) {
		ss = &streamingSounds[0];
		if ( param < 0 ) {
			if ( intro && strlen( intro ) ) {
				Q_strncpyz( ss->queueStream, intro, MAX_QPATH );
				ss->queueStreamType = param;
				// Cvar for save game
				if ( param == -2 )
					Cvar_Set( "s_currentMusic", intro );
				if ( s_debugStreams->integer ) {
					if ( param == -1 ) {
						Com_Printf( "MUSIC: queueing '%s' for play once\n", intro );
					} else if ( param == -2 ) {
						Com_Printf( "MUSIC: queueing '%s' as new loop\n", intro );
					}
				}
			} else {
				ss->loopStream[0] = '\0';
				ss->queueStream[0] = '\0';
				ss->queueStreamType = 0;
				if ( s_debugStreams->integer ) {
					Com_Printf( "MUSIC: queue cleared\n" );
				}
			}
		}
	} else {
		for ( i = 1; i < MAX_STREAMING_SOUNDS; i++ ) {
			if ( !streamingSounds[i].stream && !freeTrack ) {
				freeTrack = i;
			} else if ( ( entnum >= 0 ) && ( channel != CHAN_AUTO ) &&
			     ( streamingSounds[i].entnum == entnum ) ) {
				// found a match, override this channel
				S_StopStreamingSound( i );
				ss = &streamingSounds[i];
				break;
			}
		}
	}
	
	// Normal streaming sounds, select a free track
	if ( !ss && freeTrack ) {
		ss = &streamingSounds[freeTrack];
		if ( s_debugStreams->integer ) {
			Com_Printf( "STREAM: Free slot found, %d\n", freeTrack );
		}
	}
		
	// cannot find a free track, bail...
	if ( !ss ) {
		if ( !s_muted->integer )
			Com_Printf( "STREAM: No free streaming tracks\n" );
		return 0.0f;
	}
	
	// Set the name of the track
	Q_strncpyz( ss->name, intro, MAX_QPATH );
	
	// looping track passed to stream
	if ( loop ) {
		// if we don't hit the special case in music, "onetimeonly",
		// copy the new loop stream over
		if ( !music || Q_stricmp( loop, "onetimeonly" ) )
			Q_strncpyz( ss->loopStream, loop, MAX_QPATH );
	} else {
		ss->loopStream[0] = '\0';
	}
	
	// clear current music cvar
	Cvar_Set( "s_currentMusic", "" );
	
	// set the stream parameters
	ss->channel = channel;
	// not music track, parameter is attenuation
	if ( !music ) {
		ss->attenuation = param;
	}
	ss->entnum = entnum;
	ss->fadeStartVol = 0;
	// music track, positive parameter is fadeUpTime
	if ( music && param > 0 ) {
		ss->fadeStart = s_soundtime;
		ss->fadeEnd = ss->fadeStart + ( ( (float)( ss->stream->info.rate ) / 1000.0f ) * param );
		ss->fadeTargetVol = 1.0f;
	} else {
		ss->fadeStart = 0;
		ss->fadeEnd = 0;
		ss->fadeTargetVol = 0;
	}
	
	// close the stream
	if ( ss->stream ) {
		S_CodecCloseStream( ss->stream );
	}
	// Open stream
	ss->stream = S_CodecOpenStream( intro );
	if ( !ss->stream ) {
		Com_Printf( S_COLOR_YELLOW "WARNING: couldn't open stream file %s\n", intro );
		return 0.0f;
	}
	
	if ( ss->stream->info.channels != 2 || ss->stream->info.rate != 22050 ) {
		Com_DPrintf( S_COLOR_YELLOW "WARNING: stream file %s is not 22k stereo\n", intro );
	}
	
	// return the length of sound
	return ( ss->stream->info.samples / (float)ss->stream->info.rate ) * 1000.0f;
}

/*
==============
S_StartStreamingSound

==============
*/
float S_Base_StartStreamingSound( const char *intro, const char *loop, int entnum, int channel, int attenuation ) {
	return S_StartStreamingSoundEx( intro, loop, entnum, channel, qfalse, attenuation );
}

/*
==============
S_FreeStreamingSound

Frees a streaming sound so that it can be used again, but does not terminate the sound.
==============
*/
void S_FreeStreamingSound( int stream ) {
	streamingSound_t *ss;
	
	if ( stream < 0 && stream >= MAX_STREAMING_SOUNDS ) {
		return;
	}
	
	ss = &streamingSounds[stream];
	
	if ( !ss->stream ) {
		return;
	}
		
	S_CodecCloseStream( ss->stream );
	ss->stream = NULL;
	ss->loopStream[0] = '\0';
	ss->queueStream[0] = '\0';
	ss->queueStreamType = 0;
}


/*
==============
S_StopStreamingSound

Stops a streaming sound completely in its tracks.
==============
*/
void S_StopStreamingSound( int stream ) {
	S_FreeStreamingSound( stream );
	s_rawend[RAW_STREAM( stream )] = 0;
}

/*
==============
S_StopEntStreamingSound

==============
*/
void S_Base_StopEntStreamingSound( int entnum ) {
	int i;
	for ( i = 1; i < MAX_STREAMING_SOUNDS; i++ ) {
		// is the stream active
		if ( !streamingSounds[i].stream )
			continue;
		// is it the right entity or is it all
		if ( streamingSounds[i].entnum != entnum && entnum != -1 )
			continue;
		S_StopStreamingSound( i );
	}
}

/*
==============
S_FadeAllSounds

==============
*/
void S_Base_FadeAllSounds( float targetVol, int time, qboolean stopsounds ) {
	// TAT 11/15/2002
	//		Because of strange timing issues, sometimes we try to fade up before the fade down completed
	//		If that's the case, just force an immediate stop to all sounds
	if ( s_soundtime < s_volTime2 && s_stopSounds ) {
		S_StopAllSounds();
	}

	s_volStart = s_volCurrent;
	s_volTarget = targetVol;

	s_volTime1 = s_soundtime;
	s_volTime2 = s_soundtime + ( ( (float)( dma.speed ) / 1000.0f ) * time );

	s_stopSounds = stopsounds;

	// instant
	if ( !time ) {
		s_volTarget = s_volStart = s_volCurrent = targetVol;  // set it
		s_volTime1 = s_volTime2 = 0;  // no fading
	}
}

/*
==============
S_FadeStreamingSound
==============
*/
void S_Base_FadeStreamingSound( float targetVol, int time, int stream ) {
	streamingSound_t *ss;
	
	if ( stream < 0 || stream >= MAX_STREAMING_SOUNDS ) {
		return;
	}
	
	ss = &streamingSounds[stream];

	if ( !ss->stream ) {
		return;
	}
	
	ss->fadeStartVol = 1.0f;

	if ( stream == 0 ) {
		if ( s_debugStreams->integer ) {
			Com_Printf( "MUSIC: Fade: %0.2f %d\n", targetVol, time );
		}
	}
	
	// get current fraction if already fading
	if ( ss->fadeStart ) {
		ss->fadeStartVol = ( ss->fadeEnd <= s_soundtime ) ? ss->fadeTargetVol :
			( (float)( s_soundtime - ss->fadeStart ) / (float)( ss->fadeEnd - ss->fadeStart ) );
	}
			
	ss->fadeStart = s_soundtime;
	ss->fadeEnd = s_soundtime + ( ( (float)( ss->stream->info.rate ) / 1000.0f ) * time );
	ss->fadeTargetVol = targetVol;
}

/*
==============
S_GetStreamingFade
==============
*/
float S_GetStreamingFade( streamingSound_t *ss ) {
	float oldfrac, newfrac;

	// no fading, use full volume
	if ( !ss->fadeStart ) {
		return 1.0f;
	}

	if ( ss->fadeEnd <= s_soundtime ) {    // it's hit it's target
		return ss->fadeTargetVol;
	}

	newfrac = (float)( s_soundtime - ss->fadeStart ) / (float)( ss->fadeEnd - ss->fadeStart );
	oldfrac = 1.0f - newfrac;

	return ( oldfrac * ss->fadeStartVol ) + ( newfrac * ss->fadeTargetVol );
}

/*
======================
S_StopBackgroundTrack
======================
*/
void S_Base_StopBackgroundTrack( void ) {
	S_StopStreamingSound( 0 );
}

/*
======================
S_StartBackgroundTrack
======================
*/
void S_Base_StartBackgroundTrack( const char *intro, const char *loop, int fadeupTime ) {
	S_StartStreamingSoundEx( intro, loop, -1, -1, qtrue, fadeupTime );
}

/*
======================
S_UpdateStreamingSounds
======================
*/
void S_UpdateStreamingSounds( void ) {
	int bufferSamples;
	int fileSamples;
	byte raw[30000];        // just enough to fit in a mac stack frame
	int fileBytes;
	int r;
	int i, j;
	float lvol, rvol;
	float streamingVol = 1.0f;
	
	for ( i = 0; i < MAX_STREAMING_SOUNDS; i++ ) {
		streamingSound_t *ss = &streamingSounds[i];
		if ( !ss->stream ) {
			continue;
		}
		// don't bother playing anything if musicvolume is 0
		if ( i == 0 && s_musicVolume->value <= 0.0f ) {
			continue;
		}
		// get the raw stream index
		j = RAW_STREAM( i );
		
		// see how many samples should be copied into the raw buffer
		if ( s_rawend[j] < s_soundtime ) {
			s_rawend[j] = s_soundtime;
		}
		
		while ( s_rawend[j] < s_soundtime + MAX_RAW_SAMPLES ) {
			bufferSamples = MAX_RAW_SAMPLES - ( s_rawend[j] - s_soundtime );

			// decide how much data needs to be read from the file
			fileSamples = bufferSamples * ss->stream->info.rate / dma.speed;

			if ( !fileSamples )
				break;

			// our max buffer size
			fileBytes = fileSamples * ( ss->stream->info.width * ss->stream->info.channels );
			if ( fileBytes > sizeof( raw ) ) {
				fileBytes = sizeof( raw );
				fileSamples = fileBytes / ( ss->stream->info.width * ss->stream->info.channels );
			}

			// Read
			r = S_CodecReadStream( ss->stream, fileBytes, raw );
			if ( r < fileBytes ) {
				fileBytes = r;
				fileSamples = r / ( ss->stream->info.width * ss->stream->info.channels );
			}

			// calculate the streaming volume based on stream fade and global fade
			streamingVol = S_GetStreamingFade( ss );
			// stop the stream if we had faded out of existence
			if ( streamingVol == 0.0f ) {
				S_StopStreamingSound( i );
				break;
			}
			streamingVol *= s_volCurrent;

			if ( r > 0 ) {
				if ( i == 0 ) {
					lvol = rvol = s_musicVolume->value * streamingVol;
				} else {
					// attenuate if required
					if ( ss->entnum >= 0 && ss->attenuation ) {
						int r, l;
						S_SpatializeOrigin( entityPositions[ss->entnum], s_volume->value * 255.0f, &l, &r, SOUND_RANGE_DEFAULT, qfalse );
						if ( ( lvol = ( (float)l / 255.0f ) ) > 1.0f )
							lvol = 1.0f;
						if ( ( rvol = ( (float)r / 255.0f ) ) > 1.0f )
							rvol = 1.0f;
						lvol *= streamingVol;
						rvol *= streamingVol;
					} else {
						lvol = rvol = s_volume->value * streamingVol;
					}
				}
				// add to raw buffer
				S_Base_RawSamples( j, fileSamples, ss->stream->info.rate,
				                   ss->stream->info.width,
				                   ss->stream->info.channels,
				                   raw, lvol, rvol );
			} else {
				if ( s_debugStreams->integer ) {
					Com_Printf( "STREAM %d: ending...\n", i );
					Com_Printf( "Queue stream: %s\nLoopStream: %s\n", ss->queueStream, ss->loopStream );
				}
				// special case for music track queue
				if ( i == 0 && ss->queueStreamType && *ss->queueStream ) {
					switch ( ss->queueStreamType ) {
						case QUEUED_PLAY_ONCE_SILENT:
							break;
						case QUEUED_PLAY_ONCE:
							S_StartBackgroundTrack( ss->queueStream,  ss->name, 0 );
							break;
						case QUEUED_PLAY_LOOPED:
							S_StartBackgroundTrack( ss->queueStream,  ss->queueStream, 0 );
							break;
					}
					// queue is done, clear it
					ss->queueStream[0] = '\0';
					ss->queueStreamType = 0;
					break;
				}

				// loop
				if ( *ss->loopStream ) {
					// TODO: Implement a rewind?
					char loopStream[MAX_QPATH];
					Q_strncpyz(loopStream, ss->loopStream, MAX_QPATH);
					S_StopStreamingSound( i );
					S_StartStreamingSoundEx( loopStream, loopStream,
						ss->entnum, ss->channel, i == 0, i == 0 ? 0 : ss->attenuation );
				} else {
					S_FreeStreamingSound( i );
				}
				break;
			}
		}
	}
}

/*
======================
S_GetVoiceAmplitude
======================
*/
int S_Base_GetVoiceAmplitude( int entnum ) {
	return 0;
}

/*
======================
S_GetCurrentSoundTime
Returns how long the sound lasts in milliseconds
======================
*/
int S_Base_GetSoundLength( sfxHandle_t sfxHandle ) {
	if ( sfxHandle < 0 || sfxHandle >= numSfx ) {
		Com_DPrintf( S_COLOR_YELLOW "S_StartSound: handle %i out of range\n", sfxHandle );
		return -1;
	}
	return (int)( (float)knownSfx[sfxHandle].soundLength / dma.speed * 1000.0 );
}

/*
======================
S_GetCurrentSoundTime
For looped sound synchronisation
======================
*/
int S_Base_GetCurrentSoundTime( void ) {
	return s_soundtime + dma.speed;
}

/*
======================
S_FreeOldestSound
======================
*/
void S_FreeOldestSound( void ) {
	int i, oldest, used;
	sfx_t   *sfx;
	sndBuffer   *buffer, *nbuffer;

	oldest = Com_Milliseconds();
	used = 0;

	for ( i = 1 ; i < numSfx ; i++ ) {
		sfx = &knownSfx[i];
		if ( sfx->inMemory && sfx->lastTimeUsed < oldest ) {
			used = i;
			oldest = sfx->lastTimeUsed;
		}
	}

	sfx = &knownSfx[used];

	Com_DPrintf( "S_FreeOldestSound: freeing sound %s\n", sfx->soundName );

	buffer = sfx->soundData;
	while ( buffer != NULL ) {
		nbuffer = buffer->next;
		SND_free( buffer );
		buffer = nbuffer;
	}
	sfx->inMemory = qfalse;
	sfx->soundData = NULL;
}

// =======================================================================
// Shutdown sound engine
// =======================================================================

void S_Base_Shutdown( void ) {
	if ( !s_soundStarted ) {
		return;
	}

	SNDDMA_Shutdown();

	s_soundStarted = 0;

	Cmd_RemoveCommand( "s_info" );
}

/*
================
S_Init
================
*/
qboolean S_Base_Init( soundInterface_t *si ) {
	qboolean r;

	if ( !si ) {
		return qfalse;
	}

	s_khz = Cvar_Get( "s_khz", "22", CVAR_ARCHIVE );
	s_mixahead = Cvar_Get( "s_mixahead", "0.2", CVAR_ARCHIVE );
	s_mixPreStep = Cvar_Get( "s_mixPreStep", "0.05", CVAR_ARCHIVE );
	s_show = Cvar_Get( "s_show", "0", CVAR_CHEAT );
	s_testsound = Cvar_Get( "s_testsound", "0", CVAR_CHEAT );
	s_debugStreams = Cvar_Get( "s_debugStreams", "0", CVAR_TEMP );

	r = SNDDMA_Init();

	if ( r ) {
		s_soundStarted = 1;
		s_soundMuted = 1;
//		numSfx = 0;

		Com_Memset( streamingSounds, 0, sizeof( streamingSound_t ) * MAX_STREAMING_SOUNDS );
		Com_Memset( sfxHash, 0, sizeof( sfx_t * ) * LOOP_HASH );

		s_soundtime = 0;
		s_paintedtime = 0;

		S_Base_StopAllSounds();
	} else {
		return qfalse;
	}

	si->Shutdown = S_Base_Shutdown;
	si->Reload = S_Base_Reload;
	si->StartSound = S_Base_StartSound;
	si->StartSoundEx = S_Base_StartSoundEx;
	si->StartLocalSound = S_Base_StartLocalSound;
	si->StartBackgroundTrack = S_Base_StartBackgroundTrack;
	si->StopBackgroundTrack = S_Base_StopBackgroundTrack;
	si->StartStreamingSound = S_Base_StartStreamingSound;
	si->StopEntStreamingSound = S_Base_StopEntStreamingSound;
	si->FadeStreamingSound = S_Base_FadeStreamingSound;
	si->RawSamples = S_Base_RawSamples;
	si->ClearSounds = S_Base_ClearSounds;
	si->StopAllSounds = S_Base_StopAllSounds;
	si->FadeAllSounds = S_Base_FadeAllSounds;
	si->ClearLoopingSounds = S_Base_ClearLoopingSounds;
	si->AddLoopingSound = S_Base_AddLoopingSound;
	si->AddRealLoopingSound = S_Base_AddRealLoopingSound;
	si->Respatialize = S_Base_Respatialize;
	si->UpdateEntityPosition = S_Base_UpdateEntityPosition;
	si->Update = S_Base_Update;
	si->DisableSounds = S_Base_DisableSounds;
	si->BeginRegistration = S_Base_BeginRegistration;
	si->RegisterSound = S_Base_RegisterSound;
	si->ClearSoundBuffer = S_Base_ClearSoundBuffer;
	si->SoundInfo = S_Base_SoundInfo;
	si->SoundList = S_Base_SoundList;
	si->GetVoiceAmplitude = S_Base_GetVoiceAmplitude;
	si->GetSoundLength = S_Base_GetSoundLength;
	si->GetCurrentSoundTime = S_Base_GetCurrentSoundTime;

#ifdef USE_VOIP
	si->StartCapture = S_Base_StartCapture;
	si->AvailableCaptureSamples = S_Base_AvailableCaptureSamples;
	si->Capture = S_Base_Capture;
	si->StopCapture = S_Base_StopCapture;
	si->MasterGain = S_Base_MasterGain;
#endif

	return qtrue;
}
