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

#include "beat_machine.h"


// --------------------------------------------------------------------------------
static PlaydateAPI* pd = NULL;
static BeatMachine* pBeatMachine = NULL;
static DecodeData decodeData;


// --------------------------------------------------------------------------------
static char* szSoundSrcType[] =
{
	"sampler",
	"sine",
	"square",
	"sawtooth",
	"triangle",
	"noise",
	"phase",
	"digital",
	"vosim",
	"wavetable"
};


// --------------------------------------------------------------------------------
void* Engine_MemAlloc(int nSize)
{
	return pd->system->realloc(NULL, nSize);

}


// --------------------------------------------------------------------------------
void Engine_MemFree(void* pData)
{
	pd->system->realloc(pData, 0);

}


// --------------------------------------------------------------------------------
char* Engine_StrDup(const char* str)
{
	int len = strlen(str);
	char* s = Engine_MemAlloc(len + 1);
	memcpy(s, str, len);
	s[len] = '\0';
	return s;
}


// --------------------------------------------------------------------------------
char** BeatMachineGetSoundSrcStrings()
{
	return szSoundSrcType;
}


// --------------------------------------------------------------------------------
BeatMachine* BeatMachineCreate(PlaydateAPI* playdateApi)
{
	pd = playdateApi;

	const int nVersion = 1;

	int nMemSize = sizeof(BeatMachine);
	pBeatMachine = Engine_MemAlloc(nMemSize);

	pBeatMachine->pScaleManager = ScaleManagerCreate();

	pBeatMachine->pSequence = pd->sound->sequence->newSequence();

	pBeatMachine->nVersion = nVersion;

	pBeatMachine->szBeatName = NULL;
	pBeatMachine->szProducer = NULL;

	pBeatMachine->nBeatLength = 0;

	decodeData.nStateCount = 0;


	nMemSize = sizeof(BeatMachineTrack);
	for (int i = 0; i < BM_MAX_TRACK; ++i)
	{
		pBeatMachine->pTracks[i] = Engine_MemAlloc(nMemSize);

		memset(pBeatMachine->pTracks[i], 0, nMemSize);

	}

	BeatMachineSetBPM(120);

	return pBeatMachine;
}


// --------------------------------------------------------------------------------
void BeatMachineDestroy()
{
	if (pd->sound->sequence->isPlaying(pBeatMachine->pSequence))
		pd->sound->sequence->stop(pBeatMachine->pSequence);

	for (int nTrack = 0; nTrack < BM_MAX_TRACK; nTrack++)
	{
		if (pBeatMachine->pTracks[nTrack]->pTrack)
		{
			if (pBeatMachine->pTracks[nTrack]->pSampleName)
				Engine_MemFree(pBeatMachine->pTracks[nTrack]->pSampleName);

			if (pBeatMachine->pTracks[nTrack]->filter)
				pd->sound->effect->twopolefilter->freeFilter(pBeatMachine->pTracks[nTrack]->filter);

			if (pBeatMachine->pTracks[nTrack]->delay)
				pd->sound->effect->delayline->freeDelayLine(pBeatMachine->pTracks[nTrack]->delay);

			if (pBeatMachine->pTracks[nTrack]->bitCrusher)
				pd->sound->effect->bitcrusher->freeBitCrusher(pBeatMachine->pTracks[nTrack]->bitCrusher);

			pd->sound->synth->freeSynth(pBeatMachine->pTracks[nTrack]->pSynth);
			pd->sound->instrument->freeInstrument(pBeatMachine->pTracks[nTrack]->pInstrument);
			pd->sound->channel->freeChannel(pBeatMachine->pTracks[nTrack]->pChannel);

			pd->sound->track->freeTrack(pBeatMachine->pTracks[nTrack]->pTrack);
		}
	}

	Engine_MemFree(pBeatMachine->pScaleManager);
	Engine_MemFree(pBeatMachine);

}


// --------------------------------------------------------------------------------
void BeatMachineSetBPM(int nBPM)
{
	float fBPM = (float)nBPM;

	float stepsPerBeat = 4.0f;
	float beatsPerSecond = fBPM / 60.f;
	float stepsPerSecond = stepsPerBeat * beatsPerSecond;

	if (pBeatMachine && pBeatMachine->pSequence)
	{
		pd->sound->sequence->setTempo(pBeatMachine->pSequence, stepsPerSecond);
		pBeatMachine->nBPM = nBPM;
	}


}


// --------------------------------------------------------------------------------
void BeatMachineSetADSR(int nTrack, float a, float d, float s, float r)
{
	if (pBeatMachine && pBeatMachine->pTracks[nTrack])
	{
		pBeatMachine->pTracks[nTrack]->fAttack = a;
		pd->sound->synth->setAttackTime(pBeatMachine->pTracks[nTrack]->pSynth, a);

		pBeatMachine->pTracks[nTrack]->fDecay = d;
		pd->sound->synth->setDecayTime(pBeatMachine->pTracks[nTrack]->pSynth, d);

		pBeatMachine->pTracks[nTrack]->fSustain = s;
		pd->sound->synth->setSustainLevel(pBeatMachine->pTracks[nTrack]->pSynth, s);

		pBeatMachine->pTracks[nTrack]->fRelease = r;
		pd->sound->synth->setReleaseTime(pBeatMachine->pTracks[nTrack]->pSynth, r);
	}

}


// --------------------------------------------------------------------------------
void BeatMachineSetSample(int nTrack, const char* szPath, const char* szSampleName)
{
	char szFullPath[256];
	memset(szFullPath, 0, 256);

	strcpy(szFullPath, szPath);
	strcat(szFullPath, szSampleName);

	if (pBeatMachine && pBeatMachine->pTracks[nTrack])
	{
		AudioSample* pSample = pd->sound->sample->load(szFullPath);
		pd->sound->synth->setSample(pBeatMachine->pTracks[nTrack]->pSynth, pSample, 0, 0);
		pd->sound->sample->freeSample(pSample);

		if (pBeatMachine->pTracks[nTrack]->pSampleName)
			Engine_MemFree(pBeatMachine->pTracks[nTrack]->pSampleName);

		pBeatMachine->pTracks[nTrack]->pSampleName = Engine_StrDup(szSampleName);
	}

}


// --------------------------------------------------------------------------------
void BeatMachineCreateSynth(int nTrack, int nWaveFormIndex)
{
	SoundWaveform waveforms[] =
	{
		0,
		kWaveformSine,
		kWaveformSquare,
		kWaveformSawtooth,
		kWaveformTriangle,
		kWaveformNoise,
		kWaveformPOPhase,
		kWaveformPODigital,
		kWaveformPOVosim
	};

	if (pBeatMachine && pBeatMachine->pTracks[nTrack])
	{
		int bCreateNew = FALSE;

		if (pBeatMachine->pTracks[nTrack]->pChannel == NULL)
		{
			bCreateNew = TRUE;
			pBeatMachine->pTracks[nTrack]->pChannel = pd->sound->channel->newChannel();
		}

		if (pBeatMachine->pTracks[nTrack]->pInstrument == NULL)
			pBeatMachine->pTracks[nTrack]->pInstrument = pd->sound->instrument->newInstrument();

		if (pBeatMachine->pTracks[nTrack]->pSynth == NULL)
			pBeatMachine->pTracks[nTrack]->pSynth = pd->sound->synth->newSynth();

		pd->sound->synth->setWaveform(pBeatMachine->pTracks[nTrack]->pSynth, waveforms[nWaveFormIndex]);

		pd->sound->synth->setAttackTime(pBeatMachine->pTracks[nTrack]->pSynth, 0);
		pd->sound->synth->setDecayTime(pBeatMachine->pTracks[nTrack]->pSynth, .2f);
		pd->sound->synth->setSustainLevel(pBeatMachine->pTracks[nTrack]->pSynth, .3f);
		pd->sound->synth->setReleaseTime(pBeatMachine->pTracks[nTrack]->pSynth, .5f);
		pBeatMachine->pTracks[nTrack]->fAttack = 0.0f;
		pBeatMachine->pTracks[nTrack]->fDecay = .2f;
		pBeatMachine->pTracks[nTrack]->fSustain = .3f;
		pBeatMachine->pTracks[nTrack]->fRelease = .5f;

		if (bCreateNew)
		{
			pd->sound->instrument->addVoice(pBeatMachine->pTracks[nTrack]->pInstrument, pBeatMachine->pTracks[nTrack]->pSynth, 24, 127, 0);

			pd->sound->channel->addSource(pBeatMachine->pTracks[nTrack]->pChannel, (SoundSource*)pBeatMachine->pTracks[nTrack]->pInstrument);

			pBeatMachine->pTracks[nTrack]->pTrack = pd->sound->sequence->addTrack(pBeatMachine->pSequence);
			pd->sound->track->setInstrument(pBeatMachine->pTracks[nTrack]->pTrack, pBeatMachine->pTracks[nTrack]->pInstrument);
		}

		pBeatMachine->pTracks[nTrack]->nSoundSource = nWaveFormIndex;
	}
}


// --------------------------------------------------------------------------------
void BeatMachineCreateSampler(int nTrack)
{
	if (pBeatMachine && pBeatMachine->pTracks[nTrack])
	{
		int bCreateNew = FALSE;

		if (pBeatMachine->pTracks[nTrack]->pChannel == NULL)
		{
			bCreateNew = TRUE;
			pBeatMachine->pTracks[nTrack]->pChannel = pd->sound->channel->newChannel();
		}

		if (pBeatMachine->pTracks[nTrack]->pInstrument == NULL)
			pBeatMachine->pTracks[nTrack]->pInstrument = pd->sound->instrument->newInstrument();

		if (pBeatMachine->pTracks[nTrack]->pSynth == NULL)
			pBeatMachine->pTracks[nTrack]->pSynth = pd->sound->synth->newSynth();

		if (bCreateNew)
		{
			pd->sound->instrument->addVoice(pBeatMachine->pTracks[nTrack]->pInstrument, pBeatMachine->pTracks[nTrack]->pSynth, 24, 127, 0);

			pd->sound->channel->addSource(pBeatMachine->pTracks[nTrack]->pChannel, (SoundSource*)pBeatMachine->pTracks[nTrack]->pInstrument);

			pBeatMachine->pTracks[nTrack]->pTrack = pd->sound->sequence->addTrack(pBeatMachine->pSequence);
			pd->sound->track->setInstrument(pBeatMachine->pTracks[nTrack]->pTrack, pBeatMachine->pTracks[nTrack]->pInstrument);
		}

		pBeatMachine->pTracks[nTrack]->nSoundSource = BM_TYPE_SAMPLE;

	}

}


// --------------------------------------------------------------------------------
void BeatMachineSetChordTrack(int nTrack, int bFlag)
{
	if (bFlag)
	{
		pBeatMachine->pTracks[nTrack]->bIsChordTrack = bFlag;
		for (int v = 0; v < 2; v++)
		{
			// make it polyphony for chord track
			pd->sound->instrument->addVoice(pBeatMachine->pTracks[nTrack]->pInstrument, pd->sound->synth->copy(pBeatMachine->pTracks[nTrack]->pSynth), 24, 127, 0);
		}
	}

}


// --------------------------------------------------------------------------------
void BeatMachineCreateSynthByName(int nTrack, const char* szWaveFormName)
{
	for (int i = 0; i < BM_MAX_SOUND_TYPE; i++)
	{
		if (strcmp(szSoundSrcType[i], szWaveFormName) == 0)
		{
			BeatMachineCreateSynth(nTrack, i);
			break;
		}
	}
}


// --------------------------------------------------------------------------------
void BeatMachineEnableFilter(int nTrack, int nType, int nFreq, float resonant, float mix)
{
	if (pBeatMachine && pBeatMachine->pTracks[nTrack])
	{
		pBeatMachine->pTracks[nTrack]->bFilterEnabled = TRUE;
		pBeatMachine->pTracks[nTrack]->filter = pd->sound->effect->twopolefilter->newFilter();

		pBeatMachine->pTracks[nTrack]->nFilterType = nType;
		pd->sound->effect->twopolefilter->setType(pBeatMachine->pTracks[nTrack]->filter, (TwoPoleFilterType)nType);

		pBeatMachine->pTracks[nTrack]->nFilterFreq = nFreq;
		pd->sound->effect->twopolefilter->setFrequency(pBeatMachine->pTracks[nTrack]->filter, nFreq);

		pBeatMachine->pTracks[nTrack]->fFilterResn = resonant;
		pd->sound->effect->twopolefilter->setResonance(pBeatMachine->pTracks[nTrack]->filter, resonant);

		pBeatMachine->pTracks[nTrack]->fFilterMix = mix;
		pd->sound->effect->setMix(pBeatMachine->pTracks[nTrack]->filter, mix);
	}

}


// --------------------------------------------------------------------------------
void BeatMachineEnableDelay(int nTrack, float feedback, float mix)
{
	if (pBeatMachine && pBeatMachine->pTracks[nTrack])
	{
		pBeatMachine->pTracks[nTrack]->bDelayEnabled = TRUE;
		pBeatMachine->pTracks[nTrack]->delay = pd->sound->effect->delayline->newDelayLine(128, 2);
		pd->sound->effect->delayline->setFeedback(pBeatMachine->pTracks[nTrack]->delay, 0.5f);
		pd->sound->effect->setMix(pBeatMachine->pTracks[nTrack]->delay, 0.5f);

		pBeatMachine->pTracks[nTrack]->fDelayFeedback = feedback;
		pd->sound->effect->delayline->setFeedback(pBeatMachine->pTracks[nTrack]->delay, feedback);

		pBeatMachine->pTracks[nTrack]->fDelayMix = mix;
		pd->sound->effect->setMix(pBeatMachine->pTracks[nTrack]->delay, mix);
	}
}


// --------------------------------------------------------------------------------
void BeatMachineEnableBitCrusher(int nTrack, float amount, float mix)
{
	if (pBeatMachine && pBeatMachine->pTracks[nTrack])
	{
		pBeatMachine->pTracks[nTrack]->bBitCrusherEnabled = TRUE;
		pBeatMachine->pTracks[nTrack]->bitCrusher = pd->sound->effect->bitcrusher->newBitCrusher();
		pd->sound->effect->bitcrusher->setAmount(pBeatMachine->pTracks[nTrack]->bitCrusher, 0.5f);
		pd->sound->effect->setMix(pBeatMachine->pTracks[nTrack]->bitCrusher, 0.5f);

		pBeatMachine->pTracks[nTrack]->fBitcrusherAmount = amount;
		pd->sound->effect->bitcrusher->setAmount(pBeatMachine->pTracks[nTrack]->bitCrusher, amount);

		pBeatMachine->pTracks[nTrack]->fBitcrusherMix= mix;
		pd->sound->effect->setMix(pBeatMachine->pTracks[nTrack]->bitCrusher, mix);
	}

}


// --------------------------------------------------------------------------------
void BeatMachineSetVolume(int nTrack, float fVolume)
{

	pBeatMachine->pTracks[nTrack]->fVolume= fVolume;

	pd->sound->channel->setVolume(pBeatMachine->pTracks[nTrack]->pChannel, fVolume);


}


// --------------------------------------------------------------------------------
void BeatMachineSetPanning(int nTrack, float fValue)
{
	pBeatMachine->pTracks[nTrack]->fPanning = fValue;
	pd->sound->channel->setPan(pBeatMachine->pTracks[nTrack]->pChannel, fValue);

}


// --------------------------------------------------------------------------------
void BeatMachineMuteTrack(int nTrack, int bFlag)
{
	pBeatMachine->pTracks[nTrack]->bMuted = bFlag;
	pd->sound->track->setMuted(pBeatMachine->pTracks[nTrack]->pTrack, bFlag);
}


// --------------------------------------------------------------------------------
void decodeError(json_decoder* decoder, const char* error, int linenum)
{
	pd->system->logToConsole("decode error line %i: %s", linenum, error);
}


// --------------------------------------------------------------------------------
const char* typeToName(json_value_type type)
{
	switch (type)
	{
	case kJSONNull: return "null";
	case kJSONTrue: return "true";
	case kJSONFalse: return "false";
	case kJSONInteger: return "integer";
	case kJSONFloat: return "float";
	case kJSONString: return "string";
	case kJSONArray: return "array";
	case kJSONTable: return "table";
	default: return "???";
	}
}


// --------------------------------------------------------------------------------
void willDecodeSublist(json_decoder* decoder, const char* name, json_value_type type)
{

	if (strcmp(name, "beat") == 0)
	{
		decodeData.nLoadStates[decodeData.nStateCount++] = LOAD_STATE_HEADER;
	}
	else if (strcmp(name, "tracks") == 0)
	{
		decodeData.nLoadStates[decodeData.nStateCount++] = LOAD_STATE_TRACK_INFO;
	}
	else if (strcmp(name, "env") == 0)
	{
		decodeData.nLoadStates[decodeData.nStateCount++] = LOAD_STATE_ENVOLOPE;
	}
	else if (strcmp(name, "loop") == 0)
	{
		decodeData.nLoadStates[decodeData.nStateCount++] = LOAD_STATE_LOOP;
	}
	else if (strcmp(name, "filter") == 0)
	{
		decodeData.nLoadStates[decodeData.nStateCount++] = LOAD_STATE_FILTER;
	}
	else if (strcmp(name, "delay") == 0)
	{
		decodeData.nLoadStates[decodeData.nStateCount++] = LOAD_STATE_DELAY;
	}
	else if (strcmp(name, "bitcrush") == 0)
	{
		decodeData.nLoadStates[decodeData.nStateCount++] = LOAD_STATE_BITCRUSHER;
	}
	else if (strcmp(name, "notes") == 0)
	{
		decodeData.nLoadStates[decodeData.nStateCount++] = LOAD_STATE_NOTES;
	}
	else if (strcmp(name, "labels") == 0)
	{
		decodeData.nLoadStates[decodeData.nStateCount++] = LOAD_STATE_LABELS;
	}
	else if (strcmp(name, "scale") == 0)
	{
		decodeData.nLoadStates[decodeData.nStateCount++] = LOAD_STATE_SCALE;
		memset(decodeData.szBuffer, 0, 128);
		memset(decodeData.szBufferSmall, 0, 32);
	}
	else if (strcmp(name, "options") == 0)
	{
		decodeData.nLoadStates[decodeData.nStateCount++] = LOAD_STATE_OPTIONS;
	}
	
}


// --------------------------------------------------------------------------------
int shouldDecodeTableValueForKey(json_decoder* decoder, const char* key)
{
	
	return 1;
}


// --------------------------------------------------------------------------------
void didDecodeTableValue(json_decoder* decoder, const char* key, json_value value)
{
	

	if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_TRACK_INFO)
	{
		if (strcmp(key, "id") == 0)
		{
			decodeData.nTrack = json_intValue(value);
		}
		else if (strcmp(key, "name") == 0)
		{
			strcpy(pBeatMachine->pTracks[decodeData.nTrack]->szTrackName, json_stringValue(value));

		}
		else if (strcmp(key, "vol") == 0)
		{
			BeatMachineSetVolume(decodeData.nTrack, json_floatValue(value));
		}
		else if (strcmp(key, "pan") == 0)
		{
			BeatMachineSetPanning(decodeData.nTrack, json_floatValue(value));
		}
		else if (strcmp(key, "color") == 0)
		{
			
		}
		else if (strcmp(key, "sample") == 0)
		{
			BeatMachineSetSample(decodeData.nTrack, "samples/", json_stringValue(value));
		}
		else if (strcmp(key, "type") == 0)
		{
			char* szName = json_stringValue(value);
			if (strcmp(szName, "sampler") == 0)
			{
				BeatMachineCreateSampler(decodeData.nTrack);
			}
			else
			{
				BeatMachineCreateSynthByName(decodeData.nTrack, szName);
			}
		}
		else if (strcmp(key, "mute") == 0)
		{
			BeatMachineMuteTrack(decodeData.nTrack, json_intValue(value));
		}
		else if (strcmp(key, "chord") == 0)
		{
			BeatMachineSetChordTrack(decodeData.nTrack, json_intValue(value));
			
		}
		
	}
	else if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_NOTES)
	{
		if (strcmp(key, "step") == 0)
		{
			decodeData.nStep = json_intValue(value);
		}
		else if (strcmp(key, "pitch") == 0)
		{
			int nPitch = json_intValue(value);
			decodeData.note.pitch = nPitch;
		}
		else if (strcmp(key, "len") == 0)
		{
			decodeData.note.len = json_intValue(value);
		}
		else if (strcmp(key, "vel") == 0)
		{
			decodeData.note.velocity = json_floatValue(value);
		}
	}

	else if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_HEADER)
	{
		if (strcmp(key, "ver") == 0)
		{
			decodeData.nFileVersion = json_intValue(value);
		}
		else if (strcmp(key, "BPM") == 0)
		{
			BeatMachineSetBPM(json_intValue(value));
		}

	}
	else if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_ENVOLOPE)
	{
		if (strcmp(key, "a") == 0)
		{
			decodeData.fValue1 = json_floatValue(value);
		}
		else if (strcmp(key, "d") == 0)
		{
			decodeData.fValue2 = json_floatValue(value);
		}
		else if (strcmp(key, "s") == 0)
		{
			decodeData.fValue3 = json_floatValue(value);
		}
		else if (strcmp(key, "r") == 0)
		{
			decodeData.fValue4 = json_floatValue(value);
		}

	}
	else if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_SCALE)
	{
		if (strcmp(key, "type") == 0)
		{
			strcpy(decodeData.szBuffer, json_stringValue(value));
		}
		else if (strcmp(key, "base") == 0)
		{
			strcpy(decodeData.szBufferSmall, json_stringValue(value));
		}
	}
	else if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_FILTER)
	{
		if (strcmp(key, "freq") == 0)
		{
			decodeData.nValue = json_intValue(value);
		}
		else if (strcmp(key, "type") == 0)
		{
			decodeData.nExtra = json_intValue(value);
		}
		else if (strcmp(key, "resn") == 0)
		{
			decodeData.fValue1 = json_floatValue(value);
		}
		else if (strcmp(key, "mix") == 0)
		{
			decodeData.fValue2 = json_floatValue(value);
		}

	}
	else if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_DELAY)
	{
		if (strcmp(key, "feedback") == 0)
		{
			decodeData.fValue1 = json_floatValue(value);
		}
		else if (strcmp(key, "mix") == 0)
		{
			decodeData.fValue2 = json_floatValue(value);
		}

	}
	else if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_BITCRUSHER)
	{
		if (strcmp(key, "amount") == 0)
		{
			decodeData.fValue1 = json_floatValue(value);
		}
		else if (strcmp(key, "mix") == 0)
		{
			decodeData.fValue2 = json_floatValue(value);
		}

	}
	

}


// --------------------------------------------------------------------------------
int shouldDecodeArrayValueAtIndex(json_decoder* decoder, int pos)
{

	if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_NOTES)
	{
		decodeData.nArrayPos = pos;
		decodeData.note.pitch = 0;
		decodeData.note.len = 0;
		decodeData.note.velocity = 0.0f;
	}
	else if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_LABELS)
	{
		decodeData.nArrayPos = pos;
		memset(decodeData.szBuffer, 0, 128);
		decodeData.nValue = 0;
	}

	return 1;
}


// --------------------------------------------------------------------------------
void didDecodeArrayValue(json_decoder* decoder, int pos, json_value value)
{

	if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_NOTES)
	{
		if (decodeData.nArrayPos == pos)
		{
			if (pBeatMachine)
			{
				int nStep = decodeData.nStep;
				int nTrack = decodeData.nTrack;

				pd->sound->track->addNoteEvent(pBeatMachine->pTracks[nTrack]->pTrack, nStep, decodeData.note.len, decodeData.note.pitch, decodeData.note.velocity);

				int nLength = nStep + decodeData.note.len;
				if (nLength > pBeatMachine->nBeatLength)
					pBeatMachine->nBeatLength = nLength;

				if (pBeatMachine->pTracks[nTrack]->bIsChordTrack && pBeatMachine->pScaleManager->nCurrentScale != SCALE_CHROMATIC)
				{
					int nPitchIndex = pBeatMachine->pScaleManager->nPitchToIndexTable[decodeData.note.pitch];

					int nPitch3Index = nPitchIndex + 2;
					if (nPitch3Index > pBeatMachine->pScaleManager->nMaxIndex)
						nPitch3Index -= pBeatMachine->pScaleManager->nCurrentScalePitchCount;
					int nPitch3 = pBeatMachine->pScaleManager->nCurrentPitchTable[nPitch3Index];
					pd->sound->track->addNoteEvent(pBeatMachine->pTracks[nTrack]->pTrack, nStep, decodeData.note.len, nPitch3, decodeData.note.velocity);

					int nPitch5Index = nPitchIndex + 4;
					if (nPitch5Index > pBeatMachine->pScaleManager->nMaxIndex)
						nPitch5Index -= pBeatMachine->pScaleManager->nCurrentScalePitchCount;
					int nPitch5 = pBeatMachine->pScaleManager->nCurrentPitchTable[nPitch5Index];
					pd->sound->track->addNoteEvent(pBeatMachine->pTracks[nTrack]->pTrack, nStep, decodeData.note.len, nPitch5, decodeData.note.velocity);


				}

			}
		}
	}
	

}


// --------------------------------------------------------------------------------
void* didDecodeSublist(json_decoder* decoder, const char* name, json_value_type type)
{

	if (strcmp(name, "beat") == 0)
	{
		if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_HEADER)
			decodeData.nStateCount--;

	}
	else if (strcmp(name, "tracks") == 0)
	{
	
		if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_TRACK_INFO)
			decodeData.nStateCount--;

	}
	else if (strcmp(name, "env") == 0)
	{
		if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_ENVOLOPE)
		{
			decodeData.nStateCount--;

			BeatMachineSetADSR(decodeData.nTrack, decodeData.fValue1, decodeData.fValue2, decodeData.fValue3, decodeData.fValue4);
		}
		
	}
	else if (strcmp(name, "loop") == 0)
	{
		if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_LOOP)
			decodeData.nStateCount--;

	}
	else if (strcmp(name, "lpf") == 0)
	{
		if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_FILTER)
		{
			decodeData.nStateCount--;
			BeatMachineEnableFilter(decodeData.nTrack, decodeData.nExtra, decodeData.nValue, decodeData.fValue1, decodeData.fValue2);
		}

	}
	else if (strcmp(name, "delay") == 0)
	{
		if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_DELAY)
		{
			decodeData.nStateCount--;
			BeatMachineEnableDelay(decodeData.nTrack, decodeData.fValue1, decodeData.fValue2);
			
		}

	}
	else if (strcmp(name, "bitcrush") == 0)
	{
		if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_BITCRUSHER)
		{
			decodeData.nStateCount--;
			BeatMachineEnableBitCrusher(decodeData.nTrack, decodeData.fValue1, decodeData.fValue2);

		}

	}
	else if (strcmp(name, "notes") == 0)
	{
		if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_NOTES)
			decodeData.nStateCount--;
	}
	else if (strcmp(name, "scale") == 0)
	{
		if (decodeData.nLoadStates[decodeData.nStateCount - 1] == LOAD_STATE_SCALE)
		{
			decodeData.nStateCount--;

			SetupScaleWithString(pBeatMachine->pScaleManager, decodeData.szBuffer, decodeData.szBufferSmall);
		}
	}
	


	return NULL;
}


// --------------------------------------------------------------------------------
int BeatMachineLoadBeat(const char* szName)
{

	json_decoder decoder =
	{
		.decodeError = decodeError,
		.willDecodeSublist = willDecodeSublist,
		.shouldDecodeTableValueForKey = shouldDecodeTableValueForKey,
		.didDecodeTableValue = didDecodeTableValue,
		.shouldDecodeArrayValueAtIndex = shouldDecodeArrayValueAtIndex,
		.didDecodeArrayValue = didDecodeArrayValue,
		.didDecodeSublist = didDecodeSublist
	};

	char szPath[256];
	memset(szPath, 0, 256);
	strcpy(szPath, "beats/");
	strcat(szPath, szName);

	if (pBeatMachine)
	{
		if (pBeatMachine->szBeatName)
			Engine_MemFree(pBeatMachine->szBeatName);

		pBeatMachine->szBeatName = Engine_StrDup(szName);
	}

	SDFile* file = pd->file->open(szPath, kFileRead | kFileReadData);

	if (file == NULL)
		pd->system->logToConsole("filerror: %s", pd->file->geterr());

	json_value val;

	pd->json->decode(&decoder, (json_reader) { .read = (json_readFunc*)pd->file->read, .userdata = file }, & val);
	pd->file->close(file);

	return 0;
}


// --------------------------------------------------------------------------------
void BeatMachinePlayTheBeat(int nLoops)
{

	if (pBeatMachine && pBeatMachine->pSequence)
	{
		pd->sound->sequence->setLoops(pBeatMachine->pSequence, 0, pBeatMachine->nBeatLength, nLoops);

		pd->sound->sequence->setCurrentStep(pBeatMachine->pSequence, 0, 0, 0);

		pd->sound->sequence->play(pBeatMachine->pSequence, NULL, NULL);
	}

}


// --------------------------------------------------------------------------------
void BeatMachineStopTheBeat()
{
	if (pBeatMachine && pBeatMachine->pSequence)
		pd->sound->sequence->stop(pBeatMachine->pSequence);
}
