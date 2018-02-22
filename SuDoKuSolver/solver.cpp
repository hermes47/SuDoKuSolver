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
  for (UINT i = 0; i < N; ++i) {
    if (!this->_grid.GetCell(i).IsFixed()) to_solve.set(i);
  }
  to_run.emplace_back(this->_initial, to_solve);
  
  UINT iters = 0;
  _count = 0;
  _max = 2;
  
  while (to_run.size()) {
    SolveState s = to_run.back();
    to_run.pop_back();
    ++iters;
    
    // Check all to be solved cells have potential bits set
    // At the same time, find the cell with the least amount of options
    UINT pos = __find_first(s.second), best = s.second.size();
    // Check if solved
    if (pos == s.second.size()) {
      ++_count;
      this->_solved = s.first;
      if (_count == _max) break;  // bail out
      continue;
    }
    
    bool valid = true;
    while (pos < s.second.size()) {
      if (_AT(s.first, pos).none()) {
        valid = false;
        break;
      }
      if (best == s.second.size()) best = pos;
      if (_AT(s.first, pos).count() < _AT(s.first, best).count()) best = pos;
      pos = __find_next(s.second, pos);
    }
    if (!valid) continue;
    
    
    
    // Branch on all options available to cell
    UINT val = __find_first(_AT(s.first, best));
    while (val < _AT(s.first, best).size()) {
      GridState newState;
      AllCells new_tosolve(s.second);
      new_tosolve.reset(best);
      std::copy(s.first.begin(), s.first.end(), newState.begin());
      _AT(newState, best).reset();
      _AT(newState, best).set(val);
      // Propagate the setting (should only affect cells to solve still)
      const AllCells& a = _AT(this->_affected, best);
      UINT a_pos = __find_first(a);
      while (a_pos < new_tosolve.size()) {
        if (new_tosolve[a_pos]) _AT(newState, a_pos).reset(val);
        a_pos = __find_next(a, a_pos);
      }
      
      to_run.emplace_back(newState, new_tosolve);
      val = __find_next(_AT(s.first, best), val);
    }
  }
  return _count == 1;
}

// Logical solver implementation
template <UINT H, UINT W, UINT N>
LogicalSolver<H,W,N>::LogicalSolver(SudokuGrid<H,W,N>& grid)
: ISudokuSolver<H, W, N>(grid)
{
  for (UINT i = 0; i < this->_groups.size(); ++i) {
    for (UINT j = this->_groups.size() - 1; j > i; --j) {
      AllCells intersect = _AT(this->_groups, i) & _AT(this->_groups, j);
      if (intersect.count() < 2) continue;
      _intersects.emplace_back(intersect, i, j);
    }
  }
}

template <UINT H, UINT W, UINT N>
bool LogicalSolver<H,W,N>::Solve() {
  _order.clear();
  _actions.clear();
  _solve_state = std::make_pair(GridState(this->_initial), AllCells(0));
  _action_next = 0;
  for (UINT i = 0; i < N; ++i) {
    if (!this->_grid.GetCell(i).IsFixed()) _solve_state.second.set(i);
  }

  while (true) {
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
    
    if (GroupIntersection()) continue;
    
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
    switch (action.first) {
      case Action::REMOVE:
        _AT(_solve_state.first, action.third).reset(action.second);
        break;
      case Action::COMPLETE:
        _solve_state.second.reset(action.second);
        break;
    }
//    auto gs = GetCellGroups<H,W,N>(action.second);
//    std::cout << "Remove " << action.first + 1 << " from r" << gs.first + 1 << "c" << gs.second + 1 << std::endl;
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
          if (remove != val) _actions.emplace_back(Action::REMOVE, remove, ga_idx);
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
              _actions.emplace_back(Action::REMOVE, remove, idx);
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
            _actions.emplace_back(Action::REMOVE, remove, idx);
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
bool LogicalSolver<H,W,N>::GroupIntersection() {
  for (Intersection& intersect : _intersects) {
    AllCells int_cells = intersect.first & _solve_state.second;
    AllCells a_cells = _AT(this->_groups, intersect.second) & ~intersect.first;
    a_cells &= _solve_state.second;
    AllCells b_cells = _AT(this->_groups, intersect.third) & ~intersect.first;
    b_cells &= _solve_state.second;
    
    if (int_cells.count() < 2) continue;
//    if(a_cells.count() < 2) continue;
//    if(b_cells.count() < 2) continue;
    // Values in the intersect region
    Values i_vals(0);
    UINT i_idx = __find_first(int_cells);
    while (i_idx < int_cells.size()) {
      i_vals |= _AT(_solve_state.first, i_idx);
      i_idx = __find_next(int_cells, i_idx);
    }
    // Values in group A excluding intersect region
    Values a_vals(0);
    UINT a_idx = __find_first(a_cells);
    while (a_idx < a_cells.size()) {
      a_vals |= _AT(_solve_state.first, a_idx);
      a_idx = __find_next(a_cells, a_idx);
    }
    // Values in group B excluding intersect region
    Values b_vals(0);
    UINT b_idx = __find_first(b_cells);
    while (b_idx < b_cells.size()) {
      b_vals |= _AT(_solve_state.first, b_idx);
      b_idx = __find_next(b_cells, b_idx);
    }
    
    Values i_not_a = i_vals & ~a_vals;
    Values i_not_b = i_vals & ~b_vals;
    
    // Remove i_not_a from b_cells
    UINT rval_b = __find_first(i_not_a);
    while (rval_b < i_not_a.size()) {
      UINT ridx_b = __find_first(b_cells);
      while (ridx_b < b_cells.size()) {
        if (_solve_state.first[ridx_b][rval_b])
          _actions.emplace_back(Action::REMOVE, rval_b, ridx_b);
        ridx_b = __find_next(b_cells, ridx_b);
      }
      rval_b = __find_next(i_not_a, rval_b);
    }
    
    
    // Remove i_not_b from a_cells
    UINT rval_a = __find_first(i_not_b);
    while (rval_a < i_not_b.size()) {
      UINT ridx_a = __find_first(a_cells);
      while (ridx_a < a_cells.size()) {
        if (_solve_state.first[ridx_a][rval_a])
          _actions.emplace_back(Action::REMOVE, rval_a, ridx_a);
        ridx_a = __find_next(a_cells, ridx_a);
      }
      rval_a = __find_next(i_not_b, rval_a);
    }
  
  }
  if (_actions.size() > _action_next) {
    _order.push_back(LogicOperation::INTERSECTION_REMOVAL);
    return true;
  }
  return false;
}

template <UINT H, UINT W, UINT N>
void LogicalSolver<H,W,N>::SetSingleValue(UINT val, UINT idx) {
  // Adds actions to remove val from all cells affected by idx
  UINT affect = __find_first<N>(_AT(this->_affected, idx));
  while (affect < N) {
    if (_solve_state.first[affect][val])
      _actions.emplace_back(Action::REMOVE, val, affect);
    affect = __find_next<N>(_AT(this->_affected, idx), affect);
  }
  // Complete the cell
  _actions.emplace_back(Action::COMPLETE, idx, idx);
}

// explicit init
#define GRID_SIZE(x,y,z)\
template class BruteForceSolver<x,y,z>;\
template class LogicalSolver<x,y,z>;\
template class ISudokuSolver<x,y,z>;

#include "gridsizes.itm"
#undef GRID_SIZE
