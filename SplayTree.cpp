//
// Created by Jazmin Selene Ortega on 22/03/26.
//

#include "SplayTree.h"

SplayTree::SplayTree() {
    _root = nullptr;
}

SplayTree::~SplayTree() {
}

bool SplayTree::insert(int id, string title, int level) {
    this->_root = helperInsert(this->_root, id, title, level);
    this->_root = splay(this->_root, level, id);
    return true;
}

Book *SplayTree::helperInsert(Book *helpRoot, int id, string title, int level) {
    if (helpRoot == nullptr) {
        return new Book(id, title, level);
    } else if (level < helpRoot->_readingLevel || (level == helpRoot->_readingLevel && id < helpRoot->_id)) {
        helpRoot->_left = helperInsert(helpRoot->_left, id, title, level);
    } else {
        helpRoot->_right = helperInsert(helpRoot->_right, id, title, level);
    }
    return helpRoot;
}

Book *SplayTree::zag(Book *helpRoot) {
    if (helpRoot != nullptr && helpRoot->_right != nullptr) {
        Book* tempNode = helpRoot;
        Book* tempNode2 = helpRoot->_right->_left;
        helpRoot = helpRoot->_right;
        helpRoot->_left = tempNode;
        helpRoot->_left->_right = tempNode2;
    }
    return helpRoot;
}

Book* SplayTree::zig(Book* helpRoot) {
    if (helpRoot != nullptr && helpRoot->_left != nullptr) {
        Book* tempNode = helpRoot;
        Book* tempNode2 = helpRoot->_left->_right;
        helpRoot = helpRoot->_left;
        helpRoot->_right = tempNode;
        helpRoot->_right->_left = tempNode2;
    }
    return helpRoot;
}
Book* SplayTree::splay(Book *helpRoot, int level, int id) {
    if (helpRoot == nullptr) {
        return nullptr;
    }
    if (level == helpRoot->_readingLevel && id == helpRoot->_id) {
        return helpRoot;
    }
    if (level < helpRoot->_readingLevel || (level == helpRoot->_readingLevel && id < helpRoot->_id)) {
        // Explore left subtree
        if ((helpRoot->_left != nullptr) && (level < helpRoot->_left->_readingLevel || (level == helpRoot->_left->_readingLevel && id < helpRoot->_left->_id))) {
            // Zig-Zig case
            Book* rotateNode = zig(helpRoot->_left);
            helpRoot->_left = rotateNode;
            return splay(zig(helpRoot), level, id);
        } else if ((helpRoot->_left != nullptr) && (level > helpRoot->_left->_readingLevel || (level == helpRoot->_left->_readingLevel && id > helpRoot->_left->_id))) {
            // Zig-zag case
            Book* rotateNode = zag(helpRoot->_left);
            helpRoot->_left = rotateNode;
            return splay(zig(helpRoot), level, id);
        }
        // Zig case
        if (helpRoot->_left != nullptr) {
            helpRoot = zig(helpRoot);
        }
        return helpRoot;
    } else if (level > helpRoot->_readingLevel || (level == helpRoot->_readingLevel && id > helpRoot->_id)) {
        // Explore right subtree
        if ((helpRoot->_right != nullptr) && (level > helpRoot->_right->_readingLevel || (level == helpRoot->_right->_readingLevel && id > helpRoot->_right->_id))) {
            // Zag-Zag case
            Book* rotateNode = zag(helpRoot->_right);
            helpRoot->_right = rotateNode;
            return splay(zag(helpRoot), level, id);
        } else if ((helpRoot->_right != nullptr) && (level < helpRoot->_right->_readingLevel || (level == helpRoot->_right->_readingLevel && id < helpRoot->_right->_id))) {
            // Zag-zig case
            Book* rotateNode = zig(helpRoot->_right);
            helpRoot->_right = rotateNode;
            return splay(zag(helpRoot), level, id);
        }
        // Zag case
        if (helpRoot->_right != nullptr) {
            helpRoot = zag(helpRoot);
        }
        return helpRoot;
    }
    return helpRoot;
}