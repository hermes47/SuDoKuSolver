//
//  grid_3x3.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#ifndef SUDOKUSOLVER_GRID_3X3_HPP
#define SUDOKUSOLVER_GRID_3X3_HPP

#include "grid_old.hpp"

class Grid3x3 : public _SudokuGrid<3> {
  using _SudokuGrid<3>::_SudokuGrid;
public:
  
};

bool SolveGrid(Grid3x3&);
bool LogicallySolveGrid(Grid3x3&, bool=false);

#endif /* SUDOKUSOLVER_GRID_3X3_HPP */
