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
//namespace std_x {
//  template <UINT _Size>
//  class bitset : public std::bitset<_Size> {
//    using std::bitset<_Size>::bitset;
//  public:
//    inline bool at(UINT pos) const { return std::bitset<_Size>::test(pos); }
//  };
//}

// Functions to find set bits within a bitset
//  GCC has methods to do
template <UINT N>
inline UINT __find_first(const std::bitset<N>& bs) {
  // finds index of first set bit. Returns N if no bits set.
  if (bs.none()) return N;
  UINT idx = 0;
  while (!bs[idx]) ++idx;
  return idx;
}

template <UINT N>
inline UINT __find_next(const std::bitset<N>& bs, const UINT pos) {
  // finds index of next set bit from pos. Returns N if no bits set.
  UINT idx = pos + 1;
  while (idx < N && !bs[idx]) ++idx;
  return idx;
}

// Function to determine the row, column and block of a given index
// of a regular sudoku
template<UINT H, UINT W, UINT N>
inline std_x::triple<UINT, UINT, UINT> GetCellGroups(UINT i) {
  static_assert(N == H * W * W * H, "With non-regular sudokus, need a different method to obtain cell groups");
  UINT c = i % (H * W);
  UINT R = i / (H * W * H);
  UINT C = c / W;
  return std_x::make_triple(i / (H * W), c, R * H + C);
}


#endif /* SUDOKUSOLVER_UTILITY_HPP */
