//
//  grid_2x2.cpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#include "grid_2x2.hpp"
#include "logical_solver.hpp"
#include "solver.hpp"

bool SolveGrid(Grid2x2& grid) {
  return SolveGrid<2,2>(grid);
}

bool LogicallySolveGrid(Grid2x2& grid) {
  return LogicallySolveGrid<2,2>(grid);
}
