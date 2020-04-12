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

#include <numeric>

#include "lemni/Interop.h"

template<typename Ratio>
static inline Ratio simplifyRatio(Ratio q){
	auto n = std::gcd(q.num, q.den);
	if(n != 1){
		q.num /= n;
		q.den /= n;
	}

	if(q.den < 0){
		q.num *= -1;
		q.den *= -1;
	}

	return q;
}

LemniRatio32 lemniSimplifyRatio32(const LemniRatio32 q32){ return simplifyRatio(q32); }
LemniRatio64 lemniSimplifyRatio64(const LemniRatio64 q64){ return simplifyRatio(q64); }
LemniRatio128 lemniSimplifyRatio128(const LemniRatio128 q128){ return simplifyRatio(q128); }
