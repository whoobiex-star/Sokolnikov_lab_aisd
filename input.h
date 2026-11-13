#ifndef INPUT_H
#define INPUT_H

#include <string>

const int MAX_INPUT = 100; // наибольшая длина вводимой строки
const int MIN_ID = 0; // границы идентификатора участника
const int MAX_ID = 100;
const int MIN_SCORE = 0; // границы количества баллов
const int MAX_SCORE = 100;

// Ввод с клавиатуры
int readInt(const std::string& prompt, int min_val, int max_val); // пункт меню; -1, если поток ввода закрыт
bool readLineEsc(const std::string& prompt, std::string& out); // строка; false, если нажата ESC
bool readIntEsc(const std::string& prompt, int min_val, int max_val, int& value); // целое число в диапазоне
bool readNameEsc(const std::string& prompt, std::string& name); // имя участника с проверкой
bool readFileNameEsc(const std::string& prompt, std::string& name); // имя файла; .txt добавляется само
int readChoice(int count); // номер пункта от 1 до count; 0, если нажата ESC
bool confirmYesNo(const std::string& question); // вопрос: 1 - да, 0 - нет

// Проверка и обработка строк
bool parseInt(const std::string& s, int min_val, int max_val, int& value); // строка в число диапазона
std::string normalizeName(const std::string& s); // убирает лишние пробелы
bool isValidName(const std::string& name); // проверка состава имени
std::string trim(const std::string& s); // удаление пробелов по краям
bool fileExists(const std::string& name); // существует ли файл

#endif