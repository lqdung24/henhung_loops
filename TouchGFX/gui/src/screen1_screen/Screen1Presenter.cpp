#include <gui/screen1_screen/Screen1View.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

Screen1Presenter::Screen1Presenter(Screen1View& v)
    : view(v)
{

}

void Screen1Presenter::activate()
{

}

void Screen1Presenter::deactivate()
{

}

void Screen1Presenter::requestChangeToHistory()
{
    // Gọi hàm chuyển màn hình (Tên hàm tự sinh trong FrontendApplicationBase.hpp thường có dạng gotoScreen...NoTransition hoặc tương tự)
    static_cast<FrontendApplication*>(touchgfx::Application::getInstance())->gotoScreen2ScreenNoTransition();
}
