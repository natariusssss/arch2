#!/bin/bash

if [ ! -f ./server ]; then
	echo "Error: server executable not found. Compile server.cpp first"
	exit 1
fi

if [ ! -f ./client ]; then
	echo "Error: client executable not found. Compile client.cpp first"
	exit 1
fi

run_test() {
local test_name=$1
local expected_text=$2

echo "Running $test_name..."

PORT=$((1111 + RANDOM % 1000))

./server $PORT &
SERVER_PID=$!
sleep 2

CLIENT_OUTPUT=$(./client)
if echo "$CLIENT_OUTPUT" | grep -q "$expected_text"; then
	echo "$test_name PASSED"
else
	echo "$test_name FAILED"
	echo "Client output"
	echo "$CLIENT_OUTPUT"
fi
if ps -p $SERVER_PID > /dev/null; then
	kill $SERVER_PID
	wait $SERVER_PID 2>dev>null
	sleep 1
fi
echo
}

run_test "Test 1:Basic ping" "pong"

echo "All tests finished"

