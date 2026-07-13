#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>

extern "C" {
    #include "cmsis_os2.h"
    extern osMessageQueueId_t guiCommandQueueHandle;
    extern osMessageQueueId_t rfidDataQueueHandle;
	#include "app_config.h"
    extern Log_t app_log[LOG_LEN];
    extern int log_counter;
}

Model::Model() : modelListener(0)
{

}

RFID_Packet_t received_packet;

const Log_t* Model::getLogs() const
{
    return app_log;
}

int Model::getLogCount() const
{
    return log_counter;
}

void Model::tick()
{
	UI_cmd_t rx_cmd;
	rx_cmd.cmd = CMD_NONE;

	// Kiểm tra Queue liên tục không đợi (timeout = 0)
	if (osMessageQueueGet(guiCommandQueueHandle, &rx_cmd, NULL, 0) == osOK)
	{
		switch (rx_cmd.cmd)
		{
			case CMD_GOTO_HISTORY:
				if (modelListener != 0) {
					modelListener->requestChangeToHistory();
				}
				break;

			case CMD_GOTO_MAIN:
				if (modelListener != 0) {
					modelListener->requestChangeToMain();
				}
				break;
			case CMD_SHOW_HIGH_ALCOHOL_DIALOG:
				if(modelListener != 0){
					modelListener->show_high_alcohol_dialog(1);
				}
				break;
			case CMD_OFF_HIGH_ALCOHOL_DIALOG:
				if(modelListener != 0){
					modelListener->show_high_alcohol_dialog(0);
				}
				break;

			case CMD_PROMPT_SCAN_CARD:
				if(modelListener != 0){
					modelListener->show_high_alcohol_dialog(0);
					modelListener->show_scan_card_dialog(1);
				}
				break;

			case CMD_START_MEASURE_OK:
				if(modelListener != 0){
					if(rx_cmd.count == 1){
			        	modelListener->show_scan_card_dialog(0);
						modelListener->show_blow_analog(1, rx_cmd.texts[0]);
					}
				}
				break;
			case CMD_MEASURE_DONE:
				if(modelListener != 0){
					modelListener->show_blow_analog(0, "1m5s");
					if(rx_cmd.count == 3){
						modelListener->show_results(rx_cmd.texts[0], rx_cmd.texts[1], rx_cmd.texts[2]);
					}
				}
				break;

			default:
				break;
		}
	}

	if (osMessageQueueGet(rfidDataQueueHandle, &received_packet, NULL, 0) == osOK)
	    {
	        if (modelListener != 0)
	        {
	            modelListener->notifyCardIdReceived(received_packet.id);
	            modelListener->updateName(received_packet.name);
	            modelListener->updateTime(received_packet.time);
	        }
	    }
}
