//
//  grid.cpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 19/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#include "grid_old.hpp"

template<dimension_t H, dimension_t W>
void _SudokuGrid<H,W>::PrintSeperatorGridLine() const {
  for (values_t i = 0; i < num_vals; ++i) {
    if (!(i % W)) std::cout << "+-";
    std::cout << "--";
  }
  std::cout << "+" << std::endl;
}
      
template<dimension_t H, dimension_t W>
void _SudokuGrid<H,W>::PrintRowGridLine(const AllCells &row) const {
  values_t col = 0;
  for (values_t i = 0; i < row.size(); ++i) {
    if (!row[i]) continue;
    if (!(col % W)) std::cout << "| ";
    if (_cells[i].GetValue()) std::cout << (int)_cells[i].GetValue() << " ";
    else std::cout << "  ";
    ++col;
  }
  std::cout << "|" << std::endl;
}

// Grid size init
#define GRID_SIZE(x,y)\
template class _SudokuGrid<x,y>;

#include "gridsizes.itm"
#undef GRID_SIZE

