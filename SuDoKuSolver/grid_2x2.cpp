//
//  grid_2x2.cpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#include "grid_2x2.hpp"
#include "solver.hpp"

bool SolveGrid(Grid2x2& grid) {
  BruteForceSolver<2> solve(grid);
  return solve.Solve();
}

bool LogicallySolveGrid(Grid2x2& grid) {
  LogicalSolver<2> solve(grid);
  return solve.Solve();
}
