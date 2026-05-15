# YADRO_kvadra_team_4

## Запуск 
Структура проекта
```
├── crawler.cpp
├── Makefile
├── server_side.cpp
└── server_side.h
```
Сборка
```
make
```
Запуск
```
./crawler_server
#или
./crawler_server <интервал между просмотрами>
```
## Работа
Для проверки работоспоссобности, можно перейти по ссылке http://localhost:1234/media_files в браузере, или использовать curl в консоли
```
curl "http://localhost:1234/media_files"
```
Код разрабатывался на arch linux. Сборка и работа проверялась на нем же и на ВМ kali linux 
```
Kali Rolling (2026.1) x64
2026-03-20
```
