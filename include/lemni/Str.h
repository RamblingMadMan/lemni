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

#ifndef LEMNI_STR_H
#define LEMNI_STR_H 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <string.h>
#include <stdint.h>

typedef struct {
	const char *ptr;
	size_t len;
} LemniStr;

#define LEMNICSTR(str) ((LemniStr){ .ptr = str, .len = sizeof(str)-1 })

inline LemniStr lemniStrFrom(const char *str){
	return { .ptr = str, .len = strlen(str) };
}

inline int lemniStrCmp(LemniStr lhs, LemniStr rhs){
	if(lhs.len < rhs.len) return INT32_MIN;
	else if(lhs.len > rhs.len) return INT32_MAX;
	else return strncmp(lhs.ptr, rhs.ptr, lhs.len);
}
#ifdef __cplusplus
}

#ifndef LEMNI_NO_CPP
#include <string_view>
#include <ostream>

inline bool operator<(const LemniStr &lhs, const LemniStr &rhs) noexcept{
	return lemniStrCmp(lhs, rhs) < 0;
}

inline bool operator>(const LemniStr &lhs, const LemniStr &rhs) noexcept{
	return lemniStrCmp(lhs, rhs) > 0;
}

inline bool operator==(const LemniStr &lhs, const LemniStr &rhs) noexcept{
	return lemniStrCmp(lhs, rhs) == 0;
}

inline bool operator!=(const LemniStr &lhs, const LemniStr &rhs) noexcept{
	return lemniStrCmp(lhs, rhs) != 0;
}

inline std::ostream &operator<<(std::ostream &str, const LemniStr &s){
	str << std::string_view(s.ptr, s.len);
	return str;
}
#endif // !LEMNI_NO_CPP
#endif // __cplusplus

#endif // !LEMNI_STR_H
