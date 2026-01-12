#!/bin/bash

echo "Cleaning transmitter..."
cd transmitter && idf.py fullclean

echo "Cleaning receiver..."
cd ../receiver && idf.py fullclean

if [ "$1" = "--all" ]; then
    echo "Removing parent build directory..."
    rm -rf ../build
fi

echo "All build artifacts removed"
