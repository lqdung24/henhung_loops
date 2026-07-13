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
   static_cast<FrontendApplication*>(touchgfx::Application::getInstance())->gotoScreen2ScreenNoTransition();
}

void Screen1Presenter::notifyCardIdReceived(const char* cardId)
{
    view.updateCardId(cardId);
}

void Screen1Presenter::updateName(const char* name)
{
    view.updateName(name);
}

void Screen1Presenter::updateTime(const char* time)
{
    view.updateTime(time);
}

void Screen1Presenter::show_high_alcohol_dialog(int state)
{
    view.show_high_alcohol_dialog(state);
}

void Screen1Presenter::show_blow_analog(int state, const char* time)
{
    view.show_blow_analog(state, time);
}

void Screen1Presenter::show_scan_card_dialog(int state)
{
    view.show_scan_card_dialog(state);
}

void Screen1Presenter::show_results(const char *raw_voltage,
		const char *alcohol_lv, const char *fine) {
	view.show_results(raw_voltage, alcohol_lv, fine);
}
