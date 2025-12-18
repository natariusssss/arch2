#!/bin/bash

set -e  

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_section() {
    echo -e "\n${BLUE}▶ $1${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

cleanup() {
    echo -e "\n${YELLOW}Очистка...${NC}"
    pkill -f "./server" 2>/dev/null || true
    rm -f server.log test_output.txt client*.log test_protocol test_protocol.cpp
}

cleanup

print_section "1. КОМПИЛЯЦИЯ"
echo "Компиляция сервера..."
g++ -std=c++17 -pthread -o server server.cpp protocol.cpp utils.cpp 2>&1
if [ $? -eq 0 ]; then
    print_success "Сервер скомпилирован"
else
    print_error "Ошибка компиляции сервера"
    exit 1
fi

echo "Компиляция клиента..."
g++ -std=c++17 -o client client.cpp utils.cpp 2>&1
if [ $? -eq 0 ]; then
    print_success "Клиент скомпилирован"
else
    print_error "Ошибка компиляции клиента"
    exit 1
fi

print_section "2. БАЗОВЫЕ ТЕСТЫ"

echo "Запуск сервера..."
./server > server.log 2>&1 &
SERVER_PID=$!
sleep 2

if ps -p $SERVER_PID > /dev/null; then
    print_success "Сервер запущен (PID: $SERVER_PID)"
else
    print_error "Сервер не запустился"
    cat server.log
    exit 1
fi

echo "Тест Ping-Pong..."
echo -e "ping\nexit" | timeout 3 ./client 2>/dev/null | grep -q "\[Server\]: pong"
if [ $? -eq 0 ]; then
    print_success "Ping-Pong работает"
else
    print_error "Ping-Pong не работает"
fi

echo "Тест общего сообщения..."
echo -e "hello\nexit" | timeout 3 ./client 2>/dev/null | grep -q "Message received"
if [ $? -eq 0 ]; then
    print_success "Обработка общих сообщений работает"
else
    print_error "Обработка общих сообщений не работает"
fi

echo "Тест exit команды..."
echo "exit" | nc -w 1 127.0.0.1 1111 >/dev/null 2>&1
sleep 1
if ps -p $SERVER_PID > /dev/null 2>&1; then
    print_error "Сервер не завершился по exit команде"
else
    print_success "Exit команда работает"
    ./server > server.log 2>&1 &
    SERVER_PID=$!
    sleep 2
fi

print_section "3. МНОГОПОТОЧНОСТЬ"
echo "Тест 3 одновременных подключений..."

for i in {1..3}; do
    (echo "ping"; sleep 0.5; echo "exit") | timeout 2 ./client > "client$i.log" 2>&1 &
done
wait

success_count=0
for i in {1..3}; do
    if grep -q "\[Server\]: pong" "client$i.log"; then
        ((success_count++))
    fi
    rm -f "client$i.log"
done

if [ $success_count -eq 3 ]; then
    print_success "Многопоточность работает ($success_count/3 клиентов получили ответ)"
else
    print_error "Проблемы с многопоточностью ($success_count/3 клиентов получили ответ)"
fi

print_section "4. ТЕСТ ПРОТОКОЛА"
echo "Тест упаковки/распаковки сообщений..."

cat > test_protocol.cpp << 'EOF'
#include <iostream>
#include "protocol.h"
int main() {
    Message msg;
    char buf[10];
    
    msg.type = MSG_RING;
    if (pack_msg(msg, buf, 10) != 1) return 1;
    if (buf[0] != 1) return 1;
    
    msg.type = MSG_PONG;
    if (pack_msg(msg, buf, 10) != 1) return 1;
    if (buf[0] != 2) return 1;
    
    Message out;
    buf[0] = 1;
    if (unpack_msg(buf, 1, out) != 0) return 1;
    if (out.type != MSG_RING) return 1;
    
    std::cout << "Protocol OK" << std::endl;
    return 0;
}
EOF

g++ -std=c++17 -o test_protocol test_protocol.cpp protocol.cpp 2>&1
if [ $? -eq 0 ] && ./test_protocol; then
    print_success "Протокол работает корректно"
else
    print_error "Проблемы с протоколом"
fi
rm -f test_protocol test_protocol.cpp

print_section "5. ЗАВЕРШЕНИЕ"
echo "Остановка сервера..."
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

echo -e "${GREEN}         ВСЕ ТЕСТЫ ЗАВЕРШЕНЫ          ${NC}"

cleanup
