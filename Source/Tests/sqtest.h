/*
Copyright (c) 2011 James Whitworth

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/
#ifndef _SQTEST_H_
#define _SQTEST_H_

#ifndef _SQ64
#define _SQ64
#endif

#include "squirrel.h"

/*
 * \def SQTEST_API
 * \brief Define SQTEST_EXPORTS if building a dll version of squirreltest.
 */
#if !defined(SQTEST_API)
# if defined(WIN32)
#  if !defined(SQTEST_STATIC)
#   if defined(SQTEST_EXPORTS)
#    define SQTEST_API __declspec(dllexport)
#   else
#    define SQTEST_API __declspec(dllimport)
#   endif /* defined(SQTEST_EXPORTS) */
#  else
#   define SQTEST_API extern
#  endif /* !defined(SQTEST_STATIC) */
# else
#  define SQTEST_API extern
# endif /* defined(WIN32) */
#endif /* defined(SQTEST_API) */

#ifdef __cplusplus
extern "C" {
#endif

SQTEST_API SQInteger sqtest_expect_true(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_assert_true(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_expect_false(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_assert_false(HSQUIRRELVM vm);

SQTEST_API SQInteger sqtest_expect_eq(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_assert_eq(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_expect_ne(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_assert_ne(HSQUIRRELVM vm);

SQTEST_API SQInteger sqtest_expect_lt(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_assert_lt(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_expect_le(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_assert_le(HSQUIRRELVM vm);

SQTEST_API SQInteger sqtest_expect_gt(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_assert_gt(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_expect_ge(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_assert_ge(HSQUIRRELVM vm);

SQTEST_API SQInteger sqtest_expect_streq(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_assert_streq(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_expect_strne(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_assert_strne(HSQUIRRELVM vm);

SQTEST_API SQInteger sqtest_expect_strcaseeq(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_assert_strcaseeq(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_expect_strcasene(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_assert_strcasene(HSQUIRRELVM vm);

SQTEST_API SQInteger sqtest_expect_float_eq(HSQUIRRELVM vm);
SQTEST_API SQInteger sqtest_assert_float_eq(HSQUIRRELVM vm);

SQTEST_API void sqtest_addtest(HSQUIRRELVM vm, const SQChar *filename);

SQTEST_API SQRESULT sqtest_register_lib(HSQUIRRELVM vm);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* _SQTEST_H_ */
