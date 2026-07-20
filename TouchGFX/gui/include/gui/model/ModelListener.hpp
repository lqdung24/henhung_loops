#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP

#include <gui/model/Model.hpp>

extern "C" {
	#include "app_config.h"
}

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

    virtual const Log_t* getLogs() {}
    virtual uint8_t getLogCount() {}

    virtual void requestChangeToHistory() {}
	virtual void requestChangeToMain() {}

	virtual void notifyCardIdReceived(const char* cardId) {}

	virtual void updateName(const char* name) {}

	virtual void updateTime(const char *time) {}

	virtual void show_high_alcohol_dialog(int state) {}

	virtual void show_blow_analog(int state, const char *time) {}

	virtual void show_scan_card_dialog(int state) {}

	virtual void show_results(const char *raw_voltage, const char *alcohol_lv, const char *fine) {}

    void bind(Model* m)
    {
        model = m;
    }

    const Log_t* getLogs() const;
    uint8_t getLogCount() const;

private:
    Log_t logs[LOG_LEN];
    uint8_t logCount;

protected:
    Model* model;
};

#endif // MODELLISTENER_HPP
