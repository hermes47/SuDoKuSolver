//
//  utility.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 20/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#ifndef SUDOKUSOLVER_UTILITY_HPP
#define SUDOKUSOLVER_UTILITY_HPP

#include "defines.hpp"

#include <bitset>

// Functions to find set bits within a bitset
//  GCC has methods to do
template <val_t N>
inline val_t __find_first(const std::bitset<N>& bs) {
  // finds index of first set bit. Returns N if no bits set.
  if (bs.none()) return N;
  val_t idx = 0;
  while (!bs[idx]) ++idx;
  return idx;
}

template <val_t N>
inline val_t __find_next(const std::bitset<N>& bs, const size_t pos) {
  // finds index of next set bit from pos. Returns N if no bits set.
  val_t idx = pos + 1;
  while (idx < N && !bs[idx]) ++idx;
  return idx;
}

#endif /* SUDOKUSOLVER_UTILITY_HPP */
