//
//  grid_2x3.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#ifndef SUDOKUSOLVER_GRID_2X3_HPP
#define SUDOKUSOLVER_GRID_2X3_HPP

#include "grid_old.hpp"

class Grid2x3 : public _SudokuGrid<2,3> {
  using _SudokuGrid<2,3>::_SudokuGrid;
public:
  
};

bool SolveGrid(Grid2x3& grid);
bool LogicallySolveGrid(Grid2x3&);

#endif /* SUDOKUSOLVER_GRID_2X3_HPP */
