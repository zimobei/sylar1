//*****************************************************************************
//
//
//   此头文件封装了常用宏
//  
//
//*****************************************************************************

#ifndef SYLAR_MACRO_H 
#define SYLAR_MACRO_H

#include "Log.h"
#include "Util.h"
#include <string>
#include <cassert>

namespace sylar
{

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#   define SYLAR_LIKELY(x)       __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#   define SYLAR_UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#   define SYLAR_LIKELY(x)      (x)
#   define SYLAR_UNLIKELY(x)      (x)
#endif

/// 断言宏封装
#define SYLAR_ASSERT(x) \
    if(SYLAR_UNLIKELY(!(x))) { \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n"  \
            << BacktraceToString(100, 0, "");  \
        assert(x); \
    }

/// 断言宏封装
#define SYLAR_ASSERT2(x, w) \
    if(SYLAR_UNLIKELY(!(x))) { \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w << "\n" \
            << BacktraceToString(100, 0, ""); \
        assert(x); \
    }

}; /* sylar */


#endif /* SYLAR_MACRO_H */ 
