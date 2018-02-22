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

#include "combinations.hpp"
#include "grid.hpp"
#include "solver.hpp"
#include "utility.hpp"

// ISudokuSolver constructor
template <UINT H, UINT W, UINT N>
ISudokuSolver<H,W,N>::ISudokuSolver(SudokuGrid<H,W,N>& grid)
: _grid(grid), _initial(grid.GetInitialState()), _solved(grid.GetInitialState()),
_groups(grid.GetAllGroups()), _affected(grid.GetAllAffected())
{ }

// BruteForce Solver implementation
template <UINT H, UINT W, UINT N>
bool BruteForceSolver<H,W,N>::Solve() {
  // Pair of state and what cells need to be solved in that state
  typedef std::pair<GridState, AllCells> SolveState;
  
  std::vector<SolveState> to_run;
  AllCells to_solve(0);
  for (INT i = 0; i < N; ++i) {
    if (_AT(this->_initial, i).count() > 1) to_solve.set(i);
  }
  to_run.emplace_back(this->_initial, to_solve);
  
  UINT iters = 0;
  _count = 0;
  
  while (to_run.size()) {
    SolveState s = to_run.back();
    to_run.pop_back();
    ++iters;
    
    // Check all to be solved cells have potential bits set
    // At the same time, find the cell with the least amount of options
    UINT pos = __find_first<N>(s.second), best = N;
    // Check if solved
    if (pos == N) {
      ++_count;
      this->_solved = s.first;
      if (_count >= _max) break;  // bail out
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
      const AllCells& a = _AT(this->_affected, best);
      UINT a_pos = __find_first<N>(a);
      while (a_pos < N) {
        _AT(newState, a_pos).reset(val);
        a_pos = __find_next<N>(a, a_pos);
      }
      
      to_run.emplace_back(newState, new_tosolve);
      val = __find_next<H * W>(_AT(s.first, best), val);
    }
  }
  
  return _count == 1;
}

// Logical solver implementation
template <UINT H, UINT W, UINT N>
bool LogicalSolver<H,W,N>::Solve() {
  _order.clear();
  _actions.clear();
  _solve_state = std::make_pair(GridState(this->_initial), AllCells(0));
  _action_group = 0;
  _action_next = 0;
  for (UINT i = 0; i < N; ++i) {
    if (!this->_grid.GetCell(i).IsFixed()) _solve_state.second.set(i);
  }

  while (true) {
    ++_action_group;
    
    // Handle actions decided last round
    HandleActions();
    if (_solve_state.second.none()) break;
    
    // Search for singles
    if (NakedSingle()) continue;
    if (HiddenSingle()) continue;
    
    // Search for naked and hidden n-nuples where n <= G / 2
    bool success = false;
    for (UINT nuple = 2; nuple <= this->G / 2; ++nuple) {
      if (NakedNuple(nuple)) success = true;
      if (success) break;
    }
    if (success) continue;
    for (UINT nuple = 2; nuple <= this->G / 2; ++nuple) {
      if (HiddenNuple(nuple)) success = true;
      if (success) break;
    }
    if (success) continue;
    
    // Logic exhausted, so run brute force and exit if that fails
    if (BruteForce()) continue;
    break;
  }
  
  return _solve_state.second.none();
}

template <UINT H, UINT W, UINT N>
void LogicalSolver<H,W,N>::HandleActions() {
  while (_action_next < _actions.size()) {
    Actionable& action = _AT(_actions, _action_next);
    _AT(_solve_state.first, action.second).reset(action.first);
    ++_action_next;
  }
}

template <UINT H, UINT W, UINT N>
bool LogicalSolver<H,W,N>::NakedSingle() {
  UINT idx = __find_first<N>(_solve_state.second);
  while (idx < N) {
    if (_AT(_solve_state.first, idx).count() == 1) {
      UINT val = __find_first<H * W>(_AT(_solve_state.first, idx));
      SetSingleValue(val, idx);
      _solve_state.second.reset(idx);
    }
    idx = __find_next<N>(_solve_state.second, idx);
  }
  if (_actions.size() > _action_next) {
    _order.push_back(LogicOperation::NAKED_SINGLE);
    return true;
  }
  return false;
}

template <UINT H, UINT W, UINT N>
bool LogicalSolver<H,W,N>::HiddenSingle() {
  for (const AllCells& group : this->_groups) {
    UINT ga_idx = __find_first<N>(group);
    while (ga_idx < N) {
      Values options = _AT(_solve_state.first, ga_idx);
      UINT gb_idx = __find_first<N>(group);
      while (gb_idx < N) {
        if (gb_idx != ga_idx) options &= (~_AT(_solve_state.first, gb_idx));
        gb_idx = __find_next<N>(group, gb_idx);
      }
      
      if (options.count() == 1) {
        UINT val = __find_first<H * W>(options);
        UINT remove = __find_first<H * W>(_AT(_solve_state.first, ga_idx));
        while (remove < H*W) {
          if (remove != val) _actions.emplace_back(val, ga_idx, _action_group);
          remove = __find_next<H * W>(_AT(_solve_state.first, ga_idx), remove);
        }
      }
      ga_idx = __find_next<N>(group, ga_idx);
    }
  }
  
  if (_actions.size() > _action_next) {
    _order.push_back(LogicOperation::HIDDEN_SINGLE);
    return true;
  }
  return false;
}

template <UINT H, UINT W, UINT N>
bool LogicalSolver<H,W,N>::NakedNuple(UINT nuple) {
  for (const AllCells& group : this->_groups) {
    // Find all unsets (ie still to solve) in group
    std::vector<UINT> valids, combination, complement;
    UINT g_idx = __find_first<N>(group);
    while (g_idx < N) {
      if (_solve_state.second[g_idx]) valids.emplace_back(g_idx);
      g_idx = __find_next<N>(group, g_idx);
    }
    
    // Iterate over all nuple length combinations of valids
    auto generate = MakeCombinations(valids.begin(), valids.end(), nuple);
    while (generate(std::back_inserter(combination),
                    std::back_inserter(complement))) {
      Values combo_options, comple_options, intersect;
      for (UINT idx : combination) combo_options |= _AT(_solve_state.first,idx);
      
      // Check if naked nuple
      if (combo_options.count() == nuple) {
        for (UINT idx : complement) comple_options |= _AT(_solve_state.first,idx);
        intersect = comple_options & combo_options;
        
        // Make sure there is overlap between the nuple and rest of group
        if (intersect.any()) {
          // Add actions to remove the nuple values from the other cells
          for (UINT idx : complement) {
            intersect = _AT(_solve_state.first, idx) & combo_options;
            if (intersect.none()) continue;
            UINT remove = __find_first(intersect);
            while (remove < intersect.size()) {
              _actions.emplace_back(remove, idx, _action_group);
              remove = __find_next(intersect, remove);
            }
          }
        }
      }
      combination.clear();
      complement.clear();
    }
  }
  
  if (_actions.size() > _action_next) {
    if (nuple == 2) _order.push_back(LogicOperation::NAKED_PAIR);
    if (nuple == 3) _order.push_back(LogicOperation::NAKED_TRIPLE);
    if (nuple == 4) _order.push_back(LogicOperation::NAKED_QUAD);
    if (nuple > 4) _order.push_back(LogicOperation::NAKED_NUPLE);
    return true;
  }
  return false;
}

template <UINT H, UINT W, UINT N>
bool LogicalSolver<H,W,N>::HiddenNuple(UINT nuple) {
  for (const AllCells& group : this->_groups) {
    // Find all unsets (ie still to solve) in group
    std::vector<UINT> valids, combination, complement;
    UINT g_idx = __find_first<N>(group);
    while (g_idx < N) {
      if (_solve_state.second[g_idx]) valids.emplace_back(g_idx);
      g_idx = __find_next<N>(group, g_idx);
    }
    
    // Iterate over all nuple length combinations of valids
    auto generate = MakeCombinations(valids.begin(), valids.end(), nuple);
    while (generate(std::back_inserter(combination),
                    std::back_inserter(complement))) {
      
      Values combo_options, comple_options, unique;
      for (UINT idx : combination) combo_options |= _AT(_solve_state.first,idx);
      for (UINT idx : complement) comple_options |= _AT(_solve_state.first,idx);
      unique = combo_options & (~comple_options);
      
      // Check if hidden nuple
      if (unique.count() == nuple) {
        // Add actions to remove excess values from nuple cells
        for (UINT idx : combination) {
          Values intersect = _AT(_solve_state.first, idx) & (~unique);
          if (intersect.none()) continue;
          UINT remove = __find_first(intersect);
          while (remove < intersect.size()) {
            _actions.emplace_back(remove, idx, _action_group);
            remove = __find_next(intersect, remove);
          }
        }
      }
      complement.clear();
      combination.clear();
    }
  }
  
  if (_actions.size() > _action_next) {
    if (nuple == 2) _order.push_back(LogicOperation::HIDDEN_PAIR);
    if (nuple == 3) _order.push_back(LogicOperation::HIDDEN_TRIPLE);
    if (nuple == 4) _order.push_back(LogicOperation::HIDDEN_QUAD);
    if (nuple > 4) _order.push_back(LogicOperation::HIDDEN_NUPLE);
    return true;
  }
  return false;
}

template <UINT H, UINT W, UINT N>
bool LogicalSolver<H,W,N>::BruteForce() {
  _order.push_back(LogicOperation::BRUTE_FORCE);
  return false;
}

template <UINT H, UINT W, UINT N>
void LogicalSolver<H,W,N>::SetSingleValue(UINT val, UINT idx) {
  // Adds actions to remove val from all cells affected by idx
  UINT affect = __find_first<N>(_AT(this->_affected, idx));
  while (affect < N) {
    _actions.emplace_back(val, affect, _action_group);
    affect = __find_next<N>(_AT(this->_affected, idx), affect);
  }
}

// explicit init
#define GRID_SIZE(x,y,z)\
template class BruteForceSolver<x,y,z>;\
template class LogicalSolver<x,y,z>;\
template class ISudokuSolver<x,y,z>;

#include "gridsizes.itm"
#undef GRID_SIZE
