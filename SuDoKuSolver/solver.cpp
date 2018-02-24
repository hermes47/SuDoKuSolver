//
//  solver_new.cpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 21/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#include "defines.hpp"
#include <iostream>
#include <list>
#include <sstream>
#include <vector>

#include "spdlog/spdlog.h"

#include "combinations.hpp"
#include "grid.hpp"
#include "solver.hpp"
#include "utility.hpp"

// ISudokuSolver constructor
template <UINT H, UINT W, UINT N>
ISudokuSolver<H,W,N>::ISudokuSolver(SudokuGrid<H,W,N>& grid)
: _grid(grid), _initial(grid.GetInitialState()), _solved(grid.GetInitialState()),
_groups(grid.GetAllGroups()), _affected(grid.GetAllAffected())
{
  _log = spdlog::get("logger");
}

// BruteForce Solver implementation
template <UINT H, UINT W, UINT N>
bool BruteForceSolver<H,W,N>::Solve() {
  // Pair of state and what cells need to be solved in that state
  typedef std_x::triple<GridState, AllCells, UINT> SolveState;
  UINT iters = 0;
  _count = 0;
  _max = 2;
  _score = 0;
  
  std::vector<SolveState> to_run;
  AllCells to_solve(0);
  for (UINT i = 0; i < N; ++i) {
    if (!this->_grid.GetCell(i).IsFixed()) {
      to_solve.set(i);
      ++_score;
    }
  }
  to_run.emplace_back(this->_initial, to_solve, 0);
  
  while (to_run.size()) {
    SolveState s = to_run.back();
    to_run.pop_back();
    ++iters;
    
    // Check all to be solved cells have potential bits set
    // At the same time, find the cell with the least amount of options
    UINT best_cell = s.second.size(), cell_count = _AT(s.first,0).size();
    bool valid = true;
    FORBITSIN(pos, s.second) {
      if (_AT(s.first, pos).none()) {
        valid = false;
        break;
      }
      if (best_cell == s.second.size()) best_cell = pos;
      if (_AT(s.first, pos).count() < cell_count) {
        best_cell = pos;
        cell_count = _AT(s.first, pos).count();
      }
    }
    if (!valid) continue;
    // Check if solved
    if (best_cell == s.second.size()) {
      ++_count;
      this->_solved = s.first;
      _score += 100 * s.third;
      if (_count == _max) break;  // bail out
      continue;
    }
    
    // Search for hidden sets smaller than current best.
    UINT best_group = this->_groups.size(), group_count = _AT(s.first,0).size();
    UINT group_val = _AT(s.first,0).size();
    if (cell_count > 1) {
      // Only perform the search if we're likely to exceed what's present
      for (UINT g = 0; g < this->_groups.size(); ++g) {
        AllCells group = _AT(this->_groups, g) & s.second;  // Only cells to solve
        if (group.none()) continue;
        // Get the counts of values can place in group
        std::array<UINT, _AT(s.first,0).size()> counts = {0};
        FORBITSIN(g_idx, group) {
          FORBITSIN(i_val, _AT(s.first, g_idx)) ++counts[i_val];
        }
        
        // If a val count is less than current group_count, change it
        for (UINT i = 0; i < counts.size(); ++i) {
          if (_AT(counts, i) < group_count && _AT(counts, i) != 0) {
            group_count = _AT(counts, i);
            group_val = i;
            best_group = g;
          }
        }
        if (group_count == 1) break;
      }
    }
    
    if (cell_count <= group_count) {
      // Branch on all options available to cell
      FORBITSIN(val, _AT(s.first, best_cell)) {
        GridState newState;
        AllCells new_tosolve(s.second);
        UINT branch = s.third;
        if (cell_count > 1) ++branch;
        new_tosolve.reset(best_cell);
        std::copy(s.first.begin(), s.first.end(), newState.begin());
        _AT(newState, best_cell).reset();
        _AT(newState, best_cell).set(val);
        // Propagate the setting (should only affect cells to solve still)
        const AllCells& a = _AT(this->_affected, best_cell);
        FORBITSIN(a_pos, a) {
          if (new_tosolve[a_pos]) _AT(newState, a_pos).reset(val);
        }
        to_run.emplace_back(newState, new_tosolve, branch);
      }
    } else {
      // Branch on group_val in all posible places in group
      AllCells group = _AT(this->_groups, best_group) & s.second;  // Only cells to solve
      FORBITSIN(cell, group) {
        if (!_AT(s.first, cell)[group_val]) continue;
        GridState newState;
        AllCells new_tosolve(s.second);
        UINT branch = s.third;
        if (group_count > 1) ++branch;
        new_tosolve.reset(cell);
        std::copy(s.first.begin(), s.first.end(), newState.begin());
        _AT(newState, cell).reset();
        _AT(newState, cell).set(group_val);
        // Propagate the setting (should only affect cells to solve still)
        const AllCells& a = _AT(this->_affected, cell);
        FORBITSIN(a_pos, a) {
          if (new_tosolve[a_pos]) _AT(newState, a_pos).reset(group_val);
        }
        to_run.emplace_back(newState, new_tosolve, branch);
      }
    }
  }
  return _count == 1;
}

// Logical solver implementation
template <UINT H, UINT W, UINT N>
LogicalSolver<H,W,N>::LogicalSolver(SudokuGrid<H,W,N>& grid)
: ISudokuSolver<H, W, N>(grid)
{
  // Build the table of intersects
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
//    this->_grid.SetState(_solve_state.first);
//    std::cout << this->_grid << std::endl;
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
    if (BugRemoval()) continue;
    if (PatternOverlay()) continue;
    
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
  FORBITSIN(idx, _solve_state.second) {
    if (_AT(_solve_state.first, idx).count() == 1) {
      UINT val = __find_first(_AT(_solve_state.first, idx));
      SetSingleValue(val, idx);
    }
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
    FORBITSIN(ga_idx, group) {
      Values options = _AT(_solve_state.first, ga_idx);
      FORBITSIN(gb_idx, group) {
        if (gb_idx != ga_idx) options &= (~_AT(_solve_state.first, gb_idx));
      }
      
      if (options.count() == 1) {
        UINT val = __find_first(options);
        FORBITSIN(remove, _AT(_solve_state.first, ga_idx)) {
          if (remove != val) _actions.emplace_back(Action::REMOVE, remove, ga_idx);
        }
      }
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
    FORBITSIN(g_idx, group) {
      if (_solve_state.second[g_idx]) valids.emplace_back(g_idx);
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
            FORBITSIN(remove, intersect)
              _actions.emplace_back(Action::REMOVE, remove, idx);
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
    FORBITSIN(g_idx, group) {
      if (_solve_state.second[g_idx]) valids.emplace_back(g_idx);
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
          FORBITSIN(remove, intersect)
            _actions.emplace_back(Action::REMOVE, remove, idx);
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
    FORBITSIN(i_idx, int_cells)
      i_vals |= _AT(_solve_state.first, i_idx);

    // Values in group A excluding intersect region
    Values a_vals(0);
    FORBITSIN(a_idx, a_cells)
      a_vals |= _AT(_solve_state.first, a_idx);
    
    // Values in group B excluding intersect region
    Values b_vals(0);
    FORBITSIN(b_idx, b_cells)
      b_vals |= _AT(_solve_state.first, b_idx);
    
    Values i_not_a = i_vals & ~a_vals;
    Values i_not_b = i_vals & ~b_vals;
    
    // Remove i_not_a from b_cells
    FORBITSIN(rval_b, i_not_a) {
      FORBITSIN(ridx_b, b_cells) {
        if (_solve_state.first[ridx_b][rval_b])
          _actions.emplace_back(Action::REMOVE, rval_b, ridx_b);
      }
    }
    
    
    // Remove i_not_b from a_cells
    FORBITSIN(rval_a, i_not_b) {
      FORBITSIN(ridx_a, a_cells) {
        if (_solve_state.first[ridx_a][rval_a])
          _actions.emplace_back(Action::REMOVE, rval_a, ridx_a);
      }
    }
  
  }
  if (_actions.size() > _action_next) {
    _order.push_back(LogicOperation::INTERSECTION_REMOVAL);
    return true;
  }
  return false;
}

template <UINT H, UINT W, UINT N>
bool LogicalSolver<H,W,N>::PatternOverlay() {
  std::array<AllCells, G> val_masks;
  for (UINT val = 0; val < G; ++val) {
    // Determine all cells that can contain val
    AllCells mask(0);
    for (UINT cell = 0; cell < _solve_state.first.size(); ++cell) {
      if (_AT(_solve_state.first, cell)[val]) mask.set(cell);
    }
    _AT(val_masks, val) = mask;
    
    // build the patterns the first time.
    if (_AT(_patterns, val).size() == 0) {
      // Generate all valid masks for val
      std::vector<std::pair<AllCells,UINT>> partials;
      partials.emplace_back(mask, 0);
      while (partials.size()) {
        std::pair<AllCells, UINT> current = partials.back();
        partials.pop_back();
        
        if (current.second == G) {
          _AT(_patterns, val).emplace_back(current.first);
          continue;
        }
        
        AllCells possibles = current.first & _AT(this->_groups, current.second);
        if (possibles.none()) continue;
        
        FORBITSIN(pos, possibles) {
          AllCells new_partial(current.first);
          FORBITSIN(remove, _AT(this->_affected, pos)) new_partial.reset(remove);
          partials.emplace_back(new_partial, current.second + 1);
        }
      }
    } else if (_AT(_patterns, val).size() != 1){
      // Filter existing masks if they no longer match
      _AT(_patterns, val).remove_if([&mask](AllCells& pattern){
        return (pattern & mask).count() != G;
      });
    }
  }
  
  // RULE 1: if no patterns use a particular cell, can remove val from that cell
  for (UINT val = 0; val < G; ++val) {
    AllCells used(0);
    for (AllCells& pattern : _AT(_patterns, val)) used |= pattern;
    AllCells not_used = _AT(val_masks, val) & ~used;
    if (not_used.none()) continue;
    FORBITSIN(cell, not_used) _actions.emplace_back(Action::REMOVE, val, cell);
  }
  
  if (_actions.size() > _action_next) {
    _order.push_back(LogicOperation::PATTERN_OVERLAY);
    return true;
  }
  
  // RULE 2:
  // Determine all the subsets of cells which cover all patterns for each val
  std::array<std::vector<AllCells>, G> coverage;
  for (UINT val = 0; val < G; ++val) {
    std::vector<std::pair<AllCells, UINT>> partials;
    partials.emplace_back(_AT(val_masks, val) & _solve_state.second, 0);
    
    while (partials.size()) {
      std::pair<AllCells, UINT> current = partials.back();
      partials.pop_back();
      if (current.first.none()) continue;
      
      if (current.second == G) {
        bool all_coverage = true;
        //        if (current.first.count() != 2) continue;
        for (AllCells& pattern : _AT(_patterns, val)) {
          if ((pattern & current.first).none()) {
            all_coverage = false;
            break;
          }
        }
        if (all_coverage) _AT(coverage, val).emplace_back(current.first);
        continue;
      }
      
      AllCells group = current.first & _AT(this->_groups, current.second);
      partials.emplace_back(current.first & ~group, current.second + 1);
      FORBITSIN(cell, group)
      partials.emplace_back(current.first & ~_AT(this->_affected, cell),
                            current.second + 1);
    }
  }
  // Filter patterns if they contain values in all cells of a coverage mask
  // of a different value
  for (UINT coverage_idx = 0; coverage_idx < G; ++coverage_idx) {
    for (UINT pattern_idx = 0; pattern_idx < G; ++pattern_idx) {
      if (coverage_idx == pattern_idx) continue;
      for (AllCells& cover : _AT(coverage, coverage_idx)) {
        _AT(_patterns, pattern_idx).remove_if([&cover](AllCells& pattern){
          return (pattern & cover).count() == cover.count();
        });
      }
    }
  }
  // Look again at rule 1 with filtered patterns
  for (UINT val = 0; val < G; ++val) {
    AllCells used(0);
    for (AllCells& pattern : _AT(_patterns, val)) used |= pattern;
    AllCells not_used = _AT(val_masks, val) & ~used;
    if (not_used.none()) continue;
    FORBITSIN(cell, not_used) _actions.emplace_back(Action::REMOVE, val, cell);
  }
  if (_actions.size() > _action_next) {
    _order.push_back(LogicOperation::PATTERN_OVERLAY);
    return true;
  }
  return false;
}

template <UINT H, UINT W, UINT N>
bool LogicalSolver<H,W,N>::BugRemoval() {
  bool seen_three = false;
  UINT three_idx = 0;
  Values odds(0);
//  odds.set();
  
  FORBITSIN(idx, _solve_state.second) {
    UINT count = _AT(_solve_state.first, idx).count();
    if (count == 2) {
      FORBITSIN(val, _AT(_solve_state.first, idx)) odds.flip(val);
    } else if (count == 3 && !seen_three) {
      FORBITSIN(val, _AT(_solve_state.first, idx)) odds.flip(val);
      seen_three = true;
      three_idx = idx;
    }
    else return false;
  }
  if (!seen_three) return false;
  
  FORBITSIN(idx, _AT(_solve_state.first, three_idx)) {
    if (!odds[idx]) _actions.emplace_back(Action::REMOVE, idx, three_idx);
  }
  
  if (_actions.size() > _action_next) {
    _order.push_back(LogicOperation::BUG_REMOVAL);
    return true;
  }
  return false;
}

template <UINT H, UINT W, UINT N>
void LogicalSolver<H,W,N>::SetSingleValue(UINT val, UINT idx) {
  // Adds actions to remove val from all cells affected by idx
#ifdef DEBUG
  std::stringstream ss;
  bool done_first = false;
#endif
  FORBITSIN(affect, _AT(this->_affected, idx)) {
    if (_solve_state.first[affect][val]) {
      _actions.emplace_back(Action::REMOVE, val, affect);
#ifdef DEBUG
      auto rcb = GetCellGroups<H,W,N>(affect);
      if (!done_first) {
        ss << "r" << rcb.first+1 << "c" << rcb.second+1;
        done_first = true;
      } else ss << ", r" << rcb.first+1 << "c" << rcb.second+1;
#endif
    }
  }
  // Complete the cell
  _actions.emplace_back(Action::COMPLETE, idx, idx);
#ifdef DEBUG
  auto rcb = GetCellGroups<H,W,N>(idx);
  if (ss.str().size())
    this->_log->debug("[NAKED SINGLE] R{}C{} can only be {}. Removes {} from {}.",
                      rcb.first+1, rcb.second+1, val+1, val+1, ss.str());
  else
    this->_log->debug("[NAKED SINGLE] R{}C{} can only be {}. No removals made.",
                      rcb.first+1, rcb.second+1, val+1);
#endif
}

// explicit init
#define GRID_SIZE(x,y,z)\
template class BruteForceSolver<x,y,z>;\
template class LogicalSolver<x,y,z>;\
template class ISudokuSolver<x,y,z>;

#include "gridsizes.itm"
#undef GRID_SIZE
