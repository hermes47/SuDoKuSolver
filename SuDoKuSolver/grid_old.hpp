//
//  grid.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#ifndef SUDOKUSOLVER_OLD_GRID_HPP
#define SUDOKUSOLVER_OLD_GRID_HPP

#include <algorithm>
#include <array>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "triple.hpp"

typedef uint8_t dimension_t;   // Type used to define grid size
typedef uint8_t values_t;     // Type used to define possible values
typedef uint8_t work_t;       // Type used for internal grid working

template <values_t N>
class _SudokuCell {
  typedef std::bitset<N> Possibles;
  static const values_t _mask = values_t(1) << (std::numeric_limits<values_t>::digits - 1);
  Possibles _possibleValues;
  values_t _value;       // logical OR with _mask to set if is a clue
  values_t _row, _col, _blk;
  work_t _idx;
  
public:
  _SudokuCell() : _value(0) { _possibleValues.set(); }
  ~_SudokuCell() {}
  inline values_t GetValue() const { return _value & (_mask - 1); }
  inline void SetValue(values_t v) { _value = v; _possibleValues.reset(); }
  inline void SetFixedValue(values_t v) { _value = v | _mask; _possibleValues.reset(); }
  inline bool IsFixed() const { return _value & 128; }
  inline void ToggleOption(values_t p) { _possibleValues.flip(p - 1); }
  inline void SetOption(values_t p) { _possibleValues.set(p - 1); }
  inline void ResetOption(values_t p) { _possibleValues.reset(p - 1); }
  inline Possibles GetPossibleOptions() const { return _possibleValues; }
  inline bool IsOption(values_t i) const { return _possibleValues[i]; }
  inline values_t NumOptions() const { return (values_t)_possibleValues.count(); }
  inline void Reset() { _value = 0; _possibleValues.set(); }
  inline values_t GetRow() const { return _row; }
  inline values_t GetColumn() const { return _col; }
  inline values_t GetBlock() const { return _blk; }
  inline work_t GetIndex() const { return _idx; }
  inline void SetRow(values_t r) { _row = r; }
  inline void SetColumn(values_t r) { _col = r; }
  inline void SetBlock(values_t r) { _blk = r; }
  inline void SetIndex(work_t i) { _idx = i; }
};

template <values_t N>
bool operator==(_SudokuCell<N>& lhs, _SudokuCell<N>& rhs) {
  return (lhs->GetRow() == rhs->GetRow()
          && lhs->GetColumn() == rhs->GetColumn()
          && lhs->GetBlock() == rhs->GetBlock());
}

template<dimension_t H, dimension_t W>
inline std_x::triple<values_t, values_t, values_t> GetCellGroups(values_t i) {
  values_t c = i % (H * W);
  values_t R = i / (H * W * H);
  values_t C = c / W;
  return std_x::make_triple(i / (H * W), c, R * H + C);
}

template <dimension_t Height, dimension_t Width = Height>
class _SudokuGrid;

template <dimension_t Height, dimension_t Width>
bool SolveGrid(_SudokuGrid<Height, Width>&, bool = false, size_t = 500);

template <dimension_t Height, dimension_t Width>
class _SudokuGrid {
  friend bool SolveGrid<Height,Width>(_SudokuGrid<Height,Width>&, bool, size_t);
  
  static const values_t num_vals = Height * Width;
  static const work_t num_cells = num_vals * num_vals;
  
public:
  typedef _SudokuCell<num_vals> Cell;
  typedef std::array<values_t, num_cells> GridState;
  typedef std::bitset<num_vals> Possibles;
  typedef std::bitset<num_cells> AllCells;
  
private:
  std::array<Cell, num_cells> _cells;
  std::array<AllCells, num_vals> _rows;
  std::array<AllCells, num_vals> _cols;
  std::array<AllCells, num_vals> _blks;
  
  GridState _initial;
  GridState _solved;
  bool _solveable;
  size_t _num_solutions;
  
public:
  _SudokuGrid() {
//    for (work_t i = 0; i < _cells.size(); ++i) _cells[i] = Cell();
//    for (values_t i = 0; i < num_cells; ++i) {
//      _rows[i] = std::bitset<num_cells>(0);
//      _cols[i] = std::bitset<num_cells>(0);
//      _blks[i] = std::bitset<num_cells>(0);
//    }
    
    for (work_t i = 0; i < _cells.size(); ++i) {
      auto rcb = GetCellGroups<Height,Width>(i);
      _rows[rcb.first].set(i);
      _cols[rcb.second].set(i);
      _blks[rcb.third].set(i);
      _cells[i].SetRow(rcb.first);
      _cells[i].SetColumn(rcb.second);
      _cells[i].SetBlock(rcb.third);
      _cells[i].SetIndex(i);
    }
  }
  
  _SudokuGrid(const std::string s) : _SudokuGrid() {
    if (s.size() != _cells.size()) {
      std::cerr << "Provided grid string is wrong size. (Expected " << _cells.size()
      << ", got " << s.size() << ")." << std::endl;
      return;
    }
    
    for (values_t i = 0; i < _cells.size(); ++i){
      values_t v = s[i] == '.' ? 0 : s[i] - 48;
      _initial[i] = v;
    }
    SetState(_initial);
    _solveable = SolveGrid(*this, true, 2);
  }
  
  inline bool IsSolvable() const { return _solveable; }
  inline size_t SolutionCount() const { return _num_solutions; }
  
  bool IsValidState() const {
    // Check all unvalued cells have possible locations
    for (const Cell& c : _cells) {
      if (!c.GetValue() && !c.NumOptions()) return false;
    }
    // Check all groups have maximum of one of each value
    std::array<std::bitset<num_vals>, ((work_t)num_vals) * 3> all_groups;
    for (values_t i = 0; i < _cells.size(); ++i) {
      values_t v = _cells[i].GetValue();
      if (!v) continue;
      v -= 1;
      values_t r = _cells[i].GetRow(), c = _cells[i].GetColumn(), b = _cells[i].GetBlock();
      if (all_groups[r][v]) return false;
      else all_groups[r].set(v);
      if (all_groups[c + num_vals][v]) return false;
      else all_groups[c + num_vals].set(v);
      if (all_groups[b + num_vals + num_vals][v]) return false;
      else all_groups[b + num_vals + num_vals].set(v);
    }
    return true;
  }
  
  bool IsSolved() {
    for (Cell& c : _cells) {
      if (!c.GetValue()) return false;
    }
    return true;
  }
  
  void GetState(GridState& state) const {
    for (values_t i = 0; i < _cells.size(); ++i) {
      state[i] = _cells[i].GetValue();
    }
  }
  
  void SetState(const GridState& state) {
    for (Cell& cell : _cells) cell.Reset();
    for (values_t i = 0; i < _cells.size(); ++i) {
      if (!state[i]) continue;
      values_t v = state[i];
      if (_initial[i]) _cells[i].SetFixedValue(v);
      else _cells[i].SetValue(v);
      auto rcb = GetCellGroups<Height, Width>(i);
      AllCells affected = _rows[rcb.first] | _cols[rcb.second] | _blks[rcb.third];
      for (values_t j = 0; j < _cells.size(); ++j) {
        if (affected[j]) _cells[j].ResetOption(v);
      }
    }
  }
  
  void SetSolvedState(const GridState& state) {
    
  }
  
  void GetSolvedState(GridState& state) const {
    
  }
  
  inline Cell& GetCell(work_t idx) { return _cells[idx]; }
  inline AllCells& GetRow(values_t idx) { return _rows[idx]; }
  inline AllCells& GetColumn(values_t idx) { return _cols[idx]; }
  inline AllCells& GetBlock(values_t idx) { return _blks[idx]; }
  
  void DisplayGrid() const {
    for (values_t row = 0; row < _rows.size(); ++row) {
      if (!(row % Height)) PrintSeperatorGridLine();
      PrintRowGridLine(_rows[row]);
    }
    PrintSeperatorGridLine();
  }
  
  void DisplayState() const {
    GridState state;
    GetState(state);
    for (values_t v : state) {
      if (v) std::cout << v;
      else std::cout << '.';
    }
    std::cout << std::endl;
  }
  
private:
  void PrintSeperatorGridLine() const;
  void PrintRowGridLine(const AllCells&) const;
};

#endif /* SUDOKUSOLVER_OLD_GRID_HPP */
