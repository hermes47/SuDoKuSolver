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
#include <iostream>
#include <string>

template <unsigned char N>
class SudokuCell {
  std::bitset<N> _possibleValues;
  unsigned char _value;       // logical OR with 128 to set if is a clue
  
public:
  SudokuCell() : _value(0) { _possibleValues.set(); }
  SudokuCell(unsigned char v) : _value(v) {
    if (_value) _value |= 128;
  }
  inline unsigned char GetValue() const { return _value & 127; }
  inline void SetValue(unsigned char v) { _value = v; }
  inline void SetFixedValue(unsigned char v) { _value = v | 128; }
  inline bool IsFixed() const { return _value & 128; }
  inline void ToggleOption(unsigned char p) { _possibleValues.flip(p - 1); }
  inline void SetOption(unsigned char p) { _possibleValues.set(p - 1); }
  inline void ResetOption(unsigned char p) { _possibleValues.reset(p - 1); }
  inline void GetPossibleOptions(std::bitset<N> &o) const { o = _possibleValues; }
  inline bool IsOption(size_t i) const { return _possibleValues[i]; }
  inline size_t NumOptions() const { return _possibleValues.count(); }
};

template <unsigned char H, unsigned char W = H>
class SudokuGrid {
  static const unsigned int N = H * W;
  static const unsigned int T = N * N;
  typedef SudokuCell<N> Cell;
  std::array<Cell, T> _cells;
  std::array<std::bitset<T>, N> _rows;
  std::array<std::bitset<T>, N> _cols;
  std::array<std::bitset<T>, N> _blks;
  
protected:
  void GetCellGroups(unsigned char i, size_t &row, size_t &col, size_t &blk) {
    row = i / N;
    col = i % N;
    size_t bigrow = i / (N * H);
    size_t bigcol = col / W;
    blk = bigrow * H + bigcol;
  }
  
public:
  SudokuGrid() {
    for (unsigned char i = 0; i < T; ++i) _cells[i] = Cell();
    for (unsigned char i = 0; i < T; ++i) {
      size_t r, c, b;
      GetCellGroups(i, r, c, b);
      _rows[r].set(i);
      _cols[c].set(i);
      _blks[b].set(i);
    }
  }
  
  SudokuGrid(const std::string s) : SudokuGrid() {
    if (s.size() != T) {
      std::cerr << "Provided grid string is wrong size. (Expected " << T
      << ", got " << s.size() << ")." << std::endl;
      return;
    }
    
    for (size_t i = 0; i < _cells.size(); ++i){
      unsigned char v = s[i] - 48;
      if (!v) continue;
      size_t r, c, b;
      GetCellGroups(i, r, c, b);
      _cells[i] = Cell(v);
      std::bitset<T> affected = _rows[r] | _cols[c] | _blks[b];
      for (size_t j = 0; j < _cells.size(); ++j) {
        if (affected[j]) _cells[j].ResetOption(v);
      }
    }
    
  }
  
  void SolveGrid() {
    // First go through setting all only values.
    bool changed = true;
    while (changed) {
      changed = false;
      for (size_t i = 0; i < _cells.size(); ++i) {
        Cell &cell = _cells[i];
        if (cell.NumOptions() == 1) {
          std::bitset<N> bits;
          cell.GetPossibleOptions(bits);
          unsigned char v = 0;
          while (bits.any()) {
            ++v;
            bits >>= 1;
          }
          cell.SetValue(v);
          size_t r, c, b;
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
    std::cout << "Rows:" << std::endl;
    for (unsigned char i = 0; i < N; ++i) {
      std::cout << (int)i << " : ";
      for (size_t j = 0; j < T; ++j) {
        if (_rows[i][j]) std::cout << j << " ";
      }
      std::cout << std::endl;
    }
    std::cout << "\nColumns:" << std::endl;
    for (unsigned char i = 0; i < N; ++i) {
      std::cout << (int)i << " : ";
      for (size_t j = 0; j < T; ++j) {
        if (_cols[i][j]) std::cout << j << " ";
      }
      std::cout << std::endl;
    }
    std::cout << "\nBlocks:" << std::endl;
    for (unsigned char i = 0; i < N; ++i) {
      std::cout << (int)i << " : ";
      for (size_t j = 0; j < T; ++j) {
        if (_blks[i][j]) std::cout << j << " ";
      }
      std::cout << std::endl;
    }
    std::cout << "\nCells:" << std::endl;
    for (unsigned char i = 0; i < T; ++i) {
      std::cout << (int)(i) << " : " << (int)_cells[i].GetValue() << " : ";
      for (size_t j = 0; j < N; ++j) {
        if (_cells[i].IsOption(j)) std::cout << j + 1 << " ";
      }
      std::cout << std::endl;
    }
  }
#undef T
#undef N
};

#endif /* SUDOKUSOLVER_GRID_TEMPLATE_HPP */
