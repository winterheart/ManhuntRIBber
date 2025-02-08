/* SPDX-FileCopyrightText: Copyright 2024-2025 Azamat H. Hackimov <azamat.hackimov@gmail.com> */
/* SPDX-License-Identifier: LGPL-2.1-or-later */
#pragma once

#include <bit>
#include <cstdint>
#include <cstdio>

namespace UTILS {

// std::byteswap from C++23
template <typename T> constexpr T byteswap(T n) {
  T m;
  for (size_t i = 0; i < sizeof(T); i++)
    reinterpret_cast<uint8_t *>(&m)[i] = reinterpret_cast<uint8_t *>(&n)[sizeof(T) - 1 - i];
  return m;
}

/**
 * Convert integer to/from BE order
 */
template <typename T> constexpr T convert_be(T val) {
  return std::endian::native == std::endian::big ? val : byteswap(val);
}

/**
 * Convert integer to/from LE order
 */
template <typename T> constexpr T convert_le(T val) {
  return std::endian::native == std::endian::little ? val : byteswap(val);
}

}
