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

#include "spdlog/spdlog.h"

#include "grid.hpp"
#include "solver.hpp"

int main(int argc, const char * argv[]) {
  auto log = spdlog::stderr_logger_st("logger");
  spdlog::set_level(spdlog::level::debug);
  log->set_pattern("%v");
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
  log->info("{} grids loaded from file.", grids_3x3.size());
  
  //  grids_3x3.clear();
  //  grids_3x3.push_back("...921..3..9....6.......5...8.4.3..6..7...8..5..7...4...3.......2....7..8..195...");
  typedef std::chrono::high_resolution_clock::time_point Time;
  typedef std::chrono::high_resolution_clock::duration Duration;
  
  std::set<std_x::triple<Duration, std::string, UINT>> uniques;
//  UINT valids = 0;
  std::array<UINT, 14> counts;
  for (UINT& i : counts) i = 0;
  
  for (std::string& g : grids_3x3) {
    SudokuGrid<3> G2(g);
    LogicalSolver<3> solver(G2);
    solver.Solve();
    Time start = std::chrono::high_resolution_clock::now();
    UINT s = G2.GetScore();
    Time end = std::chrono::high_resolution_clock::now();
    uniques.emplace(end - start, g, s);
    auto ops = solver.LogicalOperations();
/*    if (!G2.IsSolvable()) {
      std::cout << G2 << std::endl;
      continue;
    }
    std::cout << G2 << std::endl;
    LogicalSolver<3> solver(G2);
    solver.Solve();
    auto ops = solver.LogicalOperations();
    if (*std::max_element(ops.begin(), ops.end()) == LogicOperation::BRUTE_FORCE) continue;
    switch (*std::max_element(ops.begin(), ops.end())) {
      case LogicOperation::NAKED_SINGLE:
        uniques.emplace(end - start, g, "NAKED SINGLE");
        break;
      case LogicOperation::HIDDEN_SINGLE:
        uniques.emplace(end - start, g, "HIDDEN SINGLE");
        break;
      case LogicOperation::NAKED_PAIR:
        uniques.emplace(end - start, g, "NAKED PAIR");
        break;
      case LogicOperation::HIDDEN_PAIR:
        uniques.emplace(end - start, g, "HIDDEN PAIR");
        break;
      case LogicOperation::NAKED_TRIPLE:
        uniques.emplace(end - start, g, "NAKED TRIPLE");
        break;
      case LogicOperation::HIDDEN_TRIPLE:
        uniques.emplace(end - start, g, "HIDDEN TRIPLE");
        break;
      case LogicOperation::NAKED_QUAD:
        uniques.emplace(end - start, g, "NAKED QUAD");
        break;
      case LogicOperation::HIDDEN_QUAD:
        uniques.emplace(end - start, g, "HIDDEN QUAD");
        break;
      case LogicOperation::NAKED_NUPLE:
        uniques.emplace(end - start, g, "NAKED NUPLE");
        break;
      case LogicOperation::HIDDEN_NUPLE:
        uniques.emplace(end - start, g, "HIDDEN NUPLE");
        break;
      case LogicOperation::INTERSECTION_REMOVAL:
        uniques.emplace(end - start, g, "INTERSECTION REMOVAL");
        break;
      case LogicOperation::BRUTE_FORCE:
        uniques.emplace(end - start, g, "");
        break;
      default:
        break;
    }
    
    //    if (valids == 1) break;
 */
    switch (*std::max_element(ops.begin(), ops.end())) {
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
      case LogicOperation::INTERSECTION_REMOVAL:
        ++counts[10];
        break;
      case LogicOperation::PATTERN_OVERLAY:
        ++counts[11];
        break;
      case LogicOperation::BRUTE_FORCE:
        ++counts[12];
        if (solver.LogicalOperations().size() == 1) ++counts[13];
        break;
      default:
        break;
    }
    if (uniques.size() == 1) break;
  }
  UINT max_Score = 0, min_score = 20000;
  uint64_t runtime = 0;
  for (auto& s : uniques) {
    auto t = std::chrono::duration_cast<std::chrono::microseconds>(s.first).count();
    runtime += t;
    auto dots = std::count(s.second.begin(), s.second.end(), '.');
    outfile << s.second << " # " << std::setfill('0') << std::setw(3)
    << (t % 1000000000) / 1000000 << "::" << std::setw(3)
    << (t % 1000000) / 1000 << "::" << std::setw(3)
    << t % 1000 << " " << 81 - dots << " " << s.third << std::endl;
    if (s.third > max_Score) max_Score = s.third;
    if (s.third < min_score) min_score = s.third;
  }
  outfile << "# Total time: " << std::setfill('0') << std::setw(3)
  << (runtime % 1000000000) / 1000000 << "::" << std::setw(3)
  << (runtime % 1000000) / 1000 << "::" << std::setw(3)
  << runtime % 1000 << std::endl;
  outfile.close();
  
  std::cout << "Minimum score: " << min_score << std::endl;
  std::cout << "Maximum score: " << max_Score << std::endl;
  std::cout << "Naked singles: " << counts[0] << std::endl;
  std::cout << "Hidden singles: " << counts[1] << std::endl;
  std::cout << "Naked pairs: " << counts[2] << std::endl;
  std::cout << "Hidden pairs: " << counts[3] << std::endl;
  std::cout << "Naked triples: " << counts[4] << std::endl;
  std::cout << "Hidden triples: " << counts[5] << std::endl;
  std::cout << "Naked quads: " << counts[6] << std::endl;
  std::cout << "Hidden quads: " << counts[7] << std::endl;
  std::cout << "Naked nuples: " << counts[8] << std::endl;
  std::cout << "Hidden nuples: " << counts[9] << std::endl;
  std::cout << "Intersection removal: " << counts[10] << std::endl;
  std::cout << "Pattern overlay: " << counts[11] << std::endl;
  std::cout << "Brute force: " << counts[12] << std::endl;
  std::cout << "Brute force only: " << counts[13] << std::endl;
  
  return 0;
}
