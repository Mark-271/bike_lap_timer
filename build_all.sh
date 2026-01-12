#!/bin/bash

echo "Building transmitter..."
cd transmitter && idf.py build
if [ $? -ne 0 ]; then
    echo "Transmitter build failed"
    exit 1
fi

echo "Building receiver..."
cd ../receiver && idf.py build
if [ $? -ne 0 ]; then
    echo "Receiver build failed"
    exit 1
fi

echo "Copying binaries to parent build directory..."
mkdir -p ../build
cp transmitter/build/*.bin ../build/transmitter_*.bin 2>/dev/null || true
cp transmitter/build/*.elf ../build/transmitter.elf 2>/dev/null || true
cp receiver/build/*.bin ../build/receiver_*.bin 2>/dev/null || true
cp receiver/build/*.elf ../build/receiver.elf 2>/dev/null || true

echo "Both applications built successfully - binaries in build/"
