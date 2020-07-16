def createOpaqueTypeMacro(macroName):
	return \
		f"#define {macroName}(name)\\\n" + \
		f"\ttypedef struct name##T *name;\\\n" + \
		f"\ttypedef const struct name##T *name##Const"

def createBitflagEnumCasesMacro(macroName, n, separator):
	ret = ""
	
	for i in reversed(range(n)):
		ret = ret + separator + \
			f"#define {macroName}_CASES{str(i)}(prefix, caseName, ...)\\\n" + \
			"\tprefix##_##caseName = 1 << (__COUNTER__ - prefix##_BASE),\\\n" + \
			f"\t__VA_OPT__({macroName}_CASES{str(i + 1)}(prefix, __VA_ARGS__))"
	
	ret = \
		f"#define {macroName}_CASES{str(n)}(prefix, caseName, ...)\\\n" + \
		"\tstatic_assert(0, \"too many cases in bitflag macro invocation\")" + \
		ret
		
	
	return ret

def createBitflagEnumMacro(macroName, n=32, separator="\n\n"):
	ret = createBitflagEnumCasesMacro(macroName, n, separator)
	
	return ret + separator + \
		f"#define {macroName}(name, prefix, ...)\\\n" + \
		"typedef enum name##T{\\\n" + \
		"\tprefix##_BASE = __COUNTER__ + 1,\\\n" + \
		f"\t{macroName}_CASES0(prefix, __VA_ARGS__)\\\n" + \
		"\tprefix##_COUNT = __COUNTER__ - prefix##_BASE\\\n" + \
		"} name"

def createAGPLv3License(projectDesc, developer, year):
	return \
	f"""/*
	{projectDesc}
	Copyright (C) {year}  {developer}

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
*/"""

def createGuarded(header, footer, contents, separator=""):
	ret = f"{header}"
	
	for elem in contents:
		ret = f"{ret}\n{separator}{elem}"
	
	return f"{ret}\n{separator}{footer}"

def createExternC(contents, separator=""):
	header = \
		"#ifdef __cplusplus\n" + \
		"extern \"C\" {\n" + \
		"#endif"
	
	footer = \
		"#ifdef __cplusplus\n" + \
		"}\n" +\
		"#endif"
	
	return createGuarded(header, footer, contents, separator)

def createHeaderBody(name, contents, separator=""):
	header = \
		f"#ifndef {name}\n" + \
		f"#define {name} 1"
	
	footer = f"#endif // !{name}"
	
	return createGuarded(header, footer, contents, separator)

def createCIncludes(headers):
	lines = f"#include <{headers[0]}>"
	
	for i in range(len(headers)-1):
		lines = f"{lines}\n#include <{headers[i+1]}>"
	
	return createExternC([lines], separator="\n")

def main():
	license = createAGPLv3License(
		"The Lemni Programming Language - Functional computer speak",
		"Keith Hammond",
		"2020"
		)
	
	body = createHeaderBody(
		"LEMNI_MACROS_H", [
			createCIncludes(["assert.h"]),
			createOpaqueTypeMacro("LEMNI_OPAQUE_T"),
			createBitflagEnumMacro("LEMNI_BITFLAG_ENUM_T")
		],
		separator="\n\n"
		)
	
	src = f"{license}\n\n{body}"
	
	with open("Macros.h", "w") as f:
		f.write(src)

if __name__ == "__main__":
	main()
