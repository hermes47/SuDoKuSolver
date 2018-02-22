//
//  grid_2x3.cpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#include "grid_2x3.hpp"
#include "solver.hpp"

bool SolveGrid(Grid2x3& grid) {
  BruteForceSolver<2,3> solve(grid);
  return solve.Solve();
}

bool LogicallySolveGrid(Grid2x3& grid) {
  LogicalSolver<2,3> solver(grid);
  return solver.Solve();
}
