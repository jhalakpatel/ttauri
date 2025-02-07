// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pipeline_image_page.hpp"
#include "../vspan.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../geometry/matrix.hpp"
#include <span>
#include <atomic>
#include <string>

namespace tt {

template<typename T> class pixel_map;
class sfloat_rgba16;
};

namespace tt::pipeline_image {

struct vertex;
struct ImageLocation;
struct device_shared;

/** This is a image that is uploaded into the texture atlas.
 */
struct Image {
    enum class State { Uninitialized, Drawing, Uploaded };

    mutable std::atomic<State> state = State::Uninitialized;

    device_shared *parent;

    /** The width of the image in pixels.
     */
    size_t width_in_px;

    /** The height of the image in pixels.
     */
    size_t height_in_px;

    /** The width of the image in pages
     */
    size_t width_in_pages;

    /** The height of the image in pages
     */
    size_t height_in_pages;

    std::vector<Page> pages;

    Image() noexcept :
        parent(nullptr), width_in_px(0), height_in_px(0), width_in_pages(0), height_in_pages(0), pages() {}

    Image(
        device_shared *parent,
        size_t width_in_px,
        size_t height_in_px,
        size_t width_in_pages,
        size_t height_in_pages,
        std::vector<Page> &&pages) noexcept :
        parent(parent),
        width_in_px(width_in_px),
        height_in_px(height_in_px),
        width_in_pages(width_in_pages),
        height_in_pages(height_in_pages),
        pages(std::move(pages)) {}

    Image(Image &&other) noexcept;
    Image &operator=(Image &&other) noexcept;
    ~Image();

    Image(Image const &other) = delete;
    Image &operator=(Image const &other) = delete;

    /** Find the image coordinates of a page in the image.
     * @param pageIndex Index in the pages-vector.
     * @return The rectangle within the image representing a quad to be drawn.
     *         This rectangle is already size-adjusted for the quads on the edge.
     */
    aarectangle index_to_rect(size_t page_index) const noexcept;

    /*! Place vertices for this image.
     * An image is build out of atlas pages, that need to be individual rendered.
     * A page with the value std::numeric_limits<uint16_t>::max() is not rendered.
     */
    void place_vertices(vspan<vertex> &vertices, aarectangle clipping_rectangle, matrix3 transform);

    /** Upload image to atlas.
     */
    void upload(pixel_map<sfloat_rgba16> const &image) noexcept;

private:
    //! Temporary memory used for pre calculating vertices.
    std::vector<std::tuple<point3, extent2, bool>> tmpvertexPositions;

    void calculateVertexPositions(matrix3 transform, aarectangle clippingRectangle);

    void placePageVertices(vspan<vertex> &vertices, size_t index, aarectangle clippingRectangle) const;

};

}
