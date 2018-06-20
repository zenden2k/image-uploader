#ifndef IU_ENUMUTILS_H
#define IU_ENUMUTILS_H

#pragma once

#include <boost/preprocessor.hpp>
#include <map>
#include <string>
//#include <boost/assign/list_of.hpp> 
// http://stackoverflow.com/a/5094430/4569791

#define X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE(r, data, elem)    \
    case elem : return BOOST_PP_STRINGIZE(elem);

#define X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOENUM_IF(r, data, elem)    \
    {"" , elem },

#define DEFINE_ENUM_WITH_STRING_CONVERSIONS_BASE(name, enumerators, modifier) \
    enum name {                                                               \
        BOOST_PP_SEQ_ENUM(enumerators)                                        \
        };                                                                    \
                                                                              \
    inline modifier const char* EnumToString(name v)                          \
        {                                                                     \
        switch (v)                                                            \
                {                                                             \
            BOOST_PP_SEQ_FOR_EACH(                                            \
                X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOSTRING_CASE,          \
                name,                                                         \
                enumerators                                                   \
            )                                                                 \
            default: return "[Unknown " BOOST_PP_STRINGIZE(name) "]";         \
                }                                                             \
        }                                                                     \
        inline modifier name StringToEnum##name(const char* v)                      \
        {                                                                     \
            static std::map<std::string, name> enumMap = \
{\
    \
    BOOST_PP_SEQ_FOR_EACH(\
    X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOENUM_IF, \
    name, \
    enumerators                                                   \
    )\
        };      \
            auto it = enumMap.find(v);                                        \
            if (it != enumMap.end())  {                                       \
                return it->second;                                             \
            }                                                                 \
            return static_cast<name>(0);                                      \
                                                                              \
        }

/*
boost::assign::map_list_of

BOOST_PP_SEQ_FOR_EACH(                                    \
X_DEFINE_ENUM_WITH_STRING_CONVERSIONS_TOENUM_IF,              \
name,                                                         \
enumerators                                                   \
)                                                             \
};                                                                \
*/
#define DEFINE_ENUM_WITH_STRING_CONVERSIONS(name, enumerators)     DEFINE_ENUM_WITH_STRING_CONVERSIONS_BASE(name, enumerators,)
#define DEFINE_MEMBER_ENUM_WITH_STRING_CONVERSIONS(name, enumerators)  DEFINE_ENUM_WITH_STRING_CONVERSIONS_BASE(name, enumerators,static)    
#endif