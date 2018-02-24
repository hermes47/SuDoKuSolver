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

#ifdef USE_EASTL_BITSET
#include "EASTL/bitset.h"
#else
#include <bitset>
#endif
#include <utility>

#include "utility.hpp"

template <UINT N>
class SudokuCell {
  typedef BITSET(N) Values;
  
  Values _values, _initial;
  UINT _row, _col, _blk, _idx;
  bool _clue;
  
public:
  // Default constructor
  SudokuCell()
  : _row(0), _col(0), _blk(0), _idx(0), _clue(false) { _values.set(); }
  
  // Construct cell with given index
  SudokuCell(UINT i)
  : _row(0), _col(0), _blk(0), _idx(i), _clue(false) { _values.set(); }
  
  // Construct cell with given index and value. Calling grid is responsible
  // for propagation of value effect
  SudokuCell(UINT i, UINT v)
  : _row(0), _col(0), _blk(0), _idx(i), _clue(false) {
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
  
  inline UINT GetValue() const {
    return _values.count() > 1 ? 0 : __find_first(_values) + 1;
  }
  
  inline void SetValue(UINT v) {
    // Grid is responsible for propagating this set through to affected cells
    if (_clue) return;
    _values.reset();
    _values.set(v - 1);
  }
  
  inline void SetFixedValue(UINT v) {
    SetValue(v);
    _clue = true;
  }
  
  inline bool IsFixed() const { return _clue; }
  inline void ToggleOption(UINT p) { if (!_clue) _values.flip(p - 1); }
  inline void SetOption(UINT p) {  if (!_clue) _values.set(p - 1); }
  inline void ResetOption(UINT p) {  if (!_clue) _values.reset(p - 1); }
  inline Values GetPossibleValues() const { return _values; }
  inline void SetPossibleValues(const Values& v) { _values = v; }
  inline bool IsPossibleValue(UINT i) const { return _AT(_values, i); }
  inline UINT NumOptions() const { return _values.count(); }
  inline void Reset() { _values = _initial; }
  inline UINT GetRow() const { return _row; }
  inline UINT GetColumn() const { return _col; }
  inline UINT GetBlock() const { return _blk; }
  inline UINT GetIndex() const { return _idx; }
  inline void SetRow(UINT r) { _row = r; }
  inline void SetColumn(UINT r) { _col = r; }
  inline void SetBlock(UINT r) { _blk = r; }
  inline void SetIndex(UINT i) { _idx = i; }
  inline void SetCurrentAsInitial() { _initial = _values; }
};

// Comparison operators
template <UINT N>
inline bool operator==(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() == r.GetIndex();
}

template <UINT N>
inline bool operator!=(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() != r.GetIndex();
}

template <UINT N>
inline bool operator<(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() < r.GetIndex();
}

template <UINT N>
inline bool operator>(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() > r.GetIndex();
}
template <UINT N>
inline bool operator<=(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() <= r.GetIndex();
}

template <UINT N>
inline bool operator>=(SudokuCell<N>& l, SudokuCell<N>& r) {
  return l.GetIndex() >= r.GetIndex();
}


#endif /* SUDOKUSOLVER_CELL_HPP */
