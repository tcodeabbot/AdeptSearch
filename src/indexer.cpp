#include "indexer.h"
#include "tokenizer.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

Indexer::Indexer() : next_doc_id(1) {}

Indexer::DocId Indexer::assign_doc_id(const std::string &path)
{
    auto it = path_to_id.find(path);
    if (it != path_to_id.end())
        return it->second;
    DocId id = next_doc_id++;
    path_to_id[path] = id;
    return id;
}

void Indexer::index_document(const std::string &doc_path, const std::string &content)
{
    Tokenizer tok;
    auto tokens = tok.tokenize(content);
    std::unordered_map<std::string, int> freqs;
    for (auto &t : tokens)
        freqs[t]++;

    std::lock_guard<std::mutex> lock(mtx);
    DocId id = assign_doc_id(doc_path);
    int total_terms = 0;
    for (auto &p : freqs)
    {
        const std::string &term = p.first;
        int tf = p.second;
        inverted[term].push_back(Posting{id, tf});
        total_terms += tf;
    }
    doc_lengths[id] = total_terms;
}

// read file helper
static bool read_file_to_string(const std::string &path, std::string &out)
{
    std::ifstream ifs(path);
    if (!ifs)
        return false;
    std::ostringstream ss;
    ss << ifs.rdbuf();
    out = ss.str();
    return true;
}

void Indexer::index_files(const std::vector<std::string> &paths, int num_threads)
{
    std::atomic<size_t> idx{0};
    auto worker = [&]()
    {
        while (true)
        {
            size_t i = idx.fetch_add(1);
            if (i >= paths.size())
                break;
            const std::string &p = paths[i];
            std::string content;
            if (!read_file_to_string(p, content))
            {
                std::cerr << "Failed to read: " << p << "\n";
                continue;
            }
            index_document(p, content);
        }
    };
    std::vector<std::thread> thr;
    for (int i = 0; i < num_threads; ++i)
        thr.emplace_back(worker);
    for (auto &t : thr)
        t.join();
}

const std::unordered_map<std::string, std::vector<Posting>> &Indexer::get_index() const
{
    return inverted;
}

int Indexer::doc_count() const
{
    return (int)doc_lengths.size();
}

int Indexer::doc_length(DocId id) const
{
    auto it = doc_lengths.find(id);
    return it == doc_lengths.end() ? 0 : it->second;
}

void Indexer::dump(const std::string &path) const
{
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs)
        throw std::runtime_error("cannot open dump file");
    // write next_doc_id
    ofs.write(reinterpret_cast<const char *>(&next_doc_id), sizeof(next_doc_id));
    // write doc_lengths
    uint32_t doclen_sz = doc_lengths.size();
    ofs.write(reinterpret_cast<const char *>(&doclen_sz), sizeof(doclen_sz));
    for (auto &p : doc_lengths)
    {
        ofs.write(reinterpret_cast<const char *>(&p.first), sizeof(p.first));
        ofs.write(reinterpret_cast<const char *>(&p.second), sizeof(p.second));
    }
    // write path map
    uint32_t pathsz = path_to_id.size();
    ofs.write(reinterpret_cast<const char *>(&pathsz), sizeof(pathsz));
    for (auto &p : path_to_id)
    {
        uint32_t len = p.first.size();
        ofs.write(reinterpret_cast<const char *>(&len), sizeof(len));
        ofs.write(p.first.data(), len);
        ofs.write(reinterpret_cast<const char *>(&p.second), sizeof(p.second));
    }
    // write inverted index
    uint32_t terms = inverted.size();
    ofs.write(reinterpret_cast<const char *>(&terms), sizeof(terms));
    for (auto &e : inverted)
    {
        uint32_t tlen = e.first.size();
        ofs.write(reinterpret_cast<const char *>(&tlen), sizeof(tlen));
        ofs.write(e.first.data(), tlen);
        uint32_t plist = e.second.size();
        ofs.write(reinterpret_cast<const char *>(&plist), sizeof(plist));
        for (auto &post : e.second)
        {
            ofs.write(reinterpret_cast<const char *>(&post.doc_id), sizeof(post.doc_id));
            ofs.write(reinterpret_cast<const char *>(&post.tf), sizeof(post.tf));
        }
    }
}

void Indexer::load(const std::string &path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs)
        throw std::runtime_error("cannot open dump file");
    ifs.read(reinterpret_cast<char *>(&next_doc_id), sizeof(next_doc_id));
    uint32_t doclen_sz;
    ifs.read(reinterpret_cast<char *>(&doclen_sz), sizeof(doclen_sz));
    doc_lengths.clear();
    for (uint32_t i = 0; i < doclen_sz; i++)
    {
        DocId id;
        int len;
        ifs.read(reinterpret_cast<char *>(&id), sizeof(id));
        ifs.read(reinterpret_cast<char *>(&len), sizeof(len));
        doc_lengths[id] = len;
    }
    uint32_t pathsz;
    ifs.read(reinterpret_cast<char *>(&pathsz), sizeof(pathsz));
    path_to_id.clear();
    for (uint32_t i = 0; i < pathsz; i++)
    {
        uint32_t len;
        ifs.read(reinterpret_cast<char *>(&len), sizeof(len));
        std::string p(len, '\0');
        ifs.read(&p[0], len);
        DocId id;
        ifs.read(reinterpret_cast<char *>(&id), sizeof(id));
        path_to_id[p] = id;
    }
    uint32_t terms;
    ifs.read(reinterpret_cast<char *>(&terms), sizeof(terms));
    inverted.clear();
    for (uint32_t i = 0; i < terms; i++)
    {
        uint32_t tlen;
        ifs.read(reinterpret_cast<char *>(&tlen), sizeof(tlen));
        std::string term(tlen, '\0');
        ifs.read(&term[0], tlen);
        uint32_t plist;
        ifs.read(reinterpret_cast<char *>(&plist), sizeof(plist));
        std::vector<Posting> posts;
        posts.reserve(plist);
        for (uint32_t j = 0; j < plist; j++)
        {
            Posting p;
            ifs.read(reinterpret_cast<char *>(&p.doc_id), sizeof(p.doc_id));
            ifs.read(reinterpret_cast<char *>(&p.tf), sizeof(p.tf));
            posts.push_back(p);
        }
        inverted[term] = std::move(posts);
    }
}
