# Windstille - A Sci-Fi Action-Adventure Game
# Copyright (C) 2015 Ingo Ruhnke <grumbel@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.15)
project(wstsound VERSION 0.3.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/external/cmake-modules/)

include(ClangTidy)
include(MaximumWarnings)

option(BUILD_EXTRA "Build extra stuff" OFF)

if(NOT TARGET Threads::Threads)
  find_package(Threads REQUIRED)
endif()

find_package(OpenAL REQUIRED)

find_package(PkgConfig REQUIRED)

pkg_search_module(OGG REQUIRED ogg IMPORTED_TARGET)
pkg_search_module(VORBIS REQUIRED vorbis IMPORTED_TARGET)
pkg_search_module(VORBISFILE REQUIRED vorbisfile IMPORTED_TARGET)

# compile libgalapix
file(GLOB WSTSOUND_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  src/sound/*.cpp
  src/sprite2d/*.cpp
  src/sprite3d/*.cpp
  src/system/*.cpp
  src/util/*.cpp)
add_library(wstsound STATIC ${WSTSOUND_SOURCES})
target_compile_options(wstsound PUBLIC ${WARNINGS_CXX_FLAGS})
set_target_properties(wstsound PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF)
target_include_directories(wstsound SYSTEM PUBLIC
  src/
  ${OPENAL_INCLUDE_DIR})
target_link_libraries(wstsound PUBLIC
  ${OPENAL_LIBRARY}
  PkgConfig::OGG
  PkgConfig::VORBIS
  PkgConfig::VORBISFILE
  Threads::Threads)

if(BUILD_EXTRA)
  file(GLOB EXTRA_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    extra/*.cpp)
  foreach(SRC ${EXTRA_SOURCES})
    get_filename_component(SRC_BASENAME ${SRC} NAME_WE)
    add_executable(${SRC_BASENAME} ${SRC})
    target_link_libraries(${SRC_BASENAME} wstsound)
    set_target_properties(${SRC_BASENAME} PROPERTIES
      CXX_STANDARD 20
      CXX_STANDARD_REQUIRED ON
      CXX_EXTENSIONS OFF)
  endforeach(SRC)
endif()

# EOF #
