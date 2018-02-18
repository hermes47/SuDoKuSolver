//
//  logical_solver.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 17/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

#ifndef SUDOKUSOLVER_LOGICAL_SOLVER_HPP
#define SUDOKUSOLVER_LOGICAL_SOLVER_HPP

#include <iterator>
#include <map>
#include <sstream>
#include <tuple>
#include <vector>

#include "combinations.hpp"
#include "grid.hpp"

enum Action {
  SET_VALUE,
  CLEAR_VALUE,
  NUM_ACTIONS
};

typedef values_t Value;
typedef values_t CellIndex;
typedef size_t ActionGroup;
typedef std::tuple<Action, Value, CellIndex, ActionGroup> Actionable;
typedef std::vector<Actionable> Actions;

template <dimension_t Height, dimension_t Width>
class LogicalSolver {
  static const values_t num_vals = Height * Width;
  static const work_t num_cells = num_vals * num_vals;
  const std::map<values_t, std::string> nuple_names = {
    { 2, "PAIR" },
    { 3, "TRIPLE" },
    { 4, "QUAD" },
    { 5, "QUINT" },
    { 6, "HEXTUPLE" },
    { 7, "HEPTUPLE" },
    { 8, "OCTUPLE" },
    { 9, "NONUPLE" },
    {10, "DECUPLE" },
  };
  
  typedef SudokuGrid<Height, Width>& Grid;
  typedef SudokuCell<num_vals> Cell;
  typedef std::bitset<num_vals> Possibles;
  typedef std::bitset<num_cells> AllCells;
  typedef std::vector<Cell*> Group;
  typedef std::array<Group, 3 * num_vals> AllGroups;
  
  Grid _g;
  AllGroups _grps;
  Actions _acts;
  ActionGroup _group;
  size_t _next_act;
  bool _quiet;
  
private:
  LogicalSolver() = default;
public:
  LogicalSolver(Grid);
  
  bool PerformLogicalSolving(bool = false);
  
private:
  void HandleActions();
  bool NakedSingles();
  bool HiddenSingles();
  bool NakedNuples(values_t);
  bool HiddenNuples(values_t);
  
  values_t DetermineSingleValue(Possibles&);
  void DetermineSingleAffect(Cell&, values_t);
  
  template <typename iterator_t>
  Possibles DetermineCombinedOptions(iterator_t, iterator_t);
  
  void GetNakedNupleHeader(values_t, values_t, std::stringstream&);
  
  template <typename iterator_t>
  void GetNupleCombination(iterator_t, iterator_t, std::stringstream&);
  
};

#include "logical_solver.cpp"
#endif /* SUDOKUSOLVER_LOGICAL_SOLVER_HPP */
