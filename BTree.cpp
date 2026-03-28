//
// Created by Anthony Lapsley on 3/28/26.
//

#include "BTree.h"
#include <algorithm>

BTree::BTree() = default;

BTree::~BTree() {
    clearRec(root);
    root = nullptr;
}

void BTree::clearRec(Node* n) {
    if (!n) return;
    if (!n->leaf) {
        for (Node* c : n->child)
            clearRec(c);
    }
    delete n;
}

int BTree::cmpKey(double a, double b) const {
    last_cmp_++;
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

void BTree::insertIntoLeaf(Node* leaf, const LeafEntry& e) {
    auto it = std::lower_bound(
        leaf->items.begin(), leaf->items.end(), e,
        [&](const LeafEntry& a, const LeafEntry& b) {
            int ck = cmpKey(a.key, b.key);
            if (ck != 0) return ck < 0;
            last_cmp_++;
            return a.seq < b.seq;
        });
    leaf->items.insert(it, e);
}

void BTree::splitChild(Node* parent, int idx) {
    Node* y = parent->child[idx];
    if (y->leaf) {
        int mid = static_cast<int>(y->items.size()) / 2;
        Node* z = new Node;
        z->leaf = true;
        z->items.assign(y->items.begin() + mid, y->items.end());
        y->items.resize(mid);

        double upKey = z->items.front().key;
        parent->keys.insert(parent->keys.begin() + idx, upKey);
        parent->child.insert(parent->child.begin() + idx + 1, z);
    } else {
        int midKey = static_cast<int>(y->keys.size()) / 2;
        double pushUp = y->keys[midKey];

        Node* z = new Node;
        z->leaf = false;
        z->keys.assign(y->keys.begin() + midKey + 1, y->keys.end());
        z->child.assign(y->child.begin() + midKey + 1, y->child.end());

        y->keys.resize(midKey);
        y->child.resize(midKey + 1);

        parent->keys.insert(parent->keys.begin() + idx, pushUp);
        parent->child.insert(parent->child.begin() + idx + 1, z);
    }
}

void BTree::splitRoot() {
    Node* s = new Node;
    s->leaf = false;
    s->child.push_back(root);
    root = s;
    splitChild(s, 0);
}

void BTree::insertInternal(Node* node, const LeafEntry& e) {
    if (node->leaf) {
        insertIntoLeaf(node, e);
        return;
    }

    int i = 0;
    while (i < (int)node->keys.size() && cmpKey(e.key, node->keys[i]) >= 0)
        i++;

    Node* down = node->child[i];
    bool downLeafFull = down && down->leaf && (int)down->items.size() == MAX_KEYS;
    bool downInternalFull = down && !down->leaf && (int)down->keys.size() == MAX_KEYS;
    if (downLeafFull || downInternalFull) {
        splitChild(node, i);
        if (cmpKey(e.key, node->keys[i]) >= 0)
            i++;
    }
    insertInternal(node->child[i], e);
}

void BTree::insert(double key, const ScoredBook& book) {
    last_cmp_ = 0;

    LeafEntry e{key, next_seq++, book};

    if (!root) {
        root = new Node;
        root->leaf = true;
        root->items.push_back(e);
        return;
    }

    if (root->leaf) {
        if ((int)root->items.size() < MAX_KEYS) {
            insertIntoLeaf(root, e);
            return;
        }
        splitRoot();
        insertInternal(root, e);
        return;
    }

    bool rootLeafFull = root->leaf && (int)root->items.size() == MAX_KEYS;
    bool rootInternalFull = !root->leaf && (int)root->keys.size() == MAX_KEYS;
    if (rootLeafFull || rootInternalFull)
        splitRoot();

    insertInternal(root, e);
}

void BTree::rangeRec(Node* n, double lo, double hi, std::vector<ScoredBook>& out) const {
    if (!n) return;

    if (n->leaf) {
        for (const LeafEntry& e : n->items) {
            if (cmpKey(e.key, lo) >= 0 && cmpKey(e.key, hi) <= 0)
                out.push_back(e.book);
        }
        return;
    }

    for (Node* c : n->child)
        rangeRec(c, lo, hi, out);
}

std::vector<ScoredBook> BTree::rangeSearch(double lo, double hi) {
    last_cmp_ = 0;
    std::vector<ScoredBook> ans;
    if (!root) return ans;
    rangeRec(root, lo, hi, ans);
    return ans;
}
