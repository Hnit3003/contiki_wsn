#include "contiki.h"
#include "sys/mt.h"
#include <stdio.h>

static char *ptr;

PROCESS(multi_threading_process, "Multi-threading process");
AUTOSTART_PROCESSES(&multi_threading_process);

/*---------------------------------------------------------------------------*/
static void thread_func(char *str, int len)
{
    ptr = str + len;
    mt_yield();

    if(len != 0) 
    {
        thread_func(str, len - 1);
        mt_yield();
    }

    ptr = str + len;

}
/*---------------------------------------------------------------------------*/
static void thread_main(void *data)
{
    while(1) 
    {
        thread_func((char *)data, 9);
    }
    mt_exit();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(multi_threading_process, ev, data)
{
    static struct mt_thread alpha_thread;     
    static struct mt_thread count_thread;

    static struct etimer timer;
    static int toggle = 1;

    PROCESS_BEGIN();

    mt_init();
    mt_start(&alpha_thread, thread_main, "JIHGFEDCBA");
    mt_start(&count_thread, thread_main, "9876543210");

    etimer_set(&timer, CLOCK_SECOND / 2);

    while(1) 
    {
        PROCESS_WAIT_EVENT();
        if(ev == PROCESS_EVENT_TIMER) 
        {
        if(toggle) 
        {
            mt_exec(&alpha_thread);
            toggle--;
        } else 
        {
            mt_exec(&count_thread);
            toggle++;
        }
        puts(ptr);

        etimer_set(&timer, CLOCK_SECOND / 2);
        }
    }
    
    mt_stop(&alpha_thread);
    mt_stop(&count_thread);
    mt_remove();

    PROCESS_END();
}