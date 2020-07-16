# Find libarb (http://arblib.org/)

# - Try to find libarb define the variables for the binaries/headers and include 
#
# Once done this will define
#  ARB_FOUND - System has libarb
#  ARB_INCLUDE_DIRS - The package include directories
#  ARB_LIBRARIES - The libraries needed to use this package

find_library(ARB_LIBRARY NAMES arb libarb
        PATHS $ENV{ARB_DIR} ${ARB_DIR} /usr /usr/local /opt/local
        PATH_SUFFIXES lib lib64 x86_64-linux-gnu lib/x86_64-linux-gnu
)

find_path(ARB_INCLUDE_DIR arb.h
		PATHS $ENV{ARB_DIR} ${ARB_DIR} /usr /usr/local /opt/local 
        PATH_SUFFIXES include include/arb include/x86_64-linux-gnu x86_64-linux-gnu
)

include(FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set ARB_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(ARB DEFAULT_MSG
                                  ARB_LIBRARY ARB_INCLUDE_DIR)

mark_as_advanced(ARB_INCLUDE_DIR ARB_LIBRARY) 

if(ARB_FOUND)
	set(ARB_LIBRARIES ${ARB_LIBRARY})
	set(ARB_INCLUDE_DIRS ${ARB_INCLUDE_DIR})
endif()

