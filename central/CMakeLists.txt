find_package(QT NAMES Qt5 REQUIRED COMPONENTS Network Core Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network Core Sql)

add_library(central
        include/Server.h
        src/Server.cpp
        include/ClientConnection.h
        src/ClientConnection.cpp
        include/Client.h
        src/Client.cpp
        include/Central.h
        include/PaswordHasher.h
        src/PasswordHasher.cpp
        include/Callback.h
        src/Callback.cpp
        include/AuthenticationCallback.h
        src/AuthenticationCallback.cpp
        include/WorkspaceListCallback.h
        src/WorkspaceListCallback.cpp
        include/WorkspaceObject.h
        src/WorkspaceObject.cpp
        include/LayerListCallback.h
        src/LayerListCallback.cpp
        include/ObjectListCallback.h
        src/ObjectListCallback.cpp)

target_link_libraries(central
        Qt5::Core
        Qt5::Network
)

target_include_directories(central PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(central PRIVATE "C:/Program Files/OpenSSL-Win64/include")