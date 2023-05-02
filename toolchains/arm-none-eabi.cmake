
set( CROSS_COMPILE "arm-none-eabi-" CACHE STRING "cross compile prefix" )

set( CMAKE_SYSTEM_NAME          Generic )
set( CMAKE_SYSTEM_PROCESSOR     cortex-m4 )
set( CMAKE_EXECUTABLE_SUFFIX    ".elf" )

if ( WIN32 )
  set( CMAKE_ASM_COMPILER "${CROSS_COMPILE}gcc.exe" )
  set( CMAKE_AR "${CROSS_COMPILE}ar.exe" )
  set( CMAKE_ASM_COMPILER "${CROSS_COMPILE}gcc.exe" )
  set( CMAKE_C_COMPILER "${CROSS_COMPILE}gcc.exe" )
  set( CMAKE_CXX_COMPILER "${CROSS_COMPILE}g++.exe" )
  set( CMAKE_OBJCOPY "${CROSS_COMPILE}objcopy.exe"
       CACHE FILEPATH "The toolchain objcopy command " FORCE )
  set( CMAKE_OBJDUMP "${CROSS_COMPILE}objdump.exe"
       CACHE FILEPATH "The toolchain objcopy command " FORCE )
else()
  set( CMAKE_ASM_COMPILER "${CROSS_COMPILE}gcc" )
  set( CMAKE_AR "${CROSS_COMPILE}ar" )
  set( CMAKE_ASM_COMPILER "${CROSS_COMPILE}gcc" )
  set( CMAKE_C_COMPILER "${CROSS_COMPILE}gcc" )
  set( CMAKE_CXX_COMPILER "${CROSS_COMPILE}g++" )
  set( CMAKE_OBJCOPY "${CROSS_COMPILE}objcopy"
       CACHE FILEPATH "The toolchain objcopy command " FORCE )
  set( CMAKE_OBJDUMP "${CROSS_COMPILE}objdump"
       CACHE FILEPATH "The toolchain objcopy command " FORCE )
endif()

# Set the common build flags

# Common flags shared by asm, c and cpp
set( COMMON_FLAGS "--param max-inline-insns-single=500 -mlong-calls -ffunction-sections -fdata-sections -fno-exceptions -fsingle-precision-constant -Wall -Wextra -Wundef -Wdouble-promotion -Wno-expansion-to-defined -fdiagnostics-color")

set( CMAKE_ASM_FLAGS "${COMMON_FLAGS}" )
set( CMAKE_C_FLAGS "${COMMON_FLAGS} -std=gnu17" )
set( CMAKE_CXX_FLAGS "${COMMON_FLAGS} -std=gnu++17 -fno-threadsafe-statics -fno-rtti" )
set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" )

message( "Toolchain definition:" )
message( "  CROSS_COMPILE prefix: ${CROSS_COMPILE}" )
message( "  asm: " "${CMAKE_ASM_COMPILER}" )
message( "  c: " "${CMAKE_C_COMPILER}" )
message( "  cxx: " "${CMAKE_CXX_COMPILER}" )
message( "  ar: " "${CMAKE_AR}" )
message( "  objcopy: " "${CMAKE_OBJCOPY}" )
message( "  objdump: " "${CMAKE_OBJDUMP}" )
message( "  C_FLAGS: " ${CMAKE_C_FLAGS} )
message( "  CXX_FLAGS: " ${CMAKE_CXX_FLAGS} )
message( "  ASM_FLAGS: " ${CMAKE_ASM_FLAGS} )
message( "  LD_FLAGS: " ${CMAKE_EXE_LINKER_FLAGS} )
