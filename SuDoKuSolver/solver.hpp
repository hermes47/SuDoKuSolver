//
//  solver.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#ifndef SUDOKUSOLVER_SOLVER_HPP
#define SUDOKUSOLVER_SOLVER_HPP

#include <cstdint>

#include "grid_old.hpp"

template <dimension_t Height, dimension_t Width>
bool SolveGrid(_SudokuGrid<Height,Width>& grid, bool quiet, size_t max_solutions) {
  static const values_t num_vals = Height * Width;
  static const work_t num_cells = num_vals * num_vals;
  typedef _SudokuCell<num_vals> Cell;
  typedef std::array<values_t, num_cells> GridState;
  typedef std::bitset<num_vals> Possibles;
  
  GridState current;
  grid.GetState(current);
  
  std::vector<GridState> to_run;
  to_run.push_back(current);
  size_t count = 0, solutions = 0;
  while (to_run.size()) {
    GridState state = to_run.back();
    to_run.pop_back();
    grid.SetState(state);
    ++count;
    
    // Check validity and if solved
    if (!grid.IsValidState()) continue;
    if (grid.IsSolved()) {
      ++solutions;
      grid._solved = GridState(state);
      continue;
    }
    if (solutions >= max_solutions) break;
    
    // Find smallest optioned cell
    values_t smallCell = num_cells;
    for (values_t cell = 0; cell < grid._cells.size(); ++cell) {
      if (grid._cells[cell].GetValue()) continue;
      if (smallCell == num_cells) smallCell = cell;
      if (grid._cells[cell].NumOptions() == 1) {  // Never gonna get smaller options
        smallCell = cell;
        break;
      } else if (grid._cells[cell].NumOptions() < grid._cells[smallCell].NumOptions()) smallCell = cell;
    }
    
    // Branch on all the options
    Possibles options = grid._cells[smallCell].GetPossibleOptions();
    for (values_t i = 0; i < options.size(); ++i) {
      if (!options[i]) continue;
      GridState newState;
      std::copy(state.begin(), state.end(), newState.begin());
      newState.at(smallCell) = i + 1;
      to_run.push_back(newState);
    }
  }
  
  if (!quiet) {
    if (!solutions) std::cout << "No valid solution found (" << count << " iterations)." << std::endl;
    else {
      grid.SetState(grid._solved);
      std::cout << "Solution found (" << count << " iterations) : " << std::endl;
//      grid.DisplayGrid();
    }
  }
  grid.SetState(current);
  grid._num_solutions = solutions;
  return solutions;
}

#endif /* SUDOKUSOLVER_SOLVER_HPP */
