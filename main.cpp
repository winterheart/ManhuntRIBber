/* SPDX-FileCopyrightText: Copyright 2024 Azamat H. Hackimov <azamat.hackimov@gmail.com> */
/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <vector>

#include "CLI11.hpp"
#include "adpcm_codec.h"
#include "byteswap.h"
#include "codec.h"
#include "manhuntribber_version.h"

typedef struct WAV_HEADER {
  char RIFF[4] = {'R', 'I', 'F', 'F'};               // RIFF Header      Magic header
  uint32_t ChunkSize = 0;                            // RIFF Chunk Size
  char WAVE[4] = {'W', 'A', 'V', 'E'};               // WAVE Header
  char fmt[4] = {'f', 'm', 't', ' '};                // FMT header
  uint32_t Subchunk1Size = UTILS::convert_le(16);    // Size of the fmt chunk
  uint16_t AudioFormat = UTILS::convert_le(1);       // Audio format 1=PCM
  uint16_t NumOfChan = UTILS::convert_le(2);         // Number of channels 1=Mono 2=Stereo
  uint32_t SamplesPerSec = UTILS::convert_le(44100); // Sampling Frequency in Hz
  uint32_t bytesPerSec = UTILS::convert_le(176400);  // bytes per second (SamplesPerSec * blockAlign)
  uint16_t blockAlign = UTILS::convert_le(4);        // 2=16-bit mono, 4=16-bit stereo
  uint16_t bitsPerSample = UTILS::convert_le(16);    // Number of bits per sample
  char Subchunk2ID[4] = {'d', 'a', 't', 'a'};        // "data"  string
  uint32_t Subchunk2Size = 0;                        // Sampled data length
} wav_hdr;

// Some globals

int interleave = 0x10000;
int chunk_size = 0x400;
int nb_chunks_in_interleave = interleave / chunk_size;
int nb_chunk_encoded = chunk_size - 4;
int nb_chuck_decoded = 2 * nb_chunk_encoded + 1;
int nb_channels = 2;  // Mono/Stereo

void decode(const std::filesystem::path &in_file, std::filesystem::path out_file, bool is_mono, uint32_t frequency) {
  Codec codec(is_mono, frequency, 1);
  codec.decode(in_file, out_file);
}

void encode(const std::filesystem::path &in_file, std::filesystem::path out_file) {
  if (out_file.empty()) {
    (out_file = in_file).replace_extension("rib");
  }
  std::ifstream input_file(in_file, std::ios::binary);
  std::ofstream output_file(out_file, std::ios::binary);

  if (!input_file.is_open()) {
    std::cout << std::format("Can't open input file for reading {}", in_file.string()) << std::endl;
    exit(1);
  }
  if (!output_file.is_open()) {
    std::cout << std::format("Can't open output file for writing {}", out_file.string()) << std::endl;
    exit(1);
  }

  std::cout << std::format("Encoding {} to {}... ", in_file.string(), out_file.string());

  std::vector<std::vector<int16_t>> inputs(nb_channels);
  std::vector<std::shared_ptr<std::vector<int8_t>>> outputs(nb_channels);
  for (auto &out : outputs) {
    out = std::make_shared<std::vector<int8_t>>();
  }

  wav_hdr wave_header;
  input_file.read(reinterpret_cast<char *>(&wave_header), sizeof(wav_hdr));
  nb_channels = UTILS::convert_le(wave_header.NumOfChan);
  if (nb_channels == 1) {
    chunk_size = 0x200;
    nb_chunks_in_interleave = interleave / chunk_size;
    nb_chunk_encoded = chunk_size - 4;
    nb_chuck_decoded = 2 * nb_chunk_encoded + 1;
  }

  while (!input_file.eof()) {
    for (int ch = 0; ch < nb_channels; ch++) {
      int16_t r;
      input_file.read(reinterpret_cast<char *>(&r), 2);
      inputs[ch].push_back(UTILS::convert_le(r));
    }
  }
  input_file.close();
  int nb_interleaves_per_ch = std::floor(inputs[0].size() / (2 * nb_chunk_encoded * nb_chunks_in_interleave));

  for (int ch = 0; ch < nb_channels; ch++) {
    // Reserve additional size in inputs
    inputs[ch].reserve(nb_interleaves_per_ch * 2 * nb_chunk_encoded * nb_chunks_in_interleave);

    auto channel_status = std::make_shared<ADPCMChannelStatus>();
    for (int sample = 0; sample < nb_interleaves_per_ch * nb_chunks_in_interleave; sample++) {
      std::vector<int16_t> input_frame(inputs[ch].begin() + sample * nb_chuck_decoded,
                                       inputs[ch].begin() + (sample + 1) * nb_chuck_decoded);
      adpcm_rib_encode_frame(channel_status, std::make_shared<std::vector<int16_t>>(input_frame), outputs[ch]);
    }
  }

  for (int i = 0; i < nb_interleaves_per_ch * interleave; i += interleave) {
    for (int ch = 0; ch < nb_channels; ch++) {
      std::vector<int8_t> buffer(outputs[ch]->begin() + i, outputs[ch]->begin() + (i + interleave));
      output_file.write(reinterpret_cast<char *>(buffer.data()), interleave);
    }
  }

  output_file.close();
  std::cout << "done!" << std::endl;
}

int main(int argc, char *argv[]) {

  std::filesystem::path in_file;
  std::filesystem::path out_file;
  bool is_mono = false;
  uint32_t frequency = 44100;

  CLI::App app{"ManhuntRIBber - encode/decode RIB files from Rockstar's Manhunt PC game"};
  app.set_version_flag("-v", MANHUNTRIBBER_VERSION);
  argv = app.ensure_utf8(argv);
  std::cout << std::format("ManhuntRIBber {} https://github.com/winterheart/ManhuntRIBber\n"
                           "(c) 2024 Azamat H. Hackimov <azamat.hackimov@gmail.com>\n",
                           app.version())
            << std::endl;

  auto encode_cmd =
      app.add_subcommand("encode", "Encode WAV file to RIB")->callback([&]() { encode(in_file, out_file); });
  encode_cmd->add_option("input", in_file, "Input RIB file")->required()->check(CLI::ExistingFile);
  encode_cmd->add_option("output", out_file, "Output WAV file");

  auto decode_cmd =
      app.add_subcommand("decode", "Decode RIB file to WAV")->callback([&]() {
        decode(in_file, out_file, is_mono, frequency);
      });
  decode_cmd->add_option("-f", frequency, "Frequency of the stream")->default_val(frequency);
  decode_cmd->add_flag("-m", is_mono, "Threats input file as Mono stream")->default_val(is_mono);
  decode_cmd->add_option("input", in_file, "Input WAV file")->required()->check(CLI::ExistingFile);
  decode_cmd->add_option("output", out_file, "Output RIB file");

  CLI11_PARSE(app, argc, argv);

  return 0;
}
