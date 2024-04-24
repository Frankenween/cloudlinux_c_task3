#!/bin/bash

# in $1 executable is expected

# Test working pipe and file output

# prog1 prog2 prog3 file executable
prg=$1

function run_test() {
    $prg "$1" "$2" "$3" "$4"
    exit_code_got=$?
    got_ans=$(cat "$4")
    rm "$4"

    $1 && $2 | $3 > "$4"
    exit_code_expected=$?
    expected_ans=$(cat "$4")
    rm "$4"

    if [ $exit_code_expected -ne $exit_code_got ]
    then
      echo "Expected exit code $exit_code_expected, got $exit_code_got"
      exit 1
    fi
    if [ "$expected_ans" != "$got_ans" ]
    then
      echo "Output files differ"
      echo "Expected \"$expected_ans\""
      echo "Got \"$got_ans\""
      exit 2
    fi
}

# run simple tests
run_test true pwd cat got1
run_test true true pwd got2
run_test ls pwd cat got3
run_test false pwd wc should_not_be_created

# test writing to existing file
dd if=/dev/random of=existing.file bs=1K count=4 status=none
run_test true pwd cat existing.file

# test that prog2 and prog3 see created file
run_test true ls cat created_file

# test that prog2 and prog3 see prog1 side effects
run_test ./prog1_se.sh ./prog2_se.sh ./prog3_se.sh output

echo "Tests passed"
