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

#include "triple.hpp"

// Wrapper class to give std::bitset an at() method, like other containers
namespace std_x {
  template <size_t _Size>
  class bitset : public std::bitset<_Size> {
    using std::bitset<_Size>::bitset;
  public:
    inline bool at(size_t pos) const { return std::bitset<_Size>::test(pos); }
  };
}

// Functions to find set bits within a bitset
//  GCC has methods to do
template <INT N>
inline INT __find_first(const std::bitset<N>& bs) {
  // finds index of first set bit. Returns N if no bits set.
  if (bs.none()) return N;
  INT idx = 0;
  while (!bs[idx]) ++idx;
  return idx;
}

template <INT N>
inline INT __find_next(const std::bitset<N>& bs, const UINT pos) {
  // finds index of next set bit from pos. Returns N if no bits set.
  INT idx = pos + 1;
  while (idx < N && !bs[idx]) ++idx;
  return idx;
}

// Function to determine the row, column and block of a given index
// of a regular sudoku
template<INT H, INT W, INT N>
inline std_x::triple<INT, INT, INT> GetCellGroups(INT i) {
  static_assert(N == H * W * W * H, "With non-regular sudokus, need a different method to obtain cell groups");
  INT c = i % (H * W);
  INT R = i / (H * W * H);
  INT C = c / W;
  return std_x::make_triple(i / (H * W), c, R * H + C);
}


#endif /* SUDOKUSOLVER_UTILITY_HPP */
