//
//  grid.cpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 20/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//
#include "defines.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

#include "grid.hpp"

// Default constructor creates all cells, then calls SetGroups and SetAffected
// Any classes inheriting should call this constructor in initalisation list
template<INT H, INT W, INT N>
SudokuGrid<H,W,N>::SudokuGrid() {
  for (INT i = 0; i < N; ++i) _AT(_cells, i) = Cell(i);
  SetGroups();
  SetAffected();
}

// Construct from a string. Can only handle up to 62 possible values
// 1-9 are obvious, 0 is 10, A-Z is 11 to 36, a-z are 37 - 62.
// . is treated as blank. Any other characters will throw
template<INT H, INT W, INT N>
SudokuGrid<H,W,N>::SudokuGrid(const std::string& s)
: SudokuGrid() {
  static_assert(G <= 62, "String construction can only handle 62 values");
  if (s.size() != N) throw std::length_error("Input string is incorrect length.");
  
  for (INT i = 0; i < N; ++i) {
    INT v = (INT)_AT(s, i);
    if (v == '.') continue;
    else if (v >= '1' && v <= '9') v -= 48;
    else if (v == '0') v = 10;
    else if (v >= 'A' && v <= 'Z') v -= 54;
    else if (v >= 'a' && v <= 'z') v -= 60;
    else throw std::runtime_error("Unknown characters in input string.");
    if (v > G) throw std::out_of_range("Characters in input have value greater than maximum.");
    // Set fixed value and propagate consequences
    GetCell(i).SetFixedValue(v);
    const AllCells& affect = GetAffected(i);
    for (INT j = 0; j < N; ++j) {
      if (_AT(affect, j)) GetCell(j).ResetOption(v);
    }
  }
  
  // Set Initial state
  for (Cell& c : _cells) c.SetCurrentAsInitial();
}

// Set groups default method. Only works when N = H * W * H * W
template<INT H, INT W, INT N>
void SudokuGrid<H,W,N>::SetGroups() {
  static_assert(N == G * G, "With non-regular sudokus, need to override SetGroups");
  
  // Fill the vector with the required number of groups (3G)
  _grps.clear();
  _grps.reserve(3 * G);
  _grps.insert(_grps.begin(), 3 * G, AllCells(0));
  
  // Set all cell's row, column, block
  for (Cell& cell : _cells) {
    auto rcb = GetCellGroups<H, W, N>(cell.GetIndex());
    cell.SetRow(rcb.first);
    cell.SetColumn(rcb.second);
    cell.SetBlock(rcb.third);
    _AT(_grps, rcb.first).set(cell.GetIndex());
    _AT(_grps, rcb.second + G).set(cell.GetIndex());
    _AT(_grps, rcb.third + G * 2).set(cell.GetIndex());
  }
}

template<INT H, INT W, INT N>
void SudokuGrid<H,W,N>::SetAffected() {
  static_assert(N == G * G, "With non-regular sudokus, need to override SetAffected");
  for (Cell& cell : _cells) {
    AllCells tmp = AllCells(0);
    tmp |= GetRow(cell.GetRow());
    tmp |= GetColumn(cell.GetColumn());
    tmp |= GetBlock(cell.GetBlock());
    tmp.reset(cell.GetIndex());
    _AT(_affected, cell.GetIndex()) = AllCells(tmp);
  }
}

template <INT H, INT W, INT N>
void SudokuGrid<H,W,N>::DisplayGrid() const {
  for (INT r = 0; r < G; ++r) {
    if (!(r % H)) PrintSeperatorGridLine();
    PrintRowGridLine(r);
  }
  PrintSeperatorGridLine();
}

template<INT H, INT W, INT N>
void SudokuGrid<H,W,N>::PrintSeperatorGridLine() const {
  for (INT i = 0; i < G; ++i) {
    if (!(i % W)) std::cout << "+-";
    std::cout << "--";
  }
  std::cout << "+" << std::endl;
}

template<INT H, INT W, INT N>
void SudokuGrid<H,W,N>::PrintRowGridLine(INT r) const {
  INT c = 0;
  const AllCells& row = GetRow(r);
  for (INT i = 0; i < N; ++i) {
    if (!row[i]) continue;
    if (!(c % W)) std::cout << "| ";
    INT v = GetCell(i).GetValue();
    if (v >= 1 && v <= N) std::cout << (int)v << " ";
    else std::cout << "  ";
    ++c;
  }
  std::cout << "|" << std::endl;
}

// Grid size init
#define GRID_SIZE(x,y,z)\
template class SudokuGrid<x,y,z>;

GRID_SIZE(3,3,81);
//#include "gridsizes.itm"
#undef GRID_SIZE
