# Homevork_ICT

Домашние задания по курсу OTUS «Разработчик C++. Базовый курс».

Группа 212, Корепин Матвей.

## Структура репозитория

- [`02_hello_world/`](02_hello_world) — задание занятия 2 «Пишем свой Hello, World»:
  вынесение реализации функции `todo_move_me_out` в отдельные `move_me_out.h`/`move_me_out.cpp`.
- [`03_guess_the_number/`](03_guess_the_number) — задание занятия 5 «Модульность. Линковка»:
  игра «Угадай число» с таблицей рекордов, включая все 5 дополнительных заданий
  (`-max`, `-table`, `-level`, хранение минимального результата на игрока, перезапись
  результата более удачным).
- [`Эссе по занятиям OTUS 1-6.pdf`](<Эссе по занятиям OTUS 1-6.pdf>) — эссе по первым
  шести занятиям курса.
- [`Эссе Domain-Driven Design.pdf`](<Эссе Domain-Driven Design.pdf>) — эссе «Что такое
  Domain-Driven Design?».

## Сборка

Каждая папка с кодом собирается независимо через CMake:

```
cd 02_hello_world
cmake -S . -B build
cmake --build build
./build/hello_world
```

```
cd 03_guess_the_number
cmake -S . -B build
cmake --build build
./build/guess_the_number
```
