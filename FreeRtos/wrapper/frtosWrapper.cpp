/*******************************************************************************
 *  FILENAME: frtosWrapper.cpp
 *
 *  DESCRIPTION: ���������� ������ frtosWrapper
 *
 *  Copyright (c) 2015 by Sergey Sikharulidze and Sergey Kolody
 *
 *******************************************************************************/

#include "frtosWrapper.h" 

iActiveObject* cRTOS::tasks[30] = { 0 };
uint8_t cRTOS::tasksCount = 0;

/*******************************************************************************
 * Function:  constructor
 * Description:
 * Threading usage and Assumptions:  none
 ******************************************************************************/
cRTOS::cRTOS(void) {

}

/*******************************************************************************
 * Function:  startScheduler
 * Description: ������ ������������
 * Threading usage and Assumptions:  ��� ������ ������ ���� ������� � �������
 *  taskCreate() �� ������ ������ �������
 ******************************************************************************/
void cRTOS::startScheduler(void) const {
	vTaskStartScheduler();
}

/*******************************************************************************
 * Function:  taskCreate
 * Description: �������� ������, � �������� ������ ����� ��������������
 * ����������� ����� run ������ cRTOS, � ������� ����� ���������� �������� �����
 * ������� ���������� iActiveObject
 * Threading usage and Assumptions:  none
 ******************************************************************************/
//tRtosStatus cRTOS::taskCreate(iActiveObject *pActiveObject,
//		const tU16 stackDepth, tU32 priority, const char * const name) const {
//	tRtosStatus status;
//	const BaseType_t rowStatus = xTaskCreate((TaskFunction_t)cRTOS::run, name,
//			stackDepth, pActiveObject, priority,
//			&pActiveObject->taskHandle); //lint !e929 ��� tTaskFunction ������������ ��� ������������� � ��� ����������
//
//	if (rowStatus == pdTRUE) {
//		status = RS_pass;
//	} else {
//		status = RS_fail;
//	}
//	return status;
//} //lint !e1762 !e952
//1762 - ������� ��������� �� �������� const.
//952 - �������� taskCode ��������� �� �������� const
/*******************************************************************************
 * Function:  taskDelay
 * Description: ������������ ������
 * Threading usage and Assumptions:  none
 ******************************************************************************/
//void cRTOS::taskDelay(const tU32 timeIncrement) const {
//	vTaskDelay((TickType_t) timeIncrement);
//}
/*******************************************************************************
 * Function:  schedulerDisable
 * Description: ��������� ������������
 * Threading usage and Assumptions:  none
 ******************************************************************************/
void cRTOS::schedulerDisable(void) const {
	vTaskSuspendAll();
}

/*******************************************************************************
 * Function:  schedulerEnable
 * Description: �������������� ������ ������������
 * Threading usage and Assumptions:  none
 ******************************************************************************/
tRtosStatus cRTOS::schedulerEnable(void) const {
	tRtosStatus status;
	const BaseType_t rowStatus = xTaskResumeAll();

	if (rowStatus == pdTRUE) {
		status = RS_pass;
	} else {
		status = RS_fail;
	}
	return status;
}

/*******************************************************************************
 * Function:  eventGroupCreate
 * Description: �������� event group
 * Threading usage and Assumptions:  none
 ******************************************************************************/
tEventGroupHandle cRTOS::eventGroupCreate(void) const {
	tEventGroupHandle createdEventGroup;

	createdEventGroup = (tEventGroupHandle) xEventGroupCreate(); //lint !e925 ��� tEventGroupHandle ������������ ��� ������������� � ��� ����������

	return createdEventGroup;
} //lint !e1762 ������� ��������� �� �������� const.

/*******************************************************************************
 * Function:  eventGroupSetBits
 * Description: ������������� ���� � event group
 * Threading usage and Assumptions:  none
 ******************************************************************************/
tU32 cRTOS::eventGroupSetBits(tEventGroupHandle eventGroup,
		const tU32 bitsToSet) const {
	tU32 bits;

	bits = (tU32) xEventGroupSetBits((EventGroupHandle_t) eventGroup,
			(EventBits_t) bitsToSet); //lint !e925 ��� tEventGroupHandle ������������ ��� ������������� � ��� ����������

	/* From FreeRTOS documentation:
	 There are two reasons why the returned value might have the bits specified
	 by the uxBitsToSet parameter cleared:
	 1. If setting a bit results in a task that was waiting for the bit leaving
	 the	blocked state then it is possible the bit will have been cleared
	 automatically (see the xClearBitOnExit parameter of xEventGroupWaitBits()).
	 2. Any unblocked (or otherwise Ready state) task that has a priority above
	 that of the task that called xEventGroupSetBits() will execute and may
	 change the event group value before the call to xEventGroupSetBits()
	 returns. */
	return bits;
} //lint !e1762 ������� ��������� �� �������� const.

/*******************************************************************************
 * Function:  eventGroupSetBitsFromISR
 * Description: ������ eventGroupSetBits, ���������� �� ����������� ����������
 * Threading usage and Assumptions:  none
 ******************************************************************************/
tRtosStatus cRTOS::eventGroupSetBitsFromISR(tEventGroupHandle eventGroup,
		const tU32 bitsToSet) const {
	tRtosStatus status;
	BaseType_t result;
	BaseType_t xHigherPriorityTaskWoken;

	xHigherPriorityTaskWoken = (BaseType_t) FALSE;

	result = xEventGroupSetBitsFromISR((EventGroupHandle_t) eventGroup,
			(EventBits_t) bitsToSet, &xHigherPriorityTaskWoken); //lint !e925 !e632
																 // 1. - ��� tEventGroupHandle ������������ ��� ������������� � ��� ����������
																 // 2. - ������� xTimerPendFunctionCallFromISR (��. event_groups.h) ����� 4 ���������,
																 // ������ �� ������� ����� ��� 'PendedFunction_t
	if (result != pdFAIL) {
		/* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
		 switch should be requested.  The macro used is port specific and will
		 be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
		 the documentation page for the port being used. */
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		//lint !e923 !e632 ���������� ���� unsigned long � ���������, �� ������� ���������� � uint32 ���� ������������ � ���������� STM

		status = RS_pass;
	} else {
		status = RS_fail; //timer service queue was full
	}

	return status;
} //lint !e1762 ������� ��������� �� �������� const

/*******************************************************************************
 * Function:  eventGroupClearBits
 * Description: ������� ��������� ���� � event group. ���������� �������� event
 *  group �� ���� ��� ��������� ���� ���� �������
 * Threading usage and Assumptions:  none
 ******************************************************************************/
tU32 cRTOS::eventGroupClearBits(tEventGroupHandle eventGroup,
		const tU32 bitsToClear) const {
	tU32 bits;
	bits = (tU32) xEventGroupClearBits((EventGroupHandle_t) eventGroup,
			(EventBits_t) bitsToClear); //lint !e925 ��� tEventGroupHandle ������������ ��� ������������� � ��� ����������

	return bits;
} //lint !e1762 ������� ��������� �� �������� const

/*******************************************************************************
 * Function:  eventGroupClearBitsFromISR
 * Description: ������ eventGroupClearBits, ���������� �� ����������� ����������.
 *  ���������� �������� event group �� ���� ��� ��������� ���� ���� �������.
 * Threading usage and Assumptions:  none
 ******************************************************************************/
tU32 cRTOS::eventGroupClearBitsFromISR(tEventGroupHandle eventGroup,
		const tU32 bitsToClear) const {
	tU32 bits;
	bits = (tU32) xEventGroupClearBitsFromISR((EventGroupHandle_t) eventGroup,
			(EventBits_t) bitsToClear); //lint !e925 !e632
										// 1. - ��� tEventGroupHandle ������������ ��� ������������� � ��� ����������
										// 2. - ������� xTimerPendFunctionCallFromISR (��. event_groups.h) ����� 4 ���������,
										// ������ �� ������� ����� ��� 'PendedFunction_t'.
	return bits;
} //lint !e1762 ������� ��������� �� �������� const

/*******************************************************************************
 * Function:  eventGroupWaitBits
 * Description: ������� ��������� ��������� ����� � event group � ������ ������ �
 ������������� ��������� � ������� ������� ticksToWait
 * Threading usage and Assumptions:  none
 ******************************************************************************/
tU32 cRTOS::eventGroupWaitBits(const tEventGroupHandle eventGroup,
		const tU32 bitsToWaitFor, const tBoolean clearOnExit,
		const tBoolean waitForAllBits, tU32 ticksToWait) const {
	tU32 uxBits;
	uxBits = (tU32) xEventGroupWaitBits((EventGroupHandle_t) eventGroup,
			(EventBits_t) bitsToWaitFor, //lint !e925 ��� tEventGroupHandle ������������ ��� ������������� � ��� ����������
			(BaseType_t) clearOnExit, (BaseType_t) waitForAllBits,
			(TickType_t) ticksToWait);

	return uxBits;
} //lint !e1762 ������� ��������� �� �������� const

/*******************************************************************************
 * Function:  queueCreate
 * Description: ������� �������, � ������ �������� ������ ���������� NULL
 * Threading usage and Assumptions:  none
 ******************************************************************************/
tQueueHandle cRTOS::queueCreate(const tU32 queueLength,
		const tU32 itemSize) const {
	tQueueHandle queueHandle = NULL;
	queueHandle = (tQueueHandle) xQueueCreate(queueLength, itemSize);
	return queueHandle;
}

/*******************************************************************************
 * Function:  queueSend
 * Description: ���������� �������� � �������
 * Threading usage and Assumptions:  none
 ******************************************************************************/
tRtosStatus cRTOS::queueSend(const tQueueHandle queue,
		const void * pItemToQueue, const tU32 ticksToWait) const {
	tRtosStatus status;
	status = this->queueSendToBack(queue, pItemToQueue, ticksToWait);
	return status;
}

/*******************************************************************************
 * Function:  queueSend
 * Description: ���������� �������� � ������ �������
 * Threading usage and Assumptions:  none
 ******************************************************************************/
tRtosStatus cRTOS::queueSendToFront(const tQueueHandle queue,
		const void * pItemToQueue, tU32 ticksToWait) const {
	tRtosStatus status;
	status = (tRtosStatus) xQueueSendToFront((QueueHandle_t)queue, pItemToQueue,
			(TickType_t)ticksToWait);
	return status;
}

/*******************************************************************************
 * Function:  queueSend
 * Description: ���������� �������� � ����� �������
 * Threading usage and Assumptions:  none
 ******************************************************************************/
tRtosStatus cRTOS::queueSendToBack(const tQueueHandle queue,
		const void * pItemToQueue, tU32 ticksToWait) const {
	tRtosStatus status;
	status = (tRtosStatus) xQueueSend((QueueHandle_t)queue, pItemToQueue,
			(TickType_t)ticksToWait);
	return status;
}

/*******************************************************************************
 * Function:  queueReceive
 * Description: ����� �� �������
 * Threading usage and Assumptions:  none
 ******************************************************************************/
tBoolean cRTOS::queueReceive(const tQueueHandle queue, void * pBuffer,
		const tU32 ticksToWait) const {
	tBoolean status;
	status = (tBoolean) xQueueReceive((QueueHandle_t)queue, pBuffer,
			(TickType_t)ticksToWait);
	return status;
}

/*******************************************************************************
 * Function:  taskNotify
 * Description: ����������� ������ ������
 * Threading usage and Assumptions:  none
 ******************************************************************************/
tRtosStatus cRTOS::taskNotify(const tTaskHandle taskToNotify, const tU32 value,
		const tNotifyAction eAction) const {
	tRtosStatus status = RS_fail;
	const BaseType_t rowStatus = xTaskNotify(taskToNotify, value, eAction);
	if (rowStatus == pdTRUE) {
		status = RS_pass;
	} else {
		status = RS_fail;
	}
	return status;
}

/*******************************************************************************
 * Function:  taskNotifyWait
 * Description: �������� ����������� �� ������ �����
 * Threading usage and Assumptions:  none
 ******************************************************************************/
tBoolean cRTOS::taskNotifyWait(const tU32 bitsToClearOnEntry,
		const tU32 BitsToClearOnExit, const tU32 *pValue,
		const tU32 ticksToWait) const {
	tBoolean status;
	status = xTaskNotifyWait(bitsToClearOnEntry, BitsToClearOnExit,
			(uint32_t *) pValue, (TickType_t) ticksToWait);
	return status;
}

TickType_t cRTOS::fromMsToTick(float32_t ms) {
	return (ms * configTICK_RATE_HZ) / 1000;
}

TickType_t cRTOS::fromSecToTick(float32_t sec) {
	return (sec * configTICK_RATE_HZ);
}

float32_t cRTOS::fromTickToMs(TickType_t ticks) {
	return (ticks * 1000.0) / configTICK_RATE_HZ;
}

float32_t cRTOS::fromTickToSec(TickType_t ticks) {
	return ((float32_t) ticks) / configTICK_RATE_HZ;
}

UBaseType_t cRTOS::getTasks(iActiveObject** tasks) {
	tasks = cRTOS::tasks;
	return uxTaskGetNumberOfTasks();
}

iActiveObject** cRTOS::getTasks() {
	return cRTOS::tasks;
}

void cRTOS::addTask(iActiveObject* task) {
	cRTOS::tasks[tasksCount] = task;
	tasksCount++;
}

uint8_t cRTOS::getTasksCount() {
	return tasksCount;
}
