# fileserver

Простой пример передачи по сети файлов на основе asio C++ library.

Сервер асинхронный, с пулом рабочих потоков для обслуживания соединений. Файлы создаются внутри специальной рабочей директории. После успешного получения файла соединение разрывается. В случае получения неполного файла (при завершении работы сервера, обрыве соединения и т.п.) полученный не полностью файл удаляется. Ситуация, при которой несколько клиентов пишут разные файлы с одинаковым путем специально не обрабатывается и является неопределенным поведением. Файлы создаются внутри рабочей папки сервера по полному пути исходного файла, отправленного с клиента.

Клиент синхронный и однопоточный.