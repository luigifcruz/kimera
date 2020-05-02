cmake_minimum_required(VERSION 3.13)
project(kenc C)

list(APPEND ENCODER_SOURCES
  main.c
  encoder.h
  encoder.c
  unix_socket.h
  unix_socket.c
  tcp_socket.h
  tcp_socket.c
  display.h
  display.c
  utils.h
  loopback.h
  config.h
)

add_executable(kenc ${ENCODER_SOURCES})
