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

project(MauCppHttpServer)

find_package(Qt6Core)
find_package(Qt6Network)
find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_DEBUG_POSTFIX "d")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR} ${WEBCCDIR}/include)
add_definitions(-DMAUCPPHTTPSERVER_DLL)
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

add_library(MauCppHttpServer SHARED
   ${HTTPSERVER_HEADERS}
   ${HTTPSERVER_SOURCES}
   ${HTTPSERVER_RESOURCES}
)

target_link_libraries(MauCppHttpServer
   Qt6::Core
   Qt6::Network
   OpenSSL::SSL
   debug ${WEBCCDIR}/lib/webccd.lib optimized ${WEBCCDIR}/lib/webcc.lib
)

set_target_properties(MauCppHttpServer PROPERTIES PUBLIC_HEADER "${HTTPSERVER_HEADERS}")

qt_allow_non_utf8_sources(MauCppHttpServer)
qt_disable_unicode_defines(MauCppHttpServer)

install(TARGETS MauCppHttpServer 
    RUNTIME                                                                                     # .dll and .lib
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mau                                   # Headers
)
install(FILES $<TARGET_PDB_FILE:MauCppHttpServer> DESTINATION ${CMAKE_INSTALL_BINDIR} OPTIONAL) # .pdb
