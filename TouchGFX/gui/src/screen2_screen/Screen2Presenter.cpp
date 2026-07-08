#include <gui/screen2_screen/Screen2View.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>

Screen2Presenter::Screen2Presenter(Screen2View& v)
    : view(v)
{

}

void Screen2Presenter::activate()
{

}

void Screen2Presenter::deactivate()
{

}

void Screen2Presenter::requestChangeToMain()
{
    // Gọi hàm chuyển màn hình (Tên hàm tự sinh trong FrontendApplicationBase.hpp thường có dạng gotoScreen...NoTransition hoặc tương tự)
    static_cast<FrontendApplication*>(touchgfx::Application::getInstance())->gotoScreen1ScreenNoTransition();
}
