cmake_minimum_required(VERSION 2.8)
project(doge)

ENABLE_TESTING()
SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

SET(HDRS
  interpreter.h
  lexer.h
  parser.h
  tokenizer.h
  unicode.h
  error.h
)

SET(SRCS
  interpreter.c
  lexer.c
  main.c
  parser.c
  tokenizer.c
  unicode.c
  error.c
)

add_executable(doge ${SRCS} ${HDRS})
target_link_libraries(doge m)
install(
  TARGETS doge
  RUNTIME DESTINATION bin
)
