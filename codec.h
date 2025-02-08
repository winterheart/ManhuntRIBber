/* SPDX-FileCopyrightText: Copyright 2024-2025 Azamat H. Hackimov <azamat.hackimov@gmail.com> */
/* SPDX-License-Identifier: LGPL-2.1-or-later */

#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include "byteswap.h"

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

/**
 * Class for code and decode ADPCM streams
 */
class Codec {
public:
  Codec(bool is_mono, uint32_t frequency, uint32_t count_files);
  void decode(const std::filesystem::path &rib_file, const std::filesystem::path& wav_file) const;
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
