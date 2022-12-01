#!/bin/bash

wd=$(pwd)

# Set up
echo "Setting up read test..."
mkdir test-workspace

if [ $? != 0 ]
then
  exit $?
fi

touch test-workspace/new_file.txt

echo "Testing read!" >> new_file.txt

# Test

testwd="$wd/test-workspace"

echo "begin tracking: $testwd"
sudo thoth --track-dir $testwd

cd ./test-workspace

cat new_file.txt

echo "Test completed. Please check provenance logs."

# Clean up
echo "Cleaning up..."
cd ..
rm -rf test-workspace
