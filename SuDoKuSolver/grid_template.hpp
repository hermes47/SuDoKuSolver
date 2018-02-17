//
//  grid_template.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#ifndef SUDOKUSOLVER_GRID_TEMPLATE_HPP
#define SUDOKUSOLVER_GRID_TEMPLATE_HPP

#include <array>
#include <bitset>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

typedef uint8_t grid_size_t;  // Type used to define grid size
typedef uint16_t cell_value_t; // Type used to define possible values
typedef uint32_t grid_work_t;  // Type used for internal grid working

template <cell_value_t N>
class SudokuCell {
  std::bitset<N> _possibleValues;
  cell_value_t _value;       // logical OR with 128 to set if is a clue
  
public:
  SudokuCell() : _value(0) { _possibleValues.set(); }
  SudokuCell(cell_value_t v) : _value(v) {
    static cell_value_t mask = cell_value_t(1) << (std::numeric_limits<cell_value_t>::digits - 1);
    if (_value) _value |= mask;
  }
  inline cell_value_t GetValue() const { return _value & 127; }
  inline void SetValue(cell_value_t v) { _value = v; }
  inline void SetFixedValue(cell_value_t v) { _value = v | 128; }
  inline bool IsFixed() const { return _value & 128; }
  inline void ToggleOption(cell_value_t p) { _possibleValues.flip(p - 1); }
  inline void SetOption(cell_value_t p) { _possibleValues.set(p - 1); }
  inline void ResetOption(cell_value_t p) { _possibleValues.reset(p - 1); }
  inline void GetPossibleOptions(std::bitset<N> &o) const { o = _possibleValues; }
  inline bool IsOption(cell_value_t i) const { return _possibleValues[i]; }
  inline cell_value_t NumOptions() const { return (cell_value_t)_possibleValues.count(); }
};

template <grid_size_t H, grid_size_t W = H>
class SudokuGrid {
  static const cell_value_t N = H * W;
  static const grid_work_t T = N * N;
  typedef SudokuCell<N> Cell;
  typedef std::array<Cell*, N> Group;
  
  std::array<Cell, T> _cells;
  std::array<std::bitset<T>, N> _rows;
  std::array<std::bitset<T>, N> _cols;
  std::array<std::bitset<T>, N> _blks;
  
protected:
  void GetCellGroups(cell_value_t i, cell_value_t &row, cell_value_t &col, cell_value_t &blk) {
    row = i / N;
    col = i % N;
    cell_value_t bigrow = i / (N * H);
    cell_value_t bigcol = col / W;
    blk = bigrow * H + bigcol;
  }
  
public:
  SudokuGrid() {
    for (cell_value_t i = 0; i < _cells.size(); ++i) {
       _cells[i] = Cell();
      cell_value_t r, c, b;
      GetCellGroups(i, r, c, b);
      _rows[r].set(i);
      _cols[c].set(i);
      _blks[b].set(i);
    }
  }
  
  SudokuGrid(const std::string s) : SudokuGrid() {
    if (s.size() != _cells.size()) {
      std::cerr << "Provided grid string is wrong size. (Expected " << _cells.size()
      << ", got " << s.size() << ")." << std::endl;
      return;
    }
    
    for (cell_value_t i = 0; i < _cells.size(); ++i){
      cell_value_t v = s[i] - 48;
      if (!v) continue;
      cell_value_t r, c, b;
      GetCellGroups(i, r, c, b);
      _cells[i] = Cell(v);
      std::bitset<T> affected = _rows[r] | _cols[c] | _blks[b];
      for (cell_value_t j = 0; j < _cells.size(); ++j) {
        if (affected[j]) _cells[j].ResetOption(v);
      }
    }
    
  }
  
  bool IsValidState() {
    // Check all unvalued cells have possible locations
    for (Cell& c : _cells) {
      if (!c.GetValue() && !c.NumOptions()) return false;
    }
    // Check all groups have maximum of one of each value
    std::array<std::bitset<N>, ((grid_work_t)N) * 3> all_groups;
    for (cell_value_t i = 0; i < _cells.size(); ++i) {
      cell_value_t v = _cells[i].GetValue();
      if (!v) continue;
      v -= 1;
      cell_value_t row, col, blk;
      GetCellGroups(i, row, col, blk);
      if (all_groups[row][v]) return false;
      else all_groups[row].set(v);
      if (all_groups[col + N][v]) return false;
      else all_groups[col + N].set(v);
      if (all_groups[blk + N + N][v]) return false;
      else all_groups[blk + N + N].set(v);
    }
    return true;
  }
  
  void SolveGrid() {
    // First go through setting all only values.
    bool changed = true;
    while (changed) {
      changed = false;
      for (cell_value_t i = 0; i < _cells.size(); ++i) {
        Cell &cell = _cells[i];
        if (cell.NumOptions() == 1) {
          std::bitset<N> bits;
          cell.GetPossibleOptions(bits);
          cell_value_t v = 0;
          while (bits.any()) {
            ++v;
            bits >>= 1;
          }
          cell.SetValue(v);
          cell_value_t r, c, b;
          GetCellGroups(i, r, c, b);
          std::bitset<T> affected = _rows[r] | _cols[c] | _blks[b];
          for (size_t j = 0; j < _cells.size(); ++j) {
            if (affected[j]) _cells[j].ResetOption(v);
          }
          changed = true;
        }
      }
    }
    
  }
  
  void DisplayGrid() const {
    for (cell_value_t row = 0; row < _rows.size(); ++row) {
      if (!(row % H)) PrintSeperatorGridLine();
      PrintRowGridLine(_rows[row]);
    }
    PrintSeperatorGridLine();
  }
  
  
private:
  void PrintSeperatorGridLine() const {
    for (cell_value_t i = 0; i < N; ++i) {
      if (!(i % W)) std::cout << "+-";
      std::cout << "--";
    }
    std::cout << "+" << std::endl;
  }
  void PrintRowGridLine(const std::bitset<T> &row) const {
    cell_value_t col = 0;
    for (cell_value_t i = 0; i < row.size(); ++i) {
      if (!row[i]) continue;
      if (!(col % W)) std::cout << "| ";
      if (_cells[i].GetValue()) std::cout << (int)_cells[i].GetValue() << " ";
      else std::cout << "  ";
      ++col;
    }
    std::cout << "|" << std::endl;
  }

};

#endif /* SUDOKUSOLVER_GRID_TEMPLATE_HPP */
