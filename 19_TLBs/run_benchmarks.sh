#!/bin/bash

gcc -O0 tlb.c -o tlb.out
# 1. Define the name of your executable
EXE="./tlb.out"

# 2. Check if the executable exists
if [ ! -f "$EXE" ]; then
    echo "Error: $EXE not found. Compile your code first!"
    exit 1
fi

# 3. Define the ranges you want to test
# This will test 1, 2, 4, 8, 16, 32... up to 1024
PAGES=(1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768)
TRIALS=10000

echo "Starting benchmarks..."
echo "Pages, Average_Time" > results.csv

# 4. The Loop
for p in "${PAGES[@]}"
do
    echo "Testing $p pages..."

    # Run the program and capture the output
    # We use 'tail -n 1' to get just the last line if your code prints multiple lines
    output=$(taskset -c 0 $EXE $p $TRIALS | tail -n 1)

    # Optional: Save the data to a CSV file for graphing later
    echo "$p, $output" >> results.csv
done

echo "Done! Results saved to results.csv"
