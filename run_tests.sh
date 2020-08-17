#!/bin/bash

if [ "$1" != "" ]; then
    BUILD_TYPE=$1
else
    BUILD_TYPE=Release
fi

if [ "$2" != "" ]; then
    JOBS=$2
else
    JOBS=2
fi

echo "-- Build Type : $BUILD_TYPE"
echo "-- Jobs       : $JOBS"

rm -rf build && mkdir -p build && cd build
cmake -G "Unix Makefiles" -DSTRUCTOPT_SAMPLES=on -DSTRUCTOPT_TESTS=on -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..
cmake --build . --config ${BUILD_TYPE} -- -j${JOBS}
./tests/structopt_tests
