#include "contiki.h"
#include "sys/pt.h"
#include "clock.h"
#include <stdio.h>

#define ALPHA       0
#define COUNT       1
#define UP          0
#define DOWN        1
#define FIRST_MESS  ALPHA
#define FIRST_MODE  DOWN

PROCESS(protothread, "Protothread process");
AUTOSTART_PROCESSES(&protothread);

char alpha[] = "JIHGFEDCBA";
char count[] = "9876543210";
uint8_t len = 9;

PROCESS_THREAD(protothread, event, data)
{
    static struct etimer etim;
    static uint8_t toogle = FIRST_MESS;
    static uint8_t mode = FIRST_MODE;

    PROCESS_BEGIN();

    etimer_set(&etim, CLOCK_SECOND / 2);
    while(1)
    {
        PROCESS_WAIT_UNTIL(etimer_expired(&etim));

        etimer_set(&etim, CLOCK_SECOND / 2);
        if (toogle == ALPHA)
        {
            char *message = alpha + len;
            puts(message);
            toogle++;
        } else
        {
            char *message = count + len;
            puts(message);
            toogle--;
        }

        if (toogle == FIRST_MESS)
        {
            if (mode == DOWN)
            {
                len--;
                
                if (len == 0) mode = UP;
            } else
            {
                len++;
                
                if (len == 9) mode = DOWN;
            }
        }
        etimer_set(&etim, CLOCK_SECOND / 2);
    }

    PROCESS_END();
}
