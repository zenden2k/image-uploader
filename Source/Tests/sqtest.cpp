/*
	see copyright notice in sqtest.h
*/
#include "sqtest.h"

#include <list>
#include <string>

#include <sqstdio.h>

#include <gtest/gtest.h>
#include "Core/Scripting/API/ScriptAPI.h"

//----------------------------------------------------------------------------------------------------------------------
/// \def SQTEST_OVERRIDE
/// \brief
//----------------------------------------------------------------------------------------------------------------------
#ifdef _MSC_VER
// disable nonstandard extension used: override specifier 'override' warning
# pragma warning(disable: 4481)
# define SQTEST_OVERRIDE override
#else
# define SQTEST_OVERRIDE
#endif

//----------------------------------------------------------------------------------------------------------------------
/// \def SQTEST_UNUSED
/// \brief Marks a function argument as unused
//----------------------------------------------------------------------------------------------------------------------
#define SQTEST_UNUSED(X)

//----------------------------------------------------------------------------------------------------------------------
/// \def SQTEST_WARNING_PUSH
/// \brief Used to disable msvc compiler warnings within macros.
//----------------------------------------------------------------------------------------------------------------------
#if defined(_MSC_VER)
# define SQTEST_WARNING_PUSH(SQTEST_WARNINGS) \
  __pragma(warning(push)) \
  __pragma(warning(SQTEST_WARNINGS))
#else
# define SQTEST_WARNING_PUSH(SQTEST_WARNINGS)
#endif // defined(_MSC_VER)

//----------------------------------------------------------------------------------------------------------------------
/// \def SQTEST_WARNING_POP
/// \brief Used to disable msvc compiler warnings within macros.
//----------------------------------------------------------------------------------------------------------------------
#if defined(_MSC_VER)
# define SQTEST_WARNING_POP() \
  __pragma(warning(pop))
#else
# define SQTEST_WARNING_POP()
#endif // defined(_MSC_VER)

typedef std::basic_string<SQChar, std::char_traits<SQChar>, std::allocator<SQChar> > string;

const SQChar *kGTestFatalFailureString = _SC("gtest-fatal-failure");

//----------------------------------------------------------------------------------------------------------------------
/// \brief ScriptTest
//----------------------------------------------------------------------------------------------------------------------
class ScriptTest : public ::testing::Test
{
public:
  ScriptTest(HSQUIRRELVM vm, const SQChar *filename);
  ~ScriptTest();

  virtual void SetUp() SQTEST_OVERRIDE;
  virtual void TearDown() SQTEST_OVERRIDE;

  virtual void TestBody() SQTEST_OVERRIDE;

  /// \brief Returns the text used to call an expression on a given line within the script file.
  /// \code
  /// // file contains:  expect_true(condition())
  /// // called from within sqtest_expect_true
  /// // buffer == "condition()"
  /// ScriptTest::GetExpressionText(vm, "example.nut", 1, buffer, 128);
  /// \endcode
  /// The function name is found by using the squirrel debug stack info functions.
  /// /// \note Won't work with more than one test func of the same name on the same line.
  static bool GetPredicateText1(HSQUIRRELVM vm, const SQChar *file, size_t line);
  static bool GetPredicateText2(HSQUIRRELVM vm, const SQChar *file, size_t line);

protected:
  HSQUIRRELVM                 m_vm;
  const char                 *m_filename;
  char                       *m_contents;
  char                      **m_lines;
  size_t                      m_lineCount;

  /// \brief Reads in an ascii script file and breaks it up into lines
  bool ReadFile(const SQChar *filename);
};

//----------------------------------------------------------------------------------------------------------------------
/// \brief ScriptTestFactory
//----------------------------------------------------------------------------------------------------------------------
class ScriptTestFactory : public ::testing::internal::TestFactoryBase
{
public:
  ScriptTestFactory(HSQUIRRELVM vm, const char* filename);
  virtual ~ScriptTestFactory();

  virtual ::testing::Test* CreateTest() SQTEST_OVERRIDE;

protected:
  HSQUIRRELVM m_vm;
  string      m_filename;
};

//----------------------------------------------------------------------------------------------------------------------
/// \brief List of all script tests to run
//----------------------------------------------------------------------------------------------------------------------
std::list<ScriptTest> g_testcases;

//----------------------------------------------------------------------------------------------------------------------
#define SQTEST_IMPL_BOOL_FUNC(func_name, expected_bool, failure_type) \
  SQInteger sqtest_ ## func_name (HSQUIRRELVM vm) \
  { \
    SQBool result = SQTrue; \
    sq_getbool(vm, 2, &result); \
    if (const ::testing::AssertionResult gtest_ar_ = ::testing::AssertionResult((result == expected_bool) ? true : false)) \
    { \
      /* do nothing */ \
    } \
    else \
    { \
      SQStackInfos si; \
      sq_stackinfos(vm, 1, &si); \
       \
      bool result = ScriptTest::GetPredicateText1(vm, si.source, si.line); \
       \
      const SQChar* expression_text = _SC(""); \
      if (result) \
      { \
        sq_getstring(vm, -1, &expression_text); \
      } \
       \
      /* log the failure */ \
      std::string message = \
        ::testing::internal::GetBoolAssertionFailureMessage(gtest_ar_, expression_text, (expected_bool == SQTrue) ? "false" : "true", (expected_bool == SQTrue) ? "true" : "false"); \
      ::testing::internal::AssertHelper(failure_type, si.source, static_cast<int>(si.line), message.c_str()) = ::testing::Message(); \
       \
      if (result) \
      { \
        sq_pop(vm, 1); \
      } \
       \
      SQTEST_WARNING_PUSH(disable : 4127) \
      if (failure_type == ::testing::TestPartResult::kFatalFailure) \
      SQTEST_WARNING_POP() \
      { \
        return sq_throwerror(vm, kGTestFatalFailureString); \
      } \
    } \
     \
    return 0; \
  }

SQTEST_IMPL_BOOL_FUNC(expect_true,  SQTrue,  ::testing::TestPartResult::kNonFatalFailure);
SQTEST_IMPL_BOOL_FUNC(assert_true,  SQTrue,  ::testing::TestPartResult::kFatalFailure);
SQTEST_IMPL_BOOL_FUNC(expect_false, SQFalse, ::testing::TestPartResult::kNonFatalFailure);
SQTEST_IMPL_BOOL_FUNC(assert_false, SQFalse, ::testing::TestPartResult::kFatalFailure);

//----------------------------------------------------------------------------------------------------------------------
#define SQTEST_IMPL_CMP_FUNC(op_name, op_type, failure_type) \
  SQInteger sqtest_ ## op_name (HSQUIRRELVM vm) \
  { \
    if (0 op_type sq_cmp(vm)) \
                { \
                } \
                                else \
    { \
      SQStackInfos si; \
      sq_stackinfos(vm, 1, &si); \
      bool result = ScriptTest::GetPredicateText2(vm, si.source, si.line); \
       \
      const SQChar* expected_expression_text = _SC(""); \
      const SQChar* actual_expression_text = _SC(""); \
      if (result) \
                        { \
        sq_getstring(vm, -2, &expected_expression_text); \
        sq_getstring(vm, -1, &actual_expression_text); \
                        } \
       \
      sq_tostring(vm, 2); \
      const SQChar* expected_value_text = _SC(""); \
      sq_getstring(vm, -1, &expected_value_text); \
       \
      sq_tostring(vm, 3); \
      const SQChar* actual_value_text = _SC(""); \
      sq_getstring(vm, -1, &actual_value_text); \
       \
       char buffer[1024];\
       sprintf(buffer, "Expected: (%s) " #op_type " (%s), actual %s vs %s", \
        expected_expression_text, \
        actual_expression_text, \
        expected_value_text, \
        actual_value_text); \
        std::string message = buffer;\
      ::testing::internal::AssertHelper(failure_type, si.source, static_cast<int>(si.line), message.c_str()) = ::testing::Message(); \
       \
      SQInteger count = 2; \
      if (result) \
      { \
        count += 2; \
      } \
      sq_pop(vm, count); \
       \
      SQTEST_WARNING_PUSH(disable : 4127) \
      if (failure_type == ::testing::TestPartResult::kFatalFailure) \
      SQTEST_WARNING_POP() \
      { \
        return sq_throwerror(vm, kGTestFatalFailureString); \
      } \
    } \
    return 0; \
  }

SQTEST_IMPL_CMP_FUNC(expect_eq, ==, ::testing::TestPartResult::kNonFatalFailure);
SQTEST_IMPL_CMP_FUNC(assert_eq, ==, ::testing::TestPartResult::kFatalFailure);
SQTEST_IMPL_CMP_FUNC(expect_ne, !=, ::testing::TestPartResult::kNonFatalFailure);
SQTEST_IMPL_CMP_FUNC(assert_ne, !=, ::testing::TestPartResult::kFatalFailure);
SQTEST_IMPL_CMP_FUNC(expect_lt, < , ::testing::TestPartResult::kNonFatalFailure);
SQTEST_IMPL_CMP_FUNC(assert_lt, < , ::testing::TestPartResult::kFatalFailure);
SQTEST_IMPL_CMP_FUNC(expect_le, <=, ::testing::TestPartResult::kNonFatalFailure);
SQTEST_IMPL_CMP_FUNC(assert_le, <=, ::testing::TestPartResult::kFatalFailure);
SQTEST_IMPL_CMP_FUNC(expect_gt, > , ::testing::TestPartResult::kNonFatalFailure);
SQTEST_IMPL_CMP_FUNC(assert_gt, > , ::testing::TestPartResult::kFatalFailure);
SQTEST_IMPL_CMP_FUNC(expect_ge, >=, ::testing::TestPartResult::kNonFatalFailure);
SQTEST_IMPL_CMP_FUNC(assert_ge, >= , ::testing::TestPartResult::kFatalFailure);

#ifdef SQUNICODE
# define SQTEST_STRING_EQ              testing::internal::String::WideCStringEquals
# define SQTEST_STRING_NE             !testing::internal::String::WideCStringEquals
# define SQTEST_INSENSITIVE_STRING_EQ  testing::internal::String::CaseInsensitiveWideCStringEquals
# define SQTEST_INSENSITIVE_STRING_NE !testing::internal::String::CaseInsensitiveWideCStringEquals
# define SQTEST_SHOW_STRING_QUOTED     testing::internal::String::ShowWideCStringQuoted
#else
# define SQTEST_STRING_EQ              testing::internal::String::CStringEquals
# define SQTEST_STRING_NE             !testing::internal::String::CStringEquals
# define SQTEST_INSENSITIVE_STRING_EQ  testing::internal::String::CaseInsensitiveCStringEquals
# define SQTEST_INSENSITIVE_STRING_NE !testing::internal::String::CaseInsensitiveCStringEquals
# define SQTEST_SHOW_STRING_QUOTED     std::string
#endif

//----------------------------------------------------------------------------------------------------------------------
#define SQTEST_IMPL_STR_FUNC(op_name, op_type, op_symbol, failure_type) \
  SQInteger sqtest_ ## op_name (HSQUIRRELVM vm) \
  { \
    const SQChar* expected = 0; \
    sq_getstring(vm, 2, &expected); \
     \
    const SQChar* actual = 0; \
    sq_getstring(vm, 3, &actual); \
     \
    if (op_type(expected, actual)) \
                    { \
                    } \
                                                                else \
    { \
      SQStackInfos si; \
      sq_stackinfos(vm, 1, &si); \
      bool result = ScriptTest::GetPredicateText2(vm, si.source, si.line); \
       \
      const SQChar* expected_expression_text = _SC(""); \
      const SQChar* actual_expression_text = _SC(""); \
      if (result) \
                              { \
        sq_getstring(vm, -2, &expected_expression_text); \
        sq_getstring(vm, -1, &actual_expression_text); \
                              } \
       \
      sq_tostring(vm, 2); \
      const SQChar* expected_value_text = _SC(""); \
      sq_getstring(vm, -1, &expected_value_text); \
       \
      sq_tostring(vm, 3); \
      const SQChar* actual_value_text = _SC(""); \
      sq_getstring(vm, -1, &actual_value_text); \
    char buffer[1024];\
      std::string message;\
      sprintf(buffer, "Expected: (%s) " #op_symbol " (%s), actual %s vs %s", \
        expected_expression_text, \
        actual_expression_text, \
        SQTEST_SHOW_STRING_QUOTED(expected_value_text).c_str(), \
        SQTEST_SHOW_STRING_QUOTED(actual_value_text).c_str()); \
        message = buffer;\
      ::testing::internal::AssertHelper(failure_type, si.source, static_cast<int>(si.line), message.c_str()) = ::testing::Message(); \
       \
      SQInteger count = 2; \
      if (result) \
      { \
        count += 2; \
      } \
      sq_pop(vm, count); \
       \
      SQTEST_WARNING_PUSH(disable : 4127) \
      if (failure_type == ::testing::TestPartResult::kFatalFailure) \
      SQTEST_WARNING_POP() \
      { \
        return sq_throwerror(vm, kGTestFatalFailureString); \
      } \
    } \
    return 0; \
  }

SQTEST_IMPL_STR_FUNC(expect_streq, SQTEST_STRING_EQ, ==, ::testing::TestPartResult::kNonFatalFailure);
SQTEST_IMPL_STR_FUNC(assert_streq, SQTEST_STRING_EQ, ==, ::testing::TestPartResult::kFatalFailure);
SQTEST_IMPL_STR_FUNC(expect_strne, SQTEST_STRING_NE, !=, ::testing::TestPartResult::kNonFatalFailure);
SQTEST_IMPL_STR_FUNC(assert_strne, SQTEST_STRING_NE, !=, ::testing::TestPartResult::kFatalFailure);
SQTEST_IMPL_STR_FUNC(expect_strcaseeq, SQTEST_INSENSITIVE_STRING_EQ, ==, ::testing::TestPartResult::kNonFatalFailure);
SQTEST_IMPL_STR_FUNC(assert_strcaseeq, SQTEST_INSENSITIVE_STRING_EQ, ==, ::testing::TestPartResult::kFatalFailure);
SQTEST_IMPL_STR_FUNC(expect_strcasene, SQTEST_INSENSITIVE_STRING_NE, !=, ::testing::TestPartResult::kNonFatalFailure);
SQTEST_IMPL_STR_FUNC(assert_strcasene, SQTEST_INSENSITIVE_STRING_NE, != , ::testing::TestPartResult::kFatalFailure);

#ifdef SQUSEDOUBLE
# define SQTEST_FLT_FMT "%0.17f"
#else
# define SQTEST_FLT_FMT "%0.8f"
#endif

//----------------------------------------------------------------------------------------------------------------------
#define SQTEST_IMPL_FLOAT_EQ_FUNC(op_name, failure_type) \
  SQInteger sqtest_ ## op_name (HSQUIRRELVM vm) \
  { \
    SQFloat expected = static_cast<SQFloat>(0.0); \
    sq_getfloat(vm, 2, &expected); \
     \
    SQFloat actual = static_cast<SQFloat>(0.0); \
    sq_getfloat(vm, 3, &actual); \
     \
    const ::testing::internal::FloatingPoint<SQFloat> lhs(expected), rhs(actual); \
    if (lhs.AlmostEquals(rhs)) \
                    { \
                    } \
                                                                else \
    { \
      SQStackInfos si; \
      sq_stackinfos(vm, 1, &si); \
      bool result = ScriptTest::GetPredicateText2(vm, si.source, si.line); \
       \
      const SQChar* expected_expression_text = _SC(""); \
      const SQChar* actual_expression_text = _SC(""); \
      if (result) \
                              { \
        sq_getstring(vm, -2, &expected_expression_text); \
        sq_getstring(vm, -1, &actual_expression_text); \
                              } \
       \
      std::string  message;\
        char buffer[1024];\
        sprintf(buffer, "Expected: (%s) == (%s), actual " SQTEST_FLT_FMT " vs " SQTEST_FLT_FMT, \
        expected_expression_text, \
        actual_expression_text, \
        expected, \
        actual); \
        message = buffer;\
      ::testing::internal::AssertHelper(failure_type, si.source, static_cast<int>(si.line), message.c_str()) = ::testing::Message(); \
       \
      SQInteger count = 2; \
      if (result) \
      { \
        count += 2; \
      } \
      sq_pop(vm, count); \
       \
      SQTEST_WARNING_PUSH(disable : 4127) \
      if (failure_type == ::testing::TestPartResult::kFatalFailure) \
      SQTEST_WARNING_POP() \
      { \
        return sq_throwerror(vm, kGTestFatalFailureString); \
      } \
    } \
    return 0; \
  }

SQTEST_IMPL_FLOAT_EQ_FUNC(expect_float_eq, ::testing::TestPartResult::kNonFatalFailure);
SQTEST_IMPL_FLOAT_EQ_FUNC(assert_float_eq, ::testing::TestPartResult::kFatalFailure);

//----------------------------------------------------------------------------------------------------------------------
void sqtest_addtest(HSQUIRRELVM vm, const SQChar *filename)
{
  ScriptTestFactory *factory = new ScriptTestFactory(vm, filename);

  // info is just for debugging purposes, we don't need to do anything with it
  ::testing::TestInfo* info = ::testing::internal::MakeAndRegisterTestInfo(
    "sqtest", filename, NULL, NULL,
    ::testing::internal::CodeLocation(filename, 0),
    ::testing::internal::GetTestTypeId(),
    ::testing::Test::SetUpTestCase,
    ::testing::Test::TearDownTestCase,
    factory);
  (void)info;
}

//----------------------------------------------------------------------------------------------------------------------
#define _DECL_FUNC(name,nparams,pmask) { _SC(#name), sqtest_##name, nparams, pmask }
static SQRegFunction sqtest_lib_funcs[] = {
  _DECL_FUNC(expect_true, 2, _SC(".b")),
  _DECL_FUNC(assert_true, 2, _SC(".b")),
  _DECL_FUNC(expect_false, 2, _SC(".b")),
  _DECL_FUNC(assert_false, 2, _SC(".b")),
  _DECL_FUNC(expect_eq, 3, _SC("...")),
  _DECL_FUNC(assert_eq, 3, _SC("...")),
  _DECL_FUNC(expect_ne, 3, _SC("...")),
  _DECL_FUNC(assert_ne, 3, _SC("...")),
  _DECL_FUNC(expect_lt, 3, _SC("...")),
  _DECL_FUNC(assert_lt, 3, _SC("...")),
  _DECL_FUNC(expect_le, 3, _SC("...")),
  _DECL_FUNC(assert_le, 3, _SC("...")),
  _DECL_FUNC(expect_gt, 3, _SC("...")),
  _DECL_FUNC(assert_gt, 3, _SC("...")),
  _DECL_FUNC(expect_ge, 3, _SC("...")),
  _DECL_FUNC(assert_ge, 3, _SC("...")),
  _DECL_FUNC(expect_streq, 3, _SC(".ss")),
  _DECL_FUNC(assert_streq, 3, _SC(".ss")),
  _DECL_FUNC(expect_strne, 3, _SC(".ss")),
  _DECL_FUNC(assert_strne, 3, _SC(".ss")),
  _DECL_FUNC(expect_strcaseeq, 3, _SC(".ss")),
  _DECL_FUNC(assert_strcaseeq, 3, _SC(".ss")),
  _DECL_FUNC(expect_strcasene, 3, _SC(".ss")),
  _DECL_FUNC(assert_strcasene, 3, _SC(".ss")),
  _DECL_FUNC(expect_float_eq, 3, _SC(".ff")),
  _DECL_FUNC(assert_float_eq, 3, _SC(".ff")),
  { 0, 0, 0, 0 }
};

//----------------------------------------------------------------------------------------------------------------------
void sqtest_compile_error(
  HSQUIRRELVM SQTEST_UNUSED(vm),
  const SQChar *desc,
  const SQChar *source,
  SQInteger line,
  SQInteger SQTEST_UNUSED(column))
{
  ::testing::internal::AssertHelper(
    ::testing::TestPartResult::kFatalFailure,
    source,
    static_cast<int>(line),
    desc) = ::testing::Message();
}

//----------------------------------------------------------------------------------------------------------------------
SQInteger sqtest_runtime_error(HSQUIRRELVM vm)
{
  sq_tostring(vm, 2);
  const SQChar *errorString = "Failure";
  sq_getstring(vm, -1, &errorString);

  SQStackInfos stackinfos;
  sq_stackinfos(vm, 1, &stackinfos);

  ::testing::internal::AssertHelper(
    ::testing::TestPartResult::kFatalFailure,
    stackinfos.source,
    static_cast<int>(stackinfos.line),
    errorString) = ::testing::Message();

  // pop the string created by sq_tostring
  sq_pop(vm, 1);

  return 0;
}

//----------------------------------------------------------------------------------------------------------------------
SQRESULT sqtest_register_lib(HSQUIRRELVM vm)
{
  SQRESULT result = SQ_OK;
  SQInteger i = 0;
  while (sqtest_lib_funcs[i].name!=0)
  {
    sq_pushstring(vm, sqtest_lib_funcs[i].name, -1);
    sq_newclosure(vm, sqtest_lib_funcs[i].f, 0);
    if (SQ_FAILED(sq_setparamscheck(vm, sqtest_lib_funcs[i].nparamscheck, sqtest_lib_funcs[i].typemask)))
    {
      result = SQ_ERROR;
    }
    sq_setnativeclosurename(vm,-1,sqtest_lib_funcs[i].name);
    sq_createslot(vm, -3);
    ++i;
  }

  sq_setcompilererrorhandler(vm, &sqtest_compile_error);
  sq_newclosure(vm, &sqtest_runtime_error, 0);
  sq_seterrorhandler(vm);

  return result;
}

//----------------------------------------------------------------------------------------------------------------------
/// ScriptTest
//----------------------------------------------------------------------------------------------------------------------
inline ScriptTest::ScriptTest(HSQUIRRELVM vm, const SQChar* filename)
: m_vm(vm),
  m_filename(filename),
  m_contents(0),
  m_lines(0)
{
}

//----------------------------------------------------------------------------------------------------------------------
inline ScriptTest::~ScriptTest()
{
  delete [] m_contents;
  delete [] m_lines;
}

//----------------------------------------------------------------------------------------------------------------------
void ScriptTest::SetUp()
{
  ScriptAPI::SetCurrentThreadVM(m_vm);
  // add the ScriptTest entry from the registry table for use by the check functions
  sq_pushregistrytable(m_vm);
  sq_pushstring(m_vm, m_filename, -1);
  sq_pushuserpointer(m_vm, static_cast<SQUserPointer>(this));
  sq_rawset(m_vm, -3);
}

//----------------------------------------------------------------------------------------------------------------------
void ScriptTest::TearDown()
{
  // remove the ScriptTest entry from the registry table
  sq_pushstring(m_vm, m_filename, -1);
  sq_rawdeleteslot(m_vm, -2, SQFalse);

  // pop the registry table
  sq_poptop(m_vm);
}

//----------------------------------------------------------------------------------------------------------------------
void ScriptTest::TestBody()
{
  ASSERT_TRUE(ReadFile(m_filename));

  // error handling of sqstd_loadfile is done by sqtest_compile_error
  SQRESULT result = sqstd_loadfile(m_vm, m_filename, SQTrue);
  ASSERT_TRUE(SQ_SUCCEEDED(result));

  // error handling of sq_call is done by sqtest_runtime_error
  sq_pushroottable(m_vm);
  result = sq_call(m_vm, 1, SQFalse, SQTrue);
  ASSERT_TRUE(SQ_SUCCEEDED(result));
  sq_poptop(m_vm);
}

//----------------------------------------------------------------------------------------------------------------------
bool ScriptTest::GetPredicateText1(HSQUIRRELVM vm, const SQChar *file, size_t line)
{
  // use this stack info to get the funcname
  SQStackInfos si_0;
  sq_stackinfos(vm, 0, &si_0);

  sq_pushregistrytable(vm);
  sq_pushstring(vm, file, -1);
  if (SQ_SUCCEEDED(sq_rawget(vm, -2)))
  {
    if (sq_gettype(vm, -1) == OT_USERPOINTER)
    {
      SQUserPointer ptr;
      if (SQ_SUCCEEDED(sq_getuserpointer(vm, -1, &ptr)))
      {
        ScriptTest *scriptFile = reinterpret_cast<ScriptTest *>(ptr);

        size_t line_index = line - 1;
        if (line_index >= scriptFile->m_lineCount)
        {
          return false;
        }

        // find the arguments to the function
        const char *func_start = strstr(scriptFile->m_lines[line_index], si_0.funcname);
        if (func_start == 0)
        {
          return false;
        }

        const char* start = func_start + strlen(si_0.funcname);
        while (*start != '(')
        {
          ++start;
        }
        ++start;

        size_t bracketCount = 0;
        const char* current = start;
        while (*current != '\0')
        {
          if (*current == '(')
          {
            ++bracketCount;
          }
          if (*current == ')')
          {
            if (bracketCount == 0)
            {
              break;
            }
            --bracketCount;
          }
          ++current;
        }

        sq_pushstring(vm, start, static_cast<SQInteger>(current - start));

        return true;
      }
    }
  }

  return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool ScriptTest::GetPredicateText2(HSQUIRRELVM vm, const SQChar *file, size_t line)
{
  // use this stack info to get the funcname
  SQStackInfos si_0;
  sq_stackinfos(vm, 0, &si_0);

  sq_pushregistrytable(vm);
  sq_pushstring(vm, file, -1);
  if (SQ_SUCCEEDED(sq_rawget(vm, -2)))
  {
    if (sq_gettype(vm, -1) == OT_USERPOINTER)
    {
      SQUserPointer ptr;
      if (SQ_SUCCEEDED(sq_getuserpointer(vm, -1, &ptr)))
      {
        ScriptTest *scriptFile = reinterpret_cast<ScriptTest *>(ptr);

        size_t line_index = line - 1;
        if (line_index >= scriptFile->m_lineCount)
        {
          return false;
        }

        // find the arguments to the function
        const char *func_start = strstr(scriptFile->m_lines[line_index], si_0.funcname);
        if (func_start == 0)
        {
          return false;
        }

        const char* expected_start = func_start + strlen(si_0.funcname);
        const char* expected_end = 0;
        const char* actual_start = 0;

        while (*expected_start != '(')
        {
          ++expected_start;
        }
        ++expected_start;

        size_t bracketCount = 1;
        const char* current = expected_start;
        bool finished = false;
        while (!finished)
        {
          // cases are in sorted order lowest to highest
          switch (*current)
          {
          case _SC('\0'):
            finished = true;
            break;
          case _SC(' '):
            // do nothing this trims whitespace
            break;
          case _SC(','):
            if (bracketCount == 1)
            {
              if (expected_end == 0)
              {
                expected_end = current;
              }
            }
            break;
          case _SC('('):
            ++bracketCount;
            break;
          case _SC(')'):
            --bracketCount;
            if (bracketCount == 0)
            {
              finished = true;
            }
            break;
          default:
            if (expected_end != 0 && actual_start == 0)
            {
              actual_start = current;
            }
            break;
          }

          if (!finished)
          {
            ++current;
          }
        }

        sq_pushstring(vm, expected_start, static_cast<SQInteger>(expected_end - expected_start));
        sq_pushstring(vm, actual_start, static_cast<SQInteger>(current - actual_start));

        return true;
      }
    }
  }

  return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool ScriptTest::ReadFile(const SQChar *filename)
{
  SQFILE file = sqstd_fopen(filename, "rb");
  if (file == 0)
  {
    return false;
  }

  SQInteger result = sqstd_fseek(file, 0, SQ_SEEK_END);
  if (result != 0)
  {
    sqstd_fclose(file);
    return false;
  }

  SQInteger size = sqstd_ftell(file);
  m_contents = new char[size + 1];

  result = sqstd_fseek(file, 0, SQ_SEEK_SET);
  if (result != 0)
  {
    sqstd_fclose(file);
    return false;
  }

  sqstd_fread(m_contents, sizeof(char), size, file);
  m_contents[size] = '\0';
  sqstd_fclose(file);

  m_lineCount = 1;
  for (long i = 0; i != size; ++i)
  {
    switch (m_contents[i])
    {
    case '\n':
      ++m_lineCount;
      break;
    default:
      break;
    }
  }

  m_lines = new char *[m_lineCount];

  size_t currentLine = 0;
  m_lines[currentLine++] = m_contents;
  for (long i = 0; i != size; ++i)
  {
    switch (m_contents[i])
    {
    case '\n':
      m_contents[i] = '\0';
      m_lines[currentLine++] = &m_contents[i + 1];
      break;
    default:
      break;
    }
  }

  return true;
}

//----------------------------------------------------------------------------------------------------------------------
inline ScriptTestFactory::ScriptTestFactory(HSQUIRRELVM vm, const char* filename)
: m_vm(vm),
  m_filename(filename)
{
}

//----------------------------------------------------------------------------------------------------------------------
inline ScriptTestFactory::~ScriptTestFactory()
{
  int i = 0;
  ++i;
}

//----------------------------------------------------------------------------------------------------------------------
::testing::Test *ScriptTestFactory::CreateTest()
{
  ScriptTest *test = new ScriptTest(m_vm, m_filename.c_str());
  return test;
}
