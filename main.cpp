#include <iostream>
#include <unordered_map>
#include <map>
#include <set>
#include <cctype>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/encodings.h"
#include "rapidjson/istreamwrapper.h"


using wrd_cnt = std::pair<size_t, std::string>;

bool operator<(const wrd_cnt& lhs, const wrd_cnt& rhs) {
    if (lhs.first < rhs.first) {
        return true;
    } else if (lhs.first == rhs.first) {
        if (lhs.second < rhs.second) {
            return true;
        } 

        return false;
    }

    return false;
}


void add_word(std::unordered_map<std::string, size_t>& map, const std::string& line) {
    std::stringstream ss(line);
    std::string word;

    while(std::getline(ss >> std::ws, word, ' ')) {
        char prev = word.front();
        size_t rep_count = 0;
        for (const char& c : word) {
            if (c == prev) {
                rep_count++;
            } else {
                prev = c;
                rep_count = 1;
            }

            if (rep_count >= 3) break;
        }

        if (rep_count >= 3) continue;

        word.erase(
            std::unique(word.begin(), word.end(), [&](char a, char b) { 
                return (a == '\'' && b == '\''); 
            })
            , word.end()
        );

        if (word.front() == '\'') word.erase(0);
        if (word.back() == '\'') word.pop_back();

        if (word.size() > 2) {
            if (word.at(word.size() - 2) == '\'' && word.back() == 's') {
                std::string no_s = word;
                no_s.pop_back();
                no_s.pop_back();

                word = no_s;
            }
        }

        if (word.size() >= 5 && word.back() == 's') {
            std::string no_s = word;
            no_s.pop_back();

            if (map.count(no_s) != 0) {
                map.at(no_s)++;
                continue;
            }
        }

        if (word.size() >= 5 && word.back() != 's') {
            std::string add_s = word;
            add_s += 's';

            if (map.count(add_s) != 0) {
                if (map.count(word) == 0) {
                    map.emplace(word, map.count(add_s));
                    map.erase(add_s);
                } else {
                    map.at(word) += map.count(add_s);
                }
            }
        }

        while (word.front() == ' ') word.erase(0);
        if (word.empty() == true) continue;

        if (map.count(word) == 0) {
            map.emplace(word, 1);
        } else {
            map.at(word)++;
        }
    }
}


void print_stats(const std::unordered_map<std::string, size_t>& map, 
size_t mes_num, const std::string& person) {
    std::set<wrd_cnt> person_ordered;

    for (const auto& p : map) person_ordered.emplace(p.second, p.first);

    size_t person_words = 0;
    std::vector<std::string> person_top;
    person_top.reserve(100);
    size_t prsn_count = 0;

    for (const auto& p : person_ordered) {
        person_words += p.first;
        if (prsn_count >= (person_ordered.size() - 100)) {
            person_top.emplace_back(p.second);
        }

        prsn_count++;
    }

    std::string mpw = "most popular words:";

    for (size_t i = 0; i < 100; i++) {
        if (i % 10 == 0) mpw += "\n> ";

        mpw += person_top.at(i) + ' ';
    }

    double twl = 0.0, wc = 0.0;
    double awl;
    std::vector<size_t> lengths;
    size_t mwl;

    for (const auto& p : map) {
        if (p.first.find('\'') == std::string::npos) {
            twl += (float)p.first.size();
            wc += 1.0;

            lengths.emplace_back(p.first.size());
        }
    }

    awl = twl / wc;
    std::sort(lengths.begin(), lengths.end());
    mwl = lengths.at(lengths.size() / 2);

    std::cout << person << std::endl;
    std::cout << "activity: " << mes_num << " messages" << std::endl;
    std::cout << "vocabulary: " << person_ordered.size() << " unique words" << std::endl;
    std::cout << "total word count: " << person_words << std::endl;
    std::cout << "avg word length: " << awl << std::endl;
    std::cout << "median word length: " << mwl << std::endl;
    std::cout << mpw << std::endl << std::endl;

    std::string path = "./" + person + "_stats.txt";
    std::ofstream ofs(path);
    for (const auto& p : person_ordered) {
        ofs << std::to_string(p.first) + " " + p.second << std::endl;
    }
}


int main() {
    std::ifstream ifs("/home/dan/Documents/chat_stats/chat.json");
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document document;  
    rapidjson::ParseResult ok = document.ParseStream(isw);

    if (!ok) {
        std::cout << "No Chat File" << std::endl;
        return 1;
    }

    const rapidjson::Value& a = document["messages"];
    assert(a.IsArray());

    std::unordered_map<std::string, size_t> Dan;
    std::unordered_map<std::string, size_t> Jill;

    size_t Jill_count = 0;
    size_t Dan_count = 0;

    std::string punct = ",.)(][}{:;+*-|~_@?!=<>#&%$/^\"\\\'"; 
    std::string diff_punct = "‘`“”";

    for (rapidjson::SizeType i = 0; i < a.Size(); i++) {
        if (strcmp(a[i]["type"].GetString(), "message") != 0) continue;
        if (a[i]["text"].IsString() == false) continue;

        std::string cleaned_line;
        std::string temp(a[i]["text"].GetString());
        if (temp.empty()) continue;

        for (auto it = temp.begin(); it != temp.end(); it++) {
            auto pnct = std::find(punct.begin(), punct.end(), *it);
            auto diff_pnct = std::find(diff_punct.begin(), diff_punct.end(), *it);

            if (std::isalpha(*it)) {
                cleaned_line.push_back(std::tolower(*it));
            } else if (*it == '\n' || *it == ' ') {
                cleaned_line.push_back(' ');
            } else if (pnct != punct.end()) {
                if (int(*it) == 39) {
                    cleaned_line.push_back('\'');
                } else if (int(*it) == 45) {
                    cleaned_line.push_back(' ');
                } else {
                    if (int(*it) > 0) {
                        cleaned_line.push_back(' ');
                    }
                }
            } else if (diff_pnct != diff_punct.end()) {
                if (diff_pnct == diff_punct.begin() || diff_pnct == (diff_punct.begin() + 1)) {
                    cleaned_line.push_back('\'');
                } 
            }
        }

        try {
            if (strcmp(a[i]["from"].GetString(), "Jill Smith") == 0) {
                add_word(Jill, cleaned_line);
                Jill_count++;
            } else if (strcmp(a[i]["from"].GetString(), "Dan Royce") == 0) {
                add_word(Dan, cleaned_line);
                Dan_count++;
            } else {
                continue;
            }
        } catch(const std::exception& ex) {
            std::cout << "what: " << ex.what() << std::endl;
        }
    }

    print_stats(Dan, Dan_count, "Dan");
    print_stats(Jill, Jill_count, "Jill");
  
    return 0;
}