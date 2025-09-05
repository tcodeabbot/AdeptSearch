#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

// posting: document id -> term frequency
using DocId = int;
struct Posting
{
    DocId doc_id;
    int tf;
};

class Indexer
{
public:
    Indexer();
    // index one document (content)
    void index_document(const std::string &doc_path, const std::string &content);
    // bulk index from file paths (multithreaded)
    void index_files(const std::vector<std::string> &paths, int num_threads = 4);
    // search support (friend to Searcher)
    const std::unordered_map<std::string, std::vector<Posting>> &get_index() const;
    int doc_count() const;
    int doc_length(DocId id) const;
    void dump(const std::string &path) const;
    void load(const std::string &path);

private:
    mutable std::mutex mtx;
    std::unordered_map<std::string, std::vector<Posting>> inverted;
    std::unordered_map<DocId, int> doc_lengths;
    std::unordered_map<std::string, DocId> path_to_id;
    int next_doc_id;
    // helper
    DocId assign_doc_id(const std::string &path);
};
