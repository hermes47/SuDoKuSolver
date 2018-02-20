//
//  solver.cpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 19/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#include "solver.hpp"

#define GRID_SIZE(x,y)\
template bool SolveGrid<x,y>(_SudokuGrid<x,y>&, bool, size_t);

#include "gridsizes.itm"
#undef GRID_SIZE
