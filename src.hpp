// Header-only implementation for Conway's Game of Life per OJ driver.
// The OJ provides its own main.cpp that includes "src.hpp".
#pragma once

#include <bits/stdc++.h>

// Grid dimensions
inline int row = -1, col = -1;

// Represent live cells sparsely for performance
inline std::unordered_set<long long> live;

static inline long long key_of(int r, int c) {
  return (static_cast<long long>(r) << 32) ^ static_cast<unsigned int>(c);
}

// Initialize from RLE-like pattern on stdin
inline void Initialize() {
  if (!(std::cin >> col >> row)) return;
  std::string line;
  std::string pattern;
  std::string tmp;
  std::getline(std::cin, tmp); // consume end of line
  pattern.reserve(1024);
  while (std::getline(std::cin, line)) {
    pattern += line;
    if (line.find('!') != std::string::npos) break;
  }

  live.clear();

  auto add_cells = [&](int r, int c_start, int cnt, bool alive) {
    int c = c_start;
    for (int i = 0; i < cnt && r >= 0 && r < row && c < col; ++i, ++c) {
      if (c < 0) continue;
      if (alive) live.insert(key_of(r, c));
      else live.erase(key_of(r, c));
    }
    return c;
  };

  int r = 0, c = 0;
  long long num = -1; // -1 indicates no number yet
  for (size_t i = 0; i < pattern.size(); ++i) {
    char ch = pattern[i];
    if (ch >= '0' && ch <= '9') {
      if (num < 0) num = ch - '0';
      else num = num * 10 + (ch - '0');
      continue;
    }
    if (ch == '!' ) {
      break;
    }
    int cnt = (num > 0 ? static_cast<int>(num) : 1);
    if (ch == 'b' || ch == 'o') {
      bool alive = (ch == 'o');
      if (cnt <= 0) cnt = 1;
      if (c < 0) c = 0;
      if (r >= 0 && r < row && c < col) {
        c = add_cells(r, c, cnt, alive);
      } else {
        // even if out of bounds, still advance c
        c += cnt;
      }
      num = -1;
    } else if (ch == '$') {
      if (cnt <= 0) cnt = 1;
      r += cnt;
      c = 0;
      num = -1;
    } else if (ch == '\r' || ch == '\n' || ch == ' ' || ch == '\t') {
      // ignore whitespace
    } else {
      // ignore any other characters
      num = -1;
    }
  }
  // Remaining unspecified rows/cells are dead by default
}

// Simulate one tick using sparse neighbor counting
inline void Tick() {
  static const int dr[8] = {-1,-1,-1,0,0,1,1,1};
  static const int dc[8] = {-1,0,1,-1,1,-1,0,1};

  std::unordered_map<long long, int> cnt;
  cnt.reserve(live.size() * 8 + 16);

  // Count neighbors for all cells adjacent to live cells
  for (const auto &k : live) {
    int r = static_cast<int>(k >> 32);
    int c = static_cast<int>(k & 0xffffffffu);
    for (int d = 0; d < 8; ++d) {
      int nr = r + dr[d], nc = c + dc[d];
      if (nr < 0 || nr >= row || nc < 0 || nc >= col) continue;
      ++cnt[key_of(nr, nc)];
    }
  }

  std::unordered_set<long long> next;
  next.reserve(live.size() * 2 + 16);

  // For all cells that have neighbors, decide fate
  for (auto &p : cnt) {
    long long cell = p.first;
    int n = p.second;
    bool was_live = (live.find(cell) != live.end());
    if (was_live) {
      if (n == 2 || n == 3) next.insert(cell);
    } else {
      if (n == 3) next.insert(cell);
    }
  }

  // Also consider isolated live cells (with 0 neighbors) -> they die; nothing to add
  live.swap(next);
}

inline void PrintGame() {
  std::cout << col << " " << row << "\n";

  auto out_run_cells = [&](int cnt, char ch){
    if (cnt <= 0) return;
    if (cnt == 1) std::cout << ch;
    else std::cout << cnt << ch;
  };
  auto out_run_newlines = [&](int cnt){
    if (cnt <= 0) return;
    if (cnt == 1) std::cout << '$';
    else std::cout << cnt << '$';
  };

  int empty_rows_run = 0;
  for (int r = 0; r < row; ++r) {
    // Find rightmost live cell in this row
    int last_live_c = -1;
    // If the row is dense, scanning all columns is acceptable for Basic tests; for Pressure, live is sparse.
    for (int c = col - 1; c >= 0; --c) {
      if (live.find(key_of(r, c)) != live.end()) {
        last_live_c = c;
        break;
      }
    }
    if (last_live_c < 0) {
      // Entire row empty
      ++empty_rows_run;
      continue;
    }

    // Flush pending empty rows before outputting a non-empty row
    out_run_newlines(empty_rows_run);
    empty_rows_run = 0;

    int run_cnt = 0;
    bool run_live = false; // current run state
    bool run_init = false;
    for (int c = 0; c <= last_live_c; ++c) {
      bool is_live = (live.find(key_of(r, c)) != live.end());
      if (!run_init) {
        run_init = true;
        run_live = is_live;
        run_cnt = 1;
      } else if (is_live == run_live) {
        ++run_cnt;
      } else {
        out_run_cells(run_cnt, run_live ? 'o' : 'b');
        run_live = is_live;
        run_cnt = 1;
      }
    }
    if (run_init) out_run_cells(run_cnt, run_live ? 'o' : 'b');
    // End of row marker
    if (r != row - 1) std::cout << '$';
  }
  // Flush any trailing empty rows (at bottom). They can be omitted entirely per spec.
  std::cout << '!' << "\n";
}

// Return number of live cells
inline int GetLiveCell() {
  return static_cast<int>(live.size());
}
