/* SPDX-FileCopyrightText: Copyright 2024 Azamat H. Hackimov <azamat.hackimov@gmail.com> */
/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <vector>

#include "CLI11.hpp"
#include "byteswap.h"
#include "codec.h"
#include "manhuntribber_version.h"

void decode(const std::filesystem::path &in_file, std::filesystem::path out_file, bool is_mono, uint32_t frequency) {
  Codec codec(is_mono, frequency, 1);
  codec.decode(in_file, out_file);
}

void encode(const std::filesystem::path &in_file, std::filesystem::path out_file) {
  std::ifstream input_file(in_file, std::ios::binary);

  if (!input_file.is_open()) {
    std::cout << std::format("Can't open input file for reading {}", in_file.string()) << std::endl;
    exit(1);
  }

  wav_hdr wave_header;
  input_file.read(reinterpret_cast<char *>(&wave_header), sizeof(wav_hdr));
  input_file.close();
  Codec codec(UTILS::convert_le(wave_header.NumOfChan) == 1, UTILS::convert_le(wave_header.SamplesPerSec), 1);
  codec.encode({in_file}, out_file);
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
