/*
 * maintask.c
 *
 *  Created on: Jul 8, 2026
 *      Author: lqdung
 */

#include "maintask.h"

typedef enum {
    BTN_NONE = 0,
    BTN_SHORT_PRESS,
    BTN_LONG_PRESS
} ButtonEvent_t;

typedef enum {
    SCREEN_MAIN = 0,
    SCREEN_HISTORY
} ScreenState_t;

enum {
    CMD_NONE = 0,
    CMD_GOTO_HISTORY,
    CMD_GOTO_MAIN,
    CMD_START_MEASURE_OK,
    CMD_WARN_ALCOHOL_HIGH
};

void StartSensorTask(void *argument)
{
  /* USER CODE BEGIN StartSensorTask */
  ButtonEvent_t btn_evt = BTN_NONE;
  uint32_t received_avg_adc = 0;
  ScreenState_t current_screen = SCREEN_MAIN;

  /* Infinite loop */
  for(;;)
  {
    // 1. KIỂM TRA SỰ KIỆN NÚT BẤM (Đợi tối đa 20ms, không block vô tận để luồng vẫn check được ADC nếu có)
    if (osMessageQueueGet(btnEventQueueHandle, &btn_evt, NULL, 20) == osOK)
    {
        // TRƯỜNG HỢP A: BẤM GIỮ 3 GIÂY -> ĐỔI MÀN HÌNH
        if (btn_evt == BTN_LONG_PRESS)
        {
            uint8_t cmd = CMD_NONE;
            if (current_screen == SCREEN_MAIN) {
                current_screen = SCREEN_HISTORY;
                cmd = CMD_GOTO_HISTORY;
            } else {
                current_screen = SCREEN_MAIN;
                cmd = CMD_GOTO_MAIN;
            }
            osMessageQueuePut(guiCommandQueueHandle, &cmd, 0, 0);
        }

        // TRƯỜNG HỢP B: BẤM NHẢ NHANH -> KÍCH HOẠT LƯỢT ĐO MỚI
        else if (btn_evt == BTN_SHORT_PRESS)
        {
            // Chỉ cho kích hoạt đo mới khi đang đứng ở Màn hình chính
            if (current_screen == SCREEN_MAIN)
            {
                // Đọc kiểm tra giá trị ADC thô hiện tại từ cảm biến xem đã "sạch cồn" chưa
                // (Lấy giá trị từ hàng đợi ADC nếu có sẵn, hoặc dùng biến lưu trữ gần nhất)
//                uint8_t cmd = CMD_NONE;
//
//                if (received_avg_adc < ALCOHOL_CLEAN_THRESHOLD)
//                {
//                    // THỎA MÃN: Cảm biến sạch -> Kích hoạt lượt đo mới
//                    cmd = CMD_START_MEASURE_OK;
//
//                    // TODO: Thêm các lệnh logic phần cứng của bạn ở đây:
//                    // - Bật quét thẻ RFID/NFC RC522
//                    // - Kích hoạt còi buzz bíp ngắn báo hiệu bắt đầu đo
//                }
//                else
//                {
//                    // KHÔNG THỎA MÃN: Khí cồn buồng sấy còn cao -> Cảnh báo
//                    cmd = CMD_WARN_ALCOHOL_HIGH;
//                }
//
//                // Gửi trạng thái phản hồi lên giao diện GUI
//                osMessageQueuePut(guiCommandQueueHandle, &cmd, 0, 0);
            }
        }

        // Reset biến sự kiện sau khi xử lý xong
        btn_evt = BTN_NONE;
    }

    // 2. KIỂM TRA DỮ LIỆU ADC (Cập nhật nồng độ cồn liên tục từ DMA)
    if (osMessageQueueGet(adcDataQueueHandle, &received_avg_adc, NULL, 0) == osOK)
    {
        // Biến received_avg_adc này chính là giá trị trung bình nửa mảng DMA
        // Bạn có thể xử lý quy đổi sang mg/L hoặc gửi thẳng lên giao diện nếu cần hiện đồ thị thô
        // Ví dụ: tính toán quy đổi áp...
    }

    // Nghỉ ngơi nhẹ 10ms để nhường CPU cho các Task khác hoạt động hiệu quả
    osDelay(10);
  }
  /* USER CODE END StartSensorTask */
}
