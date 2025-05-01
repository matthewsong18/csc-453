cmake_minimum_required(VERSION 3.10)

project(Compiler)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

add_executable(compile
        src/features/parser/driver.c
        src/features/parser/simple-parse.c
        src/features/scanner/complex.c
        src/features/scanner/keywords.c
        src/features/scanner/operators.c
        src/features/scanner/punctuation.c
        src/features/scanner/scanner.c
        src/features/parser/token_service.c
        src/features/semantic/symbols.c
        src/features/common/safe-memory.c
)

add_executable(test_complete
        tests/test-complete-parser.c
        src/features/parser/simple-parse.c
        src/features/scanner/complex.c
        src/features/scanner/keywords.c
        src/features/scanner/operators.c
        src/features/scanner/punctuation.c
        src/features/scanner/scanner.c
        src/features/parser/token_service.c
        src/features/semantic/symbols.c
        src/features/common/safe-memory.c
)

add_executable(test_semantic
        tests/test-semantic.c
        src/features/semantic/symbols.c
        src/features/common/safe-memory.c
)

find_package(cmocka REQUIRED)

target_link_libraries(test_complete PRIVATE cmocka::cmocka)
target_link_libraries(test_semantic PRIVATE cmocka::cmocka)

target_include_directories(test_complete PRIVATE ${CMocka_INCLUDE_DIRS})
target_include_directories(test_semantic PRIVATE ${CMocka_INCLUDE_DIRS})
