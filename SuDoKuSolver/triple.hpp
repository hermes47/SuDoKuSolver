//
//  triple.hpp
//  SuDoKuSolver
//
//  Created by Ivan Welsh on 19/02/18.
//  Copyright © 2018 Hermes Productions. All rights reserved.
//

// Implementation of a triple class, based on the stl pair class
// Code bassed on: https://gcc.gnu.org/onlinedocs/libstdc++/libstdc++-html-USERS-4.3/a02030.html

#ifndef STL_SIMILAR_TRIPLE_HPP
#define STL_SIMILAR_TRIPLE_HPP

#include <utility>

namespace std {
  
  // triple hold three objects of arbitrary type
  template<class _T1, class _T2, class _T3>
  struct triple
  {
    typedef _T1 first_type;
    typedef _T2 second_type;
    typedef _T3 third_type;
    
    _T1 first;
    _T2 second;
    _T3 third;
    
    // Default constructor creates first, second and third using respective default constructors
    triple()
    : first(), second(), third() { }
    
    // Can pass objects to be copied
    triple(const _T1& __a, const _T2& __b, const _T3& __c)
    : first(__a), second(__b), third(__c) { }
    
    // Some constructor I don't understand yet
    template<class _U1, class _U2, class _U3>
    triple(_U1&& __x, _U2&& __y, _U3&& __z)
    : first(std::forward<_U1>(__x)), second(std::forward<_U2>(__y)), third(std::forward<_U3>(__z)) {}
    
    // Move constructor
    triple(triple&& __t)
    : first(std::move(__t.first)), second(std::move(__t.second)), third(std::move(__t.third)) {}
    
    // Copy constructor
    template<class _U1, class _U2, class _U3>
    triple(const triple<_U1, _U2, _U3>& __t)
    : first(__t.first), second(__t.second), third(__t.third) { }
    
    // Another move constructor
    template<class _U1, class _U2, class _U3>
    triple(triple<_U1, _U2, _U3>&& __t)
    : first(std::move(__t.first)), second(std::move(__t.second)), third(std::move(__t.third)) {}
    
    // Assignment operator
    triple& operator=(triple&& __t) {
      first = std::move(__t.first);
      second = std::move(__t.second);
      third = std::move(__t.third);
      return *this;
    }
    
    template<class _U1, class _U2, class _U3>
    triple* operator=(triple<_U1, _U2, _U3>&& __t) {
      first = std::move(__t.first);
      second = std::move(__t.second);
      third = std::move(__t.third);
      return *this;
    }
    
    void swap(triple&& __t) {
      std::swap(first, __t.first);
      std::swap(second, __t.second);
      std::swap(third, __t.third);
    }
  };
  
  // Comparison operators
  
  // Two triples of the same type are equal iff their members are equal
  template<class _T1, class _T2, class _T3>
  inline bool operator==(const triple<_T1, _T2, _T3>& __x, const triple<_T1, _T2, _T3>& __y) {
    return __x.first == __y.first && __x.second == __y.second && __x.third == __y.third;
  }
  
  template<class _T1, class _T2, class _T3>
  inline bool operator<(const triple<_T1, _T2, _T3>& __x, const triple<_T1, _T2, _T3>& __y) {
    return __x.first < __y.first
    || (!(__y.first < __x.first) && __x.second < __y.second)
    || (!(__y.first < __x.first) && !(__y.second < __x.second) && __x.third < __y.third);
  }
  
  template<class _T1, class _T2, class _T3>
  inline bool operator!=(const triple<_T1, _T2, _T3>& __x, const triple<_T1, _T2, _T3>& __y) {
    return !(__x == __y);
  }
  
  template<class _T1, class _T2, class _T3>
  inline bool operator>(const triple<_T1, _T2, _T3>& __x, const triple<_T1, _T2, _T3>& __y) {
    return __y < __x;
  }
  
  template<class _T1, class _T2, class _T3>
  inline bool operator<=(const triple<_T1, _T2, _T3>& __x, const triple<_T1, _T2, _T3>& __y) {
    return !(__y < __x);
  }
  
  template<class _T1, class _T2, class _T3>
  inline bool operator>=(const triple<_T1, _T2, _T3>& __x, const triple<_T1, _T2, _T3>& __y) {
    return !(__x < __y);
  }
  
  // swap implementation
  template<class _T1, class _T2, class _T3>
  inline void swap(triple<_T1, _T2, _T3>& __x, triple<_T1, _T2, _T3>& __y) {
    __x.swap(__y);
  }
  
  template<class _T1, class _T2, class _T3>
  inline void swap(triple<_T1, _T2, _T3>&& __x, triple<_T1, _T2, _T3>& __y) {
    __x.swap(__y);
  }
  
  template<class _T1, class _T2, class _T3>
  inline void swap(triple<_T1, _T2, _T3>& __x, triple<_T1, _T2, _T3>&& __y) {
    __x.swap(__y);
  }
  
  // Convenience wrapper for creating a triple from three objects
  
  // Some wrapper help shenanagins
//  template<typename _Tp>
//  class reference_wrapper;
  template<typename _Tp>
  struct __strip_reference_wrapper {
    typedef _Tp __type;
  };
  template<typename _Tp>
  struct __strip_reference_wrapper<reference_wrapper<_Tp>> {
    typedef _Tp& __type;
  };
  template<typename _Tp>
  struct __strip_reference_wrapper<const reference_wrapper<_Tp>> {
    typedef _Tp& __type;
  };
  
  template<typename _Tp>
  struct __decay_and_strip {
    typedef typename __strip_reference_wrapper<typename decay<_Tp>::type>::__type __type;
  };
  
  template<class _T1, class _T2, class _T3>
  inline triple<typename __decay_and_strip<_T1>::__type,
  typename __decay_and_strip<_T2>::__type,
  typename __decay_and_strip<_T3>::__type>
  make_triple(_T1&& __x, _T2&& __y, _T3&& __z) {
    return triple<typename __decay_and_strip<_T1>::__type,
    typename __decay_and_strip<_T2>::__type,
    typename __decay_and_strip<_T3>::__type>(std::forward<_T1>(__x),
                                             std::forward<_T2>(__y),
                                             std::forward<_T3>(__z));
  }
  
}

#endif /* STL_SIMILAR_TRIPLE_HPP */
