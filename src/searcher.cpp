#include "searcher.h"
#include "tokenizer.h"
#include <cmath>
#include <algorithm>
#include <iostream>

Searcher::Searcher(const Indexer &idx)
    : indexer(idx),
      k1(1.2),
      b(0.75),
      inv(idx.get_index())
{
    N = indexer.doc_count();
    double sumlen = 0;
    for (auto &p : inv)
    {
        for (auto &post : p.second)
        {
            sumlen += indexer.doc_length(post.doc_id);
        }
    }
    avg_doc_len = N > 0 ? sumlen / N : 0.0;
}

std::vector<std::string> Searcher::get_terms(const std::string &q) const
{
    Tokenizer t;
    return t.tokenize(q);
}

double Searcher::idf(const std::string &term) const
{
    auto it = inv.find(term);
    if (it == inv.end())
        return 0.0;
    int df = it->second.size();
    if (df == 0)
        return 0.0;
    return std::log((N - df + 0.5) / (df + 0.5) + 1.0);
}

std::vector<SearchResult> Searcher::query(const std::string &q, int topk) const
{
    auto terms = get_terms(q);
    std::unordered_map<DocId, double> scores;
    for (const auto &term : terms)
    {
        auto it = inv.find(term);
        if (it == inv.end())
            continue;
        double term_idf = idf(term);
        for (const auto &post : it->second)
        {
            double tf = post.tf;
            double dl = indexer.doc_length(post.doc_id);
            double denom = tf + k1 * (1 - b + b * dl / (avg_doc_len + 1e-9));
            double score = term_idf * ((tf * (k1 + 1)) / denom);
            scores[post.doc_id] += score;
        }
    }
    std::vector<SearchResult> res;
    res.reserve(scores.size());
    for (auto &p : scores)
        res.push_back({p.first, p.second});
    std::sort(res.begin(), res.end(), [](const SearchResult &a, const SearchResult &b)
              { return a.score > b.score; });
    if ((int)res.size() > topk)
        res.resize(topk);
    return res;
}
