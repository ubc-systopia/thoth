#!/bin/bash

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

sudo systemctl start thothd

thoth --track-dir ./test-workspace

cd ./test-workspace

cat new_file.txt

sudo systemctl stop thothd

echo "Test completed. Please check provenance logs."

# Clean up
echo "Cleaning up..."
cd ..
rm -rf test-workspace
