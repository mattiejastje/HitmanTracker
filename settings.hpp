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

struct Gui {
    bool topmost{false};
    float font_size{20.0f};
    Color bg_color{0.0f, 0.0f, 0.0f};
    TextStyle title{"fonts/DejaVuSansCondensed.ttf"};
    TextStyle difficulty{"fonts/DejaVuSansCondensed.ttf", 0.9f};
    TextStyle map{"fonts/DejaVuSansCondensed-Oblique.ttf"};
    TextStyle time{"fonts/DejaVuSansMono-Bold.ttf", 1.1f};
    TextStyle rating_bad{
        "fonts/DejaVuSansCondensed-Bold.ttf", 1.1f, Color{1.0f, 0.0f, 0.0f}
    };
    TextStyle rating_good{
        "fonts/DejaVuSansCondensed-Bold.ttf", 1.1f, Color{0.0f, 1.0f, 0.0f}
    };
    TextStyle rating_maybe{
        "fonts/DejaVuSansCondensed-Bold.ttf", 1.1f, Color{0.85f, 0.85f, 0.0f}
    };
    TextStyle label{
        "fonts/DejaVuSansCondensed.ttf", 0.9f, Color{0.9f, 0.9f, 0.9f}
    };
    TextStyle value{
        "fonts/DejaVuSansMono-Bold.ttf", 0.9f, Color{0.4f, 0.4f, 1.0f}
    };
};

struct HBM {
    bool real_time{false};
};

struct Settings {
    Log log;
    Gui gui;
    HBM hbm;
};

std::optional<Settings> load(int argc, char** argv);

}  // namespace settings