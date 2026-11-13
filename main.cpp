#include "tree.h"
#include "input.h"
#include "fileio.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdlib>
#include <cwchar>
#include <conio.h>
#include <windows.h>

using namespace std;

const int PAGE_SIZE = 20; // число строк, выводимых за одну страницу
const int TREE_SCREEN_MAX = 10; // при большем числе участников схему дерева предлагается сохранить в файл
const string TREE_FILE = "tree.txt"; // файл для схемы большого дерева

// Работа с экраном
static void clearConsole(); // очистка экрана
static void pauseConsole(); // пауза до нажатия Enter
static bool nextPage(size_t printed, size_t total); // остановка после каждой страницы
static void outputMenu(Node* root); // главное меню

// Вывод участников
static int placeOf(Node* root, int score); // место в рейтинге для заданных баллов
static void printParticipant(Node* root, const TDTree& p); // карточка участника
static void printTable(Node* root, const vector<TDTree>& list); // таблица участников

// Диалоги пунктов меню (возвращают true, если ничего не сделано и пауза не нужна)
static bool loadDialog(Node*& root, bool& modified);
static bool addDialog(Node*& root, bool& modified);
static bool searchDialog(Node* root);
static bool changeDialog(Node*& root, bool& modified);
static bool deleteDialog(Node*& root, bool& modified);
static bool outputDialog(Node* root);
static bool treeDialog(Node* root);
static void makeTreeText(Node* root, vector<string>& text); // строки схемы дерева и статистики
static bool saveDialog(Node* root, bool& modified);
static bool clearDialog(Node*& root, bool& modified);

// Действия подменю поиска и вывода
static void searchById(Node* root); // поиск по идентификатору
static void searchByName(Node* root); // поиск по началу слова имени
static bool outputAll(Node* root); // весь рейтинг
static bool outputTop(Node* root); // первые N записей; false, если нажата ESC
static bool outputRange(Node* root); // участники в диапазоне баллов; false, если нажата ESC

int main() {
    // ввод и вывод русского текста
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    Node* root = nullptr; // корень дерева: пока рейтинг пуст, никуда не указывает
    bool modified = false; // есть ли несохранённые изменения

    int choice;
    do {
        clearConsole();
        outputMenu(root);
        choice = readInt("Выберите действие: ", 0, 9);
        if (choice < 0) { // поток ввода закрыт - завершаем работу
            deleteTree(root);
            break;
        }
        clearConsole(); // убираем меню перед выполнением выбранного действия

        bool cancelled = false; // true, если пользователь сразу вернулся в меню
        switch (choice) {
            case 1:
                cancelled = loadDialog(root, modified);
                break;
            case 2:
                cancelled = addDialog(root, modified);
                break;
            case 3:
                cancelled = searchDialog(root);
                break;
            case 4:
                cancelled = changeDialog(root, modified);
                break;
            case 5:
                cancelled = deleteDialog(root, modified);
                break;
            case 6:
                cancelled = outputDialog(root);
                break;
            case 7:
                cancelled = treeDialog(root);
                break;
            case 8:
                cancelled = saveDialog(root, modified);
                break;
            case 9:
                cancelled = clearDialog(root, modified);
                break;
            case 0:
                // предупреждаем о несохранённых изменениях
                if (root != nullptr && modified &&
                    !confirmYesNo("Рейтинг изменён и не сохранён. Выйти без сохранения?")) {
                    choice = -1; // остаёмся в программе
                    cancelled = true;
                    break;
                }
                deleteTree(root); // освобождаем память перед выходом
                cout << "Работа с программой завершена" << endl;
                break;
        }

        if (choice != 0 && !cancelled) // после выхода по ESC сразу возвращаемся в меню
            pauseConsole();

    } while (choice != 0);

    return 0;
}

// Работа с экраном
// Очищает экран консоли
static void clearConsole() {
    system("cls");
}

// Пауза до нажатия Enter, чтобы пользователь успел прочитать результат
static void pauseConsole() {
    cout << endl << "Нажмите Enter для возврата в меню...";
    string dummy;
    getline(cin, dummy);
}

// После каждой выведенной страницы ждёт решения пользователя.
// Возвращает false, если вывод прерван клавишей ESC.
static bool nextPage(size_t printed, size_t total) {
    if (printed % PAGE_SIZE != 0 || printed >= total) // страница не закончилась или выведено всё
        return true;

    cout << "Enter - показать дальше, ESC - прервать вывод";
    while (true) {
        int ch = _getwch();
        if (ch == 27 || ch == WEOF) { // ESC или конец ввода
            cout << endl << "Вывод прерван" << endl;
            return false;
        }
        if (ch == '\r' || ch == '\n') {
            cout << endl;
            return true;
        }
    }
}

// Выводит название программы, главное меню и число участников в рейтинге
static void outputMenu(Node* root) {
    cout << "Приложение для ведения рейтинга участников соревнований" << endl << endl;
    cout << "МЕНЮ" << endl;
    cout << "Участников в рейтинге: " << countNodes(root) << endl << endl;
    cout << "1 - Загрузить рейтинг из файла" << endl;
    cout << "2 - Добавить участников" << endl;
    cout << "3 - Найти участника" << endl;
    cout << "4 - Изменить баллы участника" << endl;
    cout << "5 - Удалить участника" << endl;
    cout << "6 - Вывести рейтинг" << endl;
    cout << "7 - Показать структуру дерева и статистику" << endl;
    cout << "8 - Сохранить рейтинг в файл" << endl;
    cout << "9 - Очистить рейтинг" << endl;
    cout << "0 - Завершить работу с программой" << endl;
}

// Вывод участников. Место в рейтинге: число участников с большими баллами плюс один.
// Участники с равными баллами получают одно место.
static int placeOf(Node* root, int score) {
    return countGreater(root, score) + 1;
}

// Выводит данные одного участника и его место в рейтинге
static void printParticipant(Node* root, const TDTree& p) {
    int same = countGreater(root, p.score - 1) - countGreater(root, p.score) - 1; // остальные с теми же баллами

    cout << "Участник: " << p.name << endl;
    cout << "Идентификатор: " << p.id << endl;
    cout << "Баллы: " << p.score << endl;
    cout << "Место в рейтинге: " << placeOf(root, p.score);
    if (same > 0)
        cout << " (такой же результат ещё у участников: " << same << ")";
    cout << endl;
}

// Выводит участников таблицей с местами в рейтинге, постранично
static void printTable(Node* root, const vector<TDTree>& list) {
    cout << left << setw(7) << "Место" << setw(15) << "Идентификатор"
         << setw(7) << "Баллы" << "Имя" << endl;

    for (size_t i = 0; i < list.size(); i++) {
        cout << setw(7) << placeOf(root, list[i].score) << setw(15) << list[i].id
             << setw(7) << list[i].score << list[i].name << endl;
        if (!nextPage(i + 1, list.size()))
            break;
    }
    cout << right; // возвращаем выравнивание по умолчанию
}

// Диалоги пунктов меню
// Загрузка рейтинга из файла с предупреждением о замене текущего рейтинга
static bool loadDialog(Node*& root, bool& modified) {
    bool had_rating = (root != nullptr);
    if (had_rating) {
        string question = modified
            ? "Текущий рейтинг не сохранён и будет удалён. Продолжить?"
            : "Текущий рейтинг будет удалён и заменён данными из файла. Продолжить?";
        if (!confirmYesNo(question))
            return true;
    }

    if (!loadRating(root))
        return true;

    modified = false; // рейтинг совпадает с файлом
    if (had_rating)
        cout << "Прежний рейтинг удалён, в памяти находятся только данные из файла" << endl;
    return false;
}

// Добавление участников одного за другим, пока не нажата ESC.
static bool addDialog(Node*& root, bool& modified) {
    int added = 0;

    while (true) {
        if (countNodes(root) > MAX_ID - MIN_ID) { // заняты все идентификаторы от 0 до 100
            cout << "Свободных идентификаторов не осталось, добавить участника нельзя" << endl;
            if (added == 0) // ничего не добавлено, но сообщение должно остаться на экране
                return false;
            break;
        }

        int id;
        if (!readIntEsc("Введите идентификатор участника (0-100). ESC - завершить ввод: ", MIN_ID, MAX_ID, id))
            break;

        Node* found = findById(root, id);
        if (found != nullptr) { // идентификатор должен быть уникален
            cout << "Идентификатор " << id << " уже занят участником \"" << found->data.name
                 << "\". Повторите ввод" << endl;
            continue;
        }

        TDTree value;
        value.id = id;
        if (!readNameEsc("Введите имя участника. ESC - отменить добавление: ", value.name)) {
            cout << "Добавление участника отменено" << endl;
            continue;
        }
        if (!readIntEsc("Введите количество баллов (0-100). ESC - отменить добавление: ",
                        MIN_SCORE, MAX_SCORE, value.score)) {
            cout << "Добавление участника отменено" << endl;
            continue;
        }

        insertNode(root, value);
        modified = true;
        added++;
        cout << "Участник \"" << value.name << "\" добавлен, место в рейтинге: "
             << placeOf(root, value.score) << endl << endl;
    }

    if (added == 0)
        return true;
    cout << endl << "Добавлено участников: " << added << endl;
    return false;
}

// Поиск участника по идентификатору или по началу слова имени.
static bool searchDialog(Node* root) {
    if (root == nullptr) {
        cout << "Рейтинг пуст, искать некого" << endl;
        return false;
    }

    while (true) {
        clearConsole();
        cout << "Поиск участника:" << endl;
        cout << "1 - по идентификатору" << endl;
        cout << "2 - по началу фамилии, имени или отчества" << endl;
        cout << "ESC - возврат в меню" << endl;
        cout << "Выберите пункт: ";
        int what = readChoice(2);
        if (what == 0)
            return true;
        cout << endl;

        if (what == 1)
            searchById(root);
        else
            searchByName(root);
    }
}

// Поиск участников по идентификатору, пока не нажата ESC
static void searchById(Node* root) {
    int id;
    while (readIntEsc("Введите идентификатор участника (0-100). ESC - завершить поиск: ", MIN_ID, MAX_ID, id)) {
        Node* found = findById(root, id);
        if (found == nullptr)
            cout << "Участника с идентификатором " << id << " нет" << endl;
        else
            printParticipant(root, found->data);
        cout << endl;
    }
}

// Поиск участников по началу фамилии, имени или отчества
static void searchByName(Node* root) {
    string part;
    while (readLineEsc("Введите начало фамилии, имени или отчества. ESC - завершить поиск: ", part)) {
        part = normalizeName(part);
        if (part.empty()) {
            cout << "Строка поиска пуста. Повторите ввод" << endl;
            continue;
        }

        vector<TDTree> list;
        findByName(root, part, list);
        if (list.empty())
            cout << "Участников, у которых слово имени начинается с \"" << part << "\", нет" << endl;
        else {
            cout << "Найдено участников: " << list.size() << endl;
            printTable(root, list);
        }
        cout << endl;
    }
}

// Изменение баллов: запись удаляется с прежней позиции дерева и вставляется по новому ключу
static bool changeDialog(Node*& root, bool& modified) {
    if (root == nullptr) {
        cout << "Рейтинг пуст, изменять нечего" << endl;
        return false;
    }

    int changed = 0;
    while (true) {
        int id;
        if (!readIntEsc("Введите идентификатор участника (0-100). ESC - возврат в меню: ", MIN_ID, MAX_ID, id))
            break;

        Node* found = findById(root, id);
        if (found == nullptr) {
            cout << "Участника с идентификатором " << id << " нет. Повторите ввод" << endl;
            continue;
        }

        TDTree old = found->data; // копия: после изменения узел может оказаться в другом месте
        int old_place = placeOf(root, old.score);
        cout << "Участник \"" << old.name << "\", баллы: " << old.score
             << ", место в рейтинге: " << old_place << endl;

        int new_score;
        if (!readIntEsc("Введите новое количество баллов (0-100). ESC - отменить изменение: ",
                        MIN_SCORE, MAX_SCORE, new_score)) {
            cout << "Изменение отменено" << endl << endl;
            continue;
        }
        if (new_score == old.score) {
            cout << "Баллы не изменились, рейтинг остаётся прежним" << endl << endl;
            continue;
        }

        changeScore(root, id, new_score);
        modified = true;
        changed++;
        cout << "Баллы участника \"" << old.name << "\" изменены: " << old.score << " -> " << new_score
             << ", место в рейтинге: " << old_place << " -> " << placeOf(root, new_score) << endl << endl;
    }

    if (changed == 0)
        return true;
    cout << "Изменено записей: " << changed << endl;
    return false;
}

// Удаление участников одного за другим
static bool deleteDialog(Node*& root, bool& modified) {
    if (root == nullptr) {
        cout << "Рейтинг пуст, удалять некого" << endl;
        return false;
    }

    int removed = 0;
    while (root != nullptr) {
        int id;
        if (!readIntEsc("Введите идентификатор удаляемого участника (0-100). ESC - возврат в меню: ",
                        MIN_ID, MAX_ID, id))
            break;

        Node* found = findById(root, id);
        if (found == nullptr) {
            cout << "Участника с идентификатором " << id << " нет. Повторите ввод" << endl;
            continue;
        }

        TDTree value = found->data; // после удаления узел исчезнет
        cout << "Участник \"" << value.name << "\", баллы: " << value.score << endl;

        if (!confirmYesNo("Удалить участника \"" + value.name + "\"?")) {
            cout << "Удаление отменено" << endl << endl;
            continue;
        }

        removeById(root, id);
        modified = true;
        removed++;
        cout << "Участник \"" << value.name << "\" удалён" << endl << endl;
    }

    if (root == nullptr && removed > 0)
        cout << "Рейтинг пуст, удалять больше некого" << endl;
    if (removed == 0)
        return true;
    cout << "Удалено участников: " << removed << endl;
    return false;
}

// Вывод рейтинга: весь, первые N записей или участники в диапазоне баллов.
static bool outputDialog(Node* root) {
    if (root == nullptr) {
        cout << "Рейтинг пуст, выводить нечего" << endl;
        return false;
    }

    while (true) {
        clearConsole();
        cout << "Вывод рейтинга:" << endl;
        cout << "1 - весь рейтинг" << endl;
        cout << "2 - первые N записей рейтинга" << endl;
        cout << "3 - участники с баллами в заданном диапазоне" << endl;
        cout << "ESC - возврат в меню" << endl;
        cout << "Выберите пункт: ";
        int what = readChoice(3);
        if (what == 0)
            return true;
        cout << endl;

        bool shown; // выведен ли результат (false - ввод прерван клавишей ESC)
        if (what == 1)
            shown = outputAll(root);
        else if (what == 2)
            shown = outputTop(root);
        else
            shown = outputRange(root);

        if (shown)
            pauseConsole();
    }
}

// Выводит весь рейтинг. Всегда возвращает true: ввод не требуется.
static bool outputAll(Node* root) {
    vector<TDTree> list;
    reverseInorder(root, list);
    cout << "Рейтинг участников (всего: " << list.size() << ")" << endl;
    printTable(root, list);
    return true;
}

// Выводит первые N записей рейтинга.
static bool outputTop(Node* root) {
    int k;
    if (!readIntEsc("Сколько первых записей вывести (1-101). ESC - возврат: ", 1, MAX_ID - MIN_ID + 1, k))
        return false;

    int total = countNodes(root);
    vector<TDTree> list;
    topRecords(root, k, list);
    if (k > total)
        cout << "В рейтинге всего участников: " << total << ", выведены все" << endl;
    cout << "Первые записи рейтинга (выведено: " << list.size() << ")" << endl;
    printTable(root, list);
    return true;
}

// Выводит участников с баллами в заданном диапазоне.
static bool outputRange(Node* root) {
    int min_s, max_s;
    if (!readIntEsc("Нижняя граница баллов (0-100). ESC - возврат: ", MIN_SCORE, MAX_SCORE, min_s))
        return false;
    if (!readIntEsc("Верхняя граница баллов (0-100). ESC - возврат: ", MIN_SCORE, MAX_SCORE, max_s))
        return false;
    if (min_s > max_s) {
        int temp = min_s;
        min_s = max_s;
        max_s = temp;
        cout << "Нижняя граница была больше верхней, границы переставлены местами" << endl;
    }

    vector<TDTree> list;
    rangeRecords(root, min_s, max_s, list);
    if (list.empty())
        cout << "Участников с баллами от " << min_s << " до " << max_s << " нет" << endl;
    else {
        cout << "Участники с баллами от " << min_s << " до " << max_s
             << " (найдено: " << list.size() << ")" << endl;
        printTable(root, list);
    }
    return true;
}

// Схема дерева и его статистика. Если участников больше TREE_SCREEN_MAX, схема в консоли может отображаться некорректно, и её предлагается сохранить в файл.
static bool treeDialog(Node* root) {
    if (root == nullptr) {
        cout << "Рейтинг пуст, дерево не содержит участников" << endl;
        return false;
    }

    vector<string> text;
    makeTreeText(root, text);

    int n = countNodes(root);
    if (n > TREE_SCREEN_MAX) {
        cout << "Бинарное дерево слишком большое (участников: " << n << "), в консоли оно может отображаться некорректно." << endl;
        cout << "Для корректного отображения рекомендуется вывести его в текстовый файл \"" << TREE_FILE
             << "\" в каталоге программы." << endl;
        if (confirmYesNo("Сохранить схему дерева в файл?")) {
            if (saveTextFile(TREE_FILE, text)) {
                cout << "Бинарное дерево успешно сохранено в файл \"" << TREE_FILE << "\" в каталоге программы" << endl;
                return false;
            }
            cout << "Не удалось открыть файл \"" << TREE_FILE << "\" для записи, схема выводится на экран" << endl;
        }
        cout << endl;
    }

    for (size_t i = 0; i < text.size(); i++) {
        cout << text[i] << endl;
        if (!nextPage(i + 1, text.size()))
            break;
    }
    return false;
}

// Составляет строки схемы дерева и статистики: они выводятся на экран или в файл
static void makeTreeText(Node* root, vector<string>& text) {
    vector<TreeLine> lines;
    collectTreeLines(root, "", 0, lines);

    text.push_back("Схема дерева рейтинга. Начало дерева (корень) - слева.");
    text.push_back("Ветвь вверх ведёт к участникам выше в рейтинге, ветвь вниз - ниже в рейтинге.");
    text.push_back("У каждого участника указаны баллы, имя и идентификатор.");
    text.push_back("");
    for (size_t i = 0; i < lines.size(); i++)
        text.push_back(lines[i].prefix + to_string(lines[i].data.score) + " " + lines[i].data.name + " (id " + to_string(lines[i].data.id) + ")");

    int n = countNodes(root);
    int h = treeHeight(root);
    int h_min = 0; // наименьшее число уровней для n участников
    while ((1 << h_min) - 1 < n)
        h_min++;

    Node* best = maxNode(root);
    Node* worst = minNode(root);

    text.push_back("");
    text.push_back("Статистика:");
    text.push_back("Участников в рейтинге: " + to_string(n));
    text.push_back("Начало дерева (корень): " + root->data.name + " (баллы: " + to_string(root->data.score) + ", id " + to_string(root->data.id) + ")");
    if (h > h_min)
        text.push_back("Число уровней дерева: " + to_string(h) + " (наименьшее возможное для такого числа участников - " + to_string(h_min) + ")");
    else
        text.push_back("Число уровней дерева: " + to_string(h) + " (наименьшее возможное)");
    text.push_back("Лучший результат: " + best->data.name + " (баллы: " + to_string(best->data.score) + ")");
    text.push_back("Худший результат: " + worst->data.name + " (баллы: " + to_string(worst->data.score) + ")");
}

// Сохранение рейтинга в файл
static bool saveDialog(Node* root, bool& modified) {
    if (root == nullptr) {
        cout << "Рейтинг пуст, сохранять нечего" << endl;
        return false;
    }
    if (!saveRating(root))
        return true;

    modified = false; // рейтинг совпадает с сохранённым файлом
    return false;
}

// Полная очистка рейтинга с подтверждением
static bool clearDialog(Node*& root, bool& modified) {
    if (root == nullptr) {
        cout << "Рейтинг уже пуст" << endl;
        return false;
    }

    string question = modified
        ? "Рейтинг не сохранён и будет полностью удалён. Продолжить?"
        : "Рейтинг будет полностью удалён. Продолжить?";
    if (!confirmYesNo(question)) {
        cout << "Очистка отменена" << endl;
        return false;
    }

    int n = countNodes(root);
    deleteTree(root);
    modified = false; // удалённые данные сохранять уже нечего
    cout << "Рейтинг очищен, удалено участников: " << n << endl;
    return false;
}