# Full-Stack Web Application with Docker Compose

Трёхзвенное веб-приложение, развёрнутое в Docker Compose: **FastAPI** + **PostgreSQL** + **Nginx**.  
Реализован механизм инициализации БД тестовыми данными, проброс статики через Nginx и мониторинг здоровья сервисов.

---

## Технологический стек

| Технология       | Назначение                                      |
|------------------|-------------------------------------------------|
| Python 3.12      | Язык backend-приложения                         |
| FastAPI 0.108    | Web-фреймворк для REST API и HTML-шаблонов      |
| PostgreSQL 17    | Реляционная база данных                         |
| SQLAlchemy       | ORM для взаимодействия с БД                     |
| Jinja2           | Шаблонизатор HTML                               |
| Nginx            | Обратный прокси и раздача статики               |
| Docker / Compose | Контейнеризация и оркестрация сервисов          |
| Ubuntu 24.04     | Базовый образ backend-контейнера                |

---

## Структура проекта

docker-compose/
├── backend/
│ ├── base.py # Подключение к БД, модель User
│ ├── init_db.py # Создание таблиц и начальных данных
│ ├── main.py # FastAPI приложение
│ ├── requirements.txt # Зависимости Python
│ └── templates/ # HTML-шаблоны
├── db/
│ └── .db.env # Пароль PostgreSQL
├─── docker/
│ └── backend.Dockerfile # Сборка backend-образа
├── frontend/
│ ├── nginx.conf # Конфигурация Nginx
│ └── styles.css # Стили для веб-интерфейса
└── docker-compose.yml # Описание сервисов

---

## Зависимости

Для запуска достаточно **Docker** и **Docker Compose**.  

```bash
sudo apt-get install docker docker-compose
```

## Архитектура сервисов

`docker-compose.yml` описывает четыре сервиса:

- **database** – PostgreSQL 17.  
  Данные хранятся в именованном томе `pg_volume`, что гарантирует их сохранность между перезапусками.  
  Healthcheck: `pg_isready`.

- **init** – однократный контейнер, выполняющий `init_db.py`.  
  Создаёт таблицы (если их нет) и заполняет БД двумя тестовыми пользователями:  
  `pudge, (hook@gmail.com)` и `morphling (adaptive@gmail.com)`.  
  Запускается строго после того, как база данных станет `healthy`.

- **backend** – FastAPI на `uvicorn`.  
  Предоставляет REST API (`/api/users`) и генерирует HTML-страницы (`/`, `/users`).  
  Подключается к PostgreSQL по имени сервиса `database`.  
  Ожидает готовности БД и успешного завершения `init`.

- **frontend** – Nginx.  
  Пробрасывает порт `8189` на хост.  
  Проксирует запросы к API на `backend:8000`.  
  Раздаёт статический файл `styles.css` из папки `/usr/share/nginx/html/static`.  
  Стартует только после того, как `backend` станет `healthy`.

Сервисы разделены на две сети: `net-back` (backend + database) и `net-front` (frontend + backend). Это ограничивает прямое обращение фронтенда к базе данных.

---

## Запуск «под ключ»

```bash
git clone <url>
cd <working-dir>
docker compose up
```

Через несколько секунд все сервисы будут запущены и доступны:

- **Главная страница**: <http://localhost:8189/>

- **Список пользователей**: <http://localhost:8189/users>

- **API (JSON)**: <http://localhost:8189/api/users>

- **Статика**: <http://localhost:8189/static/styles.css>

Остановка с удалением тома:

```bash
docker compose down -v
```

## Проверка работоспособности

После запуска можно выполнить:

```bash
curl http://localhost:8189/api/users
curl http://localhost:8189/static/styles.css
```

При открытии /users в браузере шрифт элементов списка будет крупным – это доказывает успешный проброс стилей через Nginx.
