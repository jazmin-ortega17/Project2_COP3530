//
// Created by Jazmin Selene Ortega on 24/03/26.
//

#ifndef PROJECT2_COP3530_BOOK_H
#define PROJECT2_COP3530_BOOK_H


#include <string>
using namespace std;

struct Book {
    int _id;
    string _title;
    int _readingLevel;
    Book* _left;
    Book* _right;
    Book(int id, string title, int level) : _id(id), _title(title), _readingLevel(0), _left(nullptr), _right(nullptr) {};
};

#endif //PROJECT2_COP3530_BOOK_H