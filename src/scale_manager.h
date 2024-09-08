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

#ifndef SCALE_MANAGER_H
#define SCALE_MANAGER_H

#pragma once

#include <stdio.h>

#include "pd_api.h"


// --------------------------------------------------------------------------------
typedef enum
{
	SCALE_CHROMATIC,
	SCALE_MAJOR,

	SCALE_NATURALMINOR,
	SCALE_MELODICMINOR,
	SCALE_HARMONICMINOR,

	SCALE_DORIAN,
	SCALE_MIXOLYDIAN,

	SCALE_LYDIAN,
	SCALE_LYDIAN_DOMINANT,
	SCALE_LYDIAN_AUGMENTED,
	SCALE_LYDIAN_DIMINISHED,

	SCALE_PHRYGIAN,

	SCALE_LOCRIAN,
	SCALE_SUPER_LOCRIAN,

	SCALE_PERSIAN,

	SCALE_MAJOR_PENTATONIC,
	SCALE_MINOR_PENTATONIC,

	SCALE_IWATO,

	SCALE_COUNT

} SCALES;


// --------------------------------------------------------------------------------
typedef enum
{
	NOTE_C, 
	NOTE_C_SHARP, 
	NOTE_D, 
	NOTE_D_SHARP, 
	NOTE_E, 
	NOTE_F, 
	NOTE_F_SHARP, 
	NOTE_G, 
	NOTE_G_SHARP, 
	NOTE_A, 
	NOTE_A_SHARP, 
	NOTE_B,
	NOTE_COUNT
} NOTES;



// --------------------------------------------------------------------------------
typedef enum
{

	SCALE_SEMITONE_COUNT = 12,

	SCALE_NOTE_MIN = 24,	// C1
	SCALE_NOTE_MAX = 119,

	SCALE_BUFFER_SIZE = 128

} SCALE_CONSTS;


// --------------------------------------------------------------------------------
typedef struct
{
	int nOctave[SCALE_BUFFER_SIZE];

	int nPitchToIndexTable[SCALE_BUFFER_SIZE];

	int nCurrentPitchTable[SCALE_BUFFER_SIZE];

	int nCurrentScale;
	int nNoteIndex;
	int nCurrentScalePitchCount;

	int cNoteChar[SCALE_SEMITONE_COUNT];
	int bSharp[SCALE_SEMITONE_COUNT];

	int nMaxIndex;
	int nDefaultPitchIndex;

} ScaleManager;


// --------------------------------------------------------------------------------
ScaleManager* ScaleManagerCreate();
void ScaleManagerDestroy(ScaleManager* pScaleManager);
void SetupScale(ScaleManager* pScaleManager, int nScaleIndex, int nFirstPitch);

char** GetScaleNamesArray();
char** GetScaleNamesShortArray();
int GetScaleCount();
char** GetPitchNameArray();
int GetPitchCount();

const char* GetCurrentScaleName();
const char* GetCurrentBaseNoteName();

void SetupScaleWithString(ScaleManager* pScaleManager, const char* szScale, const char * szBaseNote);

int GetNoteCountForScale(int nScaleIndex);

#endif
