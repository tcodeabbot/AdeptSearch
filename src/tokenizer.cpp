#include "tokenizer.h"
#include <algorithm>
#include <cctype>
#include <unordered_set>
#include <sstream>

static const std::unordered_set<std::string> STOPWORDS = {
    "the", "is", "at", "which", "on", "and", "a", "an", "of", "for", "in", "to", "with", "by", "that"};

Tokenizer::Tokenizer() {}

std::string Tokenizer::normalize(const std::string &s) const
{
    std::string out;
    out.reserve(s.size());
    for (char c : s)
    {
        if (std::isalnum((unsigned char)c) || c == '\'')
            out.push_back(std::tolower((unsigned char)c));
        else
            out.push_back(' ');
    }
    // collapse spaces
    std::string res;
    bool prev_space = false;
    for (char c : out)
    {
        if (c == ' ')
        {
            if (!prev_space)
            {
                res.push_back(' ');
                prev_space = true;
            }
        }
        else
        {
            res.push_back(c);
            prev_space = false;
        }
    }
    // trim
    if (!res.empty() && res.front() == ' ')
        res.erase(res.begin());
    if (!res.empty() && res.back() == ' ')
        res.pop_back();
    return res;
}

// very simple suffix stripper: -ing, -ed, -s
std::string Tokenizer::simple_stem(const std::string &s) const
{
    if (s.size() > 4 && s.substr(s.size() - 3) == "ing")
        return s.substr(0, s.size() - 3);
    if (s.size() > 3 && s.substr(s.size() - 2) == "ed")
        return s.substr(0, s.size() - 2);
    if (s.size() > 2 && s.back() == 's')
        return s.substr(0, s.size() - 1);
    return s;
}

bool Tokenizer::is_stopword(const std::string &w) const
{
    return STOPWORDS.find(w) != STOPWORDS.end();
}

std::vector<std::string> Tokenizer::tokenize(const std::string &text) const
{
    std::vector<std::string> tokens;
    std::string norm = normalize(text);
    std::istringstream iss(norm);
    std::string tok;
    while (iss >> tok)
    {
        if (tok.size() == 0)
            continue;
        if (is_stopword(tok))
            continue;
        tok = simple_stem(tok);
        if (tok.size() > 1)
            tokens.push_back(tok);
    }
    return tokens;
}
