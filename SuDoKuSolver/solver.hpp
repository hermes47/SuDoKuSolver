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
#include <list>
#include <vector>

#include "spdlog/spdlog.h"

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
  INTERSECTION_REMOVAL,
  PATTERN_OVERLAY,
  BRUTE_FORCE,         // Last resort
  NUM_OPERATIONS
};

enum class Action {
  REMOVE,
  COMPLETE,
};

template<UINT H, UINT W, UINT N>
class SudokuGrid;

template <UINT H, UINT W, UINT N>
class ISudokuSolver {
protected:
  static const UINT G = H * W;
public:
  typedef BITSET(G) Values;
  typedef BITSET(N) AllCells;
  typedef std::array<Values, N> GridState;
  
protected:
  SudokuGrid<H,W,N>& _grid;
  const GridState _initial;
  GridState _solved;
  const std::vector<AllCells> _groups;
  const std::array<AllCells, N> _affected;
  bool _quiet;
  std::shared_ptr<spdlog::logger> _log;
  
private:
  ISudokuSolver() = default;
  
public:
  ISudokuSolver(SudokuGrid<H,W,N>&);
  virtual bool Solve() = 0;  // T/F if solved
  const GridState& GetSolvedState() { return _solved; }
};

template <UINT H, UINT W = H, UINT N = H * H * W * W>
class BruteForceSolver : public ISudokuSolver<H,W,N> {
  static const INT G = H * W;
public:
  typedef BITSET(G) Values;
  typedef BITSET(N) AllCells;
  typedef std::array<Values, N> GridState;
  
public:
  using ISudokuSolver<H,W,N>::ISudokuSolver;
  virtual bool Solve();
  inline UINT GetScore() { return _score; }
  
private:
  UINT _max, _count, _score;
};


template <UINT H, UINT W = H, UINT N = H * H * W * W>
class LogicalSolver : public ISudokuSolver<H,W,N> {
    static const UINT G = H * W;
public:
  typedef BITSET(G) Values;
  typedef BITSET(N) AllCells;
  typedef std::array<Values, N> GridState;
  // Value to reset, Index to perform on, Action group
  typedef std_x::triple<Action, UINT, UINT> Actionable;
  typedef std::pair<GridState, AllCells> SolveState;
  typedef std_x::triple<const AllCells, UINT, UINT> Intersection;
  typedef std::list<AllCells> Patterns;
  
public:
  LogicalSolver(SudokuGrid<H,W,N>&);
  virtual bool Solve();
  const std::vector<LogicOperation>& LogicalOperations() { return _order; }
  
private:
  SolveState _solve_state;
  std::vector<LogicOperation> _order;
  std::vector<Actionable> _actions;
  std::vector<Intersection> _intersects;
  std::array<Patterns, G> _patterns;
  UINT _action_next;
  
private:
  // Logic
  void HandleActions();
  bool NakedSingle();
  bool HiddenSingle();
  bool NakedNuple(UINT);
  bool HiddenNuple(UINT);
  bool GroupIntersection();
  bool PatternOverlay();
  bool BruteForce();
  
  // Useful utilities
  void SetSingleValue(UINT, UINT);
  
  
};


#endif /* SUDOKUSOLVER_BRUTEFORCE_SOLVER_HPP */
