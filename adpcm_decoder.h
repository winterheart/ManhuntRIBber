/* SPDX-FileCopyrightText: Copyright 2024 Azamat H. Hackimov <azamat.hackimov@gmail.com> */
/* SPDX-License-Identifier: LGPL-2.1-or-later */
#pragma once

#include <cstdint>
#include <memory>
#include <vector>

typedef struct ADPCMChannelStatus {
  int predictor;
  int16_t step_index;
  int step;
  int prev_sample;  // for encoding
} ADPCMChannelStatus;

int adpcm_ima_qt_expand_nibble(const std::shared_ptr<ADPCMChannelStatus>& c, int nibble);

int adpcm_rib_decode_frame(const std::shared_ptr<std::vector<int8_t>>& input_stream, std::shared_ptr<std::vector<int16_t>> out_stream);