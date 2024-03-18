# ManhuntRIBber - a tool converting RIB files from Rockstar's Manhunt game

## Usage

```shell
# Decoding
manhuntribber -d JOURNO.RIB JOURNO.WAV
# Encoding
manhuntribber -e JOURNO.WAV JOURNO.RIB
```

A WAV-file should be PCM encoded 16-bit stereo 44100 Hz in order to be encoded
to RIB format.

## File format

The file is a stream of samples encoded by a variation of the ADPCM IMA
algorithm (in FFMPEG this algorithm is identified as `ADPCM_IMA_QT`, but the
implementation differs in detail).

Each of the channels is in turn encoded in an interleave of 0x10000 bytes in
size into frames of 0x400 bytes in size. Thus, in one interleave there are up
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

## License

Project licensed under LGPL-2.1 or later license. See LICENSE file for more info.

ManhuntRIBber
Copyright (C) 2024  Azamat H. Hackimov

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


