//
//  solver_new.cpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 21/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#include "defines.hpp"
#include <iostream>
#include <vector>

#include "solver_new.hpp"
#include "utility.hpp"


template <INT H, INT W, INT N>
bool SolveGridNew(std::array<std_x::bitset<H * W>, N>& state,
                  const std::vector<std_x::bitset<N>>& groups,
               const std::array<std_x::bitset<N>, N>& affected,
               std::array<std_x::bitset<H * W>, N>& out_state,
               UINT& num_solve, bool quiet, UINT solve_max) {
  typedef std_x::bitset<H * W> Values;
  typedef std_x::bitset<N> AllCells;
  typedef std::array<Values, N> GridState;
  typedef std::pair<GridState, AllCells> SolveState;
  std::vector<SolveState> to_run;
  AllCells to_solve(0);
  for (INT i = 0; i < N; ++i) {
    if (_AT(state, i).count() > 1) to_solve.set(i);
  }
  to_run.emplace_back(state, to_solve);
  
  UINT iters = 0;
  num_solve = 0;
  
  while (to_run.size()) {
    SolveState s = to_run.back();
    to_run.pop_back();
    ++iters;
    
    // Check all to be solved cells have potential bits set
    // At the same time, find the cell with the least amount of options
    UINT pos = __find_first<N>(s.second), best = N;
    // Check if solved
    if (pos == N) {
      ++num_solve;
      out_state = s.first;
      if (num_solve >= solve_max) break;  // bail out
      continue;
    }
    
    bool valid = true;
    while (pos < N) {
      if (_AT(s.first, pos).none()) {
        valid = false;
        break;
      }
      if (best == N) best = pos;
      if (_AT(s.first, pos).count() < _AT(s.first, best).count()) best = pos;
      pos = __find_next<N>(s.second, pos);
    }
    
    if (!valid) continue;
    
    // Branch on all options available to cell
    UINT val = __find_first<H * W>(_AT(s.first, best));
    to_solve = s.second;
    to_solve.reset(best);
    while (val < H * W) {
      GridState newState;
      AllCells new_tosolve(to_solve);
      std::copy(s.first.begin(), s.first.end(), newState.begin());
      _AT(newState, best).reset();
      _AT(newState, best).set(val);
      // Propagate the setting (should only affect cells to solve still)
      const AllCells& a = _AT(affected, best);
      UINT a_pos = __find_first<N>(a);
      while (a_pos < N) {
        _AT(newState, a_pos).reset(val);
        a_pos = __find_next<N>(a, a_pos);
      }
      
      to_run.emplace_back(newState, new_tosolve);
      val = __find_next<H * W>(_AT(s.first, best), val);
    }
    
  }
  if (!quiet)
    std::cout << "Found " << num_solve << " solutions in " << iters << " iterations." << std::endl;
  return false;
}

// explicit init
#define GRID_SIZE(x,y,z)\
template bool SolveGridNew<x,y,z>(std::array<std_x::bitset<x * y>, z>&,\
const std::vector<std_x::bitset<z>>&,\
const std::array<std_x::bitset<z>, z>&,\
std::array<std_x::bitset<x * y>, z>&,\
UINT&, bool, UINT);

GRID_SIZE(3,3,81);
GRID_SIZE(2,2,16);
//#include "gridsizes.itm"
#undef GRID_SIZE
