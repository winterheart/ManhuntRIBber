/* SPDX-FileCopyrightText: Copyright 2025 Azamat H. Hackimov <azamat.hackimov@gmail.com> */
/* SPDX-License-Identifier: LGPL-2.1-or-later */

#include <filesystem>
#include <fstream>
#include <vector>
#include <gtest/gtest.h>

#include "codec.h"

const std::filesystem::path orig_rib_1c_44100 = "gs-16b-1c-44100hz.rib";
const std::filesystem::path orig_wav_1c_44100 = "gs-16b-1c-44100hz.wav";
const std::filesystem::path orig_rib_2c_44100 = "gs-16b-2c-44100hz.rib";
const std::filesystem::path orig_wav_2c_44100 = "gs-16b-2c-44100hz.wav";
const std::filesystem::path orig_rib_2c_22050 = "gs-16b-2c-22050hz.rib";
const std::filesystem::path orig_wav_2c_22050 = "gs-16b-2c-22050hz.wav";

const std::vector<std::filesystem::path> orig_complex_wav = {
    "717007-16b-2c-22050.wav",
    "725249-16b-2c-22050.wav",
    "732830-16b-2c-22050.wav",
    "766093-16b-2c-22050.wav",
    "766837-16b-2c-22050.wav",
    "785344-16b-2c-22050.wav",
};
std::filesystem::path orig_complex_rib = "complex.rib";

bool compare_files(const std::filesystem::path &a, const std::filesystem::path &b) {
  std::ifstream fa(a, std::ios::binary | std::ios::ate);
  std::ifstream fb(b, std::ios::binary | std::ios::ate);

  if (fa.fail() || fb.fail()) {
    return false;
  }

  if (fa.tellg() != fb.tellg()) {
    return false;
  }

  fa.seekg(0, std::ios::beg);
  fb.seekg(0, std::ios::beg);

  return std::equal(std::istreambuf_iterator<char>(fa.rdbuf()), std::istreambuf_iterator<char>(),
                    std::istreambuf_iterator<char>(fb.rdbuf()));
}

TEST(MonoSimple44100, decode) {
  std::filesystem::path gene_wav_1c_44100 = std::filesystem::temp_directory_path() / orig_wav_1c_44100;

  Codec codec(true, 44100, 1);
  codec.decode(orig_rib_1c_44100, gene_wav_1c_44100);

  EXPECT_TRUE(compare_files(gene_wav_1c_44100, orig_wav_1c_44100));

  std::filesystem::remove(gene_wav_1c_44100);
}

TEST(MonoSimple44100, encode) {
  std::filesystem::path gene_rib_1c_44100 = std::filesystem::temp_directory_path() / orig_rib_1c_44100;

  Codec codec(true, 44100, 1);
  codec.encode({orig_wav_1c_44100}, gene_rib_1c_44100);

  EXPECT_TRUE(compare_files(gene_rib_1c_44100, orig_rib_1c_44100));

  std::filesystem::remove(gene_rib_1c_44100);
}

TEST(StereoSimple44100, decode) {
  std::filesystem::path gene_wav_2c_44100 = std::filesystem::temp_directory_path() / orig_wav_2c_44100;

  Codec codec(false, 44100, 1);
  codec.decode(orig_rib_2c_44100, gene_wav_2c_44100);

  EXPECT_TRUE(compare_files(gene_wav_2c_44100, orig_wav_2c_44100));

  std::filesystem::remove(gene_wav_2c_44100);
}

TEST(StereoSimple44100, encode) {
  std::filesystem::path gene_rib_2c_44100 = std::filesystem::temp_directory_path() / orig_rib_2c_44100;

  Codec codec(false, 44100, 1);
  codec.encode({orig_wav_2c_44100}, gene_rib_2c_44100);

  EXPECT_TRUE(compare_files(gene_rib_2c_44100, orig_rib_2c_44100));

  std::filesystem::remove(gene_rib_2c_44100);
}

TEST(StereoSimple22050, decode) {
  std::filesystem::path gene_wav_2c_22050 = std::filesystem::temp_directory_path() / orig_wav_2c_22050;

  Codec codec(false, 22050, 1);
  codec.decode(orig_rib_2c_22050, gene_wav_2c_22050);

  EXPECT_TRUE(compare_files(gene_wav_2c_22050, orig_wav_2c_22050));

  std::filesystem::remove(gene_wav_2c_22050);
}

TEST(StereoSimple22050, encode) {
  std::filesystem::path gene_rib_2c_22050 = std::filesystem::temp_directory_path() / orig_rib_2c_22050;

  Codec codec(false, 22050, 1);
  codec.encode({orig_wav_2c_22050}, gene_rib_2c_22050);

  EXPECT_TRUE(compare_files(gene_rib_2c_22050, orig_rib_2c_22050));

  std::filesystem::remove(gene_rib_2c_22050);
}

TEST(StereoComplex22050, decode) {
  std::vector<std::filesystem::path> gene_complex_wav = {
      std::filesystem::temp_directory_path() / "complex_0.wav",
      std::filesystem::temp_directory_path() / "complex_1.wav",
      std::filesystem::temp_directory_path() / "complex_2.wav",
      std::filesystem::temp_directory_path() / "complex_3.wav",
      std::filesystem::temp_directory_path() / "complex_4.wav",
      std::filesystem::temp_directory_path() / "complex_5.wav",
  };

  Codec codec(false, 22050, 6);
  codec.decode(orig_complex_rib, std::filesystem::temp_directory_path() / "complex.wav");
  for (int i = 0; i < 6; i++) {
    EXPECT_TRUE(compare_files(gene_complex_wav.at(i), orig_complex_wav.at(i)));

    std::filesystem::remove(gene_complex_wav.at(i));
  }
}

TEST(StereoComplex22050, encode) {
  std::filesystem::path gene_rib_complex = std::filesystem::temp_directory_path() / orig_complex_rib;

  Codec codec(false, 22050, 6);
  codec.encode(orig_complex_wav, gene_rib_complex);

  EXPECT_TRUE(compare_files(gene_rib_complex, orig_complex_rib));
  std::filesystem::remove(gene_rib_complex);
}
