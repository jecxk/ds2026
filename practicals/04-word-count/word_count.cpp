// practicals/04-word-count/word_count.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <cctype>

// Hàm chuyển chữ hoa -> chữ thường
std::string to_lower(const std::string &s) {
    std::string r = s;
    for (char &c : r) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return r;
}

// "Mapper": nhận 1 dòng, tách thành các từ, đếm vào local map
void map_line(const std::string &line, std::map<std::string, int> &local_counts) {
    std::string word;
    for (char c : line) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            word.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        } else {
            if (!word.empty()) {
                ++local_counts[word];
                word.clear();
            }
        }
    }
    if (!word.empty()) {
        ++local_counts[word];
    }
}

// "Reducer": gộp tất cả local_counts vào global_counts
void reduce_counts(const std::map<std::string, int> &local_counts,
                   std::map<std::string, int> &global_counts) {
    for (const auto &p : local_counts) {
        global_counts[p.first] += p.second;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.txt> <output.txt>\n";
        return 1;
    }

    const std::string input_path  = argv[1];
    const std::string output_path = argv[2];

    std::ifstream in(input_path);
    if (!in) {
        std::cerr << "Cannot open input file: " << input_path << "\n";
        return 1;
    }

    std::map<std::string, int> global_counts;

    std::string line;
    while (std::getline(in, line)) {
        std::map<std::string, int> local_counts;
        map_line(line, local_counts);
        reduce_counts(local_counts, global_counts);
    }

    in.close();

    std::ofstream out(output_path);
    if (!out) {
        std::cerr << "Cannot open output file: " << output_path << "\n";
        return 1;
    }

    for (const auto &p : global_counts) {
        out << p.first << " " << p.second << "\n";
    }

    std::cout << "Word count written to: " << output_path << "\n";
    return 0;
}
