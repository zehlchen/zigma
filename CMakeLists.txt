#
# ZIGMAtiq, Copyright (C) 2024 Chase Zehl O'Byrne
#   <mail: zehl@live.com> http://zehlchen.com/
#
# This file is part of ZIGMAtiq.
#
# ZIGMAtiq is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# ZIGMAtiq is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ZIGMAtiq; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

cmake_minimum_required(VERSION 3.16)

project (zigmatiq
  VERSION 2.1.0
  DESCRIPTION "ZIGMAtiq is a command line utility for encrypting, decrypting, and signing files."
)

execute_process(
  COMMAND git rev-list --count HEAD
  OUTPUT_VARIABLE GIT_BUILD
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  OUTPUT_VARIABLE GIT_COMMIT
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND git describe --tags --abbrev=0
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  OUTPUT_VARIABLE GIT_TAG
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(ziggy)
target_sources(ziggy PRIVATE
  zigmatiq/base64.c
  zigmatiq/buffer.c
  zigmatiq/common.c
  zigmatiq/main.c
  zigmatiq/registry.c
  zigmatiq/zigma.c
)

add_compile_definitions(
  ZIGMATIQ_GIT_BUILD="${GIT_BUILD}"
  ZIGMATIQ_GIT_COMMIT="${GIT_COMMIT}"
  ZIGMATIQ_GIT_BRANCH="${GIT_BRANCH}"
  ZIGMATIQ_GIT_TAG="${GIT_TAG}"
  ZIGMATIQ_VERSION_STRING="${PROJECT_VERSION}:${GIT_BUILD}"
  ZIGMATIQ_COMPILE_FLAGS="${CMAKE_C_FLAGS}"
)
