#*****************************************************************************
#
# Copyright (C) 2024 SICK AG
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to SICK AG, Erwin-Sick-Str. 1,
# 79183 Waldkirch.
#
#*****************************************************************************

cmake_minimum_required(VERSION 3.27.4)

project(HttpServer)

find_package(Qt6Core)
set(CMAKE_AUTOMOC ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-DHTTPSERVER_DLL)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /wd4996 /wd4275 /wd4244")

###############################################################################
# Group \\
###############################################################################

set(CHUNK_OF_HEADERS
   Global.h
   Exception.h
   HttpServer.h
   HttpServerWebcc.h
)
set(CHUNK_OF_SOURCES
   Exception.cpp
   HttpServer.cpp
   HttpServerWebcc.cpp
)
list(APPEND HTTPSERVER_HEADERS ${CHUNK_OF_HEADERS})
list(APPEND HTTPSERVER_SOURCES ${CHUNK_OF_SOURCES})
source_group(\\ FILES
   ${CHUNK_OF_HEADERS}
   ${CHUNK_OF_SOURCES}
)

###############################################################################
# Group Resources
###############################################################################

set(HTTPSERVER_RESOURCES
   resource.h
   version.rc
)

source_group(Resources FILES
   ${HTTPSERVER_RESOURCES}
)

###############################################################################
# Linker
###############################################################################

add_library(HttpServer SHARED
   ${HTTPSERVER_HEADERS}
   ${HTTPSERVER_SOURCES}
   ${HTTPSERVER_RESOURCES}
)

target_link_libraries(HttpServer
   Qt6::Core
)
qt_allow_non_utf8_sources(HttpServer)
qt_disable_unicode_defines(HttpServer)

# This is hacky... we would need a proper "Install" target...
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug/)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/)
