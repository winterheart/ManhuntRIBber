/* SPDX-FileCopyrightText: Copyright 2024 Azamat H. Hackimov <azamat.hackimov@gmail.com> */
/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "adpcm_decoder.h"
#include "CLI11.hpp"

typedef struct WAV_HEADER {
  char RIFF[4] = {'R', 'I', 'F', 'F'}; // RIFF Header      Magic header
  uint32_t ChunkSize = 0;              // RIFF Chunk Size
  char WAVE[4] = {'W', 'A', 'V', 'E'}; // WAVE Header
  char fmt[4] = {'f', 'm', 't', ' '};  // FMT header
  uint32_t Subchunk1Size = 16;         // Size of the fmt chunk
  uint16_t AudioFormat = 1;            // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
  uint16_t NumOfChan = 2;              // Number of channels 1=Mono 2=Sterio
  uint32_t SamplesPerSec = 44100;      // Sampling Frequency in Hz
  uint32_t bytesPerSec = 176400;       // bytes per second (SamplesPerSec * blockAlign)
  uint16_t blockAlign = 4;             // 2=16-bit mono, 4=16-bit stereo
  uint16_t bitsPerSample = 16;         // Number of bits per sample
  char Subchunk2ID[4] = {'d', 'a', 't', 'a'}; // "data"  string
  uint32_t Subchunk2Size = 0;                 // Sampled data length
} wav_hdr;

void decode(const std::filesystem::path& in_file, std::filesystem::path out_file) {
  int interleave = 0x10000;
  int chunk_size = 0x400;
  int nb_chunk_encoded = chunk_size - 4;
  int nb_chuck_decoded = 2 * nb_chunk_encoded;
#ifdef PLACE_PREDICTOR_TWICE
  // dunno, why towav places predictor twice at beginning?
  nb_chuck_decoded++;
#endif

  if (out_file.empty()) {
    (out_file = in_file).replace_extension("wav");
  }
  std::ifstream input_file(in_file, std::ios::binary);
  std::ofstream output_file(out_file, std::ios::binary);

  if (!input_file.is_open() || !output_file.is_open()) {
    std::cout << "Can't open input or output file" << std::endl;
    return;
  }

  input_file.seekg(0, std::ios::end);
  size_t input_size = input_file.tellg();
  input_file.seekg(0, std::ios::beg);

  size_t nb_samples = input_size / (2 * interleave);
  int nb_chunks = interleave / chunk_size;

  wav_hdr wave_header;
  output_file.write(reinterpret_cast<char *>(&wave_header), sizeof(wav_hdr));

  for (int i = 0; i < nb_samples; i++) {
    std::vector<std::shared_ptr<std::vector<int16_t>>> outputs(2);
    for (auto &out : outputs) {
      out = std::make_shared<std::vector<int16_t>>();
    }

    for (int ch = 0; ch < 2; ch++) {
      for (int j = 0; j < nb_chunks; j++) {
        std::vector<int8_t> input_buffer(chunk_size);
        input_file.read(reinterpret_cast<char *>(input_buffer.data()), chunk_size);
        adpcm_rib_decode_frame(std::make_shared<std::vector<int8_t>>(input_buffer), outputs[ch]);
      }
    }
    for (int j = 0; j < nb_chuck_decoded * nb_chunks; j++) {
      output_file.write(reinterpret_cast<char *>(&outputs[0]->at(j)), 2);
      output_file.write(reinterpret_cast<char *>(&outputs[1]->at(j)), 2);
    }
  }
  size_t size = output_file.tellp();
  // Rewrite wave header with actual sizes
  wave_header.ChunkSize = size - 8;
  wave_header.Subchunk2Size = size  - 44;
  output_file.seekp(0, std::ios::beg);
  output_file.write(reinterpret_cast<char *>(&wave_header), sizeof(wav_hdr));

  input_file.close();
  output_file.close();
}

void encode(const std::filesystem::path& in_file, std::filesystem::path out_file) {
  int interleave = 0x10000;
  int chunk_size = 0x400;
  int nb_chunk_encoded = chunk_size - 4;
  int nb_chuck_decoded = 2 * nb_chunk_encoded;

#ifdef PLACE_PREDICTOR_TWICE
  // dunno, why towav places predictor twice at beginning?
  nb_chuck_decoded++;
#endif

  if (out_file.empty()) {
    (out_file = in_file).replace_extension("rib");
  }
  std::ifstream input_file(in_file, std::ios::binary);
  std::ofstream output_file(out_file, std::ios::binary);

  if (!input_file.is_open() || !output_file.is_open()) {
    std::cout << "Can't open input or output file" << std::endl;
    return;
  }

  std::vector<std::vector<int16_t>> inputs(2);

  std::vector<std::shared_ptr<std::vector<int8_t>>> outputs(2, std::make_shared<std::vector<int8_t>>());
  input_file.seekg(sizeof(wav_hdr), std::ios::beg);

  while (!input_file.eof()) {
    for (int ch = 0; ch < 2; ch++) {
      int16_t in;
      input_file.read(reinterpret_cast<char *>(&in), 2);
      inputs[ch].push_back(in);
    }
  }
  input_file.close();

  for (int ch = 0; ch < 2; ch++) {
    auto channel_status = std::make_shared<ADPCMChannelStatus>();
    channel_status->prev_sample = inputs[ch].at(0);
    for (int sample = 0; sample < interleave / chunk_size ; sample++) {
      std::vector<int16_t> input_frame(inputs[ch].begin() + sample * nb_chuck_decoded, inputs[ch].begin() + (sample + 1) * nb_chuck_decoded);
      adpcm_rib_encode_frame(channel_status, std::make_shared<std::vector<int16_t>>(input_frame), outputs[ch]);
    }
  }
  output_file.write(reinterpret_cast<const char *>(outputs[0]->data()), outputs[0]->size());
  output_file.write(reinterpret_cast<const char *>(outputs[1]->data()), outputs[0]->size());
  output_file.close();
  std::cout << outputs[0]->size() << std::endl;
}

int main(int argc, char *argv[]) {

  std::filesystem::path in_file;
  std::filesystem::path out_file;


  CLI::App app{"ManhuntRIBber - encode/decode RIB files from Rockstar's Manhunt PC game"};
  argv = app.ensure_utf8(argv);

  auto encode_cmd = app.add_subcommand("encode", "Encode WAV file to RIB")->callback([&](){
    encode(in_file, out_file);
  });

  auto decode_cmd = app.add_subcommand("decode", "Decode RIB file to WAV")->callback([&](){
    decode(in_file, out_file);
  });

  encode_cmd->add_option("input", in_file, "Input RIB file")->required()->check(CLI::ExistingFile);
  encode_cmd->add_option("output", out_file, "Output WAV file");

  decode_cmd->add_option("input", in_file, "Input WAV file")->required()->check(CLI::ExistingFile);
  decode_cmd->add_option("output", out_file, "Output RIB file");

  CLI11_PARSE(app, argc, argv);

  return 0;
}
