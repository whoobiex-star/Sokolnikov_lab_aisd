#include "fileio.h"
#include "input.h"
#include <iostream>
#include <fstream>

using namespace std;

// вспомогательные функции
static bool parseLine(const string& line, TDTree& value, string& error); // разбор строки файла

// Загрузка и сохранение

// Читает рейтинг из файла. Формат строки: идентификатор;имя;баллы.
// Пустые строки и строки пропускаются. Некорректные строки и повторяющиеся идентификаторы пропускаются с сообщением.
bool loadRating(Node*& root) {
    string fname;
    while (true) {
        if (!readFileNameEsc("Введите имя файла для чтения (расширение .txt можно не указывать). ESC - возврат в меню: ", fname))
            return false;

        ifstream fin(fname.c_str());
        if (!fin.is_open()) {
            cout << "Файл \"" << fname << "\" не найден. Повторите ввод" << endl;
            continue;
        }

        deleteTree(root); // файл открыт- прежний рейтинг заменяется

        string line;
        int line_num = 0;
        int loaded = 0;
        int skipped = 0;
        while (getline(fin, line)) {
            line_num++;
            string text = trim(line);
            if (text.empty() || text[0] == '#') // пустая строка или комментарий
                continue;

            TDTree value;
            string error;
            if (!parseLine(text, value, error)) {
                cout << "Строка " << line_num << " пропущена (" << error << "): " << text << endl;
                skipped++;
                continue;
            }
            if (findById(root, value.id) != nullptr) { // идентификатор должен быть уникальным
                cout << "Строка " << line_num << " пропущена (идентификатор " << value.id
                     << " уже встречался): " << text << endl;
                skipped++;
                continue;
            }

            insertNode(root, value);
            loaded++;
        }
        fin.close();

        cout << "Из файла \"" << fname << "\" загружено участников: " << loaded;
        if (skipped > 0)
            cout << ", пропущено строк: " << skipped;
        cout << endl;
        return true;
    }
}

// Сохраняет рейтинг в файл в порядке убывания баллов, в том же формате, что и при чтении
bool saveRating(Node* root) {
    string fname;
    while (true) {
        if (!readFileNameEsc("Введите имя файла для записи (расширение .txt можно не указывать). ESC - возврат в меню: ", fname))
            return false;

        if (fileExists(fname) && !confirmYesNo("Файл \"" + fname + "\" уже существует. Перезаписать?"))
            continue;

        ofstream fout(fname.c_str());
        if (!fout.is_open()) {
            cout << "Не удалось открыть файл \"" << fname << "\" для записи. Повторите ввод" << endl;
            continue;
        }

        vector<TDTree> list;
        reverseInorder(root, list); // участники в порядке рейтинга

        fout << "# Рейтинг участников: идентификатор;имя;баллы" << "\n";
        for (size_t i = 0; i < list.size(); i++)
            fout << list[i].id << ";" << list[i].name << ";" << list[i].score << "\n";
        fout.close();

        cout << "Рейтинг сохранён в файл \"" << fname << "\", участников: " << list.size() << endl;
        return true;
    }
}

// Записывает строки в текстовый файл, заменяя его прежнее содержимое
bool saveTextFile(const string& fname, const vector<string>& lines) {
    ofstream fout(fname.c_str());
    if (!fout.is_open())
        return false;
    for (size_t i = 0; i < lines.size(); i++)
        fout << lines[i] << "\n";
    return true;
}

// вспомогательные функции
// Разбирает строку файла вида "идентификатор;имя;баллы". При ошибке возвращает false и записывает в error её причину.
static bool parseLine(const string& line, TDTree& value, string& error) {
    size_t p1 = line.find(';');
    size_t p2 = (p1 == string::npos) ? string::npos : line.find(';', p1 + 1);
    if (p1 == string::npos || p2 == string::npos || line.find(';', p2 + 1) != string::npos) {
        error = "ожидается три поля через точку с запятой";
        return false;
    }

    string f_id = trim(line.substr(0, p1));
    string f_name = normalizeName(line.substr(p1 + 1, p2 - p1 - 1));
    string f_score = trim(line.substr(p2 + 1));

    if (!parseInt(f_id, MIN_ID, MAX_ID, value.id)) {
        error = "идентификатор должен быть целым числом от 0 до 100";
        return false;
    }
    if (!isValidName(f_name)) {
        error = "недопустимое имя";
        return false;
    }
    if (!parseInt(f_score, MIN_SCORE, MAX_SCORE, value.score)) {
        error = "баллы должны быть целым числом от 0 до 100";
        return false;
    }
    value.name = f_name;
    return true;
}