# PROJECT SAMPLE

Mô tả nội dung cần báo cáo của môn đồ án
> Sinh viên nên Fork từ dự án này

## GIỚI THIỆU

__Đề bài/Mục tiêu sản phẩm__ : _Copy lại nội dung đề bài được yêu cầu._

__Hướng tiếp cận__: _Có nhiều cách để đạt mục tiêu, nên cần giải thích cách thức đề tài sẽ làm. Ví dụ tương tác với người dùng qua nút bấm, hay tương tác qua joystick, hay tương tác qua nhận dạng hành vi bằng module sẵn có_

__Sản phẩm:__
1. Tính năng
2. Tính năng
3. Tính năng
- Ảnh chụp minh họa:\
  ![Ảnh minh họa](https://soict.hust.edu.vn/wp-content/uploads/logo-soict-hust-1-1024x416.png)

## TÁC GIẢ

- Tên nhóm:......
- Thành viên trong nhóm
  |STT|Họ tên|MSSV|Công việc|
  |--:|--|--|--|
  |1|Nguyễn Hoàng Hải|20002987|hiện thị oled, hiệu ứng, và xử lý ngắt|

## MÔI TRƯỜNG HOẠT ĐỘNG

- Liệt kê module CPU/dev kit. Ví dụ ESP32 Dev Module, hoặc STM32F429-DISC.
- Liệt kê các kit, module được sử dụng: ví dụ: stm32 cảm biến...
  __Bill of materials__
  STT|Tên linh kiện| Ý nghĩa
  --|--|--|
  1|Nút bấm 4x4|Tương tác bấm nút
  2|Oled 1"3|Màn hình hiển thị

## SO ĐỒ SCHEMATIC

_Cho biết cách nối dây, kết nối giữa các linh kiện_ 
Ví dụ có thể liệt kê dạng bảng
|STM32F429|Module ngoại vi|
|--|--|
|PA0|Nút bấm điều khiển trên board|
|PG13|Led báo trên board|
|PE2|MQ3 SCK|
|PE3|MQ3 SDA|

hoặc dạng ảnh:\
![image](https://github.com/user-attachments/assets/f15f7379-09bf-472a-b0d9-4de6d32a6ec0)

hoặc bất cứ dạng nào thể hiện được cách đấu nối giữa các linh kiện

## TÍCH HỢP HỆ THỐNG

- Mô tả các thành phần phần cứng và vai trò của chúng: máy chủ, máy trạm, thiết bị IoT, MQTT Server, module cảm biến IoT...
- Mô tả các thành phần phần mềm và vai trò của chúng, vị trí nằm trên phần cứng nào: Front-end, Back-end, Worker, Middleware...

## ĐẶC TẢ HÀM

- Giải thích một số hàm quan trọng: ý nghĩa của hàm, tham số vào, ra

  ```C
     /**
      *  Hàm tính ...
      *  @param  x  Tham số
      *  @param  y  Tham số
      */
     void abc(int x, int y = 2);
  ```
  
## KẾT QUẢ

- Các ảnh chụp với caption giải thích.
- Hoặc video sản phẩm