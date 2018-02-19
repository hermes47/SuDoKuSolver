//
//  sudoku_grid.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 19/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#ifndef SUDOKUSOLVER_CELL_HPP
#define SUDOKUSOLVER_CELL_HPP

#include "defines.hpp"

#include <bitset>
#include <limits>

#include "utility.hpp"

template <val_t N>
class SudokuCell {
  typedef std::bitset<N> Values;
  
  Values _values;
  val_t _row, _col, _blk, _idx;
  bool _clue;
  
public:
  // Default constructor
  SudokuCell()
  : _row(0), _col(0), _blk(0), _idx(0), _clue(false) { _values.set(); }
  
  
  
  inline val_t GetValue() const {
    return _values.count() > 1 ? 0 : __find_first(_values) + 1;
  }
  
  inline void SetValue(val_t v) {
    // Grid is responsible for propagating this set through to affected cells
    assert(v > 0);
    assert(!_clue);
    _values.reset();
    _values.set(v - 1);
  }
  
  inline void SetFixedValue(val_t v) {
    SetValue(v);
    _clue = true;
  }
  
  inline bool IsFixed() const { return _clue; }
  inline void ToggleOption(val_t p) { _values.flip(p - 1); }
  inline void SetOption(val_t p) { _values.set(p - 1); }
  inline void ResetOption(val_t p) { _values.reset(p - 1); }
  inline Values& GetPossibleValues() const { return _values; }
  inline bool IsPossibleValue(val_t i) const { return _values[i]; }
  inline val_t NumOptions() const { return (val_t)_values.count(); }
  inline void Reset() { _values.set(); }
  inline val_t GetRow() const { return _row; }
  inline val_t GetColumn() const { return _col; }
  inline val_t GetBlock() const { return _blk; }
  inline val_t GetIndex() const { return _idx; }
  inline void SetRow(val_t r) { _row = r; }
  inline void SetColumn(val_t r) { _col = r; }
  inline void SetBlock(val_t r) { _blk = r; }
  inline void SetIndex(val_t i) { _idx = i; }
};

// Comparison operators
template <val_t N>
inline bool operator==(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() == r.GetIndex();
}

template <val_t N>
inline bool operator!=(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() != r.GetIndex();
}

template <val_t N>
inline bool operator<(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() < r.GetIndex();
}

template <val_t N>
inline bool operator>(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() > r.GetIndex();
}
template <val_t N>
inline bool operator<=(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() <= r.GetIndex();
}

template <val_t N>
inline bool operator>=(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() >= r.GetIndex();
}


#endif /* SUDOKUSOLVER_CELL_HPP */
