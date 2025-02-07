// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "abstract_container_widget.hpp"
#include "grid_layout_delegate.hpp"
#include "../geometry/spread_sheet_address.hpp"
#include "../GUI/theme.hpp"
#include "../flow_layout.hpp"
#include <memory>

namespace tt {

class grid_layout_widget : public abstract_container_widget {
public:
    using super = abstract_container_widget;

    grid_layout_widget(
        gui_window &window,
        std::shared_ptr<abstract_container_widget> parent,
        std::weak_ptr<grid_layout_delegate> delegate = {}) noexcept :
        abstract_container_widget(window, parent), _delegate(delegate)
    {
    }

    ~grid_layout_widget()
    {
        if (auto delegate_ = _delegate.lock()) {
            delegate_->deinit(*this);
        }
    }

    void init() noexcept override
    {
        if (auto delegate_ = _delegate.lock()) {
            delegate_->init(*this);
        }
    }

    [[nodiscard]] bool
    update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;
    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override;

    /* Add a widget to the grid.
     */
    std::shared_ptr<widget> add_widget(size_t column_nr, size_t row_nr, std::shared_ptr<widget> childWidget) noexcept;

    /** Add a widget directly to this widget.
     *
     * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
     */
    template<typename T, typename... Args>
    std::shared_ptr<T> make_widget(size_t column_nr, size_t row_nr, Args &&...args)
    {
        auto tmp = std::make_shared<T>(window, shared_from_this(), std::forward<Args>(args)...);
        tmp->init();
        return std::static_pointer_cast<T>(add_widget(column_nr, row_nr, std::move(tmp)));
    }

    /** Add a widget directly to this widget.
     *
     * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
     */
    template<typename T, typename... Args>
    std::shared_ptr<T> make_widget(std::string_view address, Args &&...args)
    {
        ttlet [column_nr, row_nr] = parse_absolute_spread_sheet_address(address);
        return make_widget<T>(column_nr, row_nr, std::forward<Args>(args)...);
    }

private:
    struct cell {
        size_t column_nr;
        size_t row_nr;
        std::shared_ptr<tt::widget> widget;

        cell(size_t column_nr, size_t row_nr, std::shared_ptr<tt::widget> widget) noexcept :
            column_nr(column_nr), row_nr(row_nr), widget(std::move(widget))
        {
        }

        [[nodiscard]] aarectangle rectangle(flow_layout const &columns, flow_layout const &rows, float container_height) const noexcept
        {
            ttlet[x, width] = columns.get_offset_and_size(column_nr);
            ttlet[y, height] = rows.get_offset_and_size(row_nr);

            return {x, container_height - y - height, width, height};
        };
    };

    std::vector<cell> _cells;

    std::weak_ptr<grid_layout_delegate> _delegate;

    flow_layout _rows;
    flow_layout _columns;

    [[nodiscard]] static std::pair<size_t, size_t> calculate_grid_size(std::vector<cell> const &cells) noexcept;
    [[nodiscard]] static std::tuple<extent2, extent2, extent2>
    calculate_size(std::vector<cell> const &cells, flow_layout &rows, flow_layout &columns) noexcept;
    [[nodiscard]] bool address_in_use(size_t column_nr, size_t row_nr) const noexcept;
};

} // namespace tt
