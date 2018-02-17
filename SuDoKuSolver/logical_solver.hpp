//
//  logical_solver.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#ifndef SUDOKUSOLVER_LOGICAL_SOLVER_HPP
#define SUDOKUSOLVER_LOGICAL_SOLVER_HPP

#include <map>
#include <tuple>
#include <vector>

#include "grid.hpp"

enum Action {
  SET_VALUE,
  CLEAR_VALUE,
  NUM_ACTIONS
};

typedef values_t Value;
typedef values_t CellIndex;
typedef size_t ActionGroup;
typedef std::tuple<Action, Value, CellIndex, ActionGroup> Actionable;
typedef std::vector<Actionable> Actions;

static ActionGroup current_group;

template <dimension_t Height, dimension_t Width>
void HandleAction(Actionable& action, SudokuGrid<Height,Width>& grid) {
  SudokuCell<Height * Width> &cell = grid.GetCell(std::get<2>(action));
  switch (std::get<0>(action)) {
    case SET_VALUE:
      cell.SetValue(std::get<1>(action));
      break;
      
    case CLEAR_VALUE:
      cell.ResetOption(std::get<1>(action));
      break;
      
    default:
      break;
  }
}

template <dimension_t Height, dimension_t Width>
bool FindNakedSingles(SudokuGrid<Height,Width>& grid, Actions *actions) {
  static const values_t num_vals = Height * Width;
  static const work_t num_cells = num_vals * num_vals;
  
  bool added = false;
  for (work_t idx = 0; idx < num_cells; ++idx) {
    SudokuCell<num_vals> &cell = grid.GetCell(idx);
    if (cell.NumOptions() == 1) {
      values_t val = 0;
      std::bitset<num_vals> options;
      cell.GetPossibleOptions(options);
      while (options.any()) {
        ++val;
        options >>= 1;
      }
      std::cout << "[ONLY ONE] r" << cell.GetRow() << "c" << cell.GetColumn()
                << " must be " << val << std::endl;
      values_t r = cell.GetRow(), c = cell.GetColumn(), b = cell.GetBlock();
      std::bitset<num_cells> affected = grid.GetRow(r) | grid.GetColumn(c) | grid.GetBlock(b);
      for (values_t j = 0; j < num_cells; ++j) {
        if (affected[j]) actions->emplace_back(CLEAR_VALUE, val, j, current_group);
      }
      actions->emplace_back(SET_VALUE, val, idx, current_group);
      added = true;
    }
  }
  return added;
}

template <dimension_t Height, dimension_t Width>
bool FindHiddenSingles(SudokuGrid<Height,Width>& grid, Actions *actions) {
  static const values_t num_vals = Height * Width;
  static const work_t num_cells = num_vals * num_vals;
  static std::array<std::vector<values_t>, num_vals> row_idxs, col_idxs, blk_idxs;
  static bool init_idxs = false;
  
  if (!init_idxs) {
    std::cout << "Running init_idxs" << std::endl;
    for (work_t cell_idx = 0; cell_idx < num_cells; ++cell_idx) {
      SudokuCell<num_vals> &cell = grid.GetCell(cell_idx);
      row_idxs[cell.GetRow()].push_back(cell_idx);
      col_idxs[cell.GetColumn()].push_back(cell_idx);
      blk_idxs[cell.GetBlock()].push_back(cell_idx);
    }
    init_idxs = true;
  }
  
  
  std::map<std::tuple<values_t, values_t, values_t>, std::bitset<3>> reasons;
  bool added = false;
  std::array<SudokuCell<num_vals>, num_vals> all_cells;
  
  for (values_t row = 0; row < num_vals; ++row) {
    
  }
  
  return added;
}

template <dimension_t Height, dimension_t Width>
bool LogicallySolveGrid(SudokuGrid<Height,Width>& grid) {
  static const values_t num_vals = Height * Width;
  static const work_t num_cells = num_vals * num_vals;
  typedef SudokuCell<num_vals> Cell;
  typedef std::array<Cell*, num_vals> Group;

  if (!grid.IsSolvable()) {
    std::cout << "Grid is unsolvable." << std::endl;
    return false;
  }
  
  size_t max_actions = 0;
  for (work_t idx = 0; idx < num_cells; ++idx) {
    Cell& c = grid.GetCell(idx);
    if (!c.GetValue()) max_actions += 1 + c.NumOptions();
  }
  Actions actions;
  actions.reserve(max_actions);
  current_group = 0;
  size_t current_action = 0;
  
  while (true) {
    ++current_group;
    std::cout << "Solver round " << current_group << std::endl;
    // Handle all actions from last round
    for (; current_action < actions.size(); ++current_action)
      HandleAction<Height, Width>(actions[current_action], grid);
    
    // Check if solved:
    if (grid.IsSolved()) break;
    // Run through solve order until one of them returns true
    
    // Check for single options
    if (FindNakedSingles(grid, &actions)) continue;
    if (FindHiddenSingles(grid, &actions)) continue;
    
    
    // If we reach the end of all logical methods, then cannot currently logically solve
    std::cout << "No further logical operations can be performed." << std::endl;
    return false;
  }
  
  std::cout << "Yay! Grid has been solved using logical operations." << std::endl;
  grid.DisplayGrid();
  
  return true;
}

#include "logical_solver.cpp"
#endif /* SUDOKUSOLVER_LOGICAL_SOLVER_HPP */
