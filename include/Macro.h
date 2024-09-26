//*****************************************************************************
//
//
//   ��ͷ�ļ���װ�˳��ú�
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
/// LIKCLY ��ķ�װ, ���߱������Ż�,��������ʳ���
#   define SYLAR_LIKELY(x)       __builtin_expect(!!(x), 1)
/// LIKCLY ��ķ�װ, ���߱������Ż�,��������ʲ�����
#   define SYLAR_UNLIKELY(x)     __builtin_expect(!!(x), 0)
#else
#   define SYLAR_LIKELY(x)      (x)
#   define SYLAR_UNLIKELY(x)      (x)
#endif

/// ���Ժ��װ
#define SYLAR_ASSERT(x) \
    if(SYLAR_UNLIKELY(!(x))) { \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n"  \
            << BacktraceToString(100, 0, "");  \
        assert(x); \
    }

/// ���Ժ��װ
#define SYLAR_ASSERT2(x, w) \
    if(SYLAR_UNLIKELY(!(x))) { \
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT()) << "ASSERTION: " #x \
            << "\n" << w << "\n" \
            << BacktraceToString(100, 0, ""); \
        assert(x); \
    }

}; /* sylar */


#endif /* SYLAR_MACRO_H */ 
