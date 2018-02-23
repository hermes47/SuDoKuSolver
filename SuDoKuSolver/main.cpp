//
//  main.cpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <chrono>

#include "grid.hpp"
#include "solver.hpp"

using namespace std;

int main(int argc, const char * argv[]) {
  std::vector<std::string> grids_3x3;
  std::ifstream infile("solveable.txt");
  std::ofstream outfile("solveable_dat.txt");
  std::string grid;
  while (std::getline(infile, grid)) {
    if (grid[0] == '#') {
      outfile << grid << std::endl;
      continue;
    }
    grids_3x3.emplace_back(grid.substr(0,81));
  }
  infile.close();
  
  //  grids_3x3.clear();
  //  grids_3x3.push_back("...921..3..9....6.......5...8.4.3..6..7...8..5..7...4...3.......2....7..8..195...");
  typedef std::chrono::high_resolution_clock::time_point Time;
  typedef std::chrono::high_resolution_clock::duration Duration;
  
  std::set<std_x::triple<Duration, std::string, UINT>> uniques;
//  UINT valids = 0;
  std::array<UINT, 13> counts;
  for (UINT& i : counts) i = 0;
  
  for (std::string& g : grids_3x3) {
    Time start = std::chrono::high_resolution_clock::now();
    SudokuGrid<3> G2(g);
    if (!G2.IsSolvable()) {
      std::cout << "Bad brute force solver" << std::endl;
      continue;
    }
    Time end = std::chrono::high_resolution_clock::now();
    uniques.emplace(end - start, g, G2.GetScore());
    
    //    if (!G2.IsSolvable()) {
    //      std::cout << G2 << std::endl;
    //      continue;
    //    }
    //    std::cout << G2 << std::endl;
//    LogicalSolver<3> solver(G2);
//    solver.Solve();
//    auto ops = solver.LogicalOperations();
//    if (*std::max_element(ops.begin(), ops.end()) == LogicOperation::BRUTE_FORCE) continue;
//    switch (*std::max_element(ops.begin(), ops.end())) {
//      case LogicOperation::NAKED_SINGLE:
//        uniques.emplace(end - start, g, "NAKED SINGLE");
//        break;
//      case LogicOperation::HIDDEN_SINGLE:
//        uniques.emplace(end - start, g, "HIDDEN SINGLE");
//        break;
//      case LogicOperation::NAKED_PAIR:
//        uniques.emplace(end - start, g, "NAKED PAIR");
//        break;
//      case LogicOperation::HIDDEN_PAIR:
//        uniques.emplace(end - start, g, "HIDDEN PAIR");
//        break;
//      case LogicOperation::NAKED_TRIPLE:
//        uniques.emplace(end - start, g, "NAKED TRIPLE");
//        break;
//      case LogicOperation::HIDDEN_TRIPLE:
//        uniques.emplace(end - start, g, "HIDDEN TRIPLE");
//        break;
//      case LogicOperation::NAKED_QUAD:
//        uniques.emplace(end - start, g, "NAKED QUAD");
//        break;
//      case LogicOperation::HIDDEN_QUAD:
//        uniques.emplace(end - start, g, "HIDDEN QUAD");
//        break;
//      case LogicOperation::NAKED_NUPLE:
//        uniques.emplace(end - start, g, "NAKED NUPLE");
//        break;
//      case LogicOperation::HIDDEN_NUPLE:
//        uniques.emplace(end - start, g, "HIDDEN NUPLE");
//        break;
//      case LogicOperation::INTERSECTION_REMOVAL:
//        uniques.emplace(end - start, g, "INTERSECTION REMOVAL");
//        break;
//      case LogicOperation::BRUTE_FORCE:
//        uniques.emplace(end - start, g, "");
//        break;
//      default:
//        break;
//    }
    
    //    if (valids == 1) break;
//    switch (*std::max_element(solver.LogicalOperations().begin(), solver.LogicalOperations().end())) {
//      case LogicOperation::NAKED_SINGLE:
//        ++counts[0];
//        break;
//      case LogicOperation::HIDDEN_SINGLE:
//        ++counts[1];
//        break;
//      case LogicOperation::NAKED_PAIR:
//        ++counts[2];
//        break;
//      case LogicOperation::HIDDEN_PAIR:
//        ++counts[3];
//        break;
//      case LogicOperation::NAKED_TRIPLE:
//        ++counts[4];
//        break;
//      case LogicOperation::HIDDEN_TRIPLE:
//        ++counts[5];
//        break;
//      case LogicOperation::NAKED_QUAD:
//        ++counts[6];
//        break;
//      case LogicOperation::HIDDEN_QUAD:
//        ++counts[7];
//        break;
//      case LogicOperation::NAKED_NUPLE:
//        ++counts[8];
//        break;
//      case LogicOperation::HIDDEN_NUPLE:
//        ++counts[9];
//        break;
//      case LogicOperation::INTERSECTION_REMOVAL:
//        ++counts[10];
//        break;
//      case LogicOperation::BRUTE_FORCE:
//        ++counts[11];
//        if (solver.LogicalOperations().size() == 1) ++counts[12];
//        break;
//      default:
//        break;
//    }
//    if (uniques.size() == 10) break;
  }
  uint64_t runtime = 0;
  for (auto& s : uniques) {
    auto t = std::chrono::duration_cast<std::chrono::microseconds>(s.first).count();
    runtime += t;
    auto dots = std::count(s.second.begin(), s.second.end(), '.');
    outfile << s.second << " # " << std::setfill('0') << std::setw(3)
    << (t % 1000000000) / 1000000 << "::" << std::setw(3)
    << (t % 1000000) / 1000 << "::" << std::setw(3)
    << t % 1000 << " " << 81 - dots << " " << s.third << std::endl;
  }
  outfile << "# Total time: " << std::setfill('0') << std::setw(3)
  << (runtime % 1000000000) / 1000000 << "::" << std::setw(3)
  << (runtime % 1000000) / 1000 << "::" << std::setw(3)
  << runtime % 1000 << std::endl;
  outfile.close();
  
  return 0;
}
