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
#include <utility>

#include "utility.hpp"

template <INT N>
class SudokuCell {
  typedef std_x::bitset<N> Values;
  
  Values _values, _initial;
  INT _row, _col, _blk, _idx;
  bool _clue;
  
public:
  // Default constructor
  SudokuCell()
  : _row(-1), _col(-1), _blk(-1), _idx(-1), _clue(false) { _values.set(); }
  
  // Construct cell with given index
  SudokuCell(INT i)
  : _row(-1), _col(-1), _blk(-1), _idx(i), _clue(false) { _values.set(); }
  
  // Construct cell with given index and value. Calling grid is responsible
  // for propagation of value effect
  SudokuCell(INT i, INT v)
  : _row(-1), _col(-1), _blk(-1), _idx(i), _clue(false) {
    _values.reset();
    _values.set(v);
  }
  
  // Move constructor
  SudokuCell(SudokuCell&& c)
  : _values(std::move(c._values)), _row(std::move(c._row)),
  _col(std::move(c._col)), _blk(std::move(c._blk)),
  _idx(std::move(c._idx)), _clue(std::move(c._clue)) { }
  
  // Copy constructor
  SudokuCell(const SudokuCell& c)
  : _values(c._values), _row(c._row), _col(c._col), _blk(c._blk),
  _idx(c._idx), _clue(c._clue) { }
  
  // Assignment operator
  SudokuCell& operator=(SudokuCell&& c) {
    _values = std::move(c._values);
    _row = std::move(c._row);
    _col = std::move(c._col);
    _blk = std::move(c._blk);
    _idx = std::move(c._idx);
    _clue = std::move(c._clue);
    return *this;
  }
  
  inline INT GetValue() const {
    return _values.count() > 1 ? 0 : __find_first<N>(_values) + 1;
  }
  
  inline void SetValue(INT v) {
    // Grid is responsible for propagating this set through to affected cells
    if (_clue) return;
    _values.reset();
    _values.set(v - 1);
  }
  
  inline void SetFixedValue(INT v) {
    SetValue(v);
    _clue = true;
  }
  
  inline bool IsFixed() const { return _clue; }
  inline void ToggleOption(INT p) { if (!_clue) _values.flip(p - 1); }
  inline void SetOption(INT p) {  if (!_clue) _values.set(p - 1); }
  inline void ResetOption(INT p) {  if (!_clue) _values.reset(p - 1); }
  inline Values GetPossibleValues() const { return _values; }
  inline bool IsPossibleValue(INT i) const { return _AT(_values, i); }
  inline INT NumOptions() const { return (INT)_values.count(); }
  inline void Reset() { _values = _initial; }
  inline INT GetRow() const { return _row; }
  inline INT GetColumn() const { return _col; }
  inline INT GetBlock() const { return _blk; }
  inline INT GetIndex() const { return _idx; }
  inline void SetRow(INT r) { _row = r; }
  inline void SetColumn(INT r) { _col = r; }
  inline void SetBlock(INT r) { _blk = r; }
  inline void SetIndex(INT i) { _idx = i; }
  inline void SetCurrentAsInitial() { _initial = _values; }
};

// Comparison operators
template <INT N>
inline bool operator==(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() == r.GetIndex();
}

template <INT N>
inline bool operator!=(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() != r.GetIndex();
}

template <INT N>
inline bool operator<(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() < r.GetIndex();
}

template <INT N>
inline bool operator>(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() > r.GetIndex();
}
template <INT N>
inline bool operator<=(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() <= r.GetIndex();
}

template <INT N>
inline bool operator>=(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() >= r.GetIndex();
}


#endif /* SUDOKUSOLVER_CELL_HPP */
