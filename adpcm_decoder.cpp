/* SPDX-FileCopyrightText: Copyright 2024 Azamat H. Hackimov <azamat.hackimov@gmail.com> */
/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <algorithm>
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
static inline int adpcm_ima_qt_expand_nibble(const std::shared_ptr<ADPCMChannelStatus> &c, int nibble) {
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

// Code borrowed from FFMPEG
static inline uint8_t adpcm_ima_qt_compress_sample(const std::shared_ptr<ADPCMChannelStatus> &c, int16_t sample) {
  int delta = sample - c->prev_sample;
  int diff, step = adpcm_step_table[c->step_index];
  int nibble = 8 * (delta < 0);

  delta = abs(delta);
  diff = delta + (step >> 3);

  if (delta >= step) {
    nibble |= 4;
    delta -= step;
  }
  step >>= 1;
  if (delta >= step) {
    nibble |= 2;
    delta -= step;
  }
  step >>= 1;
  if (delta >= step) {
    nibble |= 1;
    delta -= step;
  }
  diff -= delta;

  if (nibble & 8)
    c->prev_sample -= diff;
  else
    c->prev_sample += diff;

  c->prev_sample = adpcm_clip_int16(c->prev_sample);
  c->step_index = std::clamp(c->step_index + adpcm_index_table[nibble], 0, 88);

  return nibble;
}

int adpcm_rib_decode_frame(const std::shared_ptr<std::vector<int8_t>> &in_stream,
                           const std::shared_ptr<std::vector<int16_t>> &out_stream) {
  auto channel_status = std::make_shared<ADPCMChannelStatus>();

  channel_status->predictor = (((uint32_t)in_stream->at(1)) << 8) | (uint8_t)in_stream->at(0);
  channel_status->step_index = in_stream->at(2);

  // Save first sample as is
  out_stream->push_back((int16_t)channel_status->predictor);

  for (auto pos = in_stream->cbegin() + 4; pos != in_stream->cend(); ++pos) {
    out_stream->push_back((int16_t)adpcm_ima_qt_expand_nibble(channel_status, ((uint8_t)*pos) & 0x0f));
    out_stream->push_back((int16_t)adpcm_ima_qt_expand_nibble(channel_status, ((uint8_t)*pos) >> 4));
  }

  return 0;
}

int adpcm_rib_encode_frame(const std::shared_ptr<ADPCMChannelStatus> &channel_status,
                           const std::shared_ptr<std::vector<int16_t>> &in_stream,
                           const std::shared_ptr<std::vector<int8_t>> &out_stream) {
  channel_status->prev_sample = in_stream->at(0);
  out_stream->push_back((int8_t)((uint8_t)(channel_status->prev_sample & 0xFF)));
  out_stream->push_back((uint8_t)(channel_status->prev_sample >> 8));
  out_stream->push_back((int8_t)channel_status->step_index);
  out_stream->push_back(0);

  auto pos = in_stream->cbegin() + 1;
  while (pos != in_stream->cend()) {
    uint8_t nibble1 = adpcm_ima_qt_compress_sample(channel_status, *pos++);
    uint8_t nibble2 = adpcm_ima_qt_compress_sample(channel_status, *pos++);
    out_stream->push_back((int8_t)(nibble2 << 4 | nibble1));
  }
  return 0;
}
