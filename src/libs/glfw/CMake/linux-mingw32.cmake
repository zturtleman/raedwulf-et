# Define the cross compilation environment for cross compiling from linux 
# to win32 it is to be used when debian cross compilation toolchain is 
# available.
SET(CMAKE_SYSTEM_NAME    Windows) # Target system name
SET(CMAKE_SYSTEM_VERSION 1)       # Not really used.
SET(CMAKE_C_COMPILER     "i486-mingw32-gcc")
SET(CMAKE_CXX_COMPILER   "i486-mingw32-g++")
SET(CMAKE_RANLIB         "i486-mingw32-ranlib")


#Configure the behaviour of the find commands 
SET(CMAKE_FIND_ROOT_PATH "/usr/i486-mingw32")
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
