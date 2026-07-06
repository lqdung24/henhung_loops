//
//
//
//void MQ3_Reset_DMA_Buffer(void) {
//    // 1. Dừng bộ quét DMA hiện tại
//    HAL_ADC_Stop_DMA(&hadc1);
//
//    // 2. Xóa trắng dữ liệu cũ trong mảng về 0
//    memset(ADC_Buffer, 0, sizeof(ADC_Buffer));
//}
//
//void MQ3_Start_DMA(void){
//	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_Buffer, ADC_BUF_LEN);
//}
