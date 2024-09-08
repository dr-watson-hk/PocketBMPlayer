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


#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"

#include "beat_machine.h"

// --------------------------------------------------------------------------------
LCDFont* pFont = NULL;
BeatMachine* pBeatMachine = NULL;
PlaydateAPI* pd = NULL;
int nCurrentStep = 0;


// --------------------------------------------------------------------------------
void PrintIntValueToString(char* szBuffer, int nBufferSize, int nDigitCount, int nValue)
{
	memset(szBuffer, 0, nBufferSize);

	int nRemaining = nValue;

	int nCount = 0;

	while (nRemaining > 0 || nCount < nDigitCount)
	{
		int nDigit = nRemaining % 10;
		nRemaining = nRemaining / 10;

		char* pDest = szBuffer + nBufferSize - 1;
		char* pSrc = pDest - 1;
		while (pSrc >= szBuffer)
		{
			*pDest-- = *pSrc--;
		}

		*szBuffer = '0' + nDigit;

		nCount++;


	}

}


// --------------------------------------------------------------------------------
int update(void* userData)
{
	if (pd && pBeatMachine && pFont)
	{
		if (pd->sound->sequence->isPlaying(pBeatMachine->pSequence))
		{

			int nStep = pd->sound->sequence->getCurrentStep(pBeatMachine->pSequence, 0);
			
			if (nStep != nCurrentStep)
			{
				pd->graphics->fillRect(0, 0, 400, 240, kColorWhite);

				nCurrentStep = nStep;
			
				char szBuffer[16];
				memset(szBuffer, 0, 16);
				PrintIntValueToString(szBuffer, 16, 1, nStep);

				int nLen = strlen(szBuffer);
				pd->graphics->drawText(szBuffer, nLen, kASCIIEncoding, 10, 10);
			}

		}
	}
	return 1;
}


// --------------------------------------------------------------------------------
#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg)
{
	switch (event)
	{
	case kEventInit:
	{
		const char* err;

		pd = playdate;

		pFont = pd->graphics->loadFont("assets/fonts/font-rains-1x", &err);

		pBeatMachine = BeatMachineCreate(playdate);

		BeatMachineLoadBeat("demo.bmf");
		BeatMachinePlayTheBeat(0);

		playdate->display->setRefreshRate(0);
		playdate->system->setUpdateCallback(update, 0);
	}
		break;

	case kEventTerminate:
		

		BeatMachineDestroy();
		
		break;
	}
	
	return 0;
}
