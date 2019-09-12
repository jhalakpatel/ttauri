// Copyright 2019 Pokitec
// All rights reserved.

#include "Window_base.hpp"
#include "Window.hpp"
#include "Device.hpp"
#include "Widgets/WindowWidget.hpp"

namespace TTauri::GUI {

using namespace std;

Window_base::Window_base(const std::shared_ptr<WindowDelegate> delegate, const std::string title) :
    state(State::Initializing),
    delegate(move(delegate)),
    title(move(title))
{
}

Window_base::~Window_base()
{
    try {
        gsl_suppress(f.6) {
            if (state != State::NoWindow) {
                LOG_FATAL("Window '{}' was not properly teardown before destruction.", title);
                abort();
            }
            LOG_INFO("Window '{}' has been propertly destructed.", title);
        }
    } catch (...) {
        abort();
    }
}

void Window_base::initialize()
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    widget = std::make_shared<Widgets::WindowWidget>();

    auto _window = dynamic_cast<Window *>(this);
    required_assert(_window);
    widget->setParentWindow(_window);

    openingWindow();
}

void Window_base::openingWindow() {
    Window *thisWindow = dynamic_cast<Window *>(this);
    assert(thisWindow);
    delegate->openingWindow(*thisWindow);
    state = State::NoDevice;
}

void Window_base::closingWindow() {
    Window* thisWindow = dynamic_cast<Window*>(this);
    assert(thisWindow);
    delegate->closingWindow(*thisWindow);
    state = State::NoWindow;
}

void Window_base::setDevice(Device *newDevice)
{
    std::scoped_lock lock(TTauri::GUI::mutex);

    if (device) {
        state = State::DeviceLost;
        teardown();
    }

    device = newDevice;
}



}