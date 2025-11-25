#!/bin/bash

SERVER=./server
CLIENT=./client
PORT=1111
LOG_SERVER=server.log


wait_server_ready() {
    for i in {1..50}; do
        if grep -q "Wait client" "$LOG_SERVER" 2>/dev/null; then
            return 0
        fi
        sleep 0.1
    done
    return 1
}


start_server() {
    rm -f "$LOG_SERVER"
    $SERVER >"$LOG_SERVER" 2>&1 &
    SERVER_PID=$!

    if ! wait_server_ready; then
        echo "Server did not reach accept()"
        kill $SERVER_PID 2>/dev/null
        exit 1
    fi
}

stop_server() {
    printf "exit" | nc 127.0.0.1 $PORT >/dev/null 2>&1
    sleep 0.2
    kill $SERVER_PID 2>/dev/null
}


test_ping_pong() {
    echo "[TEST] ping → pong"

    start_server

    OUTPUT=$(printf "ping\nexit\n" | $CLIENT 2>/dev/null)

    echo "$OUTPUT" | grep -q "\[Server\]: pong"
    if [ $? -ne 0 ]; then
        echo "FAIL: expected pong"
        echo "$OUTPUT"
        stop_server
        exit 1
    fi

    echo "PASS: pong received"
    stop_server
}


test_exit_closes_server() {
    echo "[TEST] exit → server terminates"

    start_server

    printf "exit" | nc 127.0.0.1 $PORT >/dev/null 2>&1
    sleep 0.3

    if ps -p $SERVER_PID >/dev/null 2>&1; then
        echo "FAIL: server did not terminate after exit"
        kill $SERVER_PID
        exit 1
    fi

    echo "PASS: server terminated correctly"
}
test_random_message() {
    echo "[TEST] random message → default server reply"

    start_server

    OUTPUT=$(printf "hello\nexit\n" | $CLIENT 2>/dev/null)

    echo "$OUTPUT" | grep -q "\[Server\]: Message received. Response from server - pong"
    if [ $? -ne 0 ]; then
        echo "FAIL: expected generic reply"
        echo "$OUTPUT"
        stop_server
        exit 1
    fi

    echo "PASS: generic reply received"
    stop_server
}


test_ping_pong
test_random_message
test_exit_closes_server


echo
echo "=============================="
echo " ALL TESTS PASSED SUCCESSFULLY "
echo "=============================="
