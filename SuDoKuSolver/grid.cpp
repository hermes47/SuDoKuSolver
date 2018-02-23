//
//  grid.cpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 20/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//
#include "defines.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

#include "grid.hpp"
#include "solver.hpp"

// Default constructor creates all cells, then calls SetGroups and SetAffected
// Any classes inheriting should call this constructor in initalisation list
template<UINT H, UINT W, UINT N>
SudokuGrid<H,W,N>::SudokuGrid() {
  for (UINT i = 0; i < N; ++i) _AT(_cells, i) = Cell(i);
  SetGroups();
  SetAffected();
}

// Construct from a string. Can only handle up to 62 possible values
// 1-9 are obvious, 0 is 10, A-Z is 11 to 36, a-z are 37 - 62.
// . is treated as blank. Any other characters will throw
template<UINT H, UINT W, UINT N>
SudokuGrid<H,W,N>::SudokuGrid(const std::string& s)
: SudokuGrid() {
  static_assert(G <= 62, "String construction can only handle 62 values");
  if (s.size() != N) throw std::length_error("Input string is incorrect length.");
  
  for (INT i = 0; i < N; ++i) {
    INT v = (INT)_AT(s, i);
    if (v == '.') continue;
    else if (v >= '1' && v <= '9') v -= 48;
    else if (v == '0') v = 10;
    else if (v >= 'A' && v <= 'Z') v -= 54;
    else if (v >= 'a' && v <= 'z') v -= 60;
    else throw std::runtime_error("Unknown characters in input string.");
    if (v > G) throw std::out_of_range("Characters in input have value greater than maximum.");
    // Set fixed value and propagate consequences
    GetCell(i).SetFixedValue(v);
    const AllCells& affect = GetAffected(i);
    for (INT j = 0; j < N; ++j) {
      if (affect[j]) GetCell(j).ResetOption(v);
    }
  }
  
  // Set Initial state
  for (Cell& c : _cells) {
    c.SetCurrentAsInitial();
    _AT(_initial, c.GetIndex()) = c.GetPossibleValues();
  }
}

template<UINT H, UINT W, UINT N>
const typename SudokuGrid<H,W,N>::GridState& SudokuGrid<H,W,N>::GetSolvedState() {
  // Set solved state
  if (_num_solutions < 0) {
    UINT count = 0;
    BruteForceSolver<H,W,N> solver(*this);
    if (solver.Solve()) {
     ++count;
      _solved = solver.GetSolvedState();
    }
//    SolveGridNew<H, W, N>(_initial, _grps, _affected, _solved, count, true, 2);
    _num_solutions = (INT)count;
  }
  return _solved;
}

template<UINT H, UINT W, UINT N>
bool SudokuGrid<H,W,N>::Solve() {
  GetSolvedState();
  if (_num_solutions != 1) return false;
  SetState(_solved);
  return true;
}

template <UINT H, UINT W, UINT N>
bool SudokuGrid<H,W,N>::LogicalSolve() {
  if (IsSolvable()) {
    LogicalSolver<H,W,N> solver(*this);
    return solver.Solve();
  }
  return false;
}

template<UINT H, UINT W, UINT N>
bool SudokuGrid<H,W,N>::CheckCurrentState() const {
  for (UINT i = 0; i < N; ++i) {
    if (_AT(_cells, i).GetValue()
        && _AT(_cells, i).GetPossibleValues() != _AT(_initial, i))
      return false;
  }
  return true;
}

template<UINT H, UINT W, UINT N>
bool SudokuGrid<H,W,N>::IsValid() const {
  // State is valid if there are no same group clashes of values
  for (const AllCells& group : _grps) {
    UINT count = 0;
    Values set(0);
    FORBITSIN(idx, group) {
      const Cell& cell = _AT(_cells, idx);
      if (cell.GetValue() && cell.GetValue() != N) {
        ++count;
        set |= cell.GetPossibleValues();
      }
    }
    if (set.count() != count) return false;
  }
  return true;
}

template<UINT H, UINT W, UINT N>
bool SudokuGrid<H,W,N>::IsSolvable() {
  GetSolvedState();
  if (_num_solutions == 1) return true;
  return false;
}

template<UINT H, UINT W, UINT N>
bool SudokuGrid<H,W,N>::IsSolved() const {
  if (!IsValid()) return false;
  for (const Cell& cell : _cells) {
    if (cell.GetValue() == 0 || cell.GetValue() == N) return false;
  }
  return true;
}

template<UINT H, UINT W, UINT N>
typename SudokuGrid<H,W,N>::GridState SudokuGrid<H,W,N>::GetCurrentState() const {
  GridState state;
  for (const Cell& c: _cells) {
    _AT(state, c.GetIndex()) = c.GetPossibleValues();
  }
  return state;
}

template<UINT H, UINT W, UINT N>
void SudokuGrid<H,W,N>::SetAffected() {
  static_assert(N == G * G, "With non-regular sudokus, need to override SetAffected");
  for (Cell& cell : _cells) {
    AllCells tmp = AllCells(0);
    tmp |= GetRow(cell.GetRow());
    tmp |= GetColumn(cell.GetColumn());
    tmp |= GetBlock(cell.GetBlock());
    tmp.reset(cell.GetIndex());
    _AT(_affected, cell.GetIndex()) = AllCells(tmp);
  }
}

template <UINT H, UINT W, UINT N>
std::ostream& SudokuGrid<H,W,N>::DisplayGrid(std::ostream& s) const {
  for (INT r = 0; r < G; ++r) {
    if (!(r % H)) PrintSeperatorGridLine(s);
    PrintRowGridLine(r, s);
  }
  PrintSeperatorGridLine(s);
  return s;
}

template <UINT H, UINT W, UINT N>
std::ostream& SudokuGrid<H,W,N>::DisplayGridString(std::ostream& s) const {
  for (const Cell& cell : _cells) {
    UINT v = cell.GetValue();
    char o;
    if (cell.NumOptions() == 0) o = '+';
    else if (v == 0) o = '.';
    else if (v >= 1 && v <= 9) o = '0' + v;
    else if (v == 10) o = '0';
    else if (v >= 11 && v <= 36) o = '6' + v;
    else o = '<' + v;
    s << o;
  }
  return s;
}

template <UINT H, UINT W, UINT N>
bool SudokuGrid<H,W,N>::SetState(const GridState & state) {
  // Check that the state doesn't alter clues
  for (UINT i = 0; i < N; ++i) {
    if (_AT(_cells, i).IsFixed() && _AT(state, i) != _AT(_initial, i))
      return false;
  }
  
  // Set state now
  for (UINT i = 0; i < N; ++i) _AT(_cells, i).SetPossibleValues(_AT(state, i));
  return true;
}

template<UINT H, UINT W, UINT N>
void SudokuGrid<H,W,N>::PrintSeperatorGridLine(std::ostream& s) const {
  for (INT i = 0; i < G; ++i) {
    if (!(i % W)) s << "+-";
    s << "--";
  }
  s << "+" << std::endl;
}

template<UINT H, UINT W, UINT N>
void SudokuGrid<H,W,N>::PrintRowGridLine(UINT r, std::ostream& s) const {
  UINT c = 0;
  const AllCells& row = GetRow(r);
  for (UINT i = 0; i < N; ++i) {
    if (!row[i]) continue;
    if (!(c % W)) s << "| ";
    UINT v = GetCell(i).GetValue();
    // Convert value to char
    char o;
    if (v == 0) o = '.';
    else if (v >= 1 && v <= 9) o = '0' + v;
    else if (v == 10) o = '0';
    else if (v >= 11 && v <= 36) o = '6' + v;
    else o = '<' + v;
    s << o << " ";
    ++c;
  }
  s << "|" << std::endl;
}

// Set groups default method. Only works when N = H * W * H * W
template<UINT H, UINT W, UINT N>
void SudokuGrid<H,W,N>::SetGroups() {
  static_assert(N == G * G,
                "With non-regular sudokus, need to override SetGroups");
  
  // Fill the vector with the required number of groups (3G)
  _grps.clear();
  _grps.reserve(3 * G);
  _grps.insert(_grps.begin(), 3 * G, AllCells(0));
  
  // Set all cell's row, column, block
  for (Cell& cell : _cells) {
    auto rcb = GetCellGroups<H, W, N>(cell.GetIndex());
    cell.SetRow(rcb.first);
    cell.SetColumn(rcb.second);
    cell.SetBlock(rcb.third);
    _AT(_grps, rcb.first).set(cell.GetIndex());
    _AT(_grps, rcb.second + G).set(cell.GetIndex());
    _AT(_grps, rcb.third + G * 2).set(cell.GetIndex());
  }
}

// Grid size init
#define GRID_SIZE(x,y,z)\
template class SudokuGrid<x,y,z>;

#include "gridsizes.itm"
#undef GRID_SIZE
