/* SPDX-FileCopyrightText: Copyright 2024 Azamat H. Hackimov <azamat.hackimov@gmail.com> */
/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <algorithm>
#include <memory>
#include <vector>

#include "adpcm_decoder.h"

// Taken from ADPCM reference
std::vector<int16_t> adpcm_step_table = {
    7,     8,     9,     10,    11,    12,    13,    14,    16,    17,    // 10
    19,    21,    23,    25,    28,    31,    34,    37,    41,    45,    // 20
    50,    55,    60,    66,    73,    80,    88,    97,    107,   118,   // 30
    130,   143,   157,   173,   190,   209,   230,   253,   279,   307,   // 40
    337,   371,   408,   449,   494,   544,   598,   658,   724,   796,   // 50
    876,   963,   1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,  // 60
    2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,  // 70
    5894,  6484,  7132,  7845,  8630,  9493,  10442, 11487, 12635, 13899, // 80
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767         // 89
};

// Taken from ADPCM reference
std::vector<int8_t> adpcm_index_table = {
    -1, -1, -1, -1, 2, 4, 6, 8, // 8
    -1, -1, -1, -1, 2, 4, 6, 8, // 16
};

// Utility helpers

static int16_t adpcm_clip_int16(int a) {
  if ((a + 0x8000U) & ~0xFFFF)
    return (a >> 31) ^ 0x7FFF;
  else
    return a;
}

// Code borrowed from FFMPEG
int adpcm_ima_qt_expand_nibble(const std::shared_ptr<ADPCMChannelStatus>& c, int nibble) {
  int step_index;
  int predictor;
  int diff, step;

  step = adpcm_step_table[c->step_index];
  step_index = c->step_index + adpcm_index_table[nibble];
  step_index = std::clamp(step_index, 0, 88);

  diff = step >> 3;
  if (nibble & 4)
    diff += step;
  if (nibble & 2)
    diff += step >> 1;
  if (nibble & 1)
    diff += step >> 2;

  if (nibble & 8)
    predictor = c->predictor - diff;
  else
    predictor = c->predictor + diff;

  c->predictor = adpcm_clip_int16(predictor);
  c->step_index = step_index;

  return c->predictor;
}

int adpcm_rib_decode_frame(const std::shared_ptr<std::vector<int8_t>>& input_stream, std::shared_ptr<std::vector<int16_t>> out_stream) {
  auto channel_status = std::make_shared<ADPCMChannelStatus>();

  channel_status->predictor = (((uint32_t)input_stream->at(1)) << 8) | (uint8_t)input_stream->at(0);
  channel_status->step_index = input_stream->at(2);

  out_stream->push_back((int16_t)channel_status->predictor);

  for (auto pos = input_stream->cbegin() + 4; pos != input_stream->cend(); ++pos) {
    out_stream->push_back((int16_t)adpcm_ima_qt_expand_nibble(channel_status, ((uint8_t)*pos) & 0x0f));
    out_stream->push_back((int16_t)adpcm_ima_qt_expand_nibble(channel_status, ((uint8_t)*pos) >> 4));
  }

  return 0;
}
