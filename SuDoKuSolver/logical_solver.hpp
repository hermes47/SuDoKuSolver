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

template <values_t num_vals>
values_t DetermineValue(std::bitset<num_vals>& options) {
  values_t val = 0;
  while (options.any()) {
    ++val;
    options >>= 1;
  }
  return val;
}

template <dimension_t Height, dimension_t Width>
void DetermineAffectedCells(work_t i, values_t val, SudokuGrid<Height,Width>& g, Actions *actions) {
  static const values_t num_vals = Height * Width;
  static const work_t num_cells = num_vals * num_vals;
  SudokuCell<num_vals> &cell = g.GetCell(i);
  values_t r = cell.GetRow(), c = cell.GetColumn(), b = cell.GetBlock();
  std::bitset<num_cells> affected = g.GetRow(r) | g.GetColumn(c) | g.GetBlock(b);
  for (values_t j = 0; j < num_cells; ++j) {
    if (affected[j]) actions->emplace_back(CLEAR_VALUE, val, j, current_group);
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
      std::bitset<num_vals> options;
      cell.GetPossibleOptions(options);
      values_t val = DetermineValue<num_vals>(options);
      std::cout << "[ONLY ONE] r" << cell.GetRow() << "c" << cell.GetColumn()
                << " must be " << val << std::endl;
      DetermineAffectedCells(idx, val, grid, actions);
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
    for (work_t cell_idx = 0; cell_idx < num_cells; ++cell_idx) {
      SudokuCell<num_vals> &cell = grid.GetCell(cell_idx);
      row_idxs[cell.GetRow()].push_back(cell_idx);
      col_idxs[cell.GetColumn()].push_back(cell_idx);
      blk_idxs[cell.GetBlock()].push_back(cell_idx);
    }
    init_idxs = true;
  }
  
  // map cell to (val,bitwise (row,col,blk))
  std::map<work_t, std::pair<values_t, std::bitset<3>>> reasons;
  bool added = false;
  std::array<SudokuCell<num_vals>, num_vals> all_cells;
  static std::bitset<3> row_k(1);
  static std::bitset<3> col_k(2);
  static std::bitset<3> blk_k(4);
  
  // Check rows
  for (std::vector<values_t>& row : row_idxs) {
    for (values_t i = 0; i < row.size(); ++i) {
      SudokuCell<num_vals> &cell = grid.GetCell(row[i]);
      std::bitset<num_vals> options;
      cell.GetPossibleOptions(options);
      for (values_t j = 0; j < row.size(); ++j) {
        if (i == j) continue;
        std::bitset<num_vals> tmp;
        grid.GetCell(row[j]).GetPossibleOptions(tmp);
        options &= (~tmp);
      }
      if (options.count() == 1) {
        values_t val = DetermineValue<num_vals>(options);
        auto v = std::make_pair(val, std::bitset<3>(row_k));
        reasons.emplace(row[i], v);
        DetermineAffectedCells(row[i], val, grid, actions);
      }
    }
  }
  
  // Check columns
  for (std::vector<values_t>& col : col_idxs) {
    for (values_t i = 0; i < col.size(); ++i) {
      SudokuCell<num_vals> &cell = grid.GetCell(col[i]);
      std::bitset<num_vals> options;
      cell.GetPossibleOptions(options);
      for (values_t j = 0; j < col.size(); ++j) {
        if (i == j) continue;
        std::bitset<num_vals> tmp;
        grid.GetCell(col[j]).GetPossibleOptions(tmp);
        options &= (~tmp);
      }
      if (options.count() == 1) {;
        if (reasons.find(col[i]) != reasons.end()) {
          reasons.at(col[i]).second |= col_k;
        } else {
          values_t val = DetermineValue<num_vals>(options);
          auto v = std::make_pair(val, std::bitset<3>(col_k));
          reasons.emplace(col[i], v);
          DetermineAffectedCells(col[i], val, grid, actions);
        }
      }
    }
  }
  
  // Check blocks
  for (std::vector<values_t>& blk : blk_idxs) {
    for (values_t i = 0; i < blk.size(); ++i) {
      SudokuCell<num_vals> &cell = grid.GetCell(blk[i]);
      std::bitset<num_vals> options;
      cell.GetPossibleOptions(options);
      for (values_t j = 0; j < blk.size(); ++j) {
        if (i == j) continue;
        std::bitset<num_vals> tmp;
        grid.GetCell(blk[j]).GetPossibleOptions(tmp);
        options &= (~tmp);
      }
      if (options.count() == 1) {
        if (reasons.find(blk[i]) != reasons.end()) {
          reasons.at(blk[i]).second |= blk_k;
        } else {
          values_t val = DetermineValue<num_vals>(options);
          auto v = std::make_pair(val, std::bitset<3>(blk_k));
          reasons.emplace(blk[i], v);
          DetermineAffectedCells(blk[i], val, grid, actions);
        }
      }
    }
  }
  
  // Prepare set actions
  for (auto kv = reasons.begin(); kv != reasons.end(); ++kv) {
    added = true;
    actions->emplace_back(SET_VALUE, kv->second.first, kv->first, current_group);
    values_t row, col, blk;
    grid.GetCellGroups(kv->first, row, col, blk);
    std::cout << "[UNIQUE] r" << row << "c" << col << " must be " << kv->second.first << " as it is the"
    << " only such instance in its";
    bool addand = false;
    if (kv->second.second[0]) {
      addand = true;
      std::cout << " Row";
    }
    if (kv->second.second[1]) {
      if (addand) std::cout << " and";
      addand = true;
      std::cout << " Column";
    }
    if (kv->second.second[2]) {
      if (addand) std::cout << " and";
      std::cout << " Block";
    }
    std::cout << "." << std::endl;
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
