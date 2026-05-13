cmake_minimum_required(VERSION 3.10)
project(cherry_beer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# базовая игровая логика
add_library(game src/game.cpp)
target_include_directories(game PUBLIC headers)
target_compile_options(game PRIVATE -Wall -Wextra -Wpedantic)

# парсер входного файла
add_library(parser src/parser.cpp)
target_include_directories(parser PUBLIC headers)
target_compile_options(parser PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(parser PUBLIC game)

# интерфейс бота 
add_library(bot INTERFACE)
target_include_directories(bot INTERFACE headers)
target_link_libraries(bot INTERFACE game)

# реализация бота
add_library(alice_bot src/alice_bot.cpp)
target_include_directories(alice_bot PUBLIC headers)
target_compile_options(alice_bot PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(alice_bot PUBLIC bot)

# исполняемый файл
add_executable(cherry_beer main.cpp)
target_compile_options(cherry_beer PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(cherry_beer PRIVATE parser alice_bot)
