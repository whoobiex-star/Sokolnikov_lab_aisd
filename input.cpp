#include "input.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cctype>
#include <cwchar>
#include <conio.h>

using namespace std;

// вспомогательные функции
static int toCp1251(int ch); // код символа Юникода в код Windows-1251; -1, если символа нет
static bool isLetter(char c); // латинская или русская буква
static bool hasTxtExtension(const string& name); // оканчивается ли имя на .txt

// Ввод с клавиатуры
// Читает целое число в диапазоне [min_val, max_val] построчно (для пункта меню)
int readInt(const string& prompt, int min_val, int max_val) {
    string line;
    while (true) {
        cout << prompt;
        if (!getline(cin, line)) // если поток ввода закрыт
            return -1;

        int value;
        if (parseInt(trim(line), min_val, max_val, value))
            return value;
        cout << "Ошибка ввода. Введите целое число от " << min_val << " до " << max_val << endl;
    }
}

// Посимвольно читает строку. Enter завершает ввод, ESC прерывает его. Клавиша читается функцией _getwch, которая возвращает код символа в Юникоде
// Возвращает false, если нажата ESC или ввод с клавиатуры закончился.
bool readLineEsc(const string& prompt, string& out) {
    cout << prompt;
    string s;
    while (true) {
        int ch = _getwch();

        if (ch == 27 || ch == WEOF) { // ESC или конец ввода - выход из поля ввода
            cout << endl;
            return false;
        }
        if (ch == '\r' || ch == '\n') { // Enter - ввод завершён
            cout << endl;
            out = trim(s);
            return true;
        }
        if (ch == 8) { // Backspace - стираем последний символ
            if (!s.empty()) {
                s.erase(s.size() - 1);
                cout << "\b \b";
            }
            continue;
        }
        if (ch == 0 || ch == 224) { // служебная клавиша присылает два кода, второй пропускаем
            _getwch();
            continue;
        }

        int c = toCp1251(ch); // символ в кодировке программы
        if (c >= 32 && (int)s.size() < MAX_INPUT) { // печатный символ, длина ввода ограничена
            s += (char)c;
            cout << (char)c;
        }
    }
}

// Читает целое число из диапазона [min_val, max_val] с возможностью выхода по ESC
bool readIntEsc(const string& prompt, int min_val, int max_val, int& value) {
    string s;
    while (true) {
        if (!readLineEsc(prompt, s))
            return false;
        if (parseInt(s, min_val, max_val, value))
            return true;
        cout << "Ошибка ввода. Нужно ввести целое число от " << min_val << " до " << max_val << endl;
    }
}

// Читает имя участника: лишние пробелы убираются, состав имени проверяется
bool readNameEsc(const string& prompt, string& name) {
    string s;
    while (true) {
        if (!readLineEsc(prompt, s))
            return false;

        s = normalizeName(s);
        if (s.empty()) {
            cout << "Имя не может быть пустым. Повторите ввод" << endl;
            continue;
        }
        if (isValidName(s)) {
            name = s;
            return true;
        }
        cout << "Ошибка ввода. Имя состоит из русских и латинских букв, может содержать пробел, "
             << "дефис, апостроф и точку." << endl;
        cout << "Имя начинается с буквы, дефис и апостроф ставятся только между буквами, "
             << "точка - только после буквы." << endl;
    }
}

// Читает имя файла. Расширение .txt можно не указывать - оно будет добавлено. Если пользователь указал его сам, имя не изменяется.
bool readFileNameEsc(const string& prompt, string& name) {
    string s;
    const string forbidden = "<>:\"/\\|?*"; // символы, запрещённые в именах файлов Windows

    while (true) {
        if (!readLineEsc(prompt, s))
            return false;

        if (s.empty()) {
            cout << "Имя файла не может быть пустым. Повторите ввод" << endl;
            continue;
        }
        if (s.find_first_of(forbidden) != string::npos) {
            cout << "В имени файла недопустимы символы < > : \" / \\ | ? *" << endl;
            continue;
        }

        name = hasTxtExtension(s) ? s : s + ".txt";
        return true;
    }
}

// Читает номер пункта подменю от 1 до count. Возвращает выбор или 0 при ESC
int readChoice(int count) {
    string s;
    int value;
    while (true) {
        if (!readLineEsc("", s))
            return 0;
        if (parseInt(s, 1, count, value))
            return value;
        cout << "Ошибка ввода. Введите число от 1 до " << count << endl;
        cout << "Выберите пункт: ";
    }
}

// Задаёт вопрос и читает ответ 1 (да) или 0 (нет) с Enter. ESC значит "нет".
bool confirmYesNo(const string& question) {
    string s;
    int value;
    while (true) {
        if (!readLineEsc(question + " (1 - да, 0 - нет): ", s))
            return false;
        if (parseInt(s, 0, 1, value))
            return value == 1;
        cout << "Ошибка ввода. Введите 1 (да) или 0 (нет)" << endl;
    }
}

// Проверка и обработка строк
// Разбирает строку в целое число из диапазона [min_val, max_val], отвергает пустую строку, буквы, дробную часть и числа длиннее 9 цифр (девятизначное число гарантированно помещается в тип long, переполнения нет).
bool parseInt(const string& s, int min_val, int max_val, int& value) {
    if (s.empty())
        return false;

    size_t start = (s[0] == '-') ? 1 : 0; // допускаем знак минус
    if (start >= s.size())
        return false;
    for (size_t i = start; i < s.size(); i++)
        if (!isdigit((unsigned char)s[i]))
            return false;
    if (s.size() - start > 9)
        return false;

    long v = atol(s.c_str());
    if (v < min_val || v > max_val)
        return false;

    value = (int)v;
    return true;
}

// Убирает пробелы по краям и заменяет несколько пробелов подряд одним
string normalizeName(const string& s) {
    string res;
    bool prev_space = false;
    for (size_t i = 0; i < s.size(); i++) {
        char c = (s[i] == '\t') ? ' ' : s[i];
        if (c == ' ') {
            if (!prev_space && !res.empty()) // пробел в начале и повторные пробелы пропускаем
                res += ' ';
            prev_space = true;
        }
        else {
            res += c;
            prev_space = false;
        }
    }
    if (!res.empty() && res[res.size() - 1] == ' ') // пробел в конце
        res.erase(res.size() - 1);
    return res;
}

// Проверяет имя участника. Разрешены русские и латинские буквы, пробел, дефис, апостроф и точка.
// Так записываются многосоставные имена: Салтыков-Щедрин, O'Brien, Smith John Jr., ван дер Ваальс, Абишулы. Имя начинается с буквы, дефис и апостроф стоят только между буквами, точка - только после буквы, после пробела идёт буква.
bool isValidName(const string& name) {
    int n = (int)name.size();
    if (n == 0 || n > MAX_INPUT || !isLetter(name[0]))
        return false;

    for (int i = 0; i < n; i++) {
        char c = name[i];
        bool prev_letter = (i > 0 && isLetter(name[i - 1]));
        bool next_letter = (i + 1 < n && isLetter(name[i + 1]));

        if (isLetter(c))
            continue;
        if (c == '-' || c == '\'') { // дефис и апостроф - только между буквами
            if (!prev_letter || !next_letter)
                return false;
        }
        else if (c == '.') { // точка - только после буквы (инициал или сокращение)
            if (!prev_letter)
                return false;
        }
        else if (c == ' ') { // после пробела должна идти буква
            if (!next_letter)
                return false;
        }
        else // любой другой символ, в том числе цифра и точка с запятой, недопустим
            return false;
    }
    return true;
}

// Удаляет пробелы и табуляции в начале и конце строки
string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos)
        return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// Проверяет, существует ли файл на диске
bool fileExists(const string& name) {
    ifstream f(name.c_str());
    return f.is_open();
}

// Вспомогательные функции
// Переводит код символа Юникода в код кодировки Windows-1251. Поддерживаются латиница, цифры, знаки препинания и русские буквы.
// Для неподходящих символов возвращается -1.
static int toCp1251(int ch) {
    if (ch >= 32 && ch <= 126) // латиница, цифры, знаки препинания совпадают
        return ch;
    if (ch >= 0x0410 && ch <= 0x044F) // русские буквы от А до я
        return 192 + (ch - 0x0410);
    if (ch == 0x0401) // Ё
        return 168;
    if (ch == 0x0451) // ё
        return 184;
    return -1;
}

// Проверяет, является ли символ латинской или русской буквой (кодировка Windows-1251)
static bool isLetter(char c) {
    unsigned char u = (unsigned char)c;
    return (u >= 'A' && u <= 'Z') || (u >= 'a' && u <= 'z') ||
           u >= 192 || u == 168 || u == 184;
}

// Проверяет, оканчивается ли имя файла на .txt, регистр букв не важен
static bool hasTxtExtension(const string& name) {
    if (name.size() < 4)
        return false;
    string ext = name.substr(name.size() - 4);
    for (size_t i = 0; i < ext.size(); i++)
        ext[i] = (char)tolower((unsigned char)ext[i]);
    return ext == ".txt";
}