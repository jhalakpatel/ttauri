
#include "TTauri/all.hpp"
#include "TTauri/GUI/all.hpp"
#include "TTauri/Widgets/all.hpp"

#include <vulkan/vulkan.hpp>

#include <Windows.h>

#include <boost/filesystem.hpp>

#include <memory>
#include <vector>

using namespace std;
using namespace TTauri;

class MyWindowDelegate : public GUI::Window::Delegate {
public:
    void openingWindow(const std::shared_ptr<GUI::Window> &window) override
    {
        auto button1 = TTauri::make_shared<Widgets::ButtonWidget>(u8"H�ll� W�rld");
        window->widget->add(button1);
        window->addConstraint(button1->box.width == 100);
        window->addConstraint(button1->box.height == 30);
        window->addConstraint(button1->box.outerLeft() == window->box().left);
        window->addConstraint(button1->box.outerBottom() == window->box().bottom);
        window->addConstraint(button1->box.outerTop() <= window->box().top());

        auto button2 = TTauri::make_shared<Widgets::ButtonWidget>(u8"Foo Bar");
        window->widget->add(button2);
        window->addConstraint(button2->box.width >= 100);
        window->addConstraint(button2->box.height == 30);
        window->addConstraint(button2->box.outerLeft() == button1->box.right());
        window->addConstraint(button2->box.outerBottom() == window->box().bottom);
        window->addConstraint(button2->box.outerRight() == window->box().right());
        window->addConstraint(button2->box.outerTop() <= window->box().top());
    }

    void closingWindow(const std::shared_ptr<GUI::Window> &window) override
    {
        LOG_INFO("Window being destroyed.");
    }
};

class MyApplicationDelegate : public Application::Delegate {
public:
    void startingLoop() override
    {
        auto myWindowDelegate = TTauri::make_shared<MyWindowDelegate>();

        get_singleton<GUI::Instance>()->createWindow(myWindowDelegate, "Hello World 1");
        //get_singleton<GUI::Instance>()->createWindow(myWindowDelegate, "Hello World 2");
    }

    void lastWindowClosed() override
    {
    }
};

#include "TTauri/Draw/TrueTypeParser.hpp"

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
    auto myApplicationDelegate = TTauri::make_shared<MyApplicationDelegate>();

    let font = TTauri::Draw::parseTrueTypeFile(std::filesystem::path("../TTauri/Draw/TestFiles/Roboto-Regular.ttf"));


    make_singleton<Application_win32>(myApplicationDelegate, hInstance, hPrevInstance, pCmdLine, nCmdShow);
    make_singleton<GUI::Instance_vulkan_win32>();

    return get_singleton<Application>()->loop();
}
