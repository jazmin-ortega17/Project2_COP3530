//
// Created by Callahan Bonifant on 3/7/26.
//

#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <stdexcept>

// #include "BTree.h"
// #include "SplayTree.h"

struct ScoredBook {
    std::string title;
    std::string author;
    std::string subject;
    double flesch_kincaid = 0.0;
    double coleman_liau = 0.0;
    double automated_readability = 0.0;
    double composite_score = 0.0;
    bool   is_real = false; // false = synthetic padding
};

// Output of a single recommendation query
struct QueryResult {
    std::vector<ScoredBook> books;
    double target_score = 0.0;
    double delta = 0.0;
    long long time_us = 0;
    int comparisons = 0;
    int candidates_found = 0;
    int real_books_found = 0;
    std::string tree_used;
};

struct BenchmarkReport {
    std::string tree_name;
    int total_queries = 0;
    double avg_time_us = 0.0;
    double avg_comparisons = 0.0;
    double avg_results = 0.0;
    long long total_time_us = 0;
    int queries_with_results = 0;
};

// RecommendationEngine configuration
struct RecommendationConfig {
    double default_delta = 1.0;
    int default_top_k = 5;
    double delta_step = 0.5;
    int max_expansions = 6;
};

// ISearchTree interface both trees must satisfy
class ISearchTree {
public:
    virtual ~ISearchTree() = default;

    virtual void insert(double key, const ScoredBook& book) = 0;

    virtual std::vector<ScoredBook> rangeSearch(double lo, double hi) = 0;

    virtual int getLastComparisonCount() const = 0;

    // Name for logging ("BTree" / "SplayTree")
    virtual std::string name() const = 0;
};

class RecommendationEngine {
public:

    explicit RecommendationEngine(RecommendationConfig cfg = {})
            : config_(cfg) {}

    // Tree Registration
    void setBTree(ISearchTree* tree) {
        if (!tree) throw std::invalid_argument("BTree pointer is null");
        btree_ = tree;
    }

    void setSplayTree(ISearchTree* tree) {
        if (!tree) throw std::invalid_argument("SplayTree pointer is null");
        splay_ = tree;
    }

    void loadDataset(const std::vector<ScoredBook>& books) {
        if (!btree_ || !splay_) {
            throw std::runtime_error("Both trees must be set before calling loadDataset()");
        }
        for (const auto& b : books) {
            btree_->insert(b.composite_score, b);
            splay_->insert(b.composite_score, b);
        }
        total_books_ = static_cast<int>(books.size());
        real_books_ = 0;
        for (const auto& b : books)
            if (b.is_real) real_books_++;

        std::cout << "[RecommendationEngine] Loaded " << total_books_
                  << " books (" << real_books_ << " real, "
                  << (total_books_ - real_books_) << " synthetic).\n";
    }


    // Ask one tree for up to topK real books near targetScore
    // Automatically widens the search window if too few results
    QueryResult query(double targetScore,
                      int topK = -1,
                      bool useBTree = true) {
        ISearchTree* tree = selectTree(useBTree);
        if (!tree) throw std::runtime_error("Requested tree is not set");

        int k = (topK < 1) ? config_.default_top_k : topK;
        double delta = config_.default_delta;

        QueryResult result;
        result.target_score = targetScore;
        result.tree_used = tree->name();

        auto start = now();

        // Widen the search window until we find enough real books
        for (int expansion = 0; expansion <= config_.max_expansions; expansion++) {

            std::vector<ScoredBook> candidates =
                    tree->rangeSearch(targetScore - delta, targetScore + delta);

            result.comparisons      = tree->getLastComparisonCount();
            result.candidates_found = static_cast<int>(candidates.size());

            // Filter to real books only
            std::vector<ScoredBook> real;
            for (auto& b : candidates)
                if (b.is_real) real.push_back(b);

            result.real_books_found = static_cast<int>(real.size());

            if (static_cast<int>(real.size()) >= k ||
                expansion == config_.max_expansions) {
                // Sort by closeness to target score
                std::sort(real.begin(), real.end(),
                          [&](const ScoredBook& a, const ScoredBook& b) {
                              return std::abs(a.composite_score - targetScore) <
                                     std::abs(b.composite_score - targetScore);
                          });

                if (static_cast<int>(real.size()) > k)
                    real.resize(k);

                result.books = std::move(real);
                result.delta = delta;
                break;
            }

            // Widen and try again
            delta += config_.delta_step;
        }

        result.time_us = elapsed_us(start);
        return result;
    }

    // Dual Query
    // Run the same query on BOTH trees and return both results.
    // Useful for side-by-side benchmarking.

    std::pair<QueryResult, QueryResult> queryBoth(double targetScore,
                                                  int topK = -1) {
        return { query(targetScore, topK, true),
                 query(targetScore, topK, false) };
    }

    // Benchmark
    // Runs a list of target scores against one or both trees
    // Returns a report for each tree

    std::vector<BenchmarkReport> benchmark(
            const std::vector<double>& queryScores,
            int  topK = -1,
            bool bothTrees = true)
    {
        std::vector<BenchmarkReport> reports;

        auto runTree = [&](bool useBTree) {
            ISearchTree* tree = selectTree(useBTree);
            if (!tree) return;

            BenchmarkReport rep;
            rep.tree_name = tree->name();
            rep.total_queries = static_cast<int>(queryScores.size());

            long long totalTime = 0;
            long long totalComps = 0;
            long long totalResults = 0;

            for (double score : queryScores) {
                QueryResult r = query(score, topK, useBTree);
                totalTime += r.time_us;
                totalComps += r.comparisons;
                totalResults += static_cast<int>(r.books.size());
                if (!r.books.empty()) rep.queries_with_results++;
            }

            rep.total_time_us = totalTime;
            rep.avg_time_us = static_cast<double>(totalTime)  / rep.total_queries;
            rep.avg_comparisons = static_cast<double>(totalComps) / rep.total_queries;
            rep.avg_results = static_cast<double>(totalResults)/ rep.total_queries;
            reports.push_back(rep);
        };

        runTree(true);  // B-Tree
        if (bothTrees) runTree(false); // Splay Tree

        return reports;
    }

    // Print helpers

    static void printResult(const QueryResult& r) {
        std::cout << "\n" << r.tree_used << " results"
                  << " (target=" << std::fixed << std::setprecision(2)
                  << r.target_score
                  << ", δ=" << r.delta
                  << ", " << r.time_us << "μs"
                  << ", " << r.comparisons << " comparisons) \n";

        if (r.books.empty()) {
            std::cout << "  No real books found in range.\n";
            return;
        }
        int rank = 1;
        for (const auto& b : r.books) {
            std::cout << "  " << rank++ << ". "
                      << b.title << " by " << b.author
                      << "  [score=" << std::fixed << std::setprecision(2)
                      << b.composite_score << ", "
                      << b.subject << "]\n";
        }
    }

    static void printReport(const BenchmarkReport& rep) {
        std::cout << "\nBenchmark: " << rep.tree_name << "\n"
                  << "  Queries run        : " << rep.total_queries       << "\n"
                  << "  Queries with results: "<< rep.queries_with_results<< "\n"
                  << "  Total time         : " << rep.total_time_us << " μs\n"
                  << "  Avg time / query   : " << std::fixed
                  << std::setprecision(2)
                  << rep.avg_time_us   << " μs\n"
                  << "  Avg comparisons    : " << rep.avg_comparisons     << "\n"
                  << "  Avg results        : " << rep.avg_results         << "\n";
    }

    static void printComparison(const BenchmarkReport& bt,
                                const BenchmarkReport& st) {
        std::cout << "\n Head-to-Head Comparison \n"
                  << std::left
                  << std::setw(28) << "Metric"
                  << std::setw(16) << bt.tree_name
                  << std::setw(16) << st.tree_name << "\n"
                  << std::string(60, '-') << "\n";

        auto row = [&](const std::string& label, double a, double b,
                       const std::string& unit = "") {
            std::cout << std::setw(28) << label
                      << std::setw(16) << (std::to_string((int)a) + unit)
                      << std::setw(16) << (std::to_string((int)b) + unit)
                      << "  " << (a < b ? bt.tree_name : st.tree_name) << " wins\n";
        };

        row("Avg time/query (μs)",  bt.avg_time_us,     st.avg_time_us,     " μs");
        row("Avg comparisons",      bt.avg_comparisons, st.avg_comparisons, "");
        row("Queries with results", bt.queries_with_results,
            st.queries_with_results, "");
    }

    // Accessors

    int totalBooks() const { return total_books_; }
    int realBooks()  const { return real_books_;  }
    RecommendationConfig& config() { return config_; }

private:

    // Internal helpers
    ISearchTree* selectTree(bool useBTree) const {
        return useBTree ? btree_ : splay_;
    }

    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    static TimePoint now() { return Clock::now(); }

    static long long elapsed_us(TimePoint start) {
        return std::chrono::duration_cast<std::chrono::microseconds>(
                Clock::now() - start).count();
    }

    // Members
    ISearchTree* btree_ = nullptr;
    ISearchTree* splay_ = nullptr;
    RecommendationConfig config_;
    int total_books_ = 0;
    int real_books_ = 0;
};