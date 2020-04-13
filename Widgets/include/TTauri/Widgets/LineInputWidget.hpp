// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/cpu_utc_clock.hpp"
#include "TTauri/GUI/Widget.hpp"
#include "TTauri/Text/EditableText.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace TTauri::GUI::Widgets {

class LineInputWidget : public Widget {
protected:
    std::string label = "<unknown>";

    Text::EditableText field;
    Text::ShapedText shapedText;

    rect leftToRightCaret = {};
    rect partialGraphemeCaret = {};
    std::vector<rect> selectionRectangles = {};

    cpu_utc_clock::time_point lastUpdateTimePoint;
public:

    LineInputWidget(
        Window &window, Widget *parent,
        std::string const label,
        Text::TextStyle style=Text::TextStyle(
            "Arial",
            Text::FontVariant{Text::FontWeight::Regular, false},
            14.0,
            vec::color(1.0,1.0,1.0),
            0.0,
            Text::TextDecoration::None
        )
    ) noexcept;

    ~LineInputWidget() {}

    LineInputWidget(const LineInputWidget &) = delete;
    LineInputWidget &operator=(const LineInputWidget &) = delete;
    LineInputWidget(LineInputWidget&&) = delete;
    LineInputWidget &operator=(LineInputWidget &&) = delete;

    void updateAndPlaceVertices(
        cpu_utc_clock::time_point displayTimePoint,
        vspan<PipelineFlat::Vertex> &flat_vertices,
        vspan<PipelineBox::Vertex> &box_vertices,
        vspan<PipelineImage::Vertex> &image_vertices,
        vspan<PipelineSDF::Vertex> &sdf_vertices
    ) noexcept override;

    void handleCommand(string_ltag command) noexcept;

    void handleMouseEvent(GUI::MouseEvent const &event) noexcept override;
    void handleKeyboardEvent(GUI::KeyboardEvent const &event) noexcept override;
    [[nodiscard]] HitBox hitBoxTest(vec position) noexcept;
    [[nodiscard]] bool acceptsFocus() noexcept override {
        return enabled;
    }

};

}
