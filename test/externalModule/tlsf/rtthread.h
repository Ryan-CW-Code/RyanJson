#pragma once

// !这个文件仅为了tlsf的测试
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include <stddef.h>
#include <assert.h>

#define rt_memcpy        memcpy
#define RT_ASSERT(EX)    assert(EX)
#define rt_always_inline static inline __attribute__((always_inline))
