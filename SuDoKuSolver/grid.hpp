//
//  grid.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 20/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#ifndef SUDOKUSOLVER_GRID_HPP
#define SUDOKUSOLVER_GRID_HPP

#include "defines.hpp"

#include <array>
#include <bitset>
#include <cassert>
#include <vector>

#include "cell.hpp"

// Forward declare
template <dim_t H, dim_t W = H, val_t N = H * H * W * W>
class SudokuGrid;

template <dim_t H, dim_t W, val_t N>
class SudokuGrid {
  static const val_t G = H * W;
  
  static_assert(G <= 100, "Only support up to 100 values per group.");
  static_assert((N % G) == 0, "N must be divisible by H*W.");
  static_assert(N < 10*G*G, "Only support upto a tenfold increase in N vs G.");
  
public:
  typedef SudokuCell<G> Cell;
  typedef std::bitset<G> Values;
  typedef std::bitset<N> AllCells;
  typedef std::array<Values, N> GridState;  // should be const Values?
  
private:
  std::array<Cell, N> _cells;
  // Order of groups should be rows, columns, blocks, other
  std::vector<AllCells> _grps;
  std::array<AllCells, N> _affected;
  GridState _initial, _solved;
  val_t _num_solutions;
  
public:
  // Default constructor
  SudokuGrid();
  
  // Move constructor
  SudokuGrid(SudokuGrid&&);
  
  // Copy constructor
  SudokuGrid(const SudokuGrid&);
  
  // Assignment operator
  SudokuGrid& operator=(SudokuGrid&&);
  
  // Reset state to initial state
  void Reset();
  
  // Check if grid is in a valid state
  bool IsValid() const;
  
  // Check if grid can be solved
  bool IsSolvable();
  
  // Check if grid is solved
  bool IsSolved();
  
  // Get a cell by index. Only do bounds checking in when DEBUG defined
  inline Cell& GetCell(val_t i) { return _AT(_cells, i); }
  inline AllCells& GetGroup(val_t i) const { return _AT(_grps, i); }
  inline virtual AllCells& GetRow(val_t i) const { return GetGroup(i); }
  inline virtual AllCells& GetColumns(val_t i) const { return GetGroup(i+G); }
  inline virtual AllCells& GetBlock(val_t i) const { return GetGroup(i+G+G); }
  
  virtual void SetGroups();
  virtual void SetAffected();
  
  
};

#endif /* SUDOKUSOLVER_GRID_HPP */
