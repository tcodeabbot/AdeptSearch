#pragma once
#include "indexer.h"
#include <vector>
#include <string>
#include <unordered_map>

struct SearchResult
{
    DocId doc_id;
    double score;
};

class Searcher
{
public:
    Searcher(const Indexer &idx);
    std::vector<SearchResult> query(const std::string &q, int topk = 10) const;

private:
    const Indexer &indexer;
    double avg_doc_len;
    double k1;
    double b;
    int N; // number of docs
    std::unordered_map<std::string, std::vector<Posting>> const &inv;
    std::vector<std::string> get_terms(const std::string &q) const;
    double idf(const std::string &term) const;
};
