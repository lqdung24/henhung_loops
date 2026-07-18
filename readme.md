# ĐỒ ÁN: MÁY THỔI NỒNG ĐỘ CỒN

> Báo cáo này trình bày chi tiết về quá trình thiết kế, tích hợp và lập trình Hệ thống Máy thổi nồng độ cồn thông minh trên kit phát triển STM32F429. Nội dung bao gồm mục tiêu sản phẩm, danh sách linh kiện, sơ đồ đấu nối phần cứng, kiến trúc phần mềm tích hợp TouchGFX & FreeRTOS, cùng các kết quả hoạt động thực tế của thiết bị.

## GIỚI THIỆU

__Đề bài/Mục tiêu sản phẩm__:
- Sử dụng STM32F429 và TouchGFX.
- Yêu cầu người dùng thổi hơi thở vào cảm biến MQ3 và quét thẻ nhân viên NFC 13.56MHz và module RC522
- Hiển thị thông tin người dùng lên màn hình gồm: Mã thẻ, họ tên (ánh xạ cố định từ file txt với mã thẻ), thời điểm đo, mức độ cồn thô (lấy từ mức điện áp từ MQ03), mức cồn qui đổi theo thang ml/100ml (theo bảng qui đổi)
- Bấm nút B1 để kích hoạt một lượt đo nồng độ cồn. Nhưng để đo lượt tiếp theo, hãy đợi cho tới khi độ cồn thô xuống dưới ngưỡng thấp thì mới cho phép, và hiện ra trên màn hình thông tin cảnh báo nếu không thỏa mãn.
- Bấm và giữ nút B1 trong 3 giây để đưa thiết bị vào màn hình lịch sử, có thể xem được 3 lượt đo gần đây nhất. Bấm và giữ nút B1 lần nữa để quay trở về màn hình chính.


__Hướng tiếp cận__:
Hệ thống sử dụng cảm biến khí MQ3 để lấy mẫu nồng độ cồn trong hơi thở người dùng và module MFRC522 để đọc thẻ định danh (NFC 13.56MHz). Dữ liệu được xử lý bởi vi điều khiển STM32F429 và hiển thị thông qua giao diện TouchGFX trực quan trên màn hình LCD.
- Tương tác chính thông qua nút bấm vật lý (Nút B1/User Button).
- Ánh xạ thông tin mã thẻ với tên nhân viên từ một tệp dữ liệu đã lưu trữ.
- Lưu lịch sử và quản lý thời gian thực với module Tiny RTC DS1307.

__Sản phẩm:__

Các tính năng chính:
1. **Định danh & Đo nồng độ cồn**: Yêu cầu người dùng quét thẻ nhân viên NFC và thổi vào cảm biến MQ3.
2. **Hiển thị thông tin trực quan**: Giao diện TouchGFX hiển thị Mã thẻ, Họ tên nhân viên, Thời điểm đo (lấy từ RTC), Mức độ cồn thô (từ điện áp ADC), và Mức cồn quy đổi (ml/100ml theo bảng quy đổi chuẩn).
3. **Quản lý phiên đo thông minh (Nút B1)**: 
   - Bấm nút B1 để kích hoạt lượt đo mới.
   - Hệ thống khóa an toàn: Lượt đo tiếp theo chỉ được phép thực hiện khi nồng độ cồn thô trong cảm biến đã giảm xuống dưới ngưỡng thấp (Reset cảm biến). Nếu chưa thỏa mãn, màn hình sẽ hiện cảnh báo.
4. **Xem lịch sử đo**: Bấm và giữ nút B1 trong 3 giây để truy cập màn hình Lịch sử, cho phép xem 3 lượt đo gần nhất. Giữ B1 thêm lần nữa để quay trở lại màn hình chính.
- Ảnh chụp minh họa:\
  ![Ảnh minh họa](https://soict.hust.edu.vn/wp-content/uploads/logo-soict-hust-1-1024x416.png)

## TÁC GIẢ

- Tên nhóm: l00ps
- Thành viên trong nhóm
  | STT | Họ tên | MSSV | Công việc |
  | --: | -- | -- | -- |
  | 1 | Lê Quang Dũng | 20235301 | Trưởng nhóm - Thiết kế giao diện TouchGFX, tích hợp hệ thống FreeRTOS & logic máy trạng thái đo |
  | 2 | Lưu Quốc Dũng | 20235302 | Giao tiếp module MFRC522 (SPI) đọc thẻ NFC và ánh xạ thông tin người dùng |
  | 3 | Lê Nghĩa Hiệp | 20235326 | Xử lý ngắt nút bấm (nhấn giữ 3s, chuyển màn hình lịch sử), tổng hợp và viết báo cáo đồ án |
  | 4 | Nguyễn Xuân Hoàng | 20230078 | Giao tiếp cảm biến MQ3 (ADC) và xây dựng thuật toán quy đổi nồng độ cồn (ml/100ml) |
  | 5 | Trần Bá Lợi | 20225358 | Giao tiếp module Tiny RTC DS1307 (I2C) cấu hình thời gian và sơ đồ đấu nối phần cứng |
  

## MÔI TRƯỜNG HOẠT ĐỘNG

- **Kit phát triển / CPU**: STM32F429I-DISC1 (ARM Cortex-M4)
- **Công cụ phần mềm**: STM32CubeIDE, TouchGFX Designer, FreeRTOS

**Bill of materials (Danh sách linh kiện)**:
| STT | Tên linh kiện | Ý nghĩa / Chức năng |
| ---: | :--- | :--- |
| 1 | Kit STM32F429I-DISCO | Vi điều khiển trung tâm và Màn hình hiển thị LCD |
| 2 | Cảm biến khí MQ3 | Đo nồng độ cồn trong hơi thở (Analog Output) |
| 3 | Module RFID RC522 | Đọc thẻ NFC 13.56MHz định danh người dùng |
| 4 | Module Tiny RTC (DS1307 + AT24C32) | Cung cấp thời gian thực (Real-time Clock) |

## SƠ ĐỒ SCHEMATIC

Kết nối các linh kiện ngoại vi với STM32F429:

| Vi điều khiển STM32F429 | Chân linh kiện ngoại vi | Chức năng / Ghi chú |
| :--- | :--- | :--- |
| **PA0** | Nút bấm User (B1) | Tương tác người dùng (Đo / Xem lịch sử) |
| **PC3** (ADC1_IN13) | Chân A0 - Cảm biến MQ3 | Tín hiệu Analog nồng độ cồn thô |
| **PF7** (SPI5_SCK) | SCK - Module RC522 | Clock giao tiếp SPI |
| **PF8** (SPI5_MISO) | MISO - Module RC522 | Dữ liệu MISO giao tiếp SPI |
| **PF9** (SPI5_MOSI) | MOSI - Module RC522 | Dữ liệu MOSI giao tiếp SPI |
| **PF6** (GPIO_Output) | SDA/CS - Module RC522 | Chip Select giao tiếp SPI |
| **PA8** (I2C3_SCL) | SCL - Tiny RTC DS1307 | Clock giao tiếp I2C |
| **PC9** (I2C3_SDA) | SDA - Tiny RTC DS1307 | Data giao tiếp I2C |

*(Các chân VCC nối 3.3V hoặc 5V tùy thuộc điện áp hoạt động linh kiện, GND nối chung mass)*

## TÍCH HỢP HỆ THỐNG

- **Phần cứng**: Kit STM32F429I-DISCO là bộ xử lý trung tâm, giao tiếp với thẻ nhớ EEPROM AT24C32/DS1307 (I2C) để lấy dữ liệu thời gian, thu thập tín hiệu nồng độ cồn từ MQ3 qua bộ chuyển đổi ADC, và đọc thẻ RFID qua SPI.
- **Phần mềm (Firmware)**:
  - **Maintask / FreeRTOS**: Quản lý các luồng (Thread) thu thập dữ liệu cảm biến, đọc thẻ RFID độc lập.
  - **Middleware (TouchGFX)**: Cập nhật giao diện màn hình, chuyển cảnh (từ Màn hình chính sang Lịch sử) khi nhận được sự kiện ngắt từ nút bấm B1 (thông qua Message Queue).
  - **Ánh xạ dữ liệu**: Firmware tích hợp tệp cấu hình danh sách sinh viên/nhân viên, tự động đối chiếu UID đọc được từ thẻ RC522 ra tên người dùng.

## ĐẶC TẢ HÀM (CÁC HÀM QUAN TRỌNG)

- **`MQ3_read()`**: Đọc tín hiệu Analog và quy đổi thành mức điện áp (thô) để xác định nồng độ cồn.
- **`TM_MFRC522_Check(uint8_t* id)`**: Kiểm tra xem thẻ RFID có đang được đặt ở gần anten không. Trả về mã UID nếu có thẻ.
- **`RTC_SetTime(...) / GetTime(...)`**: Cài đặt hoặc đọc thời gian hiện tại từ module Tiny RTC để lưu lại thời điểm người dùng bắt đầu thổi.
- **`TouchGFX_Task`**: Cập nhật UI, hiển thị các Model data như cảnh báo chưa xả nồng độ, lịch sử lượt đo gần nhất.

## KẾT QUẢ

- [Chèn ảnh thiết bị ở chế độ chờ]
- [Chèn ảnh khi quét thẻ thành công và hiển thị kết quả nồng độ cồn (kèm ml/100ml)]
- [Chèn ảnh màn hình cảnh báo khi cảm biến MQ3 chưa khôi phục về trạng thái an toàn]
- [Chèn ảnh màn hình Lịch sử chứa 3 lần đo gần nhất]