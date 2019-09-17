#if DDSRT_WITH_FREERTOS
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	printf("main() has started\n");
	//vTaskStartScheduler();
	//while(1);
	return 0;
}
#else
int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	return 0;
}
#endif
