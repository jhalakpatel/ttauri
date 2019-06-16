// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "Window_forward.hpp"
#include "TTauri/Color.hpp"

namespace TTauri::GUI {

class WindowWidget : public Widget {
public:
    enum class Type {
        WINDOW,
        PANEL,
        FULLSCREEN,
    };

    wsRGBApm backgroundColor;

    WindowWidget(const std::weak_ptr<Window> window);
    ~WindowWidget(){};

    WindowWidget(const WindowWidget&) = delete;
    WindowWidget &operator=(const WindowWidget&) = delete;
    WindowWidget(WindowWidget&&) = delete;
    WindowWidget &operator=(WindowWidget&&) = delete;
};

}