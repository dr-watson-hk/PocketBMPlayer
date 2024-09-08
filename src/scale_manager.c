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

#include "scale_manager.h"

// --------------------------------------------------------------------------------

void* Engine_MemAlloc(int nSize);

static int arrChromaticScale[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
static int arrMajorScale[] = { 0,2,4,5,7,9,11 };
static int arrNaturalMinorScale[] = { 0,2,3,5,7,8,10 };
static int arrMelodicMinorScale[] = { 0,2,3,5,7,9,11 };
static int arrHarmonicMinorScale[] = { 0,2,3,5,7,8,11 };

static int arrDorianMode[] = { 0,2,3,5,7,9,10 };
static int arrMixolydianMode[] = { 0,2,4,5,7,9,10 };
static int arrLydianMode[] = { 0,2,4,6,7,9,11 };

static int arrLydianDominant[] = { 0,2,4,6,7,9,10 };
static int arrLydianAugmented[] = { 0,2,4,6,8,9,11 };
static int arrLydianDiminished[] = { 0,2,3,6,7,9,11 };

static int arrPhrygianMode[] = { 0,1,3,5,7,8,10 };
static int arrLocrianMode[] = { 0,1,3,5,6,8,10 };
static int arrSuperLocrianMode[] = { 0,1,3,4,6,8,10 };

static int arrPersianScale[] = { 0,1,4,5,6,8,11 };

static int arrMajorPentatonicScale[] = { 0,2,4,7,9 };
static int arrMinorPentatonicScale[] = { 0,3,5,7,10 };
static int arrIwatoScale[] = { 0,1,5,6,10 };

// --------------------------------------------------------------------------------
static char* szScaleNames[] =
{
	"Chromatic",
	"Major",
	"Natural Minor",
	"Melodic Minor",
	"Harmonic Minor",

	"Dorian",
	"Mixolydian",
	"Lydian",
	"Lydian Dominant",
	"Lydian Augmented",
	"Lydian Diminished",

	"Phrygian",
	"Locrian",
	"Super Locrian",

	"Persian",

	"Major Pentatonic",
	"Minor Pentatonic",
	"Iwato"

};


// --------------------------------------------------------------------------------
static char* szScaleNamesShort[] =
{
	"Chromatic",
	"Maj",
	"Natu Min",
	"Melo Min",
	"Harm Min",

	"Dorian",
	"Mixolydian",
	"Lydian",
	"Lydin Dom",
	"Lydin Aug",
	"Lydin Dim",

	"Phrygian",
	"Locrian",
	"S Locrian",

	"Persian",

	"Maj Penta",
	"Min Penta",
	"Iwato"

};


// --------------------------------------------------------------------------------
static char* szPitchNames[] =
{
	"C",
	"C#",
	"D",
	"D#",
	"E",
	"F",
	"F#",
	"G",
	"G#",
	"A",
	"A#",
	"B"
};


// --------------------------------------------------------------------------------
static int* pScales[] =
{
	arrChromaticScale,

	arrMajorScale,
	arrNaturalMinorScale,
	arrMelodicMinorScale,
	arrHarmonicMinorScale,

	arrDorianMode,
	arrMixolydianMode,
	arrLydianMode,

	arrLydianDominant,
	arrLydianAugmented,
	arrLydianDiminished,

	arrPhrygianMode,
	arrLocrianMode,
	arrSuperLocrianMode,

	arrPersianScale,

	arrMajorPentatonicScale,
	arrMinorPentatonicScale,
	arrIwatoScale

};


// --------------------------------------------------------------------------------
static int nScalePitchCount[] =
{
	12,

	7,
	7,
	7,
	7,

	7,
	7,
	7,
	7,
	7,
	7,

	7,
	7,
	7,

	7,

	5,
	5,
	5
};


// --------------------------------------------------------------------------------
static ScaleManager* pScaleManagerCopy = NULL;


// --------------------------------------------------------------------------------
int GetNoteCountForScale(int nScaleIndex)
{
	if (nScaleIndex < SCALE_COUNT)
	{
		return nScalePitchCount[nScaleIndex];
	}

	return 0;

}


// --------------------------------------------------------------------------------
void SetupScale(ScaleManager* pScaleManager, int nScaleIndex, int nFirstPitch)
{
	
	for (int i = 0; i < SCALE_BUFFER_SIZE; i++)
	{
		pScaleManager->nPitchToIndexTable[i] = 0;
		pScaleManager->nCurrentPitchTable[i] = 0;
	}

	int* pScaleInfo = pScales[nScaleIndex];
	int nPitchCount = nScalePitchCount[nScaleIndex];

	int nCurrentPitchFirst = SCALE_NOTE_MIN + nFirstPitch;

	int nPitchIndexInScale = 1;						// lets start from 1
	while (nCurrentPitchFirst <= SCALE_NOTE_MAX)
	{
		for (int i = 0; i < nPitchCount; i++)
		{
			int nIndex = nCurrentPitchFirst + pScaleInfo[i];		// indexes of notes from a scale
			if (nIndex <= SCALE_NOTE_MAX)
			{
				pScaleManager->nPitchToIndexTable[nIndex] = nPitchIndexInScale;
				pScaleManager->nCurrentPitchTable[nPitchIndexInScale] = nIndex;

				nPitchIndexInScale++;
				
			}
		}

		nCurrentPitchFirst += SCALE_SEMITONE_COUNT;
	}

	pScaleManager->nMaxIndex = nPitchIndexInScale - 1;

	pScaleManager->nDefaultPitchIndex = nPitchCount * 3 + 1;


	pScaleManager->nCurrentScale = nScaleIndex;
	pScaleManager->nNoteIndex = nFirstPitch;
	pScaleManager->nCurrentScalePitchCount = nPitchCount;


}


// --------------------------------------------------------------------------------
ScaleManager* ScaleManagerCreate()
{
	int nMemSize = sizeof(ScaleManager);
	ScaleManager* pScaleManager = Engine_MemAlloc(nMemSize);
	
	pScaleManager->nCurrentScale = 0;
	pScaleManager->nCurrentScalePitchCount = 0;

	char cNoteChar[] = {
		'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B'

	};
	int bSharp[] = {
		0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0
	};

	for (int i = 0; i < SCALE_SEMITONE_COUNT; i++)
	{
		pScaleManager->cNoteChar[i] = cNoteChar[i];
		pScaleManager->bSharp[i] = bSharp[i];
	}

	for (int n = 0; n < SCALE_NOTE_MIN; n++)
	{

		pScaleManager->nOctave[n] = 0;

	}

	int nOctave = 1;
	for (int n = SCALE_NOTE_MIN; n < SCALE_NOTE_MAX;)
	{
		for (int i = 0; i < SCALE_SEMITONE_COUNT; i++)
		{
			pScaleManager->nOctave[n] = nOctave;

			n++;
		}

		nOctave++;
	}

	SetupScale(pScaleManager, SCALE_MAJOR, NOTE_C);

	pScaleManagerCopy = pScaleManager;

	return pScaleManager;
}


// --------------------------------------------------------------------------------
void ScaleManagerDestroy(ScaleManager* pScaleManager)
{

}


// --------------------------------------------------------------------------------
char** GetScaleNamesArray()
{
	return szScaleNames;
}


// --------------------------------------------------------------------------------
char** GetScaleNamesShortArray()
{
	return szScaleNamesShort;
}


// --------------------------------------------------------------------------------
int GetScaleCount()
{
	return SCALE_COUNT;
}


// --------------------------------------------------------------------------------
char** GetPitchNameArray()
{
	return szPitchNames;
}


// --------------------------------------------------------------------------------
int GetPitchCount()
{

	return NOTE_COUNT;
}


// --------------------------------------------------------------------------------
const char* GetCurrentScaleName()
{
	if (pScaleManagerCopy)
		return szScaleNames[pScaleManagerCopy->nCurrentScale];
	else
		return "";
}


// --------------------------------------------------------------------------------
const char* GetCurrentBaseNoteName()
{
	if (pScaleManagerCopy)
		return szPitchNames[pScaleManagerCopy->nNoteIndex];
	else
		return "";

}


// --------------------------------------------------------------------------------
void SetupScaleWithString(ScaleManager* pScaleManager, const char* szScale, const char* szBaseNote)
{
	int nScaleIndex = -1;
	int nBaseNoteIndex = -1;

	for (int i = 0; i < SCALE_COUNT; i++)
	{
		if (strcmp(szScaleNames[i], szScale) == 0)
		{
			nScaleIndex = i;
			break;
		}
	}

	for (int i = 0; i < NOTE_COUNT; i++)
	{
		if (strcmp(szPitchNames[i], szBaseNote) == 0)
		{
			nBaseNoteIndex = i;
			break;
		}
	}

	if (nScaleIndex != -1 && nBaseNoteIndex != -1)
	{
		SetupScale(pScaleManager, nScaleIndex, nBaseNoteIndex);
	}

}
