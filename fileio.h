#ifndef FILEIO_H
#define FILEIO_H

#include "tree.h"

// Загрузка рейтинга из текстового файла: прежнее дерево заменяется данными файла
// Возвращает true, если загрузка выполнена; false, если ввод имени прерван клавишей ESC.
bool loadRating(Node*& root);

// Сохранение рейтинга в текстовый файл в порядке убывания баллов
// Возвращает true, если рейтинг сохранён; false, если ввод имени прерван клавишей ESC.
bool saveRating(Node* root);

// Запись строк в текстовый файл (прежнее содержимое файла заменяется).
// Возвращает true, если файл записан; false, если его не удалось открыть.
bool saveTextFile(const std::string& fname, const std::vector<std::string>& lines);

#endif