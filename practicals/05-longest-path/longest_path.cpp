// practicals/05-longest-path/longest_path.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstddef>

struct MapResult {
    std::size_t max_len{};
    std::vector<std::string> paths;
};

// Mapper: xử lý 1 file, tìm path dài nhất trong file đó
MapResult map_file(const std::string &filename) {
    MapResult result;
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Cannot open input file: " << filename << "\n";
        return result;
    }

    std::string line;
    while (std::getline(in, line)) {
        std::size_t len = line.size();
        if (len == 0) continue;

        if (len > result.max_len) {
            result.max_len = len;
            result.paths.clear();
            result.paths.push_back(line);
        } else if (len == result.max_len) {
            result.paths.push_back(line);
        }
    }

    return result;
}

// Reducer: gộp kết quả từ 1 mapper vào kết quả toàn cục
void reduce_result(const MapResult &local,
                   std::size_t &global_max_len,
                   std::vector<std::string> &global_paths) {
    if (local.max_len == 0) return;

    if (local.max_len > global_max_len) {
        global_max_len = local.max_len;
        global_paths = local.paths;
    } else if (local.max_len == global_max_len) {
        // nối thêm các path dài nhất của mapper này
        global_paths.insert(global_paths.end(),
                            local.paths.begin(), local.paths.end());
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <paths_file1> [paths_file2 ...]\n";
        return 1;
    }

    std::size_t global_max_len = 0;
    std::vector<std::string> global_paths;

    // mô phỏng nhiều mapper: mỗi file là một mapper
    for (int i = 1; i < argc; ++i) {
        MapResult local = map_file(argv[i]);
        reduce_result(local, global_max_len, global_paths);
    }

    if (global_max_len == 0) {
        std::cout << "No paths found.\n";
        return 0;
    }

    std::cout << "Longest length: " << global_max_len << "\n";
    std::cout << "Longest path(s):\n";
    for (const auto &p : global_paths) {
        std::cout << p << "\n";
    }

    return 0;
}
