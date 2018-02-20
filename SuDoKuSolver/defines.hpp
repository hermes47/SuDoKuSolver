
#ifndef SUDOKUSOLVER_DEFINES_HPP
#define SUDOKUSOLVER_DEFINES_HPP

#include <cassert>
#include <cstdint>

typedef int32_t dim_t;        // Type used for cell values
typedef int32_t val_t;        // Type used for cell manipulation

// Bounds checking only when debug mode
#ifdef DEBUG
#define _AT(x,i)\
x.at(i)
#else
#define _AT(x,i)\
x[i]
#endif

#endif /* SUDOKUSOLVER_DEFINES_HPP */
