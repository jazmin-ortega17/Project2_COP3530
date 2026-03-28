//
// Created by Jazmin Selene Ortega on 24/03/26.
//
// Created by Anthony Lapsley on 3/28/26.
//

#include <iostream>
#include <string>
#include <vector>

#include "BTree.h"
#include "DataLoader.h"
#include "SplaySearchTree.h"
#include "RecommendationEngine.h"

static void printMenu() {
    std::cout << "\n-------- Book recommender (readability) --------\n";
    std::cout << "  1) Enter a readability score (1 - 20)\n";
    std::cout << "  2) Run benchmark on a few sample scores\n";
    std::cout << "  3) Quit\n";
    std::cout << "Choice: ";
}

static bool parseScore(const std::string& s, double& out) {
    try {
        size_t idx = 0;
        double v = std::stod(s, &idx);
        if (idx != s.size()) return false;
        out = v;
        return true;
    } catch (...) {
        return false;
    }
}

int main() {
    std::cout << "Loading csv + building both trees (be patient, its a lot of rows)\n";

    std::string realCsv = "classics.csv";
    std::string bigCsv = "output_100k.csv";

    std::vector<ScoredBook> books = loadGeneratedCSV(bigCsv, realCsv);
    if (books.empty()) {
        std::cerr << "Couldnt load data. Put classics.csv and output_100k.csv next to the exe.\n";
        return 1;
    }

    BTree btree;
    SplaySearchTree splay;

    RecommendationEngine engine;
    engine.setBTree(&btree);
    engine.setSplayTree(&splay);

    try {
        engine.loadDataset(books);
    } catch (const std::exception& e) {
        std::cerr << "loadDataset failed: " << e.what() << "\n";
        return 1;
    }

    std::cout << "Loaded " << engine.totalBooks() << " records total.\n";

    for (;;) {
        printMenu();
        std::string line;
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;

        int opt = line[0] - '0';
        if (opt == 3) {
            std::cout << "bye\n";
            break;
        }

        if (opt == 2) {
            std::mt19937 rng(42);
            std::uniform_real_distribution<double> dist(1.0, 20.0);
            std::vector<double> qs;
            for (int i = 0; i < 1000; i++) qs.push_back(dist(rng));
            auto reps = engine.benchmark(qs, 5, true);
            for (const auto& r : reps)
                RecommendationEngine::printReport(r);
            if (reps.size() == 2)
                RecommendationEngine::printComparison(reps[0], reps[1]);
            continue;
        }

        if (opt != 1) {
            std::cout << "Invalid choice.\n";
            continue;
        }

        std::cout << "Type a readability score between 1 and 20: ";
        if (!std::getline(std::cin, line)) break;
        double score = 0;
        if (!parseScore(line, score)) {
            std::cout << "That wasnt a number.\n";
            continue;
        }
        if (score < 1.0 || score > 20.0) {
            std::cout << "Please stay inside 1 - 20 (inclusive).\n";
            continue;
        }

        auto both = engine.queryBoth(score, 5);
        std::cout << "\n--- Recommendations ---\n";
        RecommendationEngine::printResult(both.first);
        std::cout << "\n";
        RecommendationEngine::printResult(both.second);

        std::cout << "\n(single query times printed above in microseconds)\n";
    }

    return 0;
}
