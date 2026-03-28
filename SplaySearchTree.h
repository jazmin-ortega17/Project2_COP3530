//
// Created by Anthony Lapsley on 3/28/26.
//

#pragma once

#include "RecommendationEngine.h"

class SplaySearchTree : public ISearchTree {
public:
    SplaySearchTree();
    ~SplaySearchTree();

    void insert(double key, const ScoredBook& book) override;
    std::vector<ScoredBook> rangeSearch(double lo, double hi) override;
    int getLastComparisonCount() const override { return last_cmp_; }
    std::string name() const override { return "SplayTree"; }

private:
    struct Node {
        double key;
        long long seq;
        ScoredBook data;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;
    };

    Node* root = nullptr;
    long long next_seq = 0;
    mutable int last_cmp_ = 0;

    void freeAll(Node* n);
    int cmpDouble(double a, double b) const;
    int cmpPair(double ka, long long sa, double kb, long long sb) const;

    void rotateLeft(Node* x);
    void rotateRight(Node* x);
    void splay(Node* x);

    void inorderCollect(Node* n, double lo, double hi,
                        std::vector<ScoredBook>& out) const;
};
