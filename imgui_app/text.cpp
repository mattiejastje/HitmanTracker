#include "text.hpp"

ImVec4 im_vec4(const std::array<float, 3>& color) {
    return ImVec4{color[0], color[1], color[2], 1.0f};
}
