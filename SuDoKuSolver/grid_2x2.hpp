//
//  grid_2x2.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#ifndef SUDOKUSOLVER_GRID_2X2_HPP
#define SUDOKUSOLVER_GRID_2X2_HPP

#include "grid.hpp"

class Grid2x2 : public SudokuGrid<2> {
  using SudokuGrid<2>::SudokuGrid;
public:

};

bool SolveGrid(Grid2x2& grid);
bool LogicallySolveGrid(Grid2x2&);

#endif /* SUDOKUSOLVER_GRID_2X2_HPP */
