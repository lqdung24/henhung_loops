#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

extern "C" {
    #include "cmsis_os2.h"
    extern osMessageQueueId_t guiCommandQueueHandle;
}

// Định nghĩa lại enum giống bên C để code C++ hiểu
enum {
    CMD_NONE = 0,
    CMD_GOTO_HISTORY,
    CMD_GOTO_MAIN,
    CMD_START_MEASURE_OK,
    CMD_WARN_ALCOHOL_HIGH
};

Model::Model() : modelListener(0)
{

}

void Model::tick()
{
	uint8_t rx_cmd = CMD_NONE;

	// Kiểm tra Queue liên tục không đợi (timeout = 0)
	if (osMessageQueueGet(guiCommandQueueHandle, &rx_cmd, NULL, 0) == osOK)
	{
		switch (rx_cmd)
		{
			case CMD_GOTO_HISTORY:
				// Nếu có hàm lắng nghe (Presenter đang kích hoạt) thì báo lệnh
				if (modelListener != 0) {
					modelListener->requestChangeToHistory();
				}
				break;

			case CMD_GOTO_MAIN:
				if (modelListener != 0) {
					modelListener->requestChangeToMain();
				}
				break;

			default:
				break;
		}
	}
}
