//
//  logical_solver.cpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#include "logical_solver.hpp"

template <dimension_t H, dimension_t W>
LogicalSolver<H,W>::LogicalSolver(Grid grid)
: _g(grid), _group(0), _next_act(0) {
  // Determine maximum number of actions that could be taken
  size_t max_actions = 0;
  for (work_t idx = 0; idx < num_cells; ++idx) {
    Cell& c = _g.GetCell(idx);
    if (!c.GetValue()) max_actions += 1 + c.NumOptions();
  }
  _acts.reserve(max_actions);
  
  // Populate the _grps with indices
  for (work_t idx = 0; idx < num_cells; ++idx) {
    Cell* cell = &_g.GetCell(idx);
    _grps[cell->GetRow()].push_back(cell);
    _grps[cell->GetColumn() + num_vals].push_back(cell);
    _grps[cell->GetBlock() + 2 * num_vals].push_back(cell);
  }
}

template <dimension_t H, dimension_t W>
void LogicalSolver<H,W>::HandleActions() {
  for (; _next_act < _acts.size(); ++_next_act) {
    Actionable &action = _acts[_next_act];
    Cell &cell = _g.GetCell(std::get<2>(action));
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
}

template <dimension_t H, dimension_t W>
bool LogicalSolver<H,W>::PerformLogicalSolving(bool quiet) {
  _quiet = quiet;
  // Check that the grid can be solved
  if (!_g.IsSolvable()) {
    std::cerr << "Grid is unsolveable." << std::endl;
    return false;
  }
  
  // Run the solver
  while (true) {
    ++_group;
//    if (!_quiet) std::cout << "Logical solver round " << _group << std::endl;
    
    // Handle actions decided last round
    HandleActions();
    
    // Check if actions solved the grid
    if (_g.IsSolved()) break;
    
    // Performed specialised searching for naked and hidden singles
    if (NakedSingles()) continue;
    if (HiddenSingles()) continue;
    
    // Search for Naked and Hidden n-tuples for 2 <= n <= num_vals / 2
    bool success = false;
    for (values_t nuple = 2; nuple <= num_vals / 2; ++nuple) {
      if (NakedNuples(nuple)) success = true;
      if (!success && HiddenNuples(nuple)) success = true;
      if (success) break;
    }
    if (success) continue;
    
    // Logic exhausted
    if (!_quiet) std::cout << "Logical options have been exhausted." << std::endl;
    return false;
  }
  
  // Only reach here if the grid has been solved.
  if (!quiet) {
    std::cout << "Grid solved using " << _acts.size() << " logically derived actions." << std::endl;
    _g.DisplayGrid();
  }
  return true;
}

template <dimension_t H, dimension_t W>
bool LogicalSolver<H,W>::NakedSingles() {
  std::stringstream ss;
  
  bool added = false;
  for (work_t idx = 0; idx < num_cells; ++idx) {
    Cell &cell = _g.GetCell(idx);
    if (cell.NumOptions() == 1) {
      Possibles options = cell.GetPossibleOptions();
      values_t val = DetermineSingleValue(options);
      ss << "[NAKED SINGLE] " << val << " is the only option in ";
      ss << "r" << cell.GetRow() << "c" << cell.GetColumn() << "." << std::endl;
      DetermineSingleAffect(cell, val);
      _acts.emplace_back(SET_VALUE, val, idx, _group);
      added = true;
    }
  }
  if (added && !_quiet) std::cout << ss.str();
  return added;
}

template <dimension_t H, dimension_t W>
bool LogicalSolver<H,W>::HiddenSingles() {
  // map cell to (val,bitwise (row,col,blk))
  std::map<Cell*, std::pair<values_t, uint8_t>> reasons;
  bool added = false;
  
  // Check groups
  for (values_t i = 0; i < _grps.size(); ++i) {
    Group &grp = _grps[i];
    for (Cell* test_cell : grp) {
      Possibles options = test_cell->GetPossibleOptions();
      for (Cell* other_cell : grp) {
        if (test_cell == other_cell) continue;
        Possibles tmp = other_cell->GetPossibleOptions();
        options &= (~tmp);
      }
      if (options.count() == 1) {
        if (reasons.find(test_cell) != reasons.end()) {
          if (i >= 2 * num_vals) reasons.at(test_cell).second |= 4;
          else if (i >= num_vals) reasons.at(test_cell).second |= 2;
          else reasons.at(test_cell).second |= 1;
        } else {
          values_t val = DetermineSingleValue(options);
          std::pair<values_t, uint8_t> v;
          if (i >= 2 * num_vals) v = std::make_pair(val, 4);
          else if (i >= num_vals) v = std::make_pair(val, 2);
          else v = std::make_pair(val, 1);
          reasons.emplace(test_cell, v);
          DetermineSingleAffect(*test_cell, val);
        }
      }
    }
  }
  
  if (reasons.size()) added = true;
  // Prepare set actions
  std::stringstream ss;
  for (auto kv = reasons.begin(); kv != reasons.end(); ++kv) {
    Cell* cell = kv->first;
    values_t val = kv->second.first;
    uint8_t mask = kv->second.second;
    _acts.emplace_back(SET_VALUE, val, cell->GetIndex(), _group);
    ss << "[HIDDEN SINGLE] r" << cell->GetRow() << "c" << cell->GetColumn();
    ss << " is the only instance of " << val << " in ";
    
    switch (mask) {
      case 1:
        ss << "r" << cell->GetRow();
        break;
      case 2:
        ss << "c" << cell->GetColumn();
        break;
      case 3:
        ss << "r" << cell->GetRow() << " and c" << cell->GetColumn();
        break;
      case 4:
        ss << "b" << cell->GetBlock();
        break;
      case 5:
        ss << "r" << cell->GetRow() << " and b" << cell->GetBlock();
        break;
      case 6:
        ss << "c" << cell->GetColumn() << " and b" << cell->GetBlock();
        break;
      case 7:
        ss << "r" << cell->GetRow() << ", c" << cell->GetColumn() << " and b" << cell->GetBlock();;
        break;
      default:
        break;
    }
    ss << "." << std::endl;
  }
  if (added && !_quiet) std::cout << ss.str();
  return added;
}

template <dimension_t H, dimension_t W>
bool LogicalSolver<H,W>::NakedNuples(values_t nuple) {
  bool added = false;
  std::stringstream ss;
  for (values_t g_idx = 0; g_idx < _grps.size(); ++g_idx) {
    // Find all cells in group that don't have values set
    Group &grp = _grps[g_idx];
    Group valids;
    for (Cell* cell : grp) {
      if (!cell->GetValue()) valids.push_back(cell);
    }
    
    // Iterate over all possible nuple length combinations of valids
    auto c_gen = MakeCombinations(valids.begin(), valids.end(), nuple);
    Group c, c_;  // combination and complement combination
    while (c_gen(std::back_inserter(c), std::back_inserter(c_))) {
      Possibles coptions = DetermineCombinedOptions(c.begin(), c.end());
      // Is a naked nuple if # of coptions is equal to nuple
      if (coptions.count() == nuple) {
        Possibles c_options = DetermineCombinedOptions(c_.begin(), c_.end());
        Possibles intersection = coptions & c_options;
        // may not be any overlap between the naked nuple and rest of group
        if (intersection.any()) {
          for (Cell* cell : c_) {
            intersection = cell->GetPossibleOptions();
            intersection &= coptions;
            if (!intersection.any()) continue;
            bool first_remove = false;
            for (values_t val = 0; val < intersection.size(); ++val) {
              if (!intersection[val]) continue;
              added = true;
              _acts.emplace_back(CLEAR_VALUE, val + 1, cell->GetIndex(), _group);
              if (!first_remove) {
                GetNakedNupleHeader(cell->GetIndex(), nuple, ss);
                ss << " ";
                GetNupleCombination(c.begin(), c.end(), ss);
                ss << " removes " << val + 1;
                first_remove = true;
              } else ss << "," << val + 1;
            }
            ss << " from r" << cell->GetRow() << "c" << cell->GetColumn() << "." << std::endl;
          }
        }
      }
      c.clear();
      c_.clear();
    }
  }
  if (added && !_quiet) std::cout << ss.str();
  return added;
}

template <dimension_t H, dimension_t W>
bool LogicalSolver<H,W>::HiddenNuples(values_t nuple) {
  bool added = false;
  std::stringstream ss;
  
  for (values_t g_idx = 0; g_idx < _grps.size(); ++g_idx) {
    // Find all cells in group that don't have values set
    Group &grp = _grps[g_idx];
    Group valids;
    for (Cell* cell : grp) {
      if (!cell->GetValue()) valids.push_back(cell);
    }
    
    // Iterate over all possible nuple length combinations of valids
    auto c_gen = MakeCombinations(valids.begin(), valids.end(), nuple);
    Group c, c_;// combination and complement combination
    while (c_gen(std::back_inserter(c), std::back_inserter(c_))) {
      Possibles coptions = DetermineCombinedOptions(c.begin(), c.end());
      Possibles c_options = DetermineCombinedOptions(c_.begin(), c_.end());
      Possibles c_unique = coptions & (~c_options);
      // Is a hidden nuple if # of unique options is equal to nuple
      if (c_unique.count() == nuple) {
        Possibles to_remove = ~c_unique;
        // Remove excess options from combination
        for (Cell* cell : c) {
          Possibles intersection = cell->GetPossibleOptions();
          intersection &= to_remove;
          
          if (!intersection.any()) continue;
          bool first_remove = false;
          for (values_t val = 0; val < intersection.size(); ++val) {
            if (!intersection[val]) continue;
            added = true;
            _acts.emplace_back(CLEAR_VALUE, val + 1, cell->GetIndex(), _group);
            if (!first_remove) {
              GetHiddenNupleHeader(cell->GetIndex(), nuple, c_unique, ss);
              ss << " removes " << val + 1;
              first_remove = true;
            } else ss << "," << val + 1;
          }
          ss << " from r" << cell->GetRow() << "c" << cell->GetColumn() << "." << std::endl;
        }
      }
      c.clear();
      c_.clear();
    }
  }
  
  if (added && !_quiet) std::cout << ss.str();
  return added;
}

template <dimension_t H, dimension_t W>
values_t LogicalSolver<H,W>::DetermineSingleValue(Possibles& p) {
  values_t val = 0;
  while (p.any()) {
    ++val;
    p >>= 1;
  }
  return val;
}

template <dimension_t H, dimension_t W>
void LogicalSolver<H,W>::DetermineSingleAffect(Cell &cell, values_t val) {
  AllCells affected = _g.GetRow(cell.GetRow());
  affected |= _g.GetColumn(cell.GetColumn());
  affected |= _g.GetBlock(cell.GetBlock());
  for (work_t idx = 0; idx < affected.size(); ++idx) {
    if (affected[idx]) _acts.emplace_back(CLEAR_VALUE, val, idx, _group);
    affected.reset(idx);
    if (affected.none()) break;
  }
}

template <dimension_t H, dimension_t W>
template <typename I>
typename LogicalSolver<H, W>::Possibles LogicalSolver<H,W>::DetermineCombinedOptions(I begin, I end) {
  Possibles o;
  for (; begin != end; ++begin) {
    Possibles tmp = (*begin)->GetPossibleOptions();
    o |= tmp;
  }
  return o;
}

template <dimension_t H, dimension_t W>
void LogicalSolver<H,W>::GetNakedNupleHeader(values_t idx, values_t nuple, std::stringstream& ss) {
  ss << "[NAKED ";
  if (idx >= 2 * num_vals) ss << "BLOCK ";
  else if (idx >= 1 * num_vals) ss << "COLUMN ";
  else ss << "ROW ";
  if (nuple_names.find(nuple) != nuple_names.end()) ss << nuple_names.at(nuple);
  else ss << nuple << "-uple";
  ss << "]";
}

template <dimension_t H, dimension_t W>
void LogicalSolver<H,W>::GetHiddenNupleHeader(values_t idx, values_t nuple, Possibles& g, std::stringstream& ss) {
  ss << "[HIDDEN ";
  if (nuple_names.find(nuple) != nuple_names.end()) ss << nuple_names.at(nuple);
  else ss << nuple << "-uple";
  ss << "] (";
  bool first_out = false;
  for (values_t i = 0; i < g.size(); ++ i) {
    if (!g[i]) continue;
    if (!first_out) {
      first_out = true;
      ss << i + 1;
    } else ss << "," << i + 1;
  }
  ss << ") in ";
  if (idx >= 2 * num_vals) ss << "b" << _g.GetCell(idx).GetBlock();
  else if (idx >= 1 * num_vals) ss << "c" << _g.GetCell(idx).GetColumn();
  else ss << "r" << _g.GetCell(idx).GetRow();
}

template <dimension_t H, dimension_t W>
template <typename I>
void LogicalSolver<H,W>::GetNupleCombination(I begin, I end, std::stringstream &ss) {
  I start = begin;
  for (; begin != end; ++begin) {
    if (begin != start) ss << ",";
    ss << "r" << (*begin)->GetRow() << "c" << (*begin)->GetColumn();
  }
}

// Explicit instanation
#define GRID_SIZE(x,y)\
template class LogicalSolver<x,y>;

#include "gridsizes.itm"
#undef GRID_SIZE
