# Install script for directory: /home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/examples/sunlinsol/spgmr/serial

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/jbishop6/sundials-5.7.0-install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/test_sunlinsol_spgmr_serial.c;/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/test_sunlinsol.h;/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/test_sunlinsol.c;/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/sundials_linearsolver.c;/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/sundials_nvector.c")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial" TYPE FILE FILES
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/examples/sunlinsol/spgmr/serial/test_sunlinsol_spgmr_serial.c"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/examples/sunlinsol/spgmr/serial/../../test_sunlinsol.h"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/examples/sunlinsol/spgmr/serial/../../test_sunlinsol.c"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/src/sundials/sundials_linearsolver.c"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/src/sundials/sundials_nvector.c"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/test_sunlinsol_spgmr_serial.c;/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/test_sunlinsol.h;/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/test_sunlinsol.c;/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/sundials_linearsolver.c;/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/sundials_nvector.c")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial" TYPE FILE FILES
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/examples/sunlinsol/spgmr/serial/test_sunlinsol_spgmr_serial.c"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/examples/sunlinsol/spgmr/serial/../../test_sunlinsol.h"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/examples/sunlinsol/spgmr/serial/../../test_sunlinsol.c"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/src/sundials/sundials_linearsolver.c"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/src/sundials/sundials_nvector.c"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/test_sunlinsol_spgmr_serial.c;/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/test_sunlinsol.h;/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/test_sunlinsol.c;/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/sundials_linearsolver.c;/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/sundials_nvector.c")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial" TYPE FILE FILES
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/examples/sunlinsol/spgmr/serial/test_sunlinsol_spgmr_serial.c"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/examples/sunlinsol/spgmr/serial/../../test_sunlinsol.h"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/examples/sunlinsol/spgmr/serial/../../test_sunlinsol.c"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/src/sundials/sundials_linearsolver.c"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/src/sundials/sundials_nvector.c"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/test_sunlinsol_spgmr_serial.c;/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/test_sunlinsol.h;/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/test_sunlinsol.c;/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/sundials_linearsolver.c;/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/sundials_nvector.c")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial" TYPE FILE FILES
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/examples/sunlinsol/spgmr/serial/test_sunlinsol_spgmr_serial.c"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/examples/sunlinsol/spgmr/serial/../../test_sunlinsol.h"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/examples/sunlinsol/spgmr/serial/../../test_sunlinsol.c"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/src/sundials/sundials_linearsolver.c"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/src/sundials/sundials_nvector.c"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/CMakeLists.txt")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial" TYPE FILE FILES "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/build/examples/sunlinsol/spgmr/serial/CMakeLists.txt")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial/Makefile")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/jbishop6/sundials-5.7.0-install/examples/sunlinsol/spgmr/serial" TYPE FILE RENAME "Makefile" FILES "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/build/examples/sunlinsol/spgmr/serial/Makefile_ex")
endif()

