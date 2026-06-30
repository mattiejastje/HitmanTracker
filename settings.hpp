#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <string>

namespace settings {

struct Log {
    int level{4};
    int flush_level{4};
};

using Color = std::array<float, 3>;

struct TextStyle {
    std::filesystem::path file;
    float scale{1.0f};
    Color color{1.0f, 1.0f, 1.0f};
};

struct HBM {
    bool real_time{false};
    bool show_shots_hit{false};
    bool show_accident_kills{false};
};

struct HMA {
    enum class RatingMode {
        X, SA, SC, SA_PLUS_SC, SA_FALLBACK_SC
    };

    bool show_sa_details{true};
    bool show_score_details{true};
    bool show_score_total{true};
    RatingMode rating_mode_unrated{RatingMode::X};
    RatingMode rating_mode_no_targets{RatingMode::SC};
    RatingMode rating_mode_targets{RatingMode::SA_FALLBACK_SC};
};

struct Gui {
    bool topmost{false};
    float font_size{20.0f};
    Color bg_color{0.0f, 0.0f, 0.0f};
    bool show_game_version{true};
    TextStyle title{"fonts/dejavu/DejaVuSansCondensed.ttf"};
    TextStyle difficulty{"fonts/dejavu/DejaVuSansCondensed.ttf", 0.9f};
    TextStyle map{"fonts/dejavu/DejaVuSansCondensed-Oblique.ttf"};
    TextStyle time{"fonts/dejavu/DejaVuSansMono-Bold.ttf", 1.1f};
    TextStyle rating_bad{
        "fonts/dejavu/DejaVuSansCondensed-Bold.ttf",
        1.1f,
        Color{1.0f, 0.0f, 0.0f}
    };
    TextStyle rating_good{
        "fonts/dejavu/DejaVuSansCondensed-Bold.ttf",
        1.1f,
        Color{0.0f, 1.0f, 0.0f}
    };
    TextStyle rating_maybe{
        "fonts/dejavu/DejaVuSansCondensed-Bold.ttf",
        1.1f,
        Color{0.85f, 0.85f, 0.0f}
    };
    TextStyle label{
        "fonts/dejavu/DejaVuSansCondensed.ttf", 0.9f, Color{0.9f, 0.9f, 0.9f}
    };
    TextStyle value{
        "fonts/dejavu/DejaVuSansMono-Bold.ttf", 0.9f, Color{0.4f, 0.4f, 1.0f}
    };
};

struct Settings {
    Log log;
    Gui gui;
    HBM hbm;
    HMA hma;
};

std::optional<Settings> load(int argc, char** argv);
void save(Settings& settings);

}  // namespace settings