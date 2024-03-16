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
  int16_t prev_sample;  // for encoding
} ADPCMChannelStatus;

int adpcm_rib_decode_frame(const std::shared_ptr<std::vector<int8_t>>&in_stream, std::shared_ptr<std::vector<int16_t>> out_stream);

int adpcm_rib_encode_frame(std::shared_ptr<ADPCMChannelStatus> channel_status, std::shared_ptr<std::vector<int16_t>> in_stream, std::shared_ptr<std::vector<int8_t>> out_stream);
