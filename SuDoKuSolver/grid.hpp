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
  
public:
  std::array<Cell, N> _cells;
  // Order of groups should be rows, columns, blocks, other
  std::vector<AllCells> _grps;
  std::array<AllCells, N> _affected;
  GridState _initial, _solved;
  INT _num_solutions = -1;
  
private:
  // Default constructor
  SudokuGrid();
  
public:
  
  SudokuGrid(const std::string&);
  
  // Move constructor
  SudokuGrid(SudokuGrid&&);
  
  // Copy constructor
  SudokuGrid(const SudokuGrid&);
  
  // Assignment operator
  SudokuGrid& operator=(SudokuGrid&&);
  
  // Get a cell by index. Only do bounds checking in when DEBUG defined
  inline Cell& GetCell(INT i) { return _AT(_cells, i); }
  inline const Cell& GetCell(INT i) const { return _AT(_cells, i); }
  inline const AllCells& GetGroup(INT i) const { return _AT(_grps, i); }
  inline virtual const AllCells& GetRow(INT i) const { return GetGroup(i); }
  inline virtual const AllCells& GetColumn(INT i) const { return GetGroup(i+G); }
  inline virtual const AllCells& GetBlock(INT i) const { return GetGroup(i+G+G); }
  
  // Get the set of cells affected by a given cell being set
  inline const AllCells& GetAffected(INT i) const { return _AT(_affected,i); }
  inline const AllCells& GetAffected(const Cell& c) const { return _AT(_affected,c.GetIndex()); }
  
  // Display the grid
  void DisplayGrid() const;
  void DisplayGridString() const;
  
  // State stuff
  inline const GridState& GetInitialState() const { return _initial; }
  bool SetState(const GridState&);
  void Reset();
  const GridState& GetSolvedState();
  // Check if grid is in a valid state
  bool IsValid() const;
  // Check if grid can be solved
  bool IsSolvable() const;
  // Check if grid is solved
  bool IsSolved() const;
  
private:
  virtual void SetGroups();
  virtual void SetAffected();
  
  virtual void PrintSeperatorGridLine() const;
  virtual void PrintRowGridLine(INT) const;
  
  
};

#endif /* SUDOKUSOLVER_GRID_HPP */
