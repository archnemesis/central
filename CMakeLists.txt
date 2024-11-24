cmake_minimum_required(VERSION 3.29)

set(CENTRAL_VERSION_MAJOR 1)
set(CENTRAL_VERSION_MINOR 0)
set(CENTRAL_VERSION_REVISION 0)
project(central VERSION "1.0.0" LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Network Core Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network Core Sql)

add_subdirectory(central)
add_subdirectory(centralserver)
add_subdirectory(centraltest)
