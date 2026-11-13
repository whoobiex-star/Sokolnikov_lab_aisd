#ifndef TREE_H
#define TREE_H

#include <string>
#include <vector>

// Данные одного участника
struct Participant {
    int id; // уникальный идентификатор (0-100)
    std::string name; // имя участника
    int score; // количество баллов (0-100)
};

typedef Participant TDTree; // тип данных узла дерева
// Узел бинарного дерева поиска
struct Node {
    TDTree data; // данные участника
    Node* left_ptr; // левое поддерево: ключи меньше
    Node* right_ptr; // правое поддерево: ключи больше
};

// Строка схемы дерева: отступ с ветвями и данные узла
struct TreeLine {
    std::string prefix; // отступ, вертикальные линии и ветвь перед узлом
    TDTree data; // данные участника
};

// Создание узла и сравнение ключей
Node* createNode(const TDTree& value); // новый узел с пустыми ссылками
bool keyLess(const TDTree& a, const TDTree& b); // true, если ключ a меньше ключа b

// Основные операции
void insertNode(Node*& ptr, const TDTree& value); // вставка по ключу (баллы, идентификатор)
Node* findById(Node* ptr, int id); // поиск обходом; nullptr, если участника нет
bool removeNode(Node*& ptr, const TDTree& key); // удаление узла с заданным ключом
bool removeById(Node*& root, int id); // удаление участника по идентификатору
bool changeScore(Node*& root, int id, int new_score); // изменение баллов с переносом узла
void deleteTree(Node*& ptr); // удаление всего дерева

// Узлы с наименьшим и наибольшим ключом
Node* minNode(Node* ptr); // самый левый узел поддерева
Node* maxNode(Node* ptr); // самый правый узел поддерева

// Обходы для вывода рейтинга
void reverseInorder(Node* ptr, std::vector<TDTree>& out); // все участники по убыванию баллов
void topRecords(Node* ptr, int k, std::vector<TDTree>& out); // первые k записей рейтинга
void rangeRecords(Node* ptr, int min_score, int max_score, std::vector<TDTree>& out); // участники с баллами в диапазоне
void findByName(Node* ptr, const std::string& part, std::vector<TDTree>& out); // поиск по началу слова имени

// Статистика дерева
int countNodes(Node* ptr); // число участников
int treeHeight(Node* ptr); // высота дерева (у пустого 0)
int countGreater(Node* ptr, int score); // число участников с баллами больше заданных
void collectTreeLines(Node* ptr, const std::string& prefix, int branch, std::vector<TreeLine>& out); // строки схемы дерева

#endif