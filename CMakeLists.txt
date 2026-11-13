cmake_minimum_required(VERSION 3.20)
project(zemskov CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(zemskov main.cpp tree.cpp input.cpp fileio.cpp)

# Стандартная библиотека C++, которую мы встраиваем в exe. Иначе при запуске exe вне CLion Windows может подключить чужую libstdc++-6.dll, и программа падает при работе с файлами.
if (MINGW)
    target_link_options(zemskov PRIVATE -static)
endif()
