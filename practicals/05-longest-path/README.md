# Practical Work 5 — The Longest Path (Mini MapReduce)

This practical implements a toy MapReduce project that finds the
longest file path(s) from a set of files. Each input file contains
one full path per line (for example, the output of `find /`).

## Build

```bash
g++ -std=c++17 -O2 -o longest_path longest_path.cpp
