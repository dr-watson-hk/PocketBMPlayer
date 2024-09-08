PocketBM is a beat making app for Playdate. This project is the source code for playing back beat files created by PocketBM.

The format of a beat file is just plain JSON so nothing fancy here, just some codes for decoding JSON, setting up synths and filling in MIDI notes in the sequencer for playback.

scale_manager.c is there for setting notes in the chord track. If you don't care about the chord track, you can remove it from the project and remove the related code from beat_machine.c.

It's very simple to use the player code, just as an example, to play a beat file call "demo":

BeatMachineCreate(playdate);

BeatMachineLoadBeat("demo.bmf");

BeatMachinePlayTheBeat(1);


--------------------------------------------------------------------------------
Copyright (C) Khors Media

Permission to use, copy, modify, and/or distribute this software for any
purpose without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
