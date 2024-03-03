#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"

#include "dev/button-sensor.h"

#include "dev/leds.h"
#include "cc2420.h"

#include <stdio.h>
#include <string.h>

#define MOTE_NUM  10    /* Number of Motes in Network */

static uint16_t packet_send_count = 1;    /* Number of packets sended */
/* Number of packet receive from all Mote*/
static uint16_t packet_receive_count[MOTE_NUM] = {0};

static signed char rssi_neigbr[MOTE_NUM] = {0};
static signed char rssi_max = -120;
static uint8_t rssi_max_mote = MOTE_NUM + 1;

/*---------------------------------------------------------------------------*/
PROCESS(example_broadcast_process, "Broadcast example");
AUTOSTART_PROCESSES(&example_broadcast_process);
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{
  packet_receive_count[from->u8[0] - 1]++;
  rssi_neigbr[from->u8[0] - 1] = cc2420_last_rssi;
  if (cc2420_last_rssi > rssi_max)
  {
    rssi_max = rssi_neigbr[from->u8[0] - 1];
    rssi_max_mote = from->u8[0];
  }
  printf("broadcast message received from %d.%d [%d times]: '%s'  RSSI: %d\n",
         from->u8[0], from->u8[1], packet_receive_count[from->u8[0] - 1], (char *)packetbuf_dataptr(), rssi_neigbr[from->u8[0] - 1]);
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);

  while(1) {

    /* Delay 2-4 seconds */
    etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4));

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    char message_send[20];
    sprintf(message_send, "ID:%d packet[%d]", linkaddr_node_addr.u8[0], packet_send_count++);
    packetbuf_copyfrom(message_send, sizeof(message_send));
    broadcast_send(&broadcast);
    printf("broadcast message sent: packet[%d]\n", (packet_send_count-1));

    printf("Max RSSI mote: %d : %d\n", rssi_max_mote, rssi_max);

  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
