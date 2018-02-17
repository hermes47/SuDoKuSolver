//
//  grid_template.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#ifndef SUDOKUSOLVER_GRID_TEMPLATE_HPP
#define SUDOKUSOLVER_GRID_TEMPLATE_HPP

#include <algorithm>
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
  static const cell_value_t _mask = cell_value_t(1) << (std::numeric_limits<cell_value_t>::digits - 1);
  std::bitset<N> _possibleValues;
  cell_value_t _value;       // logical OR with _mask to set if is a clue
  cell_value_t _row, _col, _blk;
  
  
public:
  SudokuCell() : _value(0) { _possibleValues.set(); }
  inline cell_value_t GetValue() const { return _value & (_mask - 1); }
  inline void SetValue(cell_value_t v) { _value = v; _possibleValues.reset(); }
  inline void SetFixedValue(cell_value_t v) { _value = v | _mask; _possibleValues.reset(); }
  inline bool IsFixed() const { return _value & 128; }
  inline void ToggleOption(cell_value_t p) { _possibleValues.flip(p - 1); }
  inline void SetOption(cell_value_t p) { _possibleValues.set(p - 1); }
  inline void ResetOption(cell_value_t p) { _possibleValues.reset(p - 1); }
  inline void GetPossibleOptions(std::bitset<N> &o) const { o = _possibleValues; }
  inline bool IsOption(cell_value_t i) const { return _possibleValues[i]; }
  inline cell_value_t NumOptions() const { return (cell_value_t)_possibleValues.count(); }
  inline void Reset() { _value = 0; _possibleValues.set(); }
  inline cell_value_t GetRow() const { return _row; }
  inline cell_value_t GetColumn() const { return _col; }
  inline cell_value_t GetBlock() const { return _blk; }
  inline void SetRow(cell_value_t r) { _row = r; }
  inline void SetColumn(cell_value_t r) { _col = r; }
  inline void SetBlock(cell_value_t r) { _blk = r; }
};

template <grid_size_t H, grid_size_t W = H>
class SudokuGrid {
  static const cell_value_t N = H * W;
  static const grid_work_t T = N * N;
  typedef SudokuCell<N> Cell;
  typedef std::array<Cell*, N> Group;
  typedef std::array<cell_value_t, T> GridState;
  
  std::array<Cell, T> _cells;
  std::array<std::bitset<T>, N> _rows;
  std::array<std::bitset<T>, N> _cols;
  std::array<std::bitset<T>, N> _blks;
  
  GridState _initial;
  GridState _solved;
  
protected:
  void GetCellGroups(cell_value_t i, cell_value_t &row, cell_value_t &col, cell_value_t &blk) const {
    row = i / N;
    col = i % N;
    cell_value_t bigrow = i / (N * H);
    cell_value_t bigcol = col / W;
    blk = bigrow * H + bigcol;
  }
  
  void SetState(const GridState& state) {
    for (Cell& cell : _cells) cell.Reset();
    for (cell_value_t i = 0; i < _cells.size(); ++i) {
      if (!state[i]) continue;
      cell_value_t v = state[i];
      if (_initial[i]) _cells[i].SetFixedValue(v);
      else _cells[i].SetValue(v);
      cell_value_t r, c, b;
      GetCellGroups(i, r, c, b);
      std::bitset<T> affected = _rows[r] | _cols[c] | _blks[b];
      for (cell_value_t j = 0; j < _cells.size(); ++j) {
        if (affected[j]) _cells[j].ResetOption(v);
      }
    }
  }
  
  void GetState(GridState& state) const {
    for (cell_value_t i = 0; i < _cells.size(); ++i) {
      state[i] = _cells[i].GetValue();
    }
  }
  
public:
  SudokuGrid() {
    for (cell_value_t i = 0; i < _cells.size(); ++i) {
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
      _initial[i] = v;
    }
    SetState(_initial);
  }
  
  bool IsValidState() const {
    // Check all unvalued cells have possible locations
    for (const Cell& c : _cells) {
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
  
  bool IsSolved() {
    for (Cell& c : _cells) {
      if (!c.GetValue()) return false;
    }
    return true;
  }
  
  void SolveGrid() {
    GridState current;
    GetState(current);
    
    std::vector<GridState*> to_run;
    to_run.push_back(&current);
    size_t count = 0;
    bool solved = false;
    while (to_run.size()) {
      GridState state = *to_run.back();
      to_run.pop_back();
      SetState(state);
      ++count;
      
      // Check validity and if solved
      if (!IsValidState()) continue;
      if (IsSolved() && solved) {
        std::cerr << "Grid has multiple solutions." << std::endl;
        solved = false;
        break;
      } else if (IsSolved()) {
        _solved = state;
        solved = true;
        continue;
      }
      
      // Find smallest optioned cell
      cell_value_t smallCell = T;
      for (cell_value_t cell = 0; cell < _cells.size(); ++cell) {
        if (_cells[cell].GetValue()) continue;
        if (smallCell == T) smallCell = cell;
        if (_cells[cell].NumOptions() == 1) {  // Never gonna get smaller options
          smallCell = cell;
          break;
        } else if (_cells[cell].NumOptions() < _cells[smallCell].NumOptions()) smallCell = cell;
      }
      
      // Branch on all the options
      std::bitset<N> options;
      _cells[smallCell].GetPossibleOptions(options);
      for (cell_value_t i = 0; i < options.size(); ++i) {
        if (!options[i]) continue;
        GridState *newState = new GridState();
        std::copy(state.begin(), state.end(), newState->begin());
        newState->at(smallCell) = i + 1;
        to_run.push_back(newState);
      }
      
    }
    
    if (!solved) std::cout << "No valid solution found (" << count << " iterations)." << std::endl;
    else {
      SetState(_solved);
      std::cout << "Solution found (" << count << " iterations) : " << std::endl;
      DisplayGrid();
    }
    SetState(current);
  }
  
  void DisplayGrid() const {
    for (cell_value_t row = 0; row < _rows.size(); ++row) {
      if (!(row % H)) PrintSeperatorGridLine();
      PrintRowGridLine(_rows[row]);
    }
    PrintSeperatorGridLine();
  }
  
  void DisplayState(const GridState& state) const {
    for (cell_value_t v : state) std::cout << v;
    if (IsValidState()) std::cout << " VALID!" << std::endl;
    else std::cout << " NOT VALID!" << std::endl;
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
