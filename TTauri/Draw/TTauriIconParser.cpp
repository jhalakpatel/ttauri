
#include "TTauriIconParser.hpp"

namespace TTauri::Draw {

struct big_fpoint1_13_buf_t {
    big_int16_buf_t v;

    float value() const {
        return (v.value() >> 1) / 8192.0f;
    }

    bool flag() const {
        return (v & 1) > 0;
    }
}


struct Header {
    big_uint8_buf_t magic[4];
    big_uint16_buf_t numberOfIcons;
};

struct Icon {
    big_uint16_buf_t numberOfContours;
};

struct Contour {
    big_uint8_buf_t red;
    big_uint8_buf_t green;
    big_uint8_buf_t blue;
    big_uint8_buf_t alpha;
    big_uint16_buf_t numberOfPoints;
};

struct Point {
    enum class Type { OnCurve, CubicControlPoint1, CubicControlPoint2, QuadraticControlPoint };

    big_fpoint1_13_buf_t x;
    big_fpoint1_13_buf_t y;

    glm::vec2 value() const {
        return { x.value(), y.value() };
    }

    Type type() const {
        let type = (x.flag() ? 1 : 0) | (y.flag() ? 2 : 0);
        switch (type) {
        case 0b00: return Type::OnCurve;
        case 0b11: return Type::QuadraticControlPoint;
        case 0x01: return Type::CubicControlPoint1;
        case 0x10: return Type::CubicControlPoint2;
        default: std::terminate();
        }
    }
};


}
