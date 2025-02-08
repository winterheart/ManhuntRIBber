/* SPDX-FileCopyrightText: Copyright 2024-2025 Azamat H. Hackimov <azamat.hackimov@gmail.com> */
/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <cmath>
#include <format>
#include <fstream>
#include <iostream>

#include "adpcm_codec.h"
#include "byteswap.h"
#include "codec.h"

Codec::Codec(bool is_mono, uint32_t frequency, uint32_t count_files) {
  m_count_files = count_files;
  m_frequency = frequency;
  m_chunk_size = (m_frequency == 22050) ? 0x200 : 0x400;
  m_nb_channels = is_mono ? 1 : 2;

  m_nb_chunks_in_interleave = m_interleave / m_chunk_size;
  m_nb_chunk_encoded = m_chunk_size - 4;
  m_nb_chunk_decoded = 2 * m_nb_chunk_encoded + 1;
}

void Codec::decode(const std::filesystem::path &rib_file, const std::filesystem::path& wav_file) const {
  std::filesystem::path wav_filename = wav_file;
  if (wav_filename.empty()) {
    (wav_filename = rib_file).replace_extension("wav");
  }
  std::vector<std::pair<std::filesystem::path, std::ofstream>> output_files;
  if (m_count_files == 1) {
    output_files.emplace_back(wav_filename, std::ofstream(wav_filename, std::ios::binary));
  } else {
    for (uint32_t i = 0; i < m_count_files; i++) {
      std::filesystem::path construct_file =
          wav_filename.parent_path() / std::format("{}_{}", wav_filename.stem().string(), i);
      construct_file.replace_extension(wav_filename.extension());
      output_files.emplace_back(construct_file, std::ofstream(construct_file, std::ios::binary));
    }
  }
  std::ifstream input_file(rib_file, std::ios::binary | std::ios::ate);

  if (!input_file.is_open()) {
    std::cout << std::format("Can't open input file for reading {}", rib_file.string()) << std::endl;
    exit(1);
  }

  for (auto const &itm : output_files) {
    if (!itm.second.is_open()) {
      std::cout << std::format("Can't open output file for writing {}", itm.first.string()) << std::endl;
      exit(1);
    }
  }

  std::cout << std::format("Decoding {} to {} ... ", rib_file.string(), wav_filename.string());

  size_t input_size = input_file.tellg();
  input_file.seekg(0, std::ios::beg);

  size_t nb_interleaves = input_size / (m_nb_channels * m_interleave);
  uint32_t nb_chunks = m_interleave / m_chunk_size;


  for (auto &itm : output_files) {
    itm.second.seekp(sizeof(wav_hdr), std::ios::beg);
  }

  for (int i = 0; i < nb_interleaves; i++) {
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
        output_files.at(i % m_count_files).second.write(reinterpret_cast<char *>(&r), 2);
      }
    }
  }

  for (int i = 0; i < m_count_files; i++) {
    size_t size = output_files.at(i).second.tellp();
    // Write wave header with actual sizes
    wav_hdr wave_header;
    wave_header.ChunkSize = UTILS::convert_le(size - 8);
    wave_header.Subchunk2Size = UTILS::convert_le(size - 44);
    wave_header.NumOfChan = UTILS::convert_le(m_nb_channels);

    wave_header.SamplesPerSec = UTILS::convert_le(m_frequency);
    wave_header.blockAlign = m_nb_channels * 2;
    wave_header.bytesPerSec = UTILS::convert_le(m_frequency * m_nb_channels * 2);

    output_files.at(i).second.seekp(0, std::ios::beg);
    output_files.at(i).second.write(reinterpret_cast<char *>(&wave_header), sizeof(wav_hdr));
    output_files.at(i).second.close();
  }

  input_file.close();
  std::cout << "done!" << std::endl;
}

void Codec::encode(std::vector<std::filesystem::path> in_files, std::filesystem::path rib_file) const {
  const auto& in_file = in_files.front();

  if (rib_file.empty()) {
    (rib_file = in_file).replace_extension("rib");
  }
  std::vector<std::pair<std::filesystem::path, std::ifstream>> input_files;
  std::ofstream output_file(rib_file, std::ios::binary);


  for (const auto &itm : in_files) {
    input_files.emplace_back(itm, std::ifstream(itm, std::ios::binary | std::ios::ate));
  }

  for (auto const &itm : input_files) {
    if (!itm.second.is_open()) {
      std::cout << std::format("Can't open input file for writing {}", itm.first.string()) << std::endl;
      exit(1);
    }
  }

  if (!output_file.is_open()) {
    std::cout << std::format("Can't open output file for writing {}", rib_file.string()) << std::endl;
    exit(1);
  }

  std::cout << std::format("Encoding {} to {} ... ", in_file.string(), rib_file.string());

  size_t input_size = (size_t)input_files.front().second.tellg() - sizeof(wav_hdr);
  size_t interleave_size_decoded = m_nb_chunks_in_interleave * m_nb_channels * m_nb_chunk_decoded * sizeof(int16_t);
  size_t nb_interleaves = std::ceil((float)input_size / (float)(interleave_size_decoded)) * m_count_files;

  for (auto &itm : input_files) {
    itm.second.seekg(sizeof(wav_hdr), std::ios::beg);
  }

  std::vector<std::vector<std::shared_ptr<ADPCMChannelStatus>>> channel_status(
      m_count_files,
      std::vector<std::shared_ptr<ADPCMChannelStatus>>(m_nb_channels)
      );

  for (int i = 0; i < m_count_files; i++) {
    for (int ch = 0; ch < m_nb_channels; ch++) {
      channel_status.at(i).at(ch) = std::make_shared<ADPCMChannelStatus>();
    }
  }

  for (int i = 0; i < nb_interleaves; i++) {
    std::vector<std::shared_ptr<std::vector<int8_t>>> outputs(m_nb_channels);

    for (int ch = 0; ch < m_nb_channels; ch++) {
      outputs.at(ch) = std::make_shared<std::vector<int8_t>>();
    }

    for (int k = 0; k < m_nb_chunks_in_interleave; k++) {
      std::vector<std::shared_ptr<std::vector<int16_t>>> inputs(m_nb_channels);
      for (int ch = 0; ch < m_nb_channels; ch++) {
        inputs.at(ch) = std::make_shared<std::vector<int16_t>>();
      }

      for (int j = 0; j < m_nb_chunk_decoded; j++) {
        for (int ch = 0; ch < m_nb_channels; ch++) {
          int16_t r;
          input_files.at(i % m_count_files).second.read(reinterpret_cast<char *>(&r), 2);
          inputs.at(ch)->push_back(UTILS::convert_le(r));
        }
      }

      for (int ch = 0; ch < m_nb_channels; ch++) {
        adpcm_rib_encode_frame(channel_status.at(i % m_count_files).at(ch), inputs.at(ch), outputs.at(ch));
      }
    }

    for (int ch = 0; ch < m_nb_channels; ch++) {
      output_file.write(reinterpret_cast<char *>(outputs.at(ch)->data()), outputs.at(ch)->size());
    }
  }

  output_file.close();
  std::cout << "done!" << std::endl;
}
