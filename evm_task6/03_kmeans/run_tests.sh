#!/bin/sh
# Группа 212, Корепин Матвей
# Тесты: собирает программу, прогоняет команды из tests.txt
# и рисует картинки gnuplot. Запуск из папки программы: sh run_tests.sh

g++ -std=c++11 -Wall -Wextra -Werror -pedantic *.cpp -o prog || exit 1
./prog < tests.txt | tee test_output.txt

if command -v gnuplot > /dev/null 2>&1; then
    for f in plot_*.gp; do
        gnuplot "$f"
    done
    echo "Картинки:"
    ls *.png
else
    echo "gnuplot не найден: картинки не построены"
fi

if grep -q "ОШИБКА" test_output.txt; then
    echo "ЕСТЬ ОШИБКИ - смотрите test_output.txt"
else
    echo "Все проверки прошли"
fi
