cmake_minimum_required(VERSION 3.16)
project(microgame C)

enable_testing()

add_executable(microgame_tests tests/test_microgame.c)
target_compile_features(microgame_tests PRIVATE c_std_99)

if(MSVC)
    target_compile_options(microgame_tests PRIVATE /W4 /WX)
else()
    target_compile_options(microgame_tests PRIVATE -Wall -Wextra -Werror)
endif()

add_test(NAME microgame_physics COMMAND microgame_tests)
