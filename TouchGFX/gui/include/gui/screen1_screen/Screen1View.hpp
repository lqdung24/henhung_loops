#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    void updateCardId(const char* cardId);
    void updateName(const char* name);
    void updateTime(const char* time);
    void show_high_alcohol_dialog(int state);
    void show_blow_analog(int state, const char* time);
    void show_scan_card_dialog(int state);

    void show_results(const char *raw_voltage, const char *alcohol_lv, const char *fine);
protected:
};

#endif // SCREEN1VIEW_HPP
