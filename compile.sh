#/bin/sh

compiler='clang'

echo Compiling for $compiler

$compiler -o gosh gosh.c -Wno-error