/* SPDX-FileCopyrightText: Copyright 2024 Azamat H. Hackimov <azamat.hackimov@gmail.com> */
/* SPDX-License-Identifier: LGPL-2.1-or-later */

#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

/**
 * Class for code and decode ADPCM streams
 */
class Codec {
public:
  Codec(bool is_mono, uint32_t frequency, uint32_t count_files);
  void decode(const std::filesystem::path &rib_file, std::filesystem::path wav_file);
  void encode(std::vector<std::filesystem::path> in_files, std::filesystem::path rib_file);

private:
  /// Count of files in RIB. Mostly is 1, but for music files (M variant) is 6.
  uint32_t m_count_files;
  /// Interleave
  uint32_t m_interleave = 0x10000;
  /// Chunk size. Depends on frequency.
  uint32_t m_chunk_size;
  /// Number of chunks in interleave.
  uint32_t m_nb_chunks_in_interleave;
  /// Number of encoded chunks. Chunk size - 4.
  uint32_t m_nb_chunk_encoded;
  /// Number of decoded chunk. 2 * number of encoded chunks + 1.
  uint32_t m_nb_chunk_decoded;
  /// Number of channels. 1 - mono, 2 - stereo.
  uint32_t m_nb_channels;
  /// Frequency. 22050 or 44100.
  uint32_t m_frequency;
};
