cmake_minimum_required(VERSION 3.5.0)
project(VFE VERSION 0.2.0 LANGUAGES C)

add_executable(fictiler compiler.c)

add_executable(vimulator virtual_emulator.c)

add_executable(devirtual decompiler.c)
