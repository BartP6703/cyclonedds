#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

int real_main(void)
{
	vTaskStartScheduler();
	while(1);
	return 0;
}
