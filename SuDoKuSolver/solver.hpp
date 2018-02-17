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

#include "grid.hpp"

template <dimension_t Height, dimension_t Width>
bool SolveGrid(SudokuGrid<Height,Width>& grid, bool quiet) {
  static const values_t num_vals = Height * Width;
  static const work_t num_cells = num_vals * num_vals;
  typedef SudokuCell<num_vals> Cell;
  typedef std::array<Cell*, num_vals> Group;
  typedef std::array<values_t, num_cells> GridState;
  
  GridState current;
  grid.GetState(current);
  
  std::vector<GridState*> to_run;
  to_run.push_back(&current);
  size_t count = 0;
  bool solved = false;
  while (to_run.size()) {
    GridState state = *to_run.back();
    to_run.pop_back();
    grid.SetState(state);
    ++count;
    
    // Check validity and if solved
    if (!grid.IsValidState()) continue;
    if (grid.IsSolved() && solved) {
      if (!quiet) std::cerr << "Grid has multiple solutions." << std::endl;
      solved = false;
      break;
    } else if (grid.IsSolved()) {
      grid._solved = state;
      solved = true;
      continue;
    }
    
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
    std::bitset<num_vals> options;
    grid._cells[smallCell].GetPossibleOptions(options);
    for (values_t i = 0; i < options.size(); ++i) {
      if (!options[i]) continue;
      GridState *newState = new GridState();
      std::copy(state.begin(), state.end(), newState->begin());
      newState->at(smallCell) = i + 1;
      to_run.push_back(newState);
    }
    
  }
  
  if (!quiet) {
    if (!solved) std::cout << "No valid solution found (" << count << " iterations)." << std::endl;
    else {
      grid.SetState(grid._solved);
      std::cout << "Solution found (" << count << " iterations) : " << std::endl;
      grid.DisplayGrid();
    }
  }
  grid.SetState(current);
  return solved;
}

#endif /* SUDOKUSOLVER_SOLVER_HPP */
