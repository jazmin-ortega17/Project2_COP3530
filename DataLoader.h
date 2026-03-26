//
// Created by Callahan Bonifant on 3/7/26.
//
#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "RecommendationEngine.h"

// CSV Parsing
std::vector<std::string> parseCSVLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (c == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                field += '"';
                i++;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            fields.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }
    fields.push_back(field);
    return fields;
}


// Print the header row so the user can identify
// which column index maps to which field
void printHeader(const std::string& csvPath) {
    std::ifstream f(csvPath);
    if (!f.is_open()) {
        std::cerr << "Cannot open: " << csvPath << "\n";
        return;
    }
    std::string line;
    std::getline(f, line);
    auto fields = parseCSVLine(line);

    std::cout << "\nColumns found in " << csvPath << ":\n";
    for (size_t i = 0; i < fields.size(); i++) {
        std::cout << "  [" << i << "] " << fields[i] << "\n";
    }
    std::cout << "\n";
}


// Column index config
struct ColumnMap {
    int title    = 3;   // "bibliography.title"
    int author   = 11;  // "bibliography.author.name"
    int subject  = 2;   // "bibliography.subjects"
    int fk       = 23;  // "metrics.difficulty.flesch kincaid grade"
    int cli      = 20;  // "metrics.difficulty.coleman liau index"
    int ari      = 19;  // "metrics.difficulty.automated readability index"
};

// Load CORGIS classics CSV into a vector of
// ScoredBook with is_real = true
std::vector<ScoredBook> loadCORGIS(const std::string& csvPath,
                                   const ColumnMap& cols = {}) {
    std::vector<ScoredBook> books;

    std::ifstream f(csvPath);
    if (!f.is_open()) {
        std::cerr << "[loadCORGIS] Error: cannot open file: " << csvPath << "\n";
        return books;
    }

    // Skip header row
    std::string line;
    std::getline(f, line);

    int lineNum  = 1;
    int skipped  = 0;
    int loaded   = 0;

    int maxCol = std::max({ cols.title, cols.author, cols.subject,
                            cols.fk, cols.cli, cols.ari });

    while (std::getline(f, line)) {
        lineNum++;
        if (line.empty()) continue;

        auto fields = parseCSVLine(line);

        // Skip malformed rows
        if ((int)fields.size() <= maxCol) {
            skipped++;
            continue;
        }

        // Skip rows with empty required fields
        if (fields[cols.title].empty() || fields[cols.fk].empty()) {
            skipped++;
            continue;
        }

        ScoredBook b;
        b.is_real = true;
        b.title   = fields[cols.title];
        b.author  = fields[cols.author];
        b.subject = fields[cols.subject];

        try {
            b.flesch_kincaid        = std::stod(fields[cols.fk]);
            b.coleman_liau          = std::stod(fields[cols.cli]);
            b.automated_readability = std::stod(fields[cols.ari]);
        } catch (...) {
            skipped++;
            continue; // non-numeric score field
        }

        b.composite_score = (b.flesch_kincaid +
                             b.coleman_liau   +
                             b.automated_readability) / 3.0;

        // Clamp to grade range [0, 20]
        b.composite_score = std::max(0.0, std::min(20.0, b.composite_score));

        books.push_back(b);
        loaded++;
    }

    std::cout << "[loadCORGIS] Loaded " << loaded  << " real books"
              << " | Skipped " << skipped << " rows"
              << " from " << csvPath << "\n";

    return books;
}

std::vector<ScoredBook> loadGeneratedCSV(const std::string& syntheticPath,
                                         const std::string& realPath = "") {
    std::vector<ScoredBook> books;

    // Load real books first
    if (!realPath.empty()) {
        ColumnMap cols; // uses corrected CORGIS defaults above
        std::vector<ScoredBook> realBooks = loadCORGIS(realPath, cols);
        for (auto& b : realBooks) books.push_back(b);
    }

    // Load synthetic books
    std::ifstream f(syntheticPath);
    if (!f.is_open()) {
        std::cerr << "[loadGeneratedCSV] Cannot open: " << syntheticPath << "\n";
        return books;
    }

    std::string line;
    std::getline(f, line);

    int loaded = 0, skipped = 0;

    while (std::getline(f, line)) {
        if (line.empty()) continue;
        auto fields = parseCSVLine(line);
        if ((int)fields.size() < 7) {
            skipped++;
            continue;
        }

        ScoredBook b;
        b.title   = fields[0];
        b.author  = fields[1];
        b.subject = fields[2];
        b.is_real = false;

        try {
            b.flesch_kincaid        = std::stod(fields[3]);
            b.coleman_liau          = std::stod(fields[4]);
            b.automated_readability = std::stod(fields[5]);
            b.composite_score       = std::stod(fields[6]);
        } catch (...) { skipped++; continue; }

        books.push_back(b);
        loaded++;
    }

    // Summary
    int real = 0, synthetic = 0;
    for (auto& b : books) b.is_real ? real++ : synthetic++;

    std::cout << "[loadGeneratedCSV] Total books : " << books.size()  << "\n"
              << "                   Real         : " << real          << "\n"
              << "                   Synthetic    : " << synthetic     << "\n";

    return books;
}
