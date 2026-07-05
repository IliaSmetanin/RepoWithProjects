# Train Your Dragon

Проект по сборке многомодульного C/C++ кода с использованием **CMake**, **GoogleTest**, покрытия кода, линтинга и профилей сборки.

---

## Цели проекта

* Интеграция внешних библиотек (GoogleTest), автоматическая загрузка модулей
* Настройка статического анализа (линтер include-директив) и генерация покрытия (lcov/genhtml)
* Управление разными конфигурациями сборки (дневные/ночные поединки)
* Создание кастомных целей (`train`, `lint`, `coverage`, `clear`)

---

## Основные возможности

* **Исполняемая цель `train`** — бинарник собирается из двух исходников и выводится в папку `results`
* **Линтер include-путей** — цель `lint` запускает скрипт, проверяющий отсутствие относительных `#include`
* **Тесты на GoogleTest** — поединки драконов с автоматическим подключением фреймворка и `-pthread`
* **Покрытие кода** — цель `coverage` скачивает модуль `CodeCoverage.cmake` и генерирует отчёт в `coverage/`
* **Профили сборки** — опции `-DDAY=ON` / `-DNIGHT=ON` задают препроцессорные макросы `day` / `night`
* **Очистка артефактов** — цель `clear` удаляет папку сборки, скачанный модуль, `coverage/` и `results/`

---

## Технологический стек

| Технология          | Назначение                                    |
|---------------------|-----------------------------------------------|
| C++17 / C17         | Языки программирования                        |
| CMake 3.11+         | Система сборки                                |
| Google Test         | Фреймворк для модульного тестирования         |
| Bash, Python 3      | Скрипты автоматизации и линтинга              |
| gcc-14, clang-14    | Компиляторы C/C++                             |
| gcov, lcov, genhtml | Инструменты анализа покрытия кода             |

---

## Структура проекта

cmake-dragons/
├── arena/              # тесты GoogleTest
├── cmake/              # CMake-модули (в т.ч. динамически загружаемый CodeCoverage.cmake)
├── drachenland/        # основная библиотека драконов (fiery, icy, fury)
├── train/              # исходники для цели train (train.cpp, trench.cpp)
├── utils/              # хедер breath_power.hpp и includeLint.py
└── CMakeLists.txt      # главный сценарий сборки

---

## Как собрать и запустить

### 1. Базовая сборка и запуск цели `train`

```bash
cmake -S . -B build
cmake --build build --target train && results/train 11
```

### 2. Запуск линтера

```bash
cmake --build build --target lint
```

### 3. Сборка и прогон тестов (дневной профиль)

```bash
cmake -S . -B build -DDAY=ON
cmake --build build --target Battle && build/Battle
```

*Аналогично для ночного: -DNIGHT=ON*

### 4. Покрытие кода

```bash
cmake -S . -B build -DDAY=ON
cmake --build build --target coverage
# отчёт в ./coverage/index.html
```

### 5. Полная очистка результатов сборки

```bash
cmake -S . -B build -DDAY=ON -DBUILD_SOURCE_DIR="build"
cmake --build build --target clear
```

или

```bash
cmake -S . -B build -DDAY=ON -DBUILD_SOURCE_DIR="build"
cmake --build build --target train
cmake --build build --target coverage
cmake --build build --target clear
```
