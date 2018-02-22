//
//  combinations.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 18/02/18.
//
//  Derived from: http://coliru.stacked-crooked.com/view?id=c11dc445d3f7d49a415e3aa0478d7ba2-542192d2d8aca3c820c7acc656fa0c68
//

#ifndef SUDOKUSOLVER_COMBINATIONS_HPP
#define SUDOKUSOLVER_COMBINATIONS_HPP

#include <algorithm>
#include <cstdint>
#include <vector>

template<class iterator_t>
class Combinations {
  iterator_t _begin, _end;
  std::vector<bool> _use;
  uint64_t _r;
  bool _is_done;
  
public:
  Combinations(iterator_t begin, iterator_t end, uint64_t r)
  : _begin(begin), _end(end) , _r(r), _is_done(false)
  {
    _use.resize(std::distance(_begin, _end), false);
    if (_use.size() >= _r) std::fill(_use.end() - _r, _use.end(), true);
  }
  template<class output_it>
  bool operator()(output_it result)
  {
    if (_use.size() < _r) return false;
    
    iterator_t c = _begin;
    for (uint64_t i = 0; i < _use.size(); ++i,++c) {
      if (_use[i]) *result++ = *c;
    }
    if (!_is_done && !std::next_permutation(_use.begin(), _use.end())) {
      _is_done = true;
      return true;
    } else if (!_is_done) return true;
    else return false;
  }
  
  template<class output_it>
  bool operator()(output_it result, output_it negative_result)
  {
    if (_use.size() < _r) return false;
    
    iterator_t c = _begin;
    for (uint64_t i = 0; i < _use.size(); ++i,++c) {
      if (_use[i]) *result++ = *c;
      else *negative_result++ = *c;
    }
    if (!_is_done && !std::next_permutation(_use.begin(), _use.end())) {
      _is_done = true;
      return true;
    } else if (!_is_done) return true;
    else return false;
  }
};

template<class iterator_t>
Combinations<iterator_t> MakeCombinations(iterator_t begin, iterator_t end, uint64_t r)
{
  return Combinations<iterator_t>(begin, end, r);
}

#endif /* SUDOKUSOLVER_COMBINATIONS_HPP */
