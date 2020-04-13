// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/ButtonWidget.hpp"
#include "TTauri/GUI/utils.hpp"
#include "TTauri/Foundation/utils.hpp"
#include <cmath>
#include <typeinfo>

namespace TTauri::GUI::Widgets {

using namespace TTauri::Text;
using namespace std::literals;

ButtonWidget::ButtonWidget(Window &window, Widget *parent, std::string const label) noexcept :
    Widget(window, parent), label(std::move(label))
{
}

void ButtonWidget::updateAndPlaceVertices(
    cpu_utc_clock::time_point displayTimePoint,
    vspan<PipelineFlat::Vertex> &flat_vertices,
    vspan<PipelineBox::Vertex> &box_vertices,
    vspan<PipelineImage::Vertex> &image_vertices,
    vspan<PipelineSDF::Vertex> &sdf_vertices) noexcept
{
    auto continueRendering = false;

    // Draw something.
    vec cornerShapes = { 10.0, 10.0, -10.0, 0.0 };

    vec backgroundColor;
    vec labelColor;
    vec borderColor;
    float shadowSize;

    if (value) {
        if (hover) {
            backgroundColor = vec::color(0.3, 0.3, 1.0);
        } else if (pressed) {
            backgroundColor = vec::color(0.1, 0.1, 0.1);
        } else {
            backgroundColor = vec::color(0.072, 0.072, 1.0);
        }
    } else {
        if (hover) {
            backgroundColor = vec::color(0.3, 0.3, 0.3);
        } else if (pressed) {
            backgroundColor = vec::color(0.072, 0.072, 1.0);
        } else {
            backgroundColor = vec::color(0.1, 0.1, 0.1);
        }
    }

    if (focus) {
        borderColor = vec::color(0.072, 0.072, 1.0);
    } else {
        borderColor = vec::color(0.3, 0.3, 0.3);
    }

    labelColor = vec{1.0, 1.0, 1.0, 1.0};

    if (value || pressed) {
        shadowSize = 0.0;
    } else {
        shadowSize = 6.0;
    }


    if (renderTrigger.check(displayTimePoint) >= 2) {
        let labelStyle = TextStyle("Times New Roman", FontVariant{FontWeight::Regular, false}, 14.0, labelColor, 0.0, TextDecoration::None);

        labelShapedText = ShapedText(label, labelStyle, HorizontalAlignment::Center, numeric_cast<float>(box.width.value()));

        window.device->SDFPipeline->prepareAtlas(labelShapedText);
    }

    PipelineBox::DeviceShared::placeVertices(
        box_vertices,
        elevation,
        box.currentRectangle(),
        backgroundColor,
        1.0f,
        borderColor,
        shadowSize,
        cornerShapes,
        expand(box.currentRectangle(), 10.0)
    );

    window.device->SDFPipeline->placeVertices(
        sdf_vertices,
        labelShapedText,
        mat::T(box.currentOffset().z(elevation)),
        box.currentRectangle()
    );

    Widget::updateAndPlaceVertices(displayTimePoint, flat_vertices, box_vertices, image_vertices, sdf_vertices);
}

void ButtonWidget::handleCommand(string_ltag command) noexcept {
    if (!enabled) {
        return;
    }

    if (command == "gui.activate"_ltag) {
        if (assign_and_compare(value, !value)) {
            ++renderTrigger;
        }
    }
    Widget::handleCommand(command);
}

void ButtonWidget::handleMouseEvent(GUI::MouseEvent const &event) noexcept {
    Widget::handleMouseEvent(event);

    if (enabled) {
        if (assign_and_compare(pressed, static_cast<bool>(event.down.leftButton))) {
            ++renderTrigger;
        }

        if (event.type == GUI::MouseEvent::Type::ButtonUp && event.cause.leftButton) {
            handleCommand("gui.activate"_ltag);
        }
    }
}

HitBox ButtonWidget::hitBoxTest(vec position) noexcept
{
    if (box.contains(position)) {
        return HitBox{this, elevation, enabled ? HitBox::Type::Button : HitBox::Type::Default};
    } else {
        return HitBox{};
    }
}

}
