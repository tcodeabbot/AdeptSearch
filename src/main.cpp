#include "indexer.h"
#include "searcher.h"
#include <iostream>
#include <filesystem>
#include <vector>
#include <string>

namespace fs = std::filesystem;

static void usage()
{
    std::cout << "AdeptSearch CLI\n";
    std::cout << "Commands:\n";
    std::cout << "  index <dir>        - index all .txt files in <dir>\n";
    std::cout << "  query \"terms\"      - run query against built index\n";
    std::cout << "  dump <file>         - dump index to binary\n";
    std::cout << "  load <file>         - load index from binary\n";
    std::cout << "Examples:\n  ./adeptsearch index ../sample_docs\n  ./adeptsearch query \"machine learning\"\n";
}

static std::vector<std::string> collect_txt_files(const std::string &dir)
{
    std::vector<std::string> files;
    for (auto &p : fs::recursive_directory_iterator(dir))
    {
        if (!p.is_regular_file())
            continue;
        auto path = p.path();
        if (path.extension() == ".txt")
            files.push_back(path.string());
    }
    return files;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        usage();
        return 1;
    }
    std::string cmd = argv[1];
    static Indexer indexer;
    if (cmd == std::string("index"))
    {
        if (argc < 3)
        {
            std::cerr << "index requires directory\n";
            return 1;
        }
        std::string dir = argv[2];
        auto files = collect_txt_files(dir);
        if (files.empty())
        {
            std::cerr << "no .txt files found in " << dir << "\n";
            return 1;
        }
        std::cout << "Indexing " << files.size() << " files...\n";
        indexer.index_files(files, std::thread::hardware_concurrency());
        std::cout << "Indexed " << indexer.doc_count() << " documents.\n";
        return 0;
    }
    else if (cmd == std::string("query"))
    {
        if (argc < 3)
        {
            std::cerr << "query requires query string\n";
            return 1;
        }
        std::string q = argv[2];
        // if not indexed, small hint
        if (indexer.doc_count() == 0)
        {
            std::cerr << "Index is empty. Try `index` first or `load` a dump.\n";
            return 1;
        }
        Searcher s(indexer);
        auto results = s.query(q, 10);
        std::cout << "Top results:\n";
        for (auto &r : results)
        {
            std::cout << "DocId=" << r.doc_id << " score=" << r.score << "\n";
        }
        return 0;
    }
    else if (cmd == std::string("dump"))
    {
        if (argc < 3)
        {
            std::cerr << "dump requires file path\n";
            return 1;
        }
        std::string path = argv[2];
        indexer.dump(path);
        std::cout << "Dumped index to " << path << "\n";
        return 0;
    }
    else if (cmd == std::string("load"))
    {
        if (argc < 3)
        {
            std::cerr << "load requires file path\n";
            return 1;
        }
        std::string path = argv[2];
        indexer.load(path);
        std::cout << "Loaded index. doc_count=" << indexer.doc_count() << "\n";
        return 0;
    }
    else
    {
        usage();
        return 1;
    }
}
