#include <gui/screen2_screen/Screen2View.hpp>

Screen2View::Screen2View()
{
}
void Screen2View::setupScreen()
{
    Screen2ViewBase::setupScreen();

    logs = presenter->getLogs();
    logCount = presenter->getLogCount();

    scrollList1.setNumberOfItems(logCount);
    scrollList1.invalidate();
}

void Screen2View::tearDownScreen()
{
    Screen2ViewBase::tearDownScreen();
}

void Screen2View::scrollList1UpdateItem(CustomContainer1& item, int16_t itemIndex)
{
    item.setData(logs[itemIndex]);
}
