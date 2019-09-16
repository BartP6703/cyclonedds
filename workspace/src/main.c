#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

extern int real_main(int argc, char *argv[]);

int real_main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	printf("main() has started\n");
	vTaskStartScheduler();
	//while(1);
	return 0;
}
