//
// Created by Jazmin Selene Ortega on 24/03/26.
//

#ifndef PROJECT2_COP3530_SPLAYTREE_H
#define PROJECT2_COP3530_SPLAYTREE_H


#include "Book.h"


class SplayTree {
private:
    Book* _root = nullptr;
    Book* zag(Book* helpRoot);
    Book* zig(Book* helpRoot);
    Book* helperInsert(Book* helpRoot, int id, string title, int level);
    Book* splay(Book* helpRoot, int level, int id);
public:
    SplayTree();
    ~SplayTree();
    bool insert(int id, string title, int level);
};


#endif //PROJECT2_COP3530_SPLAYTREE_H