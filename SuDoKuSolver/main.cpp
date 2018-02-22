//
//  main.cpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//
#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <time.h>

#include "grid_2x2.hpp"
#include "grid_2x3.hpp"
#include "grid_3x3.hpp"

#include "grid.hpp"
#include "solver_new.hpp"

using namespace std;

int main(int argc, const char * argv[]) {
  std::vector<std::string> grids_3x3;
  std::ifstream infile("solveable.txt");
  std::string grid;
  while (std::getline(infile, grid)) {
    if (grid[0] == '#') continue;
    grids_3x3.emplace_back(grid.substr(0,81));
  }
  
  std::set<std::string> uniques;
  UINT dups = 0, mults = 0, nots = 0, valids = 0;
  for (std::string& g : grids_3x3) {
    if (uniques.find(g) != uniques.end()) {
      ++dups;
      continue;
    } else uniques.emplace(g);
    SudokuGrid<3> G2(g);
    std::array<std_x::bitset<9>, 81> solution;
    UINT solution_count = 0;
    SolveGridNew<3, 3, 81>(G2._initial, G2._grps, G2._affected, solution, solution_count, true, 2);
    if (solution_count == 1) {
      ++valids;
    } else if (solution_count > 1) ++mults;
    else ++nots;
    if (valids == 10) break;
  }
  
  std::cout << "Valid grids: " << valids << std::endl;
  std::cout << "Unsolveable grids: " << nots << std::endl;
  std::cout << "Multiple solution grids: " << mults << std::endl;
  std::cout << "Duplicate solutions: " << dups << std::endl;
  
  return 0;
}
