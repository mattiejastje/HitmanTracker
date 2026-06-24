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
};

struct HMA {
    bool always_track_sa{false};
};

struct Gui {
    bool topmost{false};
    float font_size{20.0f};
    Color bg_color{0.0f, 0.0f, 0.0f};
    TextStyle title{"fonts/dejavu/DejaVuSansCondensed.ttf"};
    TextStyle difficulty{"fonts/dejavu/DejaVuSansCondensed.ttf", 0.9f};
    TextStyle map{"fonts/dejavu/DejaVuSansCondensed-Oblique.ttf"};
    TextStyle time{"fonts/dejavu/DejaVuSansMono-Bold.ttf", 1.1f};
    TextStyle rating_bad{
        "fonts/dejavu/DejaVuSansCondensed-Bold.ttf", 1.1f, Color{1.0f, 0.0f, 0.0f}
    };
    TextStyle rating_good{
        "fonts/dejavu/DejaVuSansCondensed-Bold.ttf", 1.1f, Color{0.0f, 1.0f, 0.0f}
    };
    TextStyle rating_maybe{
        "fonts/dejavu/DejaVuSansCondensed-Bold.ttf", 1.1f, Color{0.85f, 0.85f, 0.0f}
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

}  // namespace settings