/**	
 * |----------------------------------------------------------------------
 * | Copyright (C) Tilen Majerle, 2014
 * | Modification by TrungNL to work with STM32F429I and HAL (CubeF4 1.28.1)
 * | All dependencies to other TM libs such as TM_SPI are removed
 * | 
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |  
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * | 
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */

/**
 * @file   tm_stm32f4_mfrc522.c
 * @brief  Driver giao tiếp module RFID MFRC522 qua giao thức SPI.
 *
 * Module MFRC522 hoạt động ở tần số 13.56 MHz (chuẩn ISO/IEC 14443A).
 * Driver này sử dụng SPI5 của STM32F429 để truyền nhận dữ liệu với
 * MFRC522 thông qua các thanh ghi nội bộ của chip.
 *
 * Quy trình đọc thẻ RFID:
 *   1. Gửi lệnh REQA (Request command) để phát hiện thẻ trong vùng anten.
 *   2. Thực hiện Anti-collision để lấy UID (Unique ID) 4 byte của thẻ.
 *   3. Gửi lệnh HALT để đưa thẻ về trạng thái nghỉ (hibernation).
 *   4. Trả về UID cho tầng ứng dụng để ánh xạ thông tin người dùng.
 *
 * Giao tiếp SPI:
 *   - SCK  : PH6 / SPI5_SCK   (xung clock đồng bộ)
 *   - MISO : PF8 / SPI5_MISO  (dữ liệu từ MFRC522 về MCU)
 *   - MOSI : PF9 / SPI5_MOSI  (dữ liệu từ MCU sang MFRC522)
 *   - CS   : PF6              (Chip Select, tích cực mức thấp)
 */

#include "tm_stm32f4_mfrc522.h"

/* SPI5 handle - được khởi tạo bởi CubeMX trong main.c */
extern SPI_HandleTypeDef hspi5;

/**
 * @brief  Khởi tạo module MFRC522.
 *
 * Hàm này thực hiện các bước sau:
 *   - Cấu hình chân CS (Chip Select) ở mức HIGH (không chọn chip).
 *   - Gửi lệnh soft-reset tới MFRC522.
 *   - Thiết lập bộ timer nội bộ của MFRC522 (dùng để timeout khi giao tiếp thẻ).
 *   - Cấu hình độ khuếch đại anten RF ở mức 48dB.
 *   - Bật anten RF để sẵn sàng phát hiện thẻ.
 *
 * @note   SPI5 phải được khởi tạo trước khi gọi hàm này (thường bởi HAL_SPI_Init).
 */
void TM_MFRC522_Init(void) {
	TM_MFRC522_InitPins();
	//TM_SPI_Init(MFRC522_SPI, MFRC522_SPI_PINSPACK);

	/* Soft-reset MFRC522 về trạng thái mặc định */
	TM_MFRC522_Reset();

	/*
	 * Cấu hình bộ timer nội bộ của MFRC522:
	 *   - TMode[7:0]     = 0x8D : TPrescaler_Hi=8, TAuto=1
	 *   - TPrescaler[7:0] = 0x3E : TPrescaler = 0x8_3E = 2110
	 *   - TReloadVal      = 30   : Giá trị reload counter
	 *   => Timeout ~ 30 * (2110 + 1) / 13.56 MHz ≈ 4.66 ms
	 */
	TM_MFRC522_WriteRegister(MFRC522_REG_T_MODE, 0x8D);
	TM_MFRC522_WriteRegister(MFRC522_REG_T_PRESCALER, 0x3E);
	TM_MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_L, 30);           
	TM_MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_H, 0);

	/* Cấu hình độ khuếch đại anten: 48dB (0x70) */
	TM_MFRC522_WriteRegister(MFRC522_REG_RF_CFG, 0x70);
	
	/* Bật chế độ tự động điều khiển anten khi truyền */
	TM_MFRC522_WriteRegister(MFRC522_REG_TX_AUTO, 0x40);

	/* Cấu hình chế độ truyền: CRC preset = 0x6363 (chuẩn ISO 14443A) */
	TM_MFRC522_WriteRegister(MFRC522_REG_MODE, 0x3D);

	/* Bật anten RF - sẵn sàng phát hiện thẻ NFC/RFID */
	TM_MFRC522_AntennaOn();
}

/**
 * @brief  Kiểm tra sự hiện diện của thẻ RFID và đọc UID.
 *
 * Quy trình:
 *   1. Gửi lệnh REQA (0x26) - tìm thẻ chưa ở trạng thái HALT.
 *   2. Nếu phát hiện thẻ (MI_OK), thực hiện anti-collision để lấy UID.
 *   3. Gửi lệnh HALT để đưa thẻ về trạng thái nghỉ.
 *
 * @param  id  Con trỏ tới mảng 5 byte để lưu UID của thẻ.
 *              UID chỉ hợp lệ khi hàm trả về MI_OK.
 * @retval MI_OK       Đọc thẻ thành công, UID được lưu vào *id.
 * @retval MI_NOTAGERR Không phát hiện thẻ trong vùng anten.
 * @retval MI_ERR      Lỗi giao tiếp SPI hoặc collision không giải quyết được.
 */
TM_MFRC522_Status_t TM_MFRC522_Check(uint8_t* id) {
	TM_MFRC522_Status_t status;

	/* Bước 1: Gửi Request - tìm thẻ trong vùng anten */
	status = TM_MFRC522_Request(PICC_REQIDL, id);	
	if (status == MI_OK) {
		/* Bước 2: Anti-collision - lấy số serial (UID) 4 byte của thẻ */
		status = TM_MFRC522_Anticoll(id);	
	}

	/* Bước 3: Đưa thẻ vào trạng thái nghỉ (hibernation) */
	TM_MFRC522_Halt();

	return status;
}

/**
 * @brief  So sánh hai mã UID thẻ RFID.
 *
 * Hàm tiện ích dùng để đối chiếu UID vừa đọc được với UID
 * đã lưu trong cơ sở dữ liệu người dùng (flash).
 *
 * @param  CardID     Con trỏ tới UID vừa đọc (5 byte).
 * @param  CompareID  Con trỏ tới UID cần so sánh (5 byte).
 * @retval MI_OK  Hai UID trùng khớp.
 * @retval MI_ERR Hai UID khác nhau.
 */
TM_MFRC522_Status_t TM_MFRC522_Compare(uint8_t* CardID, uint8_t* CompareID) {
	uint8_t i;
	for (i = 0; i < 5; i++) {
		if (CardID[i] != CompareID[i]) {
			return MI_ERR;
		}
	}
	return MI_OK;
}

/**
 * @brief  Khởi tạo chân GPIO cho Chip Select (CS/SDA) của MFRC522.
 *
 * Chân CS được cấu hình là output push-pull, mặc định ở mức HIGH
 * (không chọn chip). Việc cấu hình GPIO đã được CubeMX thực hiện
 * nên phần code cũ sử dụng Standard Peripheral Library đã bị comment.
 *
 * @note   CS = PF6 (tích cực mức thấp - active low).
 */
void TM_MFRC522_InitPins(void) {
//	GPIO_InitTypeDef GPIO_InitStruct;
	//Enable clock
//	RCC_AHB1PeriphClockCmd(MFRC522_CS_RCC, ENABLE);

//	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
//	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
//	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	//CS pin
//	GPIO_InitStruct.GPIO_Pin = MFRC522_CS_PIN;
//	GPIO_Init(MFRC522_CS_PORT, &GPIO_InitStruct);

	/* Đặt CS = HIGH => không chọn chip MFRC522 (chờ giao tiếp) */
	MFRC522_CS_HIGH;
}

/**
 * @brief  Ghi một byte dữ liệu vào thanh ghi của MFRC522 qua SPI.
 *
 * Giao thức ghi thanh ghi MFRC522 qua SPI:
 *   Byte 1: Địa chỉ thanh ghi (bit 7 = 0 cho ghi, bit[6:1] = addr, bit 0 = 0)
 *   Byte 2: Dữ liệu cần ghi
 *
 * @param  addr  Địa chỉ thanh ghi (0x00 - 0x3F).
 * @param  val   Giá trị cần ghi vào thanh ghi.
 */
void TM_MFRC522_WriteRegister(uint8_t addr, uint8_t val) {
	/* Kéo CS xuống LOW để bắt đầu giao tiếp SPI */
	MFRC522_CS_LOW;

	/* Tạo byte địa chỉ: bit7=0 (ghi), bit[6:1]=addr, bit0=0 */
	uint8_t buf = (addr << 1) & 0x7E;
	HAL_StatusTypeDef ret = 0;

	/* Truyền byte địa chỉ */
	ret = HAL_SPI_Transmit(&hspi5, &buf, 1, 10);
	while ((ret = HAL_SPI_GetState(&hspi5)) == HAL_SPI_STATE_BUSY);

	/* Truyền byte dữ liệu */
	HAL_SPI_Transmit(&hspi5, &val, 1, 10);
	while ((ret = HAL_SPI_GetState(&hspi5)) == HAL_SPI_STATE_BUSY);

	/* Kéo CS lên HIGH để kết thúc giao tiếp */
	MFRC522_CS_HIGH;
}

/**
 * @brief  Đọc một byte dữ liệu từ thanh ghi của MFRC522 qua SPI.
 *
 * Giao thức đọc thanh ghi MFRC522 qua SPI:
 *   Byte 1 (TX): Địa chỉ thanh ghi (bit 7 = 1 cho đọc, bit[6:1] = addr, bit 0 = 0)
 *   Byte 2 (RX): Dữ liệu trả về từ MFRC522
 *
 * @param  addr  Địa chỉ thanh ghi cần đọc (0x00 - 0x3F).
 * @retval Giá trị byte đọc được từ thanh ghi.
 */
uint8_t TM_MFRC522_ReadRegister(uint8_t addr) {
	uint8_t val;

	/* Kéo CS xuống LOW để bắt đầu giao tiếp SPI */
	MFRC522_CS_LOW;

	/* Tạo byte địa chỉ: bit7=1 (đọc), bit[6:1]=addr, bit0=0 */
	uint8_t buf = ((addr << 1) & 0x7E) | 0x80;
	HAL_StatusTypeDef ret = 0;

	/* Truyền byte địa chỉ */
	ret = HAL_SPI_Transmit(&hspi5, &buf, 1, 10);
	while ((ret = HAL_SPI_GetState(&hspi5)) == HAL_SPI_STATE_BUSY);

	/* Nhận byte dữ liệu từ MFRC522 */
	ret = HAL_SPI_Receive(&hspi5, &val, 1, 10);
	while ((ret = HAL_SPI_GetState(&hspi5)) == HAL_SPI_STATE_BUSY);

	/* Kéo CS lên HIGH để kết thúc giao tiếp */
	MFRC522_CS_HIGH;

	return val;	
}

/**
 * @brief  Bật (set) các bit được chỉ định trong thanh ghi MFRC522.
 *
 * Thực hiện phép OR giữa giá trị hiện tại của thanh ghi và mask,
 * sau đó ghi lại vào thanh ghi. Các bit không thuộc mask giữ nguyên.
 *
 * @param  reg   Địa chỉ thanh ghi.
 * @param  mask  Mặt nạ bit cần bật (1 = bật, 0 = giữ nguyên).
 */
void TM_MFRC522_SetBitMask(uint8_t reg, uint8_t mask) {
	TM_MFRC522_WriteRegister(reg, TM_MFRC522_ReadRegister(reg) | mask);
}

/**
 * @brief  Xóa (clear) các bit được chỉ định trong thanh ghi MFRC522.
 *
 * Thực hiện phép AND giữa giá trị hiện tại và phần bù của mask,
 * sau đó ghi lại vào thanh ghi. Các bit không thuộc mask giữ nguyên.
 *
 * @param  reg   Địa chỉ thanh ghi.
 * @param  mask  Mặt nạ bit cần xóa (1 = xóa về 0, 0 = giữ nguyên).
 */
void TM_MFRC522_ClearBitMask(uint8_t reg, uint8_t mask){
	TM_MFRC522_WriteRegister(reg, TM_MFRC522_ReadRegister(reg) & (~mask));
} 

/**
 * @brief  Bật anten RF của MFRC522.
 *
 * Kiểm tra thanh ghi TxControlReg, nếu bit Tx1RFEn và Tx2RFEn
 * chưa được bật thì sẽ set chúng lên 1 để kích hoạt anten.
 * Anten cần được bật để module có thể phát sóng 13.56 MHz
 * và giao tiếp với thẻ RFID/NFC.
 */
void TM_MFRC522_AntennaOn(void) {
	uint8_t temp;

	temp = TM_MFRC522_ReadRegister(MFRC522_REG_TX_CONTROL);
	if (!(temp & 0x03)) {
		TM_MFRC522_SetBitMask(MFRC522_REG_TX_CONTROL, 0x03);
	}
}

/**
 * @brief  Tắt anten RF của MFRC522.
 *
 * Xóa bit Tx1RFEn và Tx2RFEn trong TxControlReg để ngắt
 * tín hiệu RF. Dùng khi cần tiết kiệm năng lượng hoặc
 * tạm dừng quá trình quét thẻ.
 */
void TM_MFRC522_AntennaOff(void) {
	TM_MFRC522_ClearBitMask(MFRC522_REG_TX_CONTROL, 0x03);
}

/**
 * @brief  Soft-reset module MFRC522.
 *
 * Ghi lệnh PCD_RESETPHASE (0x0F) vào thanh ghi Command để
 * khởi động lại MFRC522 về trạng thái mặc định. Tất cả các
 * thanh ghi sẽ được reset về giá trị ban đầu.
 */
void TM_MFRC522_Reset(void) {
	TM_MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_RESETPHASE);
}

/**
 * @brief  Gửi lệnh Request (REQA/WUPA) tới thẻ RFID.
 *
 * Lệnh này phát tín hiệu RF để phát hiện thẻ trong vùng anten.
 *   - PICC_REQIDL (0x26): Tìm thẻ chưa vào trạng thái HALT
 *   - PICC_REQALL (0x52): Tìm tất cả thẻ trong vùng anten
 *
 * @param  reqMode  Chế độ request: PICC_REQIDL hoặc PICC_REQALL.
 * @param  TagType  Con trỏ tới mảng lưu loại thẻ (ATQA - 2 byte).
 * @retval MI_OK nếu phát hiện thẻ, MI_ERR nếu không có thẻ.
 */
TM_MFRC522_Status_t TM_MFRC522_Request(uint8_t reqMode, uint8_t* TagType) {
	TM_MFRC522_Status_t status;  
	uint16_t backBits;			//The received data bits

	/* Cấu hình BitFramingReg: chỉ truyền 7 bit (short frame theo ISO 14443A) */
	TM_MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x07);

	TagType[0] = reqMode;

	/* Gửi lệnh request qua RF và chờ phản hồi ATQA từ thẻ */
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

	/* ATQA hợp lệ phải có đúng 16 bit (2 byte) */
	if ((status != MI_OK) || (backBits != 0x10)) {    
		status = MI_ERR;
	}

	return status;
}

/**
 * @brief  Truyền và nhận dữ liệu với thẻ RFID qua MFRC522.
 *
 * Đây là hàm giao tiếp mức thấp nhất với thẻ. Nó thực hiện:
 *   1. Cấu hình ngắt và FIFO buffer.
 *   2. Nạp dữ liệu cần gửi vào FIFO.
 *   3. Thực thi lệnh (TRANSCEIVE hoặc AUTHENT).
 *   4. Chờ phản hồi từ thẻ (polling trên CommIrqReg).
 *   5. Đọc dữ liệu trả về từ FIFO.
 *
 * @param  command   Lệnh PCD: PCD_TRANSCEIVE hoặc PCD_AUTHENT.
 * @param  sendData  Con trỏ tới dữ liệu cần truyền.
 * @param  sendLen   Số byte cần truyền.
 * @param  backData  Con trỏ tới buffer nhận dữ liệu trả về.
 * @param  backLen   Con trỏ lưu số bit dữ liệu nhận được.
 * @retval MI_OK nếu giao tiếp thành công.
 */
TM_MFRC522_Status_t TM_MFRC522_ToCard(uint8_t command, uint8_t* sendData, uint8_t sendLen, uint8_t* backData, uint16_t* backLen) {
	TM_MFRC522_Status_t status = MI_ERR;
	uint8_t irqEn = 0x00;
	uint8_t waitIRq = 0x00;
	uint8_t lastBits;
	uint8_t n;
	uint16_t i;

	/* Xác định cờ ngắt cần bật tùy theo loại lệnh */
	switch (command) {
		case PCD_AUTHENT: {
			irqEn = 0x12;
			waitIRq = 0x10;
			break;
		}
		case PCD_TRANSCEIVE: {
			irqEn = 0x77;
			waitIRq = 0x30;
			break;
		}
		default:
			break;
	}

	/* Bật ngắt tương ứng và xóa cờ ngắt cũ */
	TM_MFRC522_WriteRegister(MFRC522_REG_COMM_IE_N, irqEn | 0x80);
	TM_MFRC522_ClearBitMask(MFRC522_REG_COMM_IRQ, 0x80);

	/* Flush FIFO buffer */
	TM_MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);

	/* Đặt MFRC522 về trạng thái IDLE trước khi thực thi lệnh mới */
	TM_MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_IDLE);

	/* Nạp dữ liệu cần truyền vào FIFO buffer */
	for (i = 0; i < sendLen; i++) {   
		TM_MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, sendData[i]);    
	}

	/* Thực thi lệnh (TRANSCEIVE hoặc AUTHENT) */
	TM_MFRC522_WriteRegister(MFRC522_REG_COMMAND, command);
	if (command == PCD_TRANSCEIVE) {    
		TM_MFRC522_SetBitMask(MFRC522_REG_BIT_FRAMING, 0x80);		//StartSend=1,transmission of data starts  
	}   

	/*
	 * Polling chờ phản hồi từ thẻ.
	 * Timeout tối đa ~25ms (i=2000 vòng lặp).
	 * Điều kiện thoát: timeout (i==0), timer interrupt, hoặc nhận được dữ liệu.
	 */
	i = 2000;
	do {
		//CommIrqReg[7..0]
		//Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
		n = TM_MFRC522_ReadRegister(MFRC522_REG_COMM_IRQ);
		i--;
	} while ((i!=0) && !(n&0x01) && !(n&waitIRq));

	/* Dừng quá trình truyền (StartSend = 0) */
	TM_MFRC522_ClearBitMask(MFRC522_REG_BIT_FRAMING, 0x80);

	if (i != 0)  {
		/* Kiểm tra thanh ghi lỗi: BufferOvfl, CollErr, ParityErr, ProtocolErr */
		if (!(TM_MFRC522_ReadRegister(MFRC522_REG_ERROR) & 0x1B)) {
			status = MI_OK;
			if (n & irqEn & 0x01) {   
				status = MI_NOTAGERR;			
			}

			if (command == PCD_TRANSCEIVE) {
				/* Đọc số byte có trong FIFO */
				n = TM_MFRC522_ReadRegister(MFRC522_REG_FIFO_LEVEL);

				/* Tính số bit dữ liệu nhận được */
				lastBits = TM_MFRC522_ReadRegister(MFRC522_REG_CONTROL) & 0x07;
				if (lastBits) {   
					*backLen = (n - 1) * 8 + lastBits;   
				} else {   
					*backLen = n * 8;   
				}

				if (n == 0) {   
					n = 1;    
				}
				if (n > MFRC522_MAX_LEN) {   
					n = MFRC522_MAX_LEN;   
				}

				/* Đọc toàn bộ dữ liệu nhận được từ FIFO */
				for (i = 0; i < n; i++) {   
					backData[i] = TM_MFRC522_ReadRegister(MFRC522_REG_FIFO_DATA);    
				}
			}
		} else {   
			status = MI_ERR;  
		}
	}

	return status;
}

/**
 * @brief  Thực hiện Anti-collision và lấy UID của thẻ.
 *
 * Quy trình Anti-collision (theo ISO 14443-3):
 *   1. Gửi lệnh ANTICOLL (0x93) + NVB (0x20 = 2 byte đã gửi).
 *   2. Thẻ trả về UID 4 byte + BCC (Block Check Character).
 *   3. Kiểm tra BCC = XOR của 4 byte UID.
 *
 * @param  serNum  Con trỏ tới mảng >= 5 byte để lưu UID + BCC.
 * @retval MI_OK nếu thành công, MI_ERR nếu checksum sai.
 */
TM_MFRC522_Status_t TM_MFRC522_Anticoll(uint8_t* serNum) {
	TM_MFRC522_Status_t status;
	uint8_t i;
	uint8_t serNumCheck = 0;
	uint16_t unLen;

	/* Cấu hình BitFramingReg: truyền đầy đủ byte (0x00) */
	TM_MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x00);

	/* Gửi lệnh Anti-collision: command code + NVB */
	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20;
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

	if (status == MI_OK) {
		/* Kiểm tra BCC (XOR checksum) của 4 byte UID */
		for (i = 0; i < 4; i++) {   
			serNumCheck ^= serNum[i];
		}
		if (serNumCheck != serNum[i]) {   
			status = MI_ERR;    
		}
	}
	return status;
} 

/**
 * @brief  Tính CRC-A sử dụng bộ CRC nội bộ của MFRC522.
 *
 * MFRC522 có bộ tính CRC phần cứng tích hợp, giúp tính CRC
 * theo chuẩn ISO 14443A mà không cần tính bằng phần mềm.
 *
 * @param  pIndata   Con trỏ tới dữ liệu cần tính CRC.
 * @param  len       Số byte dữ liệu.
 * @param  pOutData  Con trỏ tới mảng 2 byte để lưu kết quả CRC.
 */
void TM_MFRC522_CalculateCRC(uint8_t*  pIndata, uint8_t len, uint8_t* pOutData) {
	uint8_t i, n;

	/* Xóa cờ CRCIrq và flush FIFO */
	TM_MFRC522_ClearBitMask(MFRC522_REG_DIV_IRQ, 0x04);
	TM_MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);

	/* Nạp dữ liệu vào FIFO để tính CRC */
	for (i = 0; i < len; i++) {   
		TM_MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, *(pIndata+i));   
	}

	/* Bắt đầu tính CRC */
	TM_MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_CALCCRC);

	/* Chờ cho đến khi CRC tính xong (CRCIrq = 1) */
	i = 0xFF;
	do {
		n = TM_MFRC522_ReadRegister(MFRC522_REG_DIV_IRQ);
		i--;
	} while ((i!=0) && !(n&0x04));

	/* Đọc kết quả CRC 16-bit (little-endian) */
	pOutData[0] = TM_MFRC522_ReadRegister(MFRC522_REG_CRC_RESULT_L);
	pOutData[1] = TM_MFRC522_ReadRegister(MFRC522_REG_CRC_RESULT_M);
}

/**
 * @brief  Chọn thẻ (SELECT) để bắt đầu giao tiếp dữ liệu.
 *
 * Sau khi lấy được UID qua Anti-collision, cần gửi lệnh SELECT
 * để chọn thẻ cụ thể. Thẻ sẽ trả về SAK (Select Acknowledge)
 * chứa thông tin về loại thẻ (Mifare Classic, Ultralight, v.v.)
 *
 * @param  serNum  Con trỏ tới UID 5 byte (4 byte UID + 1 byte BCC).
 * @retval Kích thước bộ nhớ của thẻ (SAK), hoặc 0 nếu lỗi.
 */
uint8_t TM_MFRC522_SelectTag(uint8_t* serNum) {
	uint8_t i;
	TM_MFRC522_Status_t status;
	uint8_t size;
	uint16_t recvBits;
	uint8_t buffer[9]; 

	/* Chuẩn bị lệnh SELECT: command + NVB + UID + CRC */
	buffer[0] = PICC_SElECTTAG;
	buffer[1] = 0x70;
	for (i = 0; i < 5; i++) {
		buffer[i+2] = *(serNum+i);
	}

	/* Tính và gắn CRC vào cuối frame */
	TM_MFRC522_CalculateCRC(buffer, 7, &buffer[7]);
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

	if ((status == MI_OK) && (recvBits == 0x18)) {   
		size = buffer[0]; 
	} else {   
		size = 0;    
	}

	return size;
}

/**
 * @brief  Xác thực (authenticate) để truy cập block dữ liệu trên thẻ Mifare.
 *
 * Trước khi đọc/ghi block dữ liệu trên thẻ Mifare Classic,
 * cần phải xác thực bằng Key A hoặc Key B của sector tương ứng.
 *
 * @param  authMode    Chế độ xác thực: PICC_AUTHENT1A (Key A) hoặc PICC_AUTHENT1B (Key B).
 * @param  BlockAddr   Địa chỉ block cần truy cập (0-63 cho thẻ 1K).
 * @param  Sectorkey   Con trỏ tới mảng 6 byte chứa key xác thực.
 * @param  serNum      Con trỏ tới UID 4 byte của thẻ.
 * @retval MI_OK nếu xác thực thành công, MI_ERR nếu thất bại.
 */
TM_MFRC522_Status_t TM_MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t* Sectorkey, uint8_t* serNum) {
	TM_MFRC522_Status_t status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[12]; 

	/* Chuẩn bị frame xác thực: AuthMode + BlockAddr + Key(6) + UID(4) */
	buff[0] = authMode;
	buff[1] = BlockAddr;
	for (i = 0; i < 6; i++) {    
		buff[i+2] = *(Sectorkey+i);   
	}
	for (i=0; i<4; i++) {    
		buff[i+8] = *(serNum+i);   
	}

	/* Gửi lệnh xác thực và kiểm tra kết quả */
	status = TM_MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);

	/* Kiểm tra bit MFCrypto1On trong Status2Reg */
	if ((status != MI_OK) || (!(TM_MFRC522_ReadRegister(MFRC522_REG_STATUS2) & 0x08))) {   
		status = MI_ERR;   
	}

	return status;
}

/**
 * @brief  Đọc dữ liệu từ một block trên thẻ Mifare (16 byte).
 *
 * @param  blockAddr  Địa chỉ block cần đọc.
 * @param  recvData   Con trỏ tới buffer >= 16 byte để lưu dữ liệu đọc được.
 * @retval MI_OK nếu đọc thành công, MI_ERR nếu có lỗi.
 *
 * @note   Cần gọi TM_MFRC522_Auth() trước khi đọc block.
 */
TM_MFRC522_Status_t TM_MFRC522_Read(uint8_t blockAddr, uint8_t* recvData) {
	TM_MFRC522_Status_t status;
	uint16_t unLen;

	/* Chuẩn bị lệnh đọc: command + block address + CRC */
	recvData[0] = PICC_READ;
	recvData[1] = blockAddr;
	TM_MFRC522_CalculateCRC(recvData,2, &recvData[2]);

	/* Gửi lệnh đọc và nhận dữ liệu 16 byte (128 bit + CRC = 0x90 bit) */
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);

	if ((status != MI_OK) || (unLen != 0x90)) {
		status = MI_ERR;
	}

	return status;
}

/**
 * @brief  Ghi dữ liệu vào một block trên thẻ Mifare (16 byte).
 *
 * Quy trình ghi theo Mifare Classic protocol:
 *   1. Gửi lệnh WRITE + block address => thẻ trả ACK (4 bit).
 *   2. Gửi 16 byte dữ liệu + CRC => thẻ trả ACK.
 *
 * @param  blockAddr  Địa chỉ block cần ghi (tránh block 0 và trailer blocks).
 * @param  writeData  Con trỏ tới mảng 16 byte dữ liệu cần ghi.
 * @retval MI_OK nếu ghi thành công, MI_ERR nếu có lỗi.
 *
 * @note   Cần gọi TM_MFRC522_Auth() trước khi ghi block.
 * @warning Không ghi vào block 0 (manufacturer block) hoặc sector trailer block.
 */
TM_MFRC522_Status_t TM_MFRC522_Write(uint8_t blockAddr, uint8_t* writeData) {
	TM_MFRC522_Status_t status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[18]; 

	/* Bước 1: Gửi lệnh WRITE + block address */
	buff[0] = PICC_WRITE;
	buff[1] = blockAddr;
	TM_MFRC522_CalculateCRC(buff, 2, &buff[2]);
	status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

	/* Kiểm tra ACK từ thẻ (4 bit, giá trị 0x0A) */
	if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) {   
		status = MI_ERR;   
	}

	if (status == MI_OK) {
		/* Bước 2: Gửi 16 byte dữ liệu + CRC */
		for (i = 0; i < 16; i++) {    
			buff[i] = *(writeData+i);   
		}
		TM_MFRC522_CalculateCRC(buff, 16, &buff[16]);
		status = TM_MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);

		/* Kiểm tra ACK lần 2 */
		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) {   
			status = MI_ERR;   
		}
	}

	return status;
}

/**
 * @brief  Đưa thẻ RFID vào trạng thái HALT (nghỉ).
 *
 * Sau khi giao tiếp xong, gửi lệnh HALT để thẻ ngừng phản hồi.
 * Thẻ sẽ chỉ được đánh thức lại bằng lệnh WUPA (PICC_REQALL).
 */
void TM_MFRC522_Halt(void) {
	uint16_t unLen;
	uint8_t buff[4]; 

	/* Chuẩn bị lệnh HALT: command + 0x00 + CRC */
	buff[0] = PICC_HALT;
	buff[1] = 0;
	TM_MFRC522_CalculateCRC(buff, 2, &buff[2]);

	/* Gửi lệnh HALT (không cần kiểm tra response) */
	TM_MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}
