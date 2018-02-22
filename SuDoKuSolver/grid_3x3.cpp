//
//  grid_3x3.cpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#include "grid_3x3.hpp"
#include "solver.hpp"

bool SolveGrid(Grid3x3& grid) {
  BruteForceSolver<3> solve(grid);
  return solve.Solve();
}

bool LogicallySolveGrid(Grid3x3& grid, bool quiet) {
  LogicalSolver<3> solve(grid);
  return solve.Solve();
}
