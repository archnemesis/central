find_package(QT NAMES Qt5 REQUIRED COMPONENTS Network Core Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network Core Sql)

add_executable(centralserver src/main.cpp
)

target_link_libraries(centralserver
        Qt5::Core
        Qt5::Network
        Qt5::Sql
        central
)

target_include_directories(centralserver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../central/include)
