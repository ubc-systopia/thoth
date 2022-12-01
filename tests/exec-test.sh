#!/bin/bash

wd=$(pwd)

# Set up
echo "Setting up execute test..."
mkdir test-workspace

if [ $? != 0 ]
then
  exit $?
fi

touch test-workspace/test.py

echo "print(\"Hello World!i\")" >> test-workspace/test.py

# Test

testwd="$wd/test-workspace"

echo "begin tracking: $testwd"
thoth --track-dir $testwd

cd ./test-workspace

python test.py

echo "Test completed. Please check provenance logs"

# Clean up
echo "Cleaning up..."
cd ..
rm -rf test-workspace
