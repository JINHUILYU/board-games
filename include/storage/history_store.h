#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "core/types.h"

namespace chess {

struct GomokuHistoryRecord {
    std::string mode;
    int board_size = 15;
    int difficulty = 0;
    GameResult result = GameResult::kOngoing;
    std::vector<Move> moves;
};

class HistoryStore {
public:
    explicit HistoryStore(std::filesystem::path history_dir);

    std::filesystem::path SaveGomoku(const GomokuHistoryRecord& record) const;
    std::vector<std::filesystem::path> ListGomokuRecords() const;
    bool LoadGomoku(const std::filesystem::path& path, GomokuHistoryRecord* out_record) const;

private:
    std::filesystem::path history_dir_;
};

}  // namespace chess
