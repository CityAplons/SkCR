/*
 * ImuSensor.cpp
 *
 *  Created on: 10 ���. 2019 �.
 *      Author: Taras.Melnik
 */

#include "ImuSensor.h"

ImuSensor* imuSensor;

ImuSensor::ImuSensor() {
	for (uint8_t i = 0; i < 12; i++)
		tByteArr[i] = 0;
}

ImuSensor::~ImuSensor() {
	// TODO Auto-generated destructor stub
}

void ImuSensor::i2cInit()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	GPIO_InitTypeDef i2cImuPin;
	i2cImuPin.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	i2cImuPin.GPIO_Mode = GPIO_Mode_AF;
	i2cImuPin.GPIO_OType = GPIO_OType_OD;
	i2cImuPin.GPIO_Speed = GPIO_Speed_100MHz;
	i2cImuPin.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &i2cImuPin);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);

	I2C_InitTypeDef i2cImu;
	i2cImu.I2C_Mode = I2C_Mode_I2C;
	i2cImu.I2C_ClockSpeed =  88000;
	i2cImu.I2C_Ack = I2C_Ack_Enable;
	i2cImu.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	i2cImu.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_Init(I2C1, &i2cImu);
//	I2C_ITConfig(I2C1, I2C_IT_EVT, ENABLE);
	I2C_Cmd(I2C1, ENABLE);

//	DMA_InitTypeDef dmaRxI2c;
//	NVIC_InitTypeDef dmaRxI2cNvic;
//	DMA_StructInit(&dmaRxI2c);
//	dmaRxI2c.DMA_PeripheralBaseAddr = (uint32_t)&(I2C1->DR);
//	dmaRxI2c.DMA_Memory0BaseAddr = (uint32_t) &(this->i2cRxArr[0]);
//	dmaRxI2c.DMA_Channel = DMA_Channel_1;
//	dmaRxI2c.DMA_DIR = DMA_DIR_PeripheralToMemory;
//	dmaRxI2c.DMA_BufferSize = 20;
//	dmaRxI2c.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//	dmaRxI2c.DMA_MemoryInc = DMA_MemoryInc_Enable;
//	dmaRxI2c.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
//	dmaRxI2c.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
//	dmaRxI2c.DMA_Mode = DMA_Mode_Normal;
//	DMA_Init(DMA1_Stream5, &dmaRxI2c);
//	DMA_Cmd(DMA1_Stream5, ENABLE);
//
//	DMA_ITConfig(DMA1_Stream5, DMA_IT_TC, ENABLE);
//	dmaRxI2cNvic.NVIC_IRQChannel = I2C1_EV_IRQn;
//	dmaRxI2cNvic.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&dmaRxI2cNvic);
//	NVIC_SetPriority(I2C1_EV_IRQn, 10);

}

void ImuSensor::getOdometry(uint8_t* byteArr)
{
	for (uint8_t i = 0; i < 12; i++)
		 byteArr[i] = this->tByteArr[i];
}

void ImuSensor::getAcceleration(uint8_t* byteArr)
{
	for (uint8_t i = 0; i < 24; i++)
		 byteArr[i] = this->accArr[i];
}

void ImuSensor::getAngularVel(uint8_t* byteArr)
{
	for (uint8_t i = 0; i < 24; i++)
		 byteArr[i] = this->gyroArr[i];
}

void ImuSensor::i2cReset()
{
	I2C_Cmd(I2C1, DISABLE);

//	I2C_DeInit(I2C2);

	GPIO_InitTypeDef i2cImuPin;
	i2cImuPin.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	i2cImuPin.GPIO_Mode = GPIO_Mode_OUT;
	i2cImuPin.GPIO_OType = GPIO_OType_OD;
	i2cImuPin.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &i2cImuPin);

	GPIO_SetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7);
	vTaskDelay(1);

	GPIO_ResetBits(GPIOB, GPIO_Pin_7);
	vTaskDelay(1);

	GPIO_ResetBits(GPIOB, GPIO_Pin_6);
	vTaskDelay(1);

	GPIO_SetBits(GPIOB, GPIO_Pin_7);
	vTaskDelay(1);

	GPIO_SetBits(GPIOB, GPIO_Pin_6);
	vTaskDelay(1);

	I2C_SoftwareResetCmd(I2C1, ENABLE);
	vTaskDelay(1);
	I2C_SoftwareResetCmd(I2C1, DISABLE);

	i2cInit();
}

void ImuSensor::i2cRead(uint8_t slaveAdr, uint8_t subRegAdr, uint8_t regNumb, uint8_t* data)
{
	 uint8_t i = 0;

	 while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	 I2C_GenerateSTART(I2C1, ENABLE);
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	 I2C_Send7bitAddress(I2C1, slaveAdr, I2C_Direction_Transmitter);
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
//	 I2C_Cmd(I2C1, ENABLE);
	 I2C_SendData(I2C1, subRegAdr);
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	 I2C_AcknowledgeConfig(I2C1, ENABLE);
	 I2C_GenerateSTART(I2C1, ENABLE);
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	 I2C_Send7bitAddress(I2C1, slaveAdr, I2C_Direction_Receiver);
	 for (i = 0; i < regNumb - 1; i++) {
		 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
		 data[i] = I2C_ReceiveData(I2C1);
	 }
	 I2C_AcknowledgeConfig(I2C1, DISABLE);
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
	 data[i++] = I2C_ReceiveData(I2C1);
	 I2C_GenerateSTOP(I2C1, ENABLE);
}

void ImuSensor::i2cWrite(uint8_t slaveAdr, uint8_t subRegAdr, uint8_t data)
{
	 while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	 I2C_GenerateSTART(I2C1, ENABLE);
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	 I2C_Send7bitAddress(I2C1, slaveAdr, I2C_Direction_Transmitter);
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	 I2C_Cmd(I2C1, ENABLE);
	 I2C_SendData(I2C1, subRegAdr);
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	 I2C_SendData(I2C1, data);
	 while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	 I2C_GenerateSTOP(I2C1, ENABLE);
}

void ImuSensor::accelInit()
{
	this->i2cWrite(ACCELEROMETER_ADDR, ACCELEROMETER_CTRL_REG1_ADDR, ACCELEROMETER_CTRL_REG1_MASK);
}

void ImuSensor::gyroInit()
{
	this->i2cWrite(GYROSCOPE_ADDR, GYROSCOPE_CTRL_REG1_ADDR, GYROSCOPE_CTRL_REG1_MASK);
}

void ImuSensor::magnetInit()
{
	this->i2cWrite(MAGNETOMETER_ADDR, MAGNETOMETER_CTRL_REG1_ADDR, MAGNETOMETER_CTRL_REG1_MASK);
	this->i2cWrite(MAGNETOMETER_ADDR, MAGNETOMETER_CTRL_REG2_ADDR, MAGNETOMETER_CTRL_REG2_MASK);
	this->i2cWrite(MAGNETOMETER_ADDR, MAGNETOMETER_CTRL_REG3_ADDR, MAGNETOMETER_CTRL_REG3_MASK);
}

void ImuSensor::accelAdjustment(int16_t* aConst)
{
	int32_t* accArr[3];
	int16_t adjArr[3] = {0};
	for (uint8_t i = 0; i < 3; i++) {
		accArr[i] = new int32_t[20];
		for (uint8_t j = 0; j < 20; j++)
			accArr[i][j] = 0;
	}
	for (uint8_t j = 1; j < 20; j++) {
		this->i2cRead(ACCELEROMETER_ADDR, ACCELEROMETER_OUT_X_L | 1 << 7, 6, i2cRxArr);
		memcpy(adjArr, i2cRxArr, sizeof(adjArr));
		for (uint8_t i = 0; i < 3; i++)
			accArr[i][j] = accArr[i][j - 1] + adjArr[i];
		vTaskDelay(oRTOS.fromMsToTick(10));
	}
	for (uint8_t i = 0; i < 3; i++)
		aConst[i] = (int16_t) (accArr[i][19] / 19);
}

void ImuSensor::gyroAdjustment(int16_t* aConst)
{
	int32_t* accArr[3];
	int16_t adjArr[3] = {0};
	for (uint8_t i = 0; i < 3; i++) {
		accArr[i] = new int32_t[20];
		for (uint8_t j = 0; j < 20; j++)
			accArr[i][j] = 0;
	}
	for (uint8_t j = 1; j < 20; j++) {
		this->i2cRead(GYROSCOPE_ADDR, GYROSCOPE_OUT_X_L | 1 << 7, 6, i2cRxArr);
		memcpy(adjArr, i2cRxArr, sizeof(adjArr));
		for (uint8_t i = 0; i < 3; i++)
			accArr[i][j] = accArr[i][j - 1] + adjArr[i];
		vTaskDelay(oRTOS.fromMsToTick(10));
	}
	for (uint8_t i = 0; i < 3; i++)
		aConst[i] = (int16_t) (accArr[i][19] / 19);
}



void ImuSensor::calculateXYTheta(float32_t* vArr, float32_t* xyalfArr)
{
	static float32_t deltAlf = 0;
	static float32_t deltDist = 0;
	static float32_t cBuff = 2 * PI;

	deltDist = vArr[0] * 0.01f;
	deltAlf = vArr[1];
																// fix an incorrect answer during shutdown
		xyalfArr[2] += deltAlf;
		if (xyalfArr[2] >= cBuff){								//lead to the 2pi range (for ARM tables)
			xyalfArr[2] -= cBuff;
		} else if (xyalfArr[2] <= -cBuff)
			xyalfArr[2] += cBuff;
		xyalfArr[1] += deltDist * arm_sin_f32(xyalfArr[2]);
		xyalfArr[0] += deltDist * arm_cos_f32(xyalfArr[2]);
}

float32_t ImuSensor::runningMean(float32_t* arr)
{
	float32_t val = 0;
	uint8_t M =  sizeof(arr);
	for (uint8_t i = 0; i < M; i++)
		val += arr[i] / M;
	return
			val;
}

void complFiltr(float32_t &thet, float32_t gyr, float32_t mag)
{
	static float32_t K = FILTER_K_COFFICIENT;
	thet = (1 - K) * (thet + gyr) + K * mag;
}

void ImuSensor::run() {
	float32_t velArr[2] = {0};
	float32_t xyThetArr[3] = {0};
	float32_t constAcc = 2.f * 9.8f / 32767.f;
	float32_t constGyro = 0.00875f / 57.3f;
	int16_t testArr16[3] = {0};
	float64_t testArr64[3] = {0};
	int16_t cArr[3] = {0};
	int16_t wArr[3] = {0};

	this->i2cReset();
	this->accelInit();
	this->gyroInit();
	this->magnetInit();
	this->accelAdjustment(cArr);
	this->gyroAdjustment(wArr);

	float32_t cBuff = 2 * PI;
	TickType_t xLastWakeTime;

	 while (1) {
		 xLastWakeTime = xTaskGetTickCount();
		 int16_t bufI2c = 0;
		 this->i2cRead(ACCELEROMETER_ADDR, ACCELEROMETER_OUT_Y_L | 1 << 7, 2, i2cRxArr);
		 memcpy(&bufI2c, i2cRxArr, sizeof(bufI2c));
		 velArr[0] += (bufI2c - cArr[1]) * constAcc * 0.01;
		 this->i2cRead(GYROSCOPE_ADDR, GYROSCOPE_OUT_Z_L | 1 << 7, 2, i2cRxArr);
		 memcpy(&bufI2c, i2cRxArr, sizeof(bufI2c));
		 velArr[1] = (bufI2c - wArr[2]) * constGyro * 0.01f;
		 calculateXYTheta(velArr, xyThetArr);
		 memcpy(tByteArr, xyThetArr, sizeof(tByteArr));

		 this->i2cRead(ACCELEROMETER_ADDR, ACCELEROMETER_OUT_X_L | 1 << 7, 6, i2cRxArr);
		 memcpy(testArr16, i2cRxArr, sizeof(testArr16));
		 for (uint8_t i = 0; i < 3; i++)
			 testArr64[i] = (float64_t) testArr16[i] * constAcc;
		 memcpy(accArr, testArr64, sizeof(accArr));

		 this->i2cRead(GYROSCOPE_ADDR, GYROSCOPE_OUT_X_L | 1 << 7, 6, i2cRxArr);
		 memcpy(testArr16, i2cRxArr, sizeof(testArr16));
		 for (uint8_t i = 0; i < 3; i++)
			 testArr64[i] = (float64_t) testArr16[i] * constGyro;
		 memcpy(gyroArr, testArr64, sizeof(gyroArr));

//		 // Test for angularVelocities
//		 this->i2cRead(GYROSCOPE_ADDR, GYROSCOPE_OUT_X_L | 1 << 7, 6, i2cRxArr);
//		 memcpy(testArr, i2cRxArr, sizeof(testArr));
//		 for (uint8_t i = 0; i < 3; i++)
//			 xyThetArr[i] += (testArr[i] - wArr[i]) * constGyro * 0.01f;
//		 memcpy(tByteArr, xyThetArr, sizeof(tByteArr));
//		 vTaskDelay(oRTOS.fromMsToTick(10));

//		 // Test for accelerations
//		 int16_t bufI2c = 0;
//		 this->i2cRead(ACCELEROMETER_ADDR, ACCELEROMETER_OUT_X_L | 1 << 7, 6, i2cRxArr);
//		 memcpy(testArr, i2cRxArr, sizeof(testArr));
//		 for (uint8_t i = 0; i < 3; i++)
//			 xyThetArr[i] += (testArr[i] - cArr[i]) * constAcc* 0.01f;
//		 memcpy(tByteArr, xyThetArr, sizeof(tByteArr));
//		 vTaskDelay(oRTOS.fromMsToTick(10));

		 taskDelayUntil(&xLastWakeTime, oRTOS.fromMsToTick(10));
	 }
}

//extern "C" {
//
//void I2C1_IRQHandler(void)
//	{
//		BaseType_t xHigherPriorityTaskWoken = pdFALSE;			// Notify task about interrupt
//		if (USART_GetITStatus(USART3, USART_IT_IDLE)) {			// Clear IDLE flag step 1
//			DMA_Cmd(DMA1_Stream1, DISABLE);						// DMA turn off to clear DMA1 counter
//			USART_ReceiveData(USART3);							// Clear IDLE flag step 2
//		}
//		if (USART_GetITStatus(USART3, USART_IT_TC)) {
//			USART_ClearITPendingBit(USART3, USART_IT_TC);
//		}
//		if (xHigherPriorityTaskWoken)
//			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);		// Run Higher priority task if exist													// between ticks
//	}
////***********************DMA_RECEIVE_INTERRUPT******************//
//	void DMA1_Stream5_IRQHandler(void)
//	{
//		BaseType_t xHigherPriorityTaskWoken = pdFALSE;			// Notify task about interrupt
//		DMA_ClearITPendingBit(DMA1_Stream1, DMA_IT_TCIF1);		// Clear DMA "transmitting complete" interrupt
//		DMA_Cmd(DMA1_Stream1, ENABLE);							// Reset DMA
//		if (xHigherPriorityTaskWoken)
//			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);		// Run Higher priority task if exist													// between ticks
//	}
//
//}






