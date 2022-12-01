#!/bin/bash

wd=$(pwd)

# Set up
echo "Setting up write test..."
mkdir test-workspace

if [ $? != 0 ]
then
  exit $?
fi

touch test-workspace/new_file.txt

# Test

testwd="$wd/test-workspace"

echo "begin tracking: $testwd"
thoth --track-dir $testwd

cd ./test-workspace

echo "Hello World" >> new_file.txt

echo "Test completed. Please check provenance logs"

# Clean up
echo "Cleaning up..."
cd ..
rm -rf test-workspace
