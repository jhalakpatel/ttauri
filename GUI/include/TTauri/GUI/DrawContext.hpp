
// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/mat.hpp"
#include "TTauri/Foundation/aarect.hpp"
#include "TTauri/Foundation/vspan.hpp"
#include "TTauri/GUI/GUIDevice.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/Theme.hpp"
#include "TTauri/GUI/PipelineImage_Image.hpp"
#include "TTauri/GUI/PipelineFlat_DeviceShared.hpp"
#include "TTauri/GUI/PipelineBox_DeviceShared.hpp"
#include "TTauri/GUI/PipelineImage_DeviceShared.hpp"
#include "TTauri/GUI/PipelineSDF_DeviceShared.hpp"
#include "TTauri/GUI/PipelineFlat_Vertex.hpp"
#include "TTauri/GUI/PipelineBox_Vertex.hpp"
#include "TTauri/GUI/PipelineImage_Vertex.hpp"
#include "TTauri/GUI/PipelineSDF_Vertex.hpp"
#include "TTauri/Text/ShapedText.hpp"
#include <type_traits>

namespace TTauri {

/** Draw context for drawing using the TTauri shaders.
 */
class DrawContext {
private:
    Window *window;
    vspan<PipelineFlat::Vertex> *flatVertices;
    vspan<PipelineBox::Vertex> *boxVertices;
    vspan<PipelineImage::Vertex> *imageVertices;
    vspan<PipelineSDF::Vertex> *sdfVertices;

public:
    /// Foreground color.
    vec color = vec::color(1.0, 1.0, 1.0, 1.0);

    /// Fill color.
    vec fillColor = vec::color(0.0, 0.0, 0.0, 0.0);

    /// Size of lines.
    float lineWidth = 1.0;

    /** Shape of the corners of a box.
     * The vector holds information for each corner:
     *  - x: left-bottom
     *  - y: right-bottom
     *  - z: left-top
     *  - w: right-top
     *
     * The value means:
     *  - zero: Sharp corner
     *  - positive: Rounded corner of that radius
     *  - negative: Cur corner of that radius
     */
    vec cornerShapes = vec{0.0, 0.0, 0.0, 0.0};

    /** The clipping rectangle when drawing.
     * The clipping rectangle is passes as-is to the pipelines and
     * is not modified by the transform.
     */
    aarect clippingRectangle;

    /** Transform used on the given coordinates.
     * The z-axis translate is used for specifying the elevation
     * (inverse depth buffer) of the shape.
     */
    mat transform = mat::I();

    DrawContext(
        Window &window,
        vspan<PipelineFlat::Vertex> &flatVertices,
        vspan<PipelineBox::Vertex> &boxVertices,
        vspan<PipelineImage::Vertex> &imageVertices,
        vspan<PipelineSDF::Vertex> &sdfVertices
    ) noexcept :
        window(&window),
        flatVertices(&flatVertices),
        boxVertices(&boxVertices),
        imageVertices(&imageVertices),
        sdfVertices(&sdfVertices),
        color(0.0, 1.0, 0.0, 1.0),
        fillColor(1.0, 1.0, 0.0, 1.0),
        lineWidth(Theme::borderWidth),
        cornerShapes(),
        clippingRectangle(static_cast<vec>(window.currentWindowExtent))
    {
        flatVertices.clear();
        boxVertices.clear();
        imageVertices.clear();
        sdfVertices.clear();
    }

    DrawContext(DrawContext const &rhs) noexcept = default;
    DrawContext(DrawContext &&rhs) noexcept = default;
    DrawContext &operator=(DrawContext const &rhs) noexcept = default;
    DrawContext &operator=(DrawContext &&rhs) noexcept = default;
    ~DrawContext() = default;

    /** Draw a polygon with four corners of one color.
     * This function will draw a polygon between the four given points.
     * This will use the current:
     *  - transform, to transform each point.
     *  - clippingRectangle
     *  - fillColor
     */
    void drawFilledQuad(vec p1, vec p2, vec p3, vec p4) const noexcept {
        ttauri_assume(flatVertices != nullptr);
        flatVertices->emplace_back(transform * p1, clippingRectangle, fillColor);
        flatVertices->emplace_back(transform * p2, clippingRectangle, fillColor);
        flatVertices->emplace_back(transform * p3, clippingRectangle, fillColor);
        flatVertices->emplace_back(transform * p4, clippingRectangle, fillColor);
    }

    /** Draw a rectangle of one color.
    * This function will draw the given rectangle.
    * This will use the current:
    *  - transform, to transform each corner of the rectangle.
    *  - clippingRectangle
    *  - fillColor
    */
    void drawFilledQuad(aarect r) const noexcept {
        drawFilledQuad(r.corner<0>(), r.corner<1>(), r.corner<2>(), r.corner<3>());
    }

    /** Draw an axis aligned box
    * This function will draw the given box.
    * This will use the current:
    *  - transform, to transform the opposite corner (rotation is not recommended).
    *  - clippingRectangle
    *  - fillColor
    *  - borderSize
    *  - borderColor
    *  - shadowSize
    *  - cornerShapes
    */
    void drawBox(aarect box) const noexcept {
        ttauri_assume(boxVertices != nullptr);

        PipelineBox::DeviceShared::placeVertices(
            *boxVertices,
            transform * box,
            fillColor,
            lineWidth,
            color,
            cornerShapes,
            clippingRectangle
        );
    }

    /** Draw an axis aligned box
    * This function will shrink to include the size of the border within
    * the given rectangle. This will make the border be drawn sharply.
    *
    * This will also adjust rounded corners to the shrunk box.
    * 
    * This function will draw the given box.
    * This will use the current:
    *  - transform, to transform the opposite corner (rotation is not recommended).
    *  - clippingRectangle
    *  - fillColor
    *  - borderSize
    *  - borderColor
    *  - shadowSize
    *  - cornerShapes
    */
    void drawBoxIncludeBorder(aarect box) const noexcept {

        ttauri_assume(boxVertices != nullptr);

        let shrink_value = lineWidth * 0.5f;

        let newBox = shrink(box, shrink_value);

        let newCornerShapes = vec{
            std::max(0.0f, cornerShapes.x() - shrink_value),
            std::max(0.0f, cornerShapes.y() - shrink_value),
            std::max(0.0f, cornerShapes.z() - shrink_value),
            std::max(0.0f, cornerShapes.w() - shrink_value)
        };

        PipelineBox::DeviceShared::placeVertices(
            *boxVertices,
            transform * newBox,
            fillColor,
            lineWidth,
            color,
            newCornerShapes,
            clippingRectangle
        );
    }

    /** Draw an image
    * This function will draw an image.
    * This will use the current:
    *  - transform, to transform the image.
    *  - clippingRectangle
    */
    void drawImage(PipelineImage::Image &image) const noexcept {
        ttauri_assume(imageVertices != nullptr);

        image.placeVertices(*imageVertices, transform, clippingRectangle);
    }

    /** Draw shaped text.
     * This function will draw the shaped text.
     * The SDF-image-atlas needs to be prepared ahead of time.
     * This will use the current:
     *  - transform, to transform the shaped-text's bounding box
     *  - clippingRectangle
     */
    void drawText(ShapedText const &text) const noexcept {
        ttauri_assume(window != nullptr);
        ttauri_assume(sdfVertices != nullptr);

        window->device->SDFPipeline->placeVertices(
            *sdfVertices,
            text,
            transform,
            clippingRectangle
        );
    }

    /** Draw shaped text.
    * This function will draw the shaped text.
    * The SDF-image-atlas needs to be prepared ahead of time.
    * This will use the current:
    *  - transform, to transform the shaped-text's bounding box
    *  - clippingRectangle
    */
    void drawTextSingleColor(ShapedText const &text) const noexcept {
        ttauri_assume(window != nullptr);
        ttauri_assume(sdfVertices != nullptr);

        window->device->SDFPipeline->placeVertices(
            *sdfVertices,
            text,
            transform,
            clippingRectangle,
            color
        );
    }

    void drawGlyph(FontGlyphIDs const &glyph, aarect box) const noexcept {
        ttauri_assume(window != nullptr);
        ttauri_assume(sdfVertices != nullptr);

        window->device->SDFPipeline->placeVertices(
            *sdfVertices,
            glyph,
            transform * box,
            color,
            clippingRectangle
        );
    }

};

}