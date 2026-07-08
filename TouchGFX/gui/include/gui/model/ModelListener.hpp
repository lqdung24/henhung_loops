#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP

#include <gui/model/Model.hpp>

class ModelListener
{
public:

    ModelListener() : model(0) {}
    
    enum GuiCommand {
            CMD_NONE = 0,
            CMD_GOTO_HISTORY,
            CMD_GOTO_MAIN,
            CMD_START_MEASURE_OK,
            CMD_WARN_ALCOHOL_HIGH
        };

    virtual ~ModelListener() {}

    virtual void requestChangeToHistory() {}
	virtual void requestChangeToMain() {}

    void bind(Model* m)
    {
        model = m;
    }
protected:
    Model* model;
};

#endif // MODELLISTENER_HPP
