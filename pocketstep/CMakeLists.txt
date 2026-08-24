cmake_minimum_required(VERSION 3.16)
project(pocketstep C)

enable_testing()

add_executable(pocketstep_tests tests/test_pocketstep.c)
target_compile_features(pocketstep_tests PRIVATE c_std_99)

add_executable(pocketstep_grid_tests tests/test_pocketstep_grid.c)
target_compile_features(pocketstep_grid_tests PRIVATE c_std_99)

add_executable(pocketstep_story_tests tests/test_pocketstep_story.c)
target_compile_features(pocketstep_story_tests PRIVATE c_std_99)

add_executable(storyclock_world_tests tests/test_storyclock_world.c)
target_compile_features(storyclock_world_tests PRIVATE c_std_99)

if(MSVC)
    target_compile_options(pocketstep_tests PRIVATE /W4 /WX)
    target_compile_options(pocketstep_grid_tests PRIVATE /W4 /WX)
    target_compile_options(pocketstep_story_tests PRIVATE /W4 /WX)
    target_compile_options(storyclock_world_tests PRIVATE /W4 /WX)
else()
    target_compile_options(pocketstep_tests PRIVATE -Wall -Wextra -Werror)
    target_compile_options(pocketstep_grid_tests PRIVATE -Wall -Wextra -Werror)
    target_compile_options(pocketstep_story_tests PRIVATE -Wall -Wextra -Werror)
    target_compile_options(storyclock_world_tests PRIVATE -Wall -Wextra -Werror)
endif()

add_test(NAME pocketstep_physics COMMAND pocketstep_tests)
add_test(NAME pocketstep_grid COMMAND pocketstep_grid_tests)
add_test(NAME pocketstep_story COMMAND pocketstep_story_tests)
add_test(NAME storyclock_world COMMAND storyclock_world_tests)
