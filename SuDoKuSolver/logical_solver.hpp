//
//  logical_solver.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#ifndef SUDOKUSOLVER_LOGICAL_SOLVER_HPP
#define SUDOKUSOLVER_LOGICAL_SOLVER_HPP

#include <iterator>
#include <map>
#include <sstream>
#include <tuple>
#include <vector>

#include "combinations.hpp"
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

template <dimension_t Height, dimension_t Width, typename iterator_t>
std::bitset<Height * Width> GetCombinedOptions(iterator_t begin, iterator_t end, SudokuGrid<Height,Width>& grid) {
  std::bitset<Height * Width> o;
  for (; begin != end; ++begin) {
    std::bitset<Height * Width> tmp;
    grid.GetCell(*begin).GetPossibleOptions(tmp);
    o |= tmp;
  }
  return o;
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
  std::stringstream ss;
  
  bool added = false;
  for (work_t idx = 0; idx < num_cells; ++idx) {
    SudokuCell<num_vals> &cell = grid.GetCell(idx);
    if (cell.NumOptions() == 1) {
      std::bitset<num_vals> options;
      cell.GetPossibleOptions(options);
      values_t val = DetermineValue<num_vals>(options);
      ss << "[ONLY ONE] r" << cell.GetRow() << "c" << cell.GetColumn();
      ss << " must be " << val << std::endl;
      DetermineAffectedCells(idx, val, grid, actions);
      actions->emplace_back(SET_VALUE, val, idx, current_group);
      added = true;
    }
  }
  if (added) std::cout << ss.str();
  return added;
}

template <dimension_t Height, dimension_t Width>
bool FindHiddenSingles(SudokuGrid<Height,Width>& grid, Actions *actions) {
  static const values_t num_vals = Height * Width;
  static const work_t num_cells = num_vals * num_vals;
  static std::array<std::vector<values_t>, 3 * num_vals> grp_idxs;
  static bool init_idxs = false;
  std::stringstream ss;
  
  if (!init_idxs) {
    for (work_t cell_idx = 0; cell_idx < num_cells; ++cell_idx) {
      SudokuCell<num_vals> &cell = grid.GetCell(cell_idx);
      grp_idxs[cell.GetRow()].push_back(cell_idx);
      grp_idxs[cell.GetColumn() + num_vals].push_back(cell_idx);
      grp_idxs[cell.GetBlock() + 2 * num_vals].push_back(cell_idx);
    }
    init_idxs = true;
  }
  
  // map cell to (val,bitwise (row,col,blk))
  std::map<work_t, std::pair<values_t, uint8_t>> reasons;
  bool added = false;
  
  // Check groups
  for (values_t i = 0; i < grp_idxs.size(); ++i) {
    std::vector<values_t>& grp = grp_idxs[i];
    for (values_t j = 0; j < grp.size(); ++j) {
      SudokuCell<num_vals> &cell = grid.GetCell(grp[j]);
      std::bitset<num_vals> options;
      cell.GetPossibleOptions(options);
      for (values_t k = 0; k < grp.size(); ++k) {
        if (j == k) continue;
        std::bitset<num_vals> tmp;
        grid.GetCell(grp[k]).GetPossibleOptions(tmp);
        options &= (~tmp);
      }
      if (options.count() == 1) {
        if (reasons.find(grp[j]) != reasons.end()) {
          if (i >= 2 * num_vals) reasons.at(grp[j]).second |= 4;
          else if (i >= num_vals) reasons.at(grp[j]).second |= 2;
          else reasons.at(grp[j]).second |= 1;
        } else {
          values_t val = DetermineValue<num_vals>(options);
          std::pair<values_t, uint8_t> v;
          if (i >= 2 * num_vals) v = std::make_pair(val, 4);
          else if (i >= num_vals) v = std::make_pair(val, 2);
          else v = std::make_pair(val, 1);
          reasons.emplace(grp[j], v);
          DetermineAffectedCells(grp[j], val, grid, actions);
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
    ss << "[UNIQUE] r" << row << "c" << col << " must be " << kv->second.first;
    ss << " as it is the only such instance in its";
    bool addand = false;
    if (kv->second.second & 1) {
      addand = true;
      ss << " Row";
    }
    if (kv->second.second & 2) {
      if (addand) ss << " and";
      addand = true;
      ss << " Column";
    }
    if (kv->second.second & 4) {
      if (addand) ss << " and";
      ss << " Block";
    }
    ss << "." << std::endl;
  }
  if (added) std::cout << ss.str();
  return added;
}

template <dimension_t Height, dimension_t Width>
bool FindNakedNuples(SudokuGrid<Height,Width>& grid, Actions *actions) {
  static const values_t num_vals = Height * Width;
  static const work_t num_cells = num_vals * num_vals;
  static const std::map<values_t, std::string> nuple_names {
    { 2, "PAIR" },
    { 3, "TRIPLE" },
    { 4, "QUAD" },
    { 5, "QUINT" },
    { 6, "HEXTUPLE" },
    { 7, "HEPTUPLE" },
    { 8, "OCTUPLE" },
    { 9, "NONUPLE" },
    {10, "DECUPLE" },
  };
  static std::array<std::vector<values_t>, 3 * num_vals> grp_idxs;
  static bool init_idxs = false;
  std::stringstream ss;
  
  if (!init_idxs) {
    for (work_t cell_idx = 0; cell_idx < num_cells; ++cell_idx) {
      SudokuCell<num_vals> &cell = grid.GetCell(cell_idx);
      grp_idxs[cell.GetRow()].push_back(cell_idx);
      grp_idxs[cell.GetColumn() + num_vals].push_back(cell_idx);
      grp_idxs[cell.GetBlock() + 2 * num_vals].push_back(cell_idx);
    }
    init_idxs = true;
  }
  
  bool added = false;
  // Begin with pairs and move up to num_vals / 2, but only if previous n-uple fails
  for (values_t nuple = 2; nuple <= num_vals / 2; ++nuple) {
    if (added) break;
    for (values_t idx = 0; idx < grp_idxs.size(); ++idx) {
      std::stringstream header;
      header << "[NAKED ";
      if (idx >= 2 * num_vals) header << "BLOCK ";
      else if (idx >= 1 * num_vals) header << "COLUMN ";
      else header << "ROW ";
      if (nuple_names.find(nuple) != nuple_names.end()) header << nuple_names.at(nuple);
      else header << nuple << "-uple";
      header << "] ";
      
      std::vector<values_t> &grp = grp_idxs[idx];
      std::vector<values_t> valids;
      for (values_t i : grp) {
        if (!grid.GetCell(i).GetValue()) valids.push_back(i);
      }
      auto combinations = MakeCombinations(valids.begin(), valids.end(), nuple);
      std::vector<values_t> combo, missing_combo;
      while (combinations(std::back_inserter(combo), std::back_inserter(missing_combo))) {
        std::bitset<num_vals> combo_options, negative_options;
        combo_options = GetCombinedOptions(combo.begin(), combo.end(), grid);
        if (combo_options.count() == nuple) {
          negative_options = GetCombinedOptions(missing_combo.begin(), missing_combo.end(), grid);
          std::bitset<num_vals> intersection = combo_options & negative_options;
          if (intersection.any()) {
            std::stringstream naked;
            for (values_t i : combo) {
              SudokuCell<num_vals> &cell = grid.GetCell(i);
              if (i != combo[0]) naked << ",";
              naked << "r" << cell.GetRow() << "c" << cell.GetColumn();
            }
            for (values_t i : missing_combo) {
              SudokuCell<num_vals> &cell = grid.GetCell(i);
              cell.GetPossibleOptions(intersection);
              intersection &= combo_options;
              if (!intersection.any()) continue;
              bool first_remove = false;
              for (values_t val = 0; val < intersection.size(); ++val) {
                if (!intersection[val]) continue;
                added = true;
                actions->emplace_back(CLEAR_VALUE, val + 1, i, current_group);
                if (!first_remove) {
                  ss << header.str() << naked.str() << " removes " << val + 1;
                  first_remove = true;
                } else ss << "," << val + 1;
              }
              ss << " from r" << cell.GetRow() << "c" << cell.GetColumn() << "." << std::endl;
            }
          }
        }
        combo.clear();
        missing_combo.clear();
      }
    }
  }
  if (added) std::cout << ss.str();
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
    std::cout << "Logical solver round " << current_group << std::endl;
    // Handle all actions from last round
    for (; current_action < actions.size(); ++current_action)
      HandleAction<Height, Width>(actions[current_action], grid);
    
    // Check if solved:
    if (grid.IsSolved()) break;
    // Run through solve order until one of them returns true
    
    // Check for single options
    if (FindNakedSingles(grid, &actions)) continue;
    if (FindHiddenSingles(grid, &actions)) continue;
    
    // Check for Naked n-uples
    if (FindNakedNuples(grid, &actions)) continue;
    
    
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
