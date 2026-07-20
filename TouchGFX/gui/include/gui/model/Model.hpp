#ifndef MODEL_HPP
#define MODEL_HPP

class ModelListener;

extern "C" {
	#include "app_config.h"
}
class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void tick();

    const Log_t* getLogs() const;
    int getLogCount() const;

protected:
    ModelListener* modelListener;
};

#endif // MODEL_HPP
