# Copyright 2020 Google LLC
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     https://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.16)
project(DicewareGenerator VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)
find_library(LIB_SODIUM sodium)
if (NOT LIB_SODIUM)
  message(FATAL_ERROR "libsodium not found. Please install it.")
endif()
link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})
add_executable(DicewareGenerator src/main.cc src/main_window.cc src/diceware.cc src/main_window.h src/diceware.h)
configure_file(src/eff_large_wordlist.txt ${CMAKE_CURRENT_BINARY_DIR}/eff_large_wordlist.txt COPYONLY)
configure_file(src/diceware.wordlist.asc ${CMAKE_CURRENT_BINARY_DIR}/diceware.wordlist.asc COPYONLY)
target_link_libraries(DicewareGenerator ${GTKMM_LIBRARIES})
target_link_libraries(DicewareGenerator ${LIB_SODIUM})
