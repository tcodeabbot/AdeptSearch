# AdeptSearch 🔍  
*A multithreaded full-text search engine in C++17*  

## Overview  
AdeptSearch is a compact but powerful search engine written in modern **C++17**.  
It implements an **inverted index** with **BM25 ranking**, supports **multithreaded indexing**, and comes with a **CLI** for indexing, querying, and managing document collections.  

This project demonstrates **systems programming**, **algorithms**, and **concurrency** in C++ — making it ideal for showcasing on a portfolio or resume.  

---

## ✨ Features  
- **Inverted Index** for efficient text retrieval  
- **BM25 Ranking Algorithm** for relevance scoring  
- **Tokenizer** with stopword removal and simple stemming  
- **Multithreaded Indexing** using `std::thread` and synchronization (`std::mutex`, `std::atomic`)  
- **Index Persistence** — save and load the index in a binary format  
- **Command-Line Interface (CLI)** to index files and run queries  
- **Scalable** — can index thousands of text files concurrently  

---

## 🛠️ Tech Stack  
- **Language:** C++17  
- **Build System:** CMake  
- **Core Libraries:** STL (`thread`, `mutex`, `atomic`, `filesystem`, `unordered_map`, etc.)  
- **Algorithms:** Inverted Index, BM25 Ranking, Tokenization, Stemming  
- **Concurrency:** Multithreading, Synchronization  
- **Persistence:** Custom binary serialization/deserialization  

---

## 📂 Project Structure  
adeptsearch/
├─ CMakeLists.txt # Build configuration
├─ README.md # Project documentation
├─ sample_docs/ # Example documents to index
│ ├─ doc1.txt
│ ├─ doc2.txt
│ └─ ...
└─ src/ # Source code
├─ main.cpp # CLI entry point
├─ tokenizer.h/.cpp # Text preprocessing
├─ indexer.h/.cpp # Inverted index + persistence
├─ searcher.h/.cpp # Query processing (BM25)

yaml
Copy code

---

## 🚀 Getting Started  

### Prerequisites  
- C++17 compatible compiler (e.g., GCC 9+, Clang 10+, MSVC 2019+)  
- [CMake](https://cmake.org/) 3.10+  

### Build  

### Clone repo
git clone https://github.com/<your-username>/adeptsearch.git
cd adeptsearch

### Create build directory
mkdir build && cd build

### Run CMake
cmake ..

### Compile
cmake --build .

