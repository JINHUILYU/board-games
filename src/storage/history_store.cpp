#include "storage/history_store.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace chess {

namespace {

std::string NowTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    return oss.str();
}

char SideToChar(Side side) {
    return side == Side::kBlack ? 'B' : 'W';
}

Side CharToSide(char c) {
    return c == 'B' ? Side::kBlack : Side::kWhite;
}

GameResult ParseGameResult(const std::string& text) {
    if (text == "BlackWin") {
        return GameResult::kBlackWin;
    }
    if (text == "WhiteWin") {
        return GameResult::kWhiteWin;
    }
    if (text == "Draw") {
        return GameResult::kDraw;
    }
    return GameResult::kOngoing;
}

}  // namespace

HistoryStore::HistoryStore(std::filesystem::path history_dir)
    : history_dir_(std::move(history_dir)) {
    std::filesystem::create_directories(history_dir_);
}

std::filesystem::path HistoryStore::SaveGomoku(const GomokuHistoryRecord& record) const {
    std::filesystem::create_directories(history_dir_);
    const auto path = history_dir_ / ("gomoku_" + NowTimestamp() + ".json");

    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("Failed to open history file for writing: " + path.string());
    }

    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"game\": \"gomoku\",\n";
    out << "  \"mode\": \"" << record.mode << "\",\n";
    out << "  \"board_size\": " << record.board_size << ",\n";
    out << "  \"difficulty\": " << record.difficulty << ",\n";
    out << "  \"result\": \"" << GameResultToString(record.result) << "\",\n";
    out << "  \"moves\": [\n";
    for (std::size_t i = 0; i < record.moves.size(); ++i) {
        const Move& move = record.moves[i];
        out << "    {\"r\": " << move.row << ", \"c\": " << move.col
            << ", \"p\": \"" << SideToChar(move.side) << "\"}";
        if (i + 1 != record.moves.size()) {
            out << ",";
        }
        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";
    return path;
}

std::vector<std::filesystem::path> HistoryStore::ListGomokuRecords() const {
    std::vector<std::filesystem::path> files;
    if (!std::filesystem::exists(history_dir_)) {
        return files;
    }

    for (const auto& entry : std::filesystem::directory_iterator(history_dir_)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto filename = entry.path().filename().string();
        if (entry.path().extension() == ".json" && filename.rfind("gomoku_", 0) == 0) {
            files.push_back(entry.path());
        }
    }

    std::sort(files.begin(), files.end(), std::greater<>());
    return files;
}

bool HistoryStore::LoadGomoku(const std::filesystem::path& path, GomokuHistoryRecord* out_record) const {
    if (!out_record || !std::filesystem::exists(path)) {
        return false;
    }

    std::ifstream in(path);
    if (!in) {
        return false;
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    const std::string text = buffer.str();

    std::smatch match;
    if (std::regex_search(text, match, std::regex("\"mode\"\\s*:\\s*\"([^\"]+)\""))) {
        out_record->mode = match[1].str();
    } else {
        out_record->mode = "unknown";
    }

    if (std::regex_search(text, match, std::regex("\"difficulty\"\\s*:\\s*(\\d+)"))) {
        out_record->difficulty = std::stoi(match[1].str());
    } else {
        out_record->difficulty = 0;
    }

    if (std::regex_search(text, match, std::regex("\"board_size\"\\s*:\\s*(\\d+)"))) {
        out_record->board_size = std::stoi(match[1].str());
    } else {
        out_record->board_size = 15;
    }

    if (std::regex_search(text, match, std::regex("\"result\"\\s*:\\s*\"([^\"]+)\""))) {
        out_record->result = ParseGameResult(match[1].str());
    } else {
        out_record->result = GameResult::kOngoing;
    }

    out_record->moves.clear();
    const std::regex move_regex(
        "\\{\"r\"\\s*:\\s*(\\d+)\\s*,\\s*\"c\"\\s*:\\s*(\\d+)\\s*,\\s*\"p\"\\s*:\\s*\"([BW])\"\\}");
    auto begin = std::sregex_iterator(text.begin(), text.end(), move_regex);
    const auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        const int row = std::stoi((*it)[1].str());
        const int col = std::stoi((*it)[2].str());
        const char side_char = (*it)[3].str()[0];
        out_record->moves.push_back(Move{-1, -1, row, col, CharToSide(side_char)});
    }
    return true;
}

}  // namespace chess
