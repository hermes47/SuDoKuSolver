//
//  solver_new.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 21/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#ifndef SUDOKUSOLVER_BRUTEFORCE_SOLVER_HPP
#define SUDOKUSOLVER_BRUTEFORCE_SOLVER_HPP

#include "defines.hpp"

#include <array>
#include <vector>

#include "utility.hpp"

enum class LogicOperation {
  NAKED_SINGLE,
  HIDDEN_SINGLE,
  NAKED_PAIR,
  HIDDEN_PAIR,
  NAKED_TRIPLE,
  HIDDEN_TRIPLE,
  NAKED_QUAD,
  HIDDEN_QUAD,
  NAKED_NUPLE,        // For larger grids
  HIDDEN_NUPLE,
  BRUTE_FORCE         // Last resort
};

template <INT H, INT W, INT N>
bool SolveGridNew(std::array<std_x::bitset<H * W>, N>& state,
                  const std::vector<std_x::bitset<N>>& groups,
                  const std::array<std_x::bitset<N>, N>& affected,
                  std::array<std_x::bitset<H * W>, N>& out_state,
                  UINT& num_solve, bool quiet = false, UINT solve_max = 500);

// Will get to a point where none of the logical operations will find anything to do
// Then can check from state with brute force method
template <INT H, INT W, INT N>
bool LogicalSolveGridNew(std::array<std_x::bitset<H * W>, N>& state,
                  const std::vector<std_x::bitset<N>>& groups,
                  const std::array<std_x::bitset<N>, N>& affected,
                  std::array<std_x::bitset<H * W>, N>& out_state,
                         std::vector<LogicOperation>& solution_order,
                  bool quiet = false);

#endif /* SUDOKUSOLVER_BRUTEFORCE_SOLVER_HPP */
