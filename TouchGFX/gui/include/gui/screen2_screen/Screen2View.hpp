#ifndef SCREEN2VIEW_HPP
#define SCREEN2VIEW_HPP

#include <gui_generated/screen2_screen/Screen2ViewBase.hpp>
#include <gui/screen2_screen/Screen2Presenter.hpp>

extern "C" {
	#include "app_config.h"
	#include "log.h"
}

class Screen2View : public Screen2ViewBase
{
public:
    Screen2View();
    virtual ~Screen2View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void scrollList1UpdateItem(CustomContainer1& item, int16_t itemIndex) override;


private:
    const Log_t* logs;
    uint8_t logCount;

protected:
};

#endif // SCREEN2VIEW_HPP
