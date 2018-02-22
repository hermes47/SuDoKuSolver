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

#include "grid_3x3.hpp"

#include "grid.hpp"
#include "solver.hpp"

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
  UINT valids = 0;
  std::array<UINT, 11> counts;
  for (UINT& i : counts) i = 0;
  
  for (std::string& g : grids_3x3) {
    SudokuGrid<3> G2(g);
    LogicalSolver<3, 3, 81> solver(G2);
    if (solver.Solve()) {
     ++valids;
      G2.DisplayGridString();
    }
    auto ops = solver.LogicalOperations();
    LogicOperation M = *std::max_element(ops.begin(), ops.end());
    switch (M) {
      case LogicOperation::NAKED_SINGLE:
        ++counts[0];
        break;
      case LogicOperation::HIDDEN_SINGLE:
        ++counts[1];
        break;
      case LogicOperation::NAKED_PAIR:
        ++counts[2];
        break;
      case LogicOperation::HIDDEN_PAIR:
        ++counts[3];
        break;
      case LogicOperation::NAKED_TRIPLE:
        ++counts[4];
        break;
      case LogicOperation::HIDDEN_TRIPLE:
        ++counts[5];
        break;
      case LogicOperation::NAKED_QUAD:
        ++counts[6];
        break;
      case LogicOperation::HIDDEN_QUAD:
        ++counts[7];
        break;
      case LogicOperation::NAKED_NUPLE:
        ++counts[8];
        break;
      case LogicOperation::HIDDEN_NUPLE:
        ++counts[9];
        break;
      case LogicOperation::BRUTE_FORCE:
        ++counts[10];
        break;
      default:
        break;
    }
//    if (valids == 10) break;
  }
  
  std::cout << "Logically solved grids: " << valids << std::endl;
  std::cout << "Naked single: " << counts[0] << std::endl;
  std::cout << "Hidden single: " << counts[1] << std::endl;
  std::cout << "Naked pair: " << counts[2] << std::endl;
  std::cout << "Hidden pair: " << counts[3] << std::endl;
  std::cout << "Naked triple: " << counts[4] << std::endl;
  std::cout << "Hidden triple: " << counts[5] << std::endl;
  std::cout << "Naked quad: " << counts[6] << std::endl;
  std::cout << "Hidden quad: " << counts[7] << std::endl;
  std::cout << "Naked nuple: " << counts[8] << std::endl;
  std::cout << "Hidden nuple: " << counts[9] << std::endl;
  std::cout << "Brute Force required: " << counts[10] << std::endl;
  
  return 0;
}
