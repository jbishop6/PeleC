# Install script for directory: /home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/src/kinsol

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
  MESSAGE("
Install KINSOL
")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64" TYPE STATIC_LIBRARY FILES "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/build/src/kinsol/libsundials_kinsol.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libsundials_kinsol.so.5.7.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libsundials_kinsol.so.5"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "/home/jbishop6/sundials-5.7.0-install/lib64")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64" TYPE SHARED_LIBRARY FILES
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/build/src/kinsol/libsundials_kinsol.so.5.7.0"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/build/src/kinsol/libsundials_kinsol.so.5"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libsundials_kinsol.so.5.7.0"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib64/libsundials_kinsol.so.5"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH ":::::::::::::::::::::::::::::::::::::::::::"
           NEW_RPATH "/home/jbishop6/sundials-5.7.0-install/lib64")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64" TYPE SHARED_LIBRARY FILES "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/build/src/kinsol/libsundials_kinsol.so")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/kinsol" TYPE FILE FILES
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/include/kinsol/kinsol.h"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/include/kinsol/kinsol_bbdpre.h"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/include/kinsol/kinsol_direct.h"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/include/kinsol/kinsol_ls.h"
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D/sundials-5.7.0/include/kinsol/kinsol_spils.h"
    )
endif()

