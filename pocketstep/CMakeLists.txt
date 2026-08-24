cmake_minimum_required(VERSION 3.16)
project(pocketstep C)

enable_testing()

add_executable(pocketstep_tests tests/test_pocketstep.c)
target_compile_features(pocketstep_tests PRIVATE c_std_99)

if(MSVC)
    target_compile_options(pocketstep_tests PRIVATE /W4 /WX)
else()
    target_compile_options(pocketstep_tests PRIVATE -Wall -Wextra -Werror)
endif()

add_test(NAME pocketstep_physics COMMAND pocketstep_tests)
