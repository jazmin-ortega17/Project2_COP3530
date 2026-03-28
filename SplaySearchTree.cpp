//
// Created by Anthony Lapsley on 3/28/26.
//

#include "SplaySearchTree.h"

SplaySearchTree::SplaySearchTree() = default;

SplaySearchTree::~SplaySearchTree() {
    freeAll(root);
    root = nullptr;
}

void SplaySearchTree::freeAll(Node* n) {
    if (!n) return;
    freeAll(n->left);
    freeAll(n->right);
    delete n;
}

int SplaySearchTree::cmpDouble(double a, double b) const {
    last_cmp_++;
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

int SplaySearchTree::cmpPair(double ka, long long sa, double kb,
                             long long sb) const {
    int c = cmpDouble(ka, kb);
    if (c != 0) return c;
    last_cmp_++;
    if (sa < sb) return -1;
    if (sa > sb) return 1;
    return 0;
}

void SplaySearchTree::rotateRight(Node* x) {
    Node* p = x->parent;
    Node* y = x->left;
    if (!y) return;
    x->left = y->right;
    if (y->right)
        y->right->parent = x;
    y->parent = p;
    if (!p)
        root = y;
    else if (p->left == x)
        p->left = y;
    else
        p->right = y;
    y->right = x;
    x->parent = y;
}

void SplaySearchTree::rotateLeft(Node* x) {
    Node* p = x->parent;
    Node* y = x->right;
    if (!y) return;
    x->right = y->left;
    if (y->left)
        y->left->parent = x;
    y->parent = p;
    if (!p)
        root = y;
    else if (p->left == x)
        p->left = y;
    else
        p->right = y;
    y->left = x;
    x->parent = y;
}

void SplaySearchTree::splay(Node* x) {
    while (x && x->parent) {
        Node* p = x->parent;
        Node* g = p->parent;
        if (!g) {
            if (p->left == x)
                rotateRight(p);
            else
                rotateLeft(p);
        } else if (g->left == p && p->left == x) {
            rotateRight(g);
            rotateRight(p);
        } else if (g->right == p && p->right == x) {
            rotateLeft(g);
            rotateLeft(p);
        } else if (g->left == p && p->right == x) {
            rotateLeft(p);
            rotateRight(g);
        } else {
            rotateRight(p);
            rotateLeft(g);
        }
    }
}

void SplaySearchTree::insert(double key, const ScoredBook& book) {
    last_cmp_ = 0;
    long long seq = next_seq++;

    Node* cur = root;
    Node* par = nullptr;
    bool goLeft = false;

    while (cur) {
        par = cur;
        int c = cmpPair(key, seq, cur->key, cur->seq);
        if (c < 0) {
            cur = cur->left;
            goLeft = true;
        } else {
            cur = cur->right;
            goLeft = false;
        }
    }

    Node* n = new Node{key, seq, book, nullptr, nullptr, nullptr};
    if (!par) {
        root = n;
        return;
    }
    n->parent = par;
    if (goLeft)
        par->left = n;
    else
        par->right = n;
    splay(n);
}

void SplaySearchTree::inorderCollect(Node* n, double lo, double hi,
                                     std::vector<ScoredBook>& out) const {
    if (!n) return;
    inorderCollect(n->left, lo, hi, out);
    if (cmpDouble(n->key, lo) >= 0 && cmpDouble(n->key, hi) <= 0)
        out.push_back(n->data);
    inorderCollect(n->right, lo, hi, out);
}

std::vector<ScoredBook> SplaySearchTree::rangeSearch(double lo, double hi) {
    last_cmp_ = 0;
    std::vector<ScoredBook> out;
    inorderCollect(root, lo, hi, out);
    return out;
}
