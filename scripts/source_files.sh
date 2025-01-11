#!/usr/bin/env bash

set -e
set -u
set -x

test -d src/

find ./src/ '(' -iname '*.cpp' -o -iname '*.h' ')' -print
