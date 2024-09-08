/*
BSD Zero Clause License
=======================

Copyright (C) Khors Media

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef BEATMACHINE_H
#define BEATMACHINE_H

#pragma once

#include <stdio.h>

#include "pd_api.h"

#include "scale_manager.h"


// --------------------------------------------------------------------------------
#define	TRUE	1	
#define FALSE	0


// --------------------------------------------------------------------------------
typedef enum
{
	LOAD_STATE_HEADER,
	LOAD_STATE_TRACK_INFO,
	LOAD_STATE_ENVOLOPE,
	LOAD_STATE_FILTER,
	LOAD_STATE_DELAY,
	LOAD_STATE_BITCRUSHER,
	LOAD_STATE_NOTES,
	LOAD_STATE_LABELS,
	LOAD_STATE_SCALE,
	LOAD_STATE_LOOP,
	LOAD_STATE_OPTIONS
} BM_LOAD_STATES;


// --------------------------------------------------------------------------------
typedef enum
{
	BM_TYPE_SAMPLE,
	BM_TYPE_SINE,
	BM_TYPE_SQUARE,
	BM_TYPE_SAWTOOTH,
	BM_TYPE_TRIANGLE,
	BM_TYPE_NOISE,
	BM_TYPE_POPHASE,
	BM_TYPE_PODIGITAL,
	BM_TYPE_POVOSIM,
	BM_TYPE_WAVETABLE,
	BM_MAX_SOUND_TYPE,

	BM_SAMLE_TRACKS = 10,
	BM_MAX_TRACK = 16,

	BM_MAX_STEP_COUNT = 1280,//1024,
	BM_MAX_BAR_NUMBER = 80,// 64,
	//BM_MAX_TRACK_COUNT = 16,

	BM_MAX_COLOUR = 4,

	BM_STEPS_PER_BAR = 16,
	BM_BEATS_PER_BAR = 4,
	BM_STEPS_PER_BEAT = 4,
	BM_TRACK_NAME_SIZE = 7,
	BM_TRACK_LABEL_SIZE = 16,
	BM_TRACK_FILENAMEL_SIZE = 48,

	BM_CHORD_TRACK = 8,

	BM_MAX_NOTE_LENGTH = 64

} BM_CONST;


// --------------------------------------------------------------------------------
typedef struct
{
	int pitch;
	int len;
	float velocity;
} Note;


// --------------------------------------------------------------------------------
typedef struct
{
	SoundChannel* pChannel;
	PDSynthInstrument* pInstrument;
	PDSynth* pSynth;
	SequenceTrack* pTrack;

	float fAttack;
	float fDecay;
	float fSustain;
	float fRelease;

	int nSoundSource;
	char szTrackName[16];

	char* pSampleName;

	float fVolume;
	float fPanning;

	int bMuted;
	int bIsChordTrack;

	DelayLine* delay;
	int bDelayEnabled;
	float fDelayFeedback;
	float fDelayMix;

	TwoPoleFilter* filter;
	int bFilterEnabled;
	int nFilterFreq;
	float fFilterResn;
	float fFilterMix;
	int nFilterType;

	BitCrusher* bitCrusher;
	int bBitCrusherEnabled;
	float fBitcrusherAmount;
	float fBitcrusherMix;


} BeatMachineTrack;


// --------------------------------------------------------------------------------
typedef struct
{
	ScaleManager* pScaleManager;

	SoundSequence* pSequence;

	BeatMachineTrack* pTracks[BM_MAX_TRACK];

	int nBeatLength;

	int nBPM;
	int nVersion;

	char* szBeatName;
	char* szProducer;


} BeatMachine;


// --------------------------------------------------------------------------------
typedef struct
{
	int nFileVersion;
	int nStep;
	Note note;
	int nTrack;

	char szBuffer[128];
	char szBufferSmall[32];
	float fValue1;
	float fValue2;
	float fValue3;
	float fValue4;
	int nValue;
	int nExtra;

	int nArrayPos;

	int nStateCount;
	int nLoadStates[16];
} DecodeData;


// --------------------------------------------------------------------------------
BeatMachine* BeatMachineCreate(PlaydateAPI* playdateApi);
void BeatMachineDestroy();

void BeatMachineSetBPM(int nBPM);

int BeatMachineLoadBeat(const char* szName);

void BeatMachineSetADSR(int nTrack, float a, float d, float s, float r);

void BeatMachineSetSample(int nTrack, const char* szPath, const char* szSampleName);

void BeatMachineCreateSynth(int nTrack, int nWaveFormIndex);
void BeatMachineCreateSynthByName(int nTrack, const char *szWaveFormName);

void BeatMachineEnableFilter(int nTrack, int nType, int nFreq, float resonant, float mix);
void BeatMachineEnableDelay(int nTrack, float feedback, float mix);
void BeatMachineEnableBitCrusher(int nTrack, float amount, float mix);

char** BeatMachineGetSoundSrcStrings();

void BeatMachinePlayTheBeat(int nLoops);
void BeatMachineStopTheBeat();


#endif
