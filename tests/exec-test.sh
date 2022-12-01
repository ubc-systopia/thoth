#!/bin/bash

# Set up
echo "Setting up execute test..."
mkdir test-workspace

if [ $? != 0 ]
then
  exit $?
fi

touch test-workspace/test.py

echo "Hello World" >> test-workspace/test.py

# Test

sudo systemctl start thothd

thoth --track-dir ./test-workspace

cd ./test-workspace

python test.py

sudo systemctl stop thothd

echo "Test completed. Please check provenance logs"

# Clean up
echo "Cleaning up..."
cd ..
rm -rf test-workspace
