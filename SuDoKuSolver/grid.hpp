//
//  grid.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#ifndef SUDOKUSOLVER_GRID_HPP
#define SUDOKUSOLVER_GRID_HPP

#include <algorithm>
#include <array>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

typedef uint8_t dimension_t;   // Type used to define grid size
typedef uint16_t values_t;     // Type used to define possible values
typedef uint64_t work_t;       // Type used for internal grid working

template <values_t N>
class SudokuCell {
  static const values_t _mask = values_t(1) << (std::numeric_limits<values_t>::digits - 1);
  std::bitset<N> _possibleValues;
  values_t _value;       // logical OR with _mask to set if is a clue
  values_t _row, _col, _blk;
  
public:
  SudokuCell() : _value(0) { _possibleValues.set(); }
  inline values_t GetValue() const { return _value & (_mask - 1); }
  inline void SetValue(values_t v) { _value = v; _possibleValues.reset(); }
  inline void SetFixedValue(values_t v) { _value = v | _mask; _possibleValues.reset(); }
  inline bool IsFixed() const { return _value & 128; }
  inline void ToggleOption(values_t p) { _possibleValues.flip(p - 1); }
  inline void SetOption(values_t p) { _possibleValues.set(p - 1); }
  inline void ResetOption(values_t p) { _possibleValues.reset(p - 1); }
  inline void GetPossibleOptions(std::bitset<N> &o) const { o = _possibleValues; }
  inline bool IsOption(values_t i) const { return _possibleValues[i]; }
  inline values_t NumOptions() const { return (values_t)_possibleValues.count(); }
  inline void Reset() { _value = 0; _possibleValues.set(); }
  inline values_t GetRow() const { return _row; }
  inline values_t GetColumn() const { return _col; }
  inline values_t GetBlock() const { return _blk; }
  inline void SetRow(values_t r) { _row = r; }
  inline void SetColumn(values_t r) { _col = r; }
  inline void SetBlock(values_t r) { _blk = r; }
};
template <dimension_t Height, dimension_t Width = Height>
class SudokuGrid;

template <dimension_t Height, dimension_t Width>
bool SolveGrid(SudokuGrid<Height, Width>&, bool = false);

template <dimension_t Height, dimension_t Width>
bool LogicallySolveGrid(SudokuGrid<Height,Width>&);

template <dimension_t Height, dimension_t Width>
class SudokuGrid {
  friend bool SolveGrid<Height,Width>(SudokuGrid<Height,Width>&, bool);
  friend bool LogicallySolveGrid<Height,Width>(SudokuGrid<Height,Width>&);
  
  static const values_t num_vals = Height * Width;
  static const work_t num_cells = num_vals * num_vals;
  
public:
  typedef SudokuCell<num_vals> Cell;
  typedef std::array<Cell*, num_vals> Group;
  typedef std::array<values_t, num_cells> GridState;
  
private:
  std::array<Cell, num_cells> _cells;
  std::array<std::bitset<num_cells>, num_vals> _rows;
  std::array<std::bitset<num_cells>, num_vals> _cols;
  std::array<std::bitset<num_cells>, num_vals> _blks;
  
  GridState _initial;
  GridState _solved;
  bool _solveable;
  
protected:
  void GetCellGroups(values_t i, values_t &row, values_t &col, values_t &blk) const {
    row = i / num_vals;
    col = i % num_vals;
    values_t bigrow = i / (num_vals * Height);
    values_t bigcol = col / Width;
    blk = bigrow * Height + bigcol;
  }
  
public:
  SudokuGrid() {
    for (values_t i = 0; i < _cells.size(); ++i) {
      values_t r, c, b;
      GetCellGroups(i, r, c, b);
      _rows[r].set(i);
      _cols[c].set(i);
      _blks[b].set(i);
      _cells[i].SetRow(r);
      _cells[i].SetColumn(c);
      _cells[i].SetBlock(b);
    }
  }
  
  SudokuGrid(const std::string s) : SudokuGrid() {
    if (s.size() != _cells.size()) {
      std::cerr << "Provided grid string is wrong size. (Expected " << _cells.size()
      << ", got " << s.size() << ")." << std::endl;
      return;
    }
    
    for (values_t i = 0; i < _cells.size(); ++i){
      values_t v = s[i] == '.' ? 0 : s[i] - 48;
      if (!v) continue;
      _initial[i] = v;
    }
    SetState(_initial);
    _solveable = SolveGrid(*this, true);
  }
  
  inline bool IsSolvable() const { return _solveable; }
  
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
      values_t r = _cells[i].GetRow(), c = _cells[i].GetColumn(), b = _cells[i].GetBlock();
      std::bitset<num_cells> affected = _rows[r] | _cols[c] | _blks[b];
      for (values_t j = 0; j < _cells.size(); ++j) {
        if (affected[j]) _cells[j].ResetOption(v);
      }
    }
  }
  
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
  }
  
private:
  void PrintSeperatorGridLine() const {
    for (values_t i = 0; i < num_vals; ++i) {
      if (!(i % Width)) std::cout << "+-";
      std::cout << "--";
    }
    std::cout << "+" << std::endl;
  }
  
  void PrintRowGridLine(const std::bitset<num_cells> &row) const {
    values_t col = 0;
    for (values_t i = 0; i < row.size(); ++i) {
      if (!row[i]) continue;
      if (!(col % Width)) std::cout << "| ";
      if (_cells[i].GetValue()) std::cout << (int)_cells[i].GetValue() << " ";
      else std::cout << "  ";
      ++col;
    }
    std::cout << "|" << std::endl;
  }
};

#endif /* SUDOKUSOLVER_GRID_HPP */
