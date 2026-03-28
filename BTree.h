//
// Created by Anthony Lapsley on 3/28/26.
//

#pragma once

#include "RecommendationEngine.h"
#include <vector>

class BTree : public ISearchTree {
public:
    BTree();
    ~BTree();

    void insert(double key, const ScoredBook& book) override;
    std::vector<ScoredBook> rangeSearch(double lo, double hi) override;
    int getLastComparisonCount() const override { return last_cmp_; }
    std::string name() const override { return "BTree"; }

private:
    static constexpr int MIN_DEG = 3;
    static constexpr int MAX_KEYS = 2 * MIN_DEG - 1;

    struct LeafEntry {
        double key;
        long long seq;
        ScoredBook book;
    };

    struct Node {
        bool leaf = true;
        std::vector<LeafEntry> items;
        std::vector<double> keys;
        std::vector<Node*> child;
    };

    Node* root = nullptr;
    long long next_seq = 0;
    mutable int last_cmp_ = 0;

    void clearRec(Node* n);
    int cmpKey(double a, double b) const;

    void insertIntoLeaf(Node* leaf, const LeafEntry& e);
    void splitRoot();
    void insertInternal(Node* node, const LeafEntry& e);
    void splitChild(Node* parent, int idx);
    void rangeRec(Node* n, double lo, double hi, std::vector<ScoredBook>& out) const;
};
