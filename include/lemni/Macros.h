/*
	The Lemni Programming Language - Functional computer speak
	Copyright (C) 2020  Keith Hammond

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LEMNI_MACROS_H
#define LEMNI_MACROS_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>

#ifdef __cplusplus
}
#endif


#if defined(__cplusplus) && !defined(LEMNI_NO_CPP)
#define LEMNI_CPP
#endif


#define LEMNI_EXPAND0(x) x
#define LEMNI_EXPAND1(...) LEMNI_EXPAND0(__VA_ARGS__)
#define LEMNI_EXPAND(...) LEMNI_EXPAND1(__VA_ARGS__)


#define LEMNI_JOIN_IMPL(a, b) a##b
#define LEMNI_JOIN(a, b) LEMNI_JOIN_IMPL(a, b)


#define LEMNI_OPAQUE_CONST_T(name)\
	typedef const struct name##T *name

#define LEMNI_OPAQUE_T(name)\
	typedef struct name##T *name;\
	LEMNI_JOIN(LEMNI_OPAQUE_CONST_T(name), Const)

#define LEMNI_OPAQUE_T_DEF(name)\
	struct name##T

#define LEMNI_BITFLAG_ENUM_T_CASES32(prefix, caseName, ...)\
	static_assert(0, "too many cases in bitflag macro invocation")

#define LEMNI_BITFLAG_ENUM_T_CASES31(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES32(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES30(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES31(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES29(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES30(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES28(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES29(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES27(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES28(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES26(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES27(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES25(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES26(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES24(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES25(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES23(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES24(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES22(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES23(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES21(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES22(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES20(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES21(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES19(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES20(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES18(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES19(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES17(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES18(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES16(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES17(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES15(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES16(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES14(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES15(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES13(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES14(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES12(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES13(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES11(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES12(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES10(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES11(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES9(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES10(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES8(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES9(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES7(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES8(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES6(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES7(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES5(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES6(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES4(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES5(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES3(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES4(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES2(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES3(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES1(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES2(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T_CASES0(prefix, caseName, ...)\
	prefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\
	__VA_OPT__(LEMNI_BITFLAG_ENUM_T_CASES1(prefix, __VA_ARGS__))

#define LEMNI_BITFLAG_ENUM_T(name, prefix, ...)\
typedef enum name##T{\
	prefix##_BASE = __COUNTER__ + 1,\
	LEMNI_BITFLAG_ENUM_T_CASES0(prefix, __VA_ARGS__)\
	prefix##_COUNT = __COUNTER__ - prefix##_BASE\
} name

#endif // !LEMNI_MACROS_H
