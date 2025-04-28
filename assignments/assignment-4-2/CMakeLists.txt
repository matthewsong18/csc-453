# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name
project(Compiler)

set(CMAKE_C_STANDARD 11)

# Add flags for compilation (optional, add as needed)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

# Define the main executable
add_executable(compile
  src/features/parser/ast.c
  src/features/parser/ast-print.c
  src/features/parser/driver.c
  src/features/parser/grammar_rule.c
  src/features/parser/mips.c
  src/features/parser/parser_interface.c
  src/features/parser/parser_rules.c
  src/features/parser/symbol_table.c
  src/features/parser/tac.c
  src/features/parser/token_service.c
  src/features/scanner/complex.c
  src/features/scanner/keywords.c
  src/features/scanner/operators.c
  src/features/scanner/punctuation.c
  src/features/scanner/scanner.c
)

# If you have tests you want to build as a separate executable:
add_executable(test_mips
  tests/tests.c
  # Include other necessary source files for the test executable
  src/features/parser/ast.c
  src/features/parser/ast-print.c
  src/features/parser/grammar_rule.c
  src/features/parser/mips.c
  src/features/parser/parser_interface.c
  src/features/parser/parser_rules.c
  src/features/parser/symbol_table.c
  src/features/parser/tac.c
  src/features/parser/token_service.c
  src/features/scanner/complex.c
  src/features/scanner/keywords.c
  src/features/scanner/operators.c
  src/features/scanner/punctuation.c
  src/features/scanner/scanner.c
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
)

find_package(cmocka REQUIRED)

target_link_libraries(test_complete PRIVATE cmocka::cmocka)

target_include_directories(test_complete PRIVATE ${CMocka_INCLUDE_DIRS})
