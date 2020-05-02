cmake_minimum_required(VERSION 3.13)
project(kdec C)

list(APPEND SOURCES
  main.c
  decoder.h
  decoder.c
  unix_socket.h
  unix_socket.c
  tcp_socket.h
  tcp_socket.c
  display.h
  display.c
  utils.h
  resample.h
  resample.c
  loopback.h
  config.h
)

add_executable(kdec ${SOURCES})
