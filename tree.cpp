#include "tree.h"

using namespace std;

// Внутренние вспомогательные функции
static char searchChar(char c); // буква в виде для поиска: нижний регистр
static string searchText(const string& s); // строка в виде для поиска
static bool wordStartsWith(const string& name, const string& part); // начинается ли слово имени с part

// Создание узла и сравнение ключей

// Создаёт узел с данными участника; ссылки на поддеревья пока пустые
Node* createNode(const TDTree& value) {
    Node* ptr = new Node;
    ptr->data = value;
    ptr->left_ptr = nullptr;
    ptr->right_ptr = nullptr;
    return ptr;
}

// Сравнивает ключи двух участников: сначала баллы, при равных баллах - идентификатор.
// Идентификатор сравнивается в обратную сторону, чтобы при выводе рейтинга
// участники с равными баллами шли по возрастанию идентификатора.
// Идентификаторы уникальны, поэтому двух равных ключей в дереве не бывает.
bool keyLess(const TDTree& a, const TDTree& b) {
    if (a.score != b.score)
        return a.score < b.score;
    return a.id > b.id;
}

// Основные операции

// Вставляет участника в дерево по ключу (баллы, идентификатор).
// ptr передаётся по ссылке: когда найдено пустое место, в него сразу
// записывается адрес нового узла (это может быть и корень дерева).
void insertNode(Node*& ptr, const TDTree& value) {
    if (ptr == nullptr) { // нашли свободное место - создаём узел
        ptr = createNode(value);
        return;
    }
    if (keyLess(value, ptr->data))
        insertNode(ptr->left_ptr, value); // ключ меньше - идём влево
    else
        insertNode(ptr->right_ptr, value); // ключ больше - идём вправо
}

// Ищет участника по идентификатору прямым обходом (корень, левое, правое поддерево).
// Спуститься по ключу нельзя: дерево упорядочено по баллам, а не по идентификатору.
Node* findById(Node* ptr, int id) {
    if (ptr == nullptr)
        return nullptr;
    if (ptr->data.id == id) // нашли - дальше не ищем
        return ptr;

    Node* found = findById(ptr->left_ptr, id);
    if (found != nullptr)
        return found;
    return findById(ptr->right_ptr, id);
}

// Удаляет из поддерева узел с заданным ключом.
// ptr передаётся по ссылке, поэтому при удалении функция сама перецепляет
// ссылку родителя (или корень дерева) на того, кто встаёт на место узла.
bool removeNode(Node*& ptr, const TDTree& key) {
    if (ptr == nullptr) // такого ключа в дереве нет
        return false;
    if (keyLess(key, ptr->data)) // ключ меньше - ищем в левом поддереве
        return removeNode(ptr->left_ptr, key);
    if (keyLess(ptr->data, key)) // ключ больше - ищем в правом поддереве
        return removeNode(ptr->right_ptr, key);

    // узел найден
    if (ptr->left_ptr == nullptr) { // лист или только правый потомок
        Node* loc_ptr = ptr;
        ptr = ptr->right_ptr; // на место узла встаёт правое поддерево (у листа - пустота)
        delete loc_ptr;
        return true;
    }
    if (ptr->right_ptr == nullptr) { // только левый потомок
        Node* loc_ptr = ptr;
        ptr = ptr->left_ptr; // на место узла встаёт левое поддерево
        delete loc_ptr;
        return true;
    }

    // Два потомка. В узел переносятся данные преемника - узла с наименьшим ключом
    // в правом поддереве. Порядок ключей при этом не нарушается, а сам преемник
    // удаляется из правого поддерева: левого потомка у него нет, это простой случай.
    TDTree succ_data = minNode(ptr->right_ptr)->data;
    ptr->data = succ_data;
    return removeNode(ptr->right_ptr, succ_data);
}

// Удаляет участника по идентификатору: обходом находит его ключ, затем удаляет по ключу
bool removeById(Node*& root, int id) {
    Node* found = findById(root, id);
    if (found == nullptr)
        return false;

    TDTree key = found->data; // копия ключа: сам узел будет удалён
    return removeNode(root, key);
}

// Изменяет баллы участника. Ключ узла меняется, поэтому просто исправить поле нельзя:
// запись удаляется с прежней позиции и вставляется заново по новому ключу.
bool changeScore(Node*& root, int id, int new_score) {
    Node* found = findById(root, id);
    if (found == nullptr)
        return false;

    TDTree value = found->data; // сохраняем данные до удаления узла
    removeNode(root, value); // убираем запись с прежней позиции
    value.score = new_score;
    insertNode(root, value); // вставляем по новому ключу
    return true;
}

// Удаляет всё дерево обратным обходом: сначала поддеревья, потом сам узел
void deleteTree(Node*& ptr) {
    if (ptr != nullptr) {
        deleteTree(ptr->left_ptr);
        deleteTree(ptr->right_ptr);
        delete ptr;
        ptr = nullptr;
    }
}

// Узлы с наименьшим и наибольшим ключом

// Возвращает самый левый узел поддерева - с наименьшим ключом
Node* minNode(Node* ptr) {
    if (ptr == nullptr)
        return nullptr;
    while (ptr->left_ptr != nullptr)
        ptr = ptr->left_ptr;
    return ptr;
}

// Возвращает самый правый узел поддерева - с наибольшим ключом
Node* maxNode(Node* ptr) {
    if (ptr == nullptr)
        return nullptr;
    while (ptr->right_ptr != nullptr)
        ptr = ptr->right_ptr;
    return ptr;
}

// Обходы для вывода рейтинга

// Обратный симметричный обход (правое поддерево, корень, левое поддерево).
// Узлы выдаются в порядке убывания ключа, то есть по убыванию баллов.
void reverseInorder(Node* ptr, vector<TDTree>& out) {
    if (ptr != nullptr) {
        reverseInorder(ptr->right_ptr, out);
        out.push_back(ptr->data);
        reverseInorder(ptr->left_ptr, out);
    }
}

// Первые k записей рейтинга: тот же обратный обход, но обход прекращается,
// как только набрано k записей.
void topRecords(Node* ptr, int k, vector<TDTree>& out) {
    if (ptr == nullptr || (int)out.size() >= k)
        return;

    topRecords(ptr->right_ptr, k, out);
    if ((int)out.size() >= k) // нужное число уже набрано в правом поддереве
        return;
    out.push_back(ptr->data);
    topRecords(ptr->left_ptr, k, out);
}

// Отбор участников с баллами от min_score до max_score обратным обходом.
// Поддеревья, в которых заведомо нет подходящих участников, не просматриваются.
void rangeRecords(Node* ptr, int min_score, int max_score, vector<TDTree>& out) {
    if (ptr == nullptr)
        return;
    int s = ptr->data.score;

    // в правом поддереве баллы не меньше s: если s больше верхней границы, там подходящих нет
    if (s <= max_score)
        rangeRecords(ptr->right_ptr, min_score, max_score, out);

    if (s >= min_score && s <= max_score)
        out.push_back(ptr->data);

    // в левом поддереве баллы не больше s: если s меньше нижней границы, там подходящих нет
    if (s >= min_score)
        rangeRecords(ptr->left_ptr, min_score, max_score, out);
}

// Поиск участников, у которых хотя бы одно слово имени начинается с заданной строки.
// Регистр букв не учитывается, буквы ё и е считаются разными.
// Обход обратный, поэтому найденные участники идут в порядке рейтинга.
void findByName(Node* ptr, const string& part, vector<TDTree>& out) {
    if (ptr != nullptr) {
        findByName(ptr->right_ptr, part, out);
        if (wordStartsWith(ptr->data.name, part))
            out.push_back(ptr->data);
        findByName(ptr->left_ptr, part, out);
    }
}

// Статистика дерева
// Возвращает число узлов дерева
int countNodes(Node* ptr) {
    if (ptr == nullptr)
        return 0;
    return 1 + countNodes(ptr->left_ptr) + countNodes(ptr->right_ptr);
}

// Возвращает высоту дерева - число уровней (у пустого дерева 0, у одного узла 1)
int treeHeight(Node* ptr) {
    if (ptr == nullptr)
        return 0;
    int left_h = treeHeight(ptr->left_ptr);
    int right_h = treeHeight(ptr->right_ptr);
    return 1 + (left_h > right_h ? left_h : right_h);
}

// Считает участников, у которых баллов больше заданного числа.
// Место участника в рейтинге равно этому числу плюс один.
int countGreater(Node* ptr, int score) {
    if (ptr == nullptr)
        return 0;
    if (ptr->data.score > score) {
        // узел подходит; в правом поддереве баллы ещё больше - подходят все его узлы
        return 1 + countNodes(ptr->right_ptr) + countGreater(ptr->left_ptr, score);
    }
    // баллы узла не больше заданных; в левом поддереве баллы не больше - его пропускаем
    return countGreater(ptr->right_ptr, score);
}

// Собирает строки схемы дерева. правое поддерево (выше в рейтинге) - над узлом, левое (ниже в рейтинге) - под ним.
// prefix - отступ с вертикальными линиями ветвей предков,
// branch - как узел связан с родителем: 0 - корень, 1 - верхняя ветвь, 2 - нижняя ветвь.
void collectTreeLines(Node* ptr, const string& prefix, int branch, vector<TreeLine>& out) {
    if (ptr == nullptr)
        return;

    // Ветвь от родителя к узлу проходит мимо одного из поддеревьев узла: у верхнего узла - мимо левого (оно ниже), у нижнего - мимо правого (оно выше).
    // Рядом с этим поддеревом рисуется вертикальная линия.
    string up_prefix = prefix; // отступ для правого поддерева
    string down_prefix = prefix; // отступ для левого поддерева
    if (branch == 1) {
        up_prefix += "    ";
        down_prefix += "|   ";
    }
    else if (branch == 2) {
        up_prefix += "|   ";
        down_prefix += "    ";
    }

    collectTreeLines(ptr->right_ptr, up_prefix, 1, out);

    TreeLine line;
    line.prefix = prefix + (branch == 1 ? "/-- " : branch == 2 ? "\\-- " : "");
    line.data = ptr->data;
    out.push_back(line);

    collectTreeLines(ptr->left_ptr, down_prefix, 2, out);
}

// вспомогательные функции
// Приводит букву к виду для поиска (кодировка Windows-1251). Заглавные латинские и русские буквы становятся строчными
static char searchChar(char c) {
    unsigned char u = (unsigned char)c;
    if (u >= 'A' && u <= 'Z')
        return (char)(u + 32);
    if (u >= 192 && u <= 223) // А-Я
        return (char)(u + 32);
    if (u == 168) // Ё
        return (char)184; // ё
    return c;
}

// Приводит всю строку к виду для поиска
static string searchText(const string& s) {
    string res = s;
    for (size_t i = 0; i < res.size(); i++)
        res[i] = searchChar(res[i]);
    return res;
}

// Проверяет, начинается ли с part хотя бы одно слово имени. Слово начинается в начале имени и после пробела или дефиса (Салтыков-Щедрин - два слова). Регистр букв не учитывается.
static bool wordStartsWith(const string& name, const string& part) {
    string n = searchText(name);
    string p = searchText(part);
    for (size_t i = 0; i < n.size(); i++) {
        bool word_start = (i == 0 || n[i - 1] == ' ' || n[i - 1] == '-');
        if (word_start && n.compare(i, p.size(), p) == 0)
            return true;
    }
    return false;
}