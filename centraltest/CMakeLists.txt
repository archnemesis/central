find_package(QT NAMES Qt5 REQUIRED COMPONENTS Network Core Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network Core Sql)

add_executable(centraltest src/main.cpp
)

target_link_libraries(centraltest
        Qt5::Core
        Qt5::Network
        Qt5::Sql
        central
)

target_include_directories(centraltest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../central/include)
target_include_directories(centraltest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)