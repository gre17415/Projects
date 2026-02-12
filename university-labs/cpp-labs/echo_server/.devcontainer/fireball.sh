#!/bin/bash
# This fireball can be used to regenerate compile_commands.json (which is used by clangd) for the whole workspace.

for cmake in $(find . -name 'CMakeLists.txt'); do
    build_dir="$(dirname "$cmake")/build";
    mkdir -p "$build_dir";
    pushd "$build_dir";
    cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1;
    popd;
done

mkdir -p ./build;
jq -s 'add' $(find . -name 'compile_commands.json') > ./build/compile_commands.json