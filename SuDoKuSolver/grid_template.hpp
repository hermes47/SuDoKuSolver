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
  inline std::bitset<N> GetPossibleOptions() const { return _possibleValues; }
  inline bool IsOption(size_t i) const { return _possibleValues[i]; }
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
  
public:
  SudokuGrid() {
    for (unsigned char i = 0; i < T; ++i) _cells[i] = Cell();
    for (unsigned char i = 0; i < T; ++i) {
      size_t r = i / N;
      size_t c = i % N;
      size_t br = i / (N * H);
      size_t bc = c / W;
      size_t b = br * H + bc;
      
      _rows[r].set(i);
      _cols[c].set(i);
      _blks[b].set(i);
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
      std::cout << (int)(i) << " : " ;
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
