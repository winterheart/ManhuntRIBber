/* SPDX-FileCopyrightText: Copyright 2024 Azamat H. Hackimov <azamat.hackimov@gmail.com> */
/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <format>
#include <fstream>
#include <iostream>

#include "adpcm_codec.h"
#include "byteswap.h"
#include "codec.h"

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

Codec::Codec(bool is_mono, uint32_t frequency, uint32_t count_files) {
  m_count_files = count_files;
  m_frequency = frequency;
  if (is_mono) {
    m_nb_channels = 1;
    m_chunk_size = 0x200;
  } else {
    m_nb_channels = 2;
    m_chunk_size = 0x400;
  }
  m_nb_chunks_in_interleave = m_interleave / m_chunk_size;
  m_nb_chunk_encoded = m_chunk_size - 4;
  m_nb_chunk_decoded = 2 * m_nb_chunk_encoded + 1;
}

void Codec::decode(const std::filesystem::path &rib_file, std::filesystem::path wav_file) {
  if (wav_file.empty()) {
    (wav_file = rib_file).replace_extension("wav");
  }
  std::ifstream input_file(rib_file, std::ios::binary);
  std::ofstream output_file(wav_file, std::ios::binary);

  if (!input_file.is_open()) {
    std::cout << std::format("Can't open input file for reading {}", rib_file.string()) << std::endl;
    exit(1);
  }
  if (!output_file.is_open()) {
    std::cout << std::format("Can't open output file for writing {}", wav_file.string()) << std::endl;
    exit(1);
  }

  std::cout << std::format("Decoding {} to {}... ", rib_file.string(), wav_file.string());

  input_file.seekg(0, std::ios::end);
  size_t input_size = input_file.tellg();
  input_file.seekg(0, std::ios::beg);

  size_t nb_samples = input_size / (m_nb_channels * m_interleave);
  uint32_t nb_chunks = m_interleave / m_chunk_size;

  wav_hdr wave_header;
  output_file.write(reinterpret_cast<char *>(&wave_header), sizeof(wav_hdr));

  for (int i = 0; i < nb_samples; i++) {
    std::vector<std::shared_ptr<std::vector<int16_t>>> outputs(m_nb_channels);
    for (auto &out : outputs) {
      out = std::make_shared<std::vector<int16_t>>();
    }

    for (int ch = 0; ch < m_nb_channels; ch++) {
      for (int j = 0; j < nb_chunks; j++) {
        std::vector<int8_t> input_buffer(m_chunk_size);
        input_file.read(reinterpret_cast<char *>(input_buffer.data()), m_chunk_size);
        adpcm_rib_decode_frame(std::make_shared<std::vector<int8_t>>(input_buffer), outputs[ch]);
      }
    }
    for (int j = 0; j < m_nb_chunk_decoded * nb_chunks; j++) {
      for (int ch = 0; ch < m_nb_channels; ch++) {
        int16_t r = UTILS::convert_le(outputs[ch]->at(j));
        output_file.write(reinterpret_cast<char *>(&r), 2);
      }
    }
  }
  size_t size = output_file.tellp();
  // Rewrite wave header with actual sizes
  wave_header.ChunkSize = UTILS::convert_le(size - 8);
  wave_header.Subchunk2Size = UTILS::convert_le(size - 44);
  wave_header.NumOfChan = UTILS::convert_le(m_nb_channels);

  wave_header.SamplesPerSec = UTILS::convert_le(m_frequency);
  wave_header.bytesPerSec = UTILS::convert_le(m_frequency * 4);

  output_file.seekp(0, std::ios::beg);
  output_file.write(reinterpret_cast<char *>(&wave_header), sizeof(wav_hdr));

  input_file.close();
  output_file.close();
  std::cout << "done!" << std::endl;
}

void Codec::encode(std::vector<std::filesystem::path> in_files, std::filesystem::path rib_file) {}
