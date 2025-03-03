# ManhuntRIBber - a tool converting RIB files from Rockstar's Manhunt game

## Usage

```shell
# Decoding
manhuntribber decode -o JOURNO.WAV JOURNO.RIB
# Encoding
manhuntribber encode -o JOURNO.RIB JOURNO.WAV
```

For more help, use `manhuntribber decode -h` and `manhuntribber encode -h`.

A WAV-file should be PCM encoded 16-bit stereo 22050/44100 Hz (or mono 44100 Hz
in case of mono stream) **without any metadata tags** in order to be encoded to
RIB format. See "File types" section for the reference.

## Examples

```shell
# Decode mono stream
manhuntribber decode -m -o AXE1A.WAV audio/PC/EXECUTE/AXE/AXE1A.RIB

# Decode complex stream
# Decoded files will be MALL_M_0.WAV .. MALL_M_5.WAV 
manhuntribber decode -c -o MALL_M.WAV audio/PC/MUSIC/MALL/MALL_M.RIB

# Encode stream
manhuntribber encode -o FE_C.RIB FE_C.WAV

# Encode complex stream
# Define exactly 6 WAV files as input
manhuntribber encode -o MALL_M.RIB MALL_M_0.WAV MALL_M_1.WAV MALL_M_2.WAV MALL_M_3.WAV MALL_M_4.WAV MALL_M_5.WAV
```

## Compilation

Project requires any modern C++20 compiler (gcc 10, clang 10, msvc 2019) and
cmake.

```shell
cmake -B build
cmake --build build
```

## File format

The file is a stream of samples encoded by a variation of the ADPCM IMA
algorithm (in FFMPEG this algorithm is identified as `ADPCM_IMA_QT`, but the
implementation differs in detail).

Stream may be mono (1 channel) or stereo (2 channels).
Each of the channels is in turn encoded in an interleave of 0x10000 bytes in
size into frames of 0x400 (for frequency 44100 Hz) or 0x200 (for frequency
22050 Hz) bytes in size. Thus, in one interleave there are up
to 64 channel frames, encoded by a variation of the `ADPCM_IMA_QT` algorithm.
The key difference from the original implementation is the storage of data for
encoder initialization: while `ADPCM_IMA_QT` uses packed storage of `predicor`
and `step_index` in a 16-bit value, in the modified algorithm `predictor` is
stored in the first and second bytes of the frame as 16-bit number
(little-endian), and `step_index` is in the third byte as an 8-bit number.
The structure is aligned to four bytes, leaving the fourth byte unused and
equal to zero.

If there is fewer data than the frame size at the end of the file, the
remaining frame and interleave space remains filled with zeros.

### Complex files

There also exists complex files format mainly for music environment (like
`ASYLUM_M.RIB`). These files are divided into six tracks. First four tracks
based on awareness of NPC about player (0 is calm music, where player is
undetected; 3 is intense music, where player is being in fight). Last two
tracks are reserved and used for some environmental sounds (like moaning and
screams of electrocuted captive in ASYLUM level). If there is no use for such
sounds in level, these tracks are filled with random tracks (white noise and
raining sound).

Each track is recorded in turn in an interleave sequence. Thus, to read
the fourth track, you need to read every fourth in the sequence of six
interleaves.

## File types

| File name                                                                            | Mono/stereo | Frequency |
|--------------------------------------------------------------------------------------|-------------|-----------|
| `audio/PC/EXECUTE/*/*.RIB`                                                           | mono        | 44100     |
| `audio/PC/MUSIC/*/*_D.RIB`, `audio/PC/MUSIC/*/*_S.RIB`, `audio/PC/SCRIPTED/*/*.RIB`, | stereo      | 44100     |
| `audio/PC/MUSIC/*/*_C.RIB`, `audio/PC/MUSIC/*/*_L.RIB`, `audio/PC/MUSIC/*/*_M.RIB`¹  | stereo      | 22050     |

¹ `audio/PC/MUSIC/*/*_M.RIB` are complex files (`-c` flag).

## License

Project licensed under LGPL-2.1 or later license. See LICENSE file for more info.

ManhuntRIBber

Copyright (C) 2024-2025  Azamat H. Hackimov

This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation; either version 2.1 of the License, or (at your option)
any later version.

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details.

You should have received a copy of the GNU Lesser General Public License along
with this library; if not, write to the Free Software Foundation, Inc., 51
Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

----

Project uses CLI11 library under a 3-Clause BSD license. See CLI11.hpp for
additional info.

----

Project uses music samples from

"Galway"
Kevin MacLeod (incompetech.com)
Licensed under Creative Commons: By Attribution 3.0
http://creativecommons.org/licenses/by/3.0/

AeroCE Ringtone 3 by ScottyD0ES -- https://freesound.org/s/717007/ -- License: Attribution NonCommercial 4.0

12492 elven welcome intro by Robinhood76 -- https://freesound.org/s/725249/ -- License: Attribution NonCommercial 4.0

Piano Logo by LiteSaturation -- https://freesound.org/s/785344/ -- License: Attribution NonCommercial 4.0

Birds Chirping by 241378 -- https://freesound.org/s/766837/ -- License: Attribution NonCommercial 4.0

Crickets chirping by Qhama -- https://freesound.org/s/766093/ -- License: Attribution NonCommercial 4.0

Cinematic 10-2 Synth Gm 105 bpm by Josee1969 -- https://freesound.org/s/732830/ -- License: Attribution NonCommercial 4.0
