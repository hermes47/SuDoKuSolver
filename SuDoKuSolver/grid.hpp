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
#include <string>
#include <vector>

#include "cell.hpp"

// Forward declare
template <INT H, INT W = H, INT N = H * H * W * W>
class SudokuGrid;

template <INT H, INT W, INT N>
class SudokuGrid {
  static const INT G = H * W;
  
  static_assert(G <= 100, "Only support up to 100 values per group.");
  static_assert((N % G) == 0, "N must be divisible by H*W.");
  static_assert(N < 10*G*G, "Only support upto a tenfold increase in N vs G.");
  
public:
  typedef SudokuCell<G> Cell;
  typedef std_x::bitset<G> Values;
  typedef std_x::bitset<N> AllCells;
  typedef std::array<Values, N> GridState;  // should be const Values?
  
private:
  std::array<Cell, N> _cells;
  // Order of groups should be rows, columns, blocks, other
  std::vector<AllCells> _grps;
  std::array<AllCells, N> _affected;
  GridState _initial, _solved;
  INT _num_solutions;
  
public:
  // Default constructor
  SudokuGrid();
  
  SudokuGrid(const std::string&);
  
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
  inline Cell& GetCell(INT i) { return _AT(_cells, i); }
  inline const Cell& GetCell(INT i) const { return _AT(_cells, i); }
  inline const AllCells& GetGroup(INT i) const { return _AT(_grps, i); }
  inline virtual const AllCells& GetRow(INT i) const { return GetGroup(i); }
  inline virtual const AllCells& GetColumn(INT i) const { return GetGroup(i+G); }
  inline virtual const AllCells& GetBlock(INT i) const { return GetGroup(i+G+G); }
  inline const AllCells& GetAffected(INT i) const { return _AT(_affected,i); }
  inline const AllCells& GetAffected(const Cell& c) const { return _AT(_affected,c.GetIndex()); }
  
  void DisplayGrid() const;
  
private:
  virtual void SetGroups();
  virtual void SetAffected();
  
  virtual void PrintSeperatorGridLine() const;
  virtual void PrintRowGridLine(INT) const;
  
  
};

#endif /* SUDOKUSOLVER_GRID_HPP */
