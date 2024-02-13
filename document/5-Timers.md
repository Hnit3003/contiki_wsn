# Timers

### The Clock Module

Clock modulde gồm các hàm quản lý thời gian hệ thống (system time).

- `clock_time()`: hàm trả về thời gian hoạt động hiện tại của hệ thống theo clock ticks. Số lượng clock ticks mỗi giây sẽ phụ thuộc vào platoform và được chỉ định bởi hằng số `CLOCK_SECOND`. System time được chỉ định với flatform dựa vào kiểu `clock_time_t`.
- `clock_seconds()`: trả về giá trị thời gian hoạt động của hệ thống theo giây.
- `clock_delay()`: block CPU trong vòng một khoảng thời gian được chỉ định.
- `clock_wait()`: block CPU trong một số lượng clock ticks.
- `clock_init()`: hàm được gọi bởi hệ thống trong khi boot-up để khởi tạo clock cho module.

*Clock Module API:*
```C
clock_time_t clock_time();              // Get the system time.
unsigned long clock_seconds();          // Get the system time in seconds.
void clock_delay(unsigned int delay);   // Delay the CPU
void clock_wait(int delay);             // Delay the CPU for a number of clock ticks.
void clock_init(void);                  // Initialize the clock module.
CLOCK_SECOND;                           // The number of ticks per second.
```

#### Porting the clock module

Clock module phụ thuộc vào platform và được khởi tạo trong file `clock.c`. Vì clock module xử lý thời gian hệ thống nên clock module cũng xử lý các thông tới thư viện `etimer` để kiểm tra tới hạn của bộ hẹn giờ của event timer.

### The Timer Library

Contiki Timer Library cung cấp các hàm cho setting, resetting và restarting timers, và cho kiểm tra xem timer có tràn hay chưa. Ứng dụng phải kiểm tra xem timer có tràn hay chưa bằng cách thủ công mà không được tự động. Timer Library sử dụng hàm `clock_timee()` để lấy giá trị thời gian hiện tại của hệ thống.

Timer được khai báo là `struct timer` và thực hiện truy cập vào timer bằng con trỏ.

- `timer_set()`: khởi tạo timer với đặt thời gian tràn từ thời điểm hiện tại và lưu khoảng thời gian trong bộ hẹn giờ.
- `timer_reset()`: dùng để restart timer từ lần tràn timer trước.
- `timer_restart()`: dùng để restart timer ngay tại thời gian hiện tại.
- `timer_expired()`: trả về xem timer có tràn hay chưa.
- `timer_remaining()`: trả về giá trị thời gian cho đến khi timer tràn. Nếu timer đã tràn, giá trị trả về của hàm sẽ không xác định.

Timer Library an toàn để sử dụng trong ngắt.

*Timer Library API:*

```C
void timer_set(struct timer *t, clock_time_t interval); // Start the timer.
void timer_reset(struct timer *t); // Restart the timer from the previous expiration time.
void timer_restart(struct timer *t); // Restart the timer from current time.
int timer_expired(struct timer *t); // Check if the timer has expired.
clock_time_t timer_remaining(struct timer *t); // Get the time until the timer expires.
```
*Ví dụ:*

```C
#include "sys/timer.h"
static struct timer rxtimer;
void init(void) 
{
    timer_set(&rxtimer, CLOCK_SECOND / 2);
}
interrupt(UART1RX_VECTOR) uart1_rx_interrupt(void)
{
    if(timer_expired(&rxtimer)) {
    /* Timeout */
    /* ... */
    }
    timer_restart(&rxtimer);
    /* ... */
}
```

### The Stimer Libray

Contiki Stimer cung cấp timer giống như cơ chế timer library nhưn giá trị thời gian tính bằng giây, cho phép thời đến khi tràn lâu hơn nhiều.

Stimer Library sử dụng hàm `clock_seconds()` của clock module để lấy giá trị thời gian hiện tại của hệ thống theo giây.

API của stimer library giống như API của timer library, điều khác biệt duy nhất chính là thời gian tính bằng giây thay vì clock ticks.

Stimer library an toàn sử dụng trong ngắt.

Stimer Library API:

```C
void stimer_set(struct stimer *t, unsigned long interval); // Start the timer.
void stimer_reset(struct stimer *t); // Restart the stimer from the previous expiration time.
void stimer_restart(struct stimer *t); // Restart the stimer from current time.
int stimer_expired(struct stimer *t); // Check if the stimer has expired.
unsigned long stimer_remaining(struct stimer *t); // Get the time until the timer expires.
```

### The Etimer Library

Contiki etimer library cung cấp các cơ chế timer để vận hành events thời gian. Event timer sẽ post `PROCESS_EVENT_TIMER` cho process đã đặt timer khi event timer tràn. Etimer sử dụng hàm `clock_time()` trong clock module để lấy thời gian hiện tại của hệ thống.

Event timer được khai báo bằng `struct etimer` và truy xuất đến event timer thông qua con trỏ đến event timer đã khai báo.

- `etimer_set()`: set timer sẽ tràn sau một khoảng thời gian được chỉ định tính từ thời điểm hiện tại.
- `etimer_reset()`: restart timer kể từ lần cuối timer tràn.
- `etimer_restart()`: restart timer từ thời điểm hiện tại.
- `etimer_stop()`: dừng timer, tràn timer ngay lập tức và không post event.
- `etimer_expired()`: xác định xem timer tràn hay chưa.

Etimer library **không an toàn** khi sử dụng trong ngắt.

*Etimer Library API:*
```C
void etimer_set(struct etimer *t, clock_time_t interval); // Start the timer.
void etimer_reset(struct etimer *t); // Restart the timer from the previous expiration time.
void etimer_restart(struct etimer *t); // Restart the timer from current time.
void etimer_stop(struct etimer *t); // Stop the timer.
int etimer_expired(struct etimer *t); // Check if the timer has expired.
int etimer_pending(); // Check if there are any non-expired event timers.
clock_time_t etimer_next_expiration_time(); // Get the next event timer expiration time.
void etimer_request_poll(); // Inform the etimer library that the system clock has changed.
```

*Ví dụ: tạo một event timer cho process chạy mỗi 1 giây*
```C
#include "sys/etimer.h"
PROCESS_THREAD(example_process, ev, data)
{
    static struct etimer et;
    PROCESS_BEGIN();
    /* Delay 1 second */
    etimer_set(&et, CLOCK_SECOND);
    while(1) 
    {
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        /* Reset the etimer to trig again in 1 second */
        etimer_reset(&et);
        /* ... */
    }
    PROCESS_END();
}
```

#### Porting the Etimer Library

Etimer library được triển khai trong `core/sys/etimer.c` và phụ thuộc vào platform nhưng vần hàm callback đến `timer_request_poll()` để xử lý event timers. Cho phép etimer library đánh thức hệ thống từ low power mode khi event timer tràn thông qua các hàm sau:

- `etimer_pending()`: kiểm tra xem có event timer nào chưa tràn không.
- `etimer_next_expiration_time()`: trả về event timer kế tiếp sẽ tràn.
- `etimer_request_poll()`: thông báo cho etimer library rằng system clock đã thay đổi và etimer có thể tràn. Hàm này an toàn khi sử dụng trong ngắt.

### The Ctimer Library

Contiki ctimer library cung cấp cơ chế timer gọi hàm callback khi callback timer tràn. Ctimer library sử dụng hàm `clock_time()` của clock module để lấy giá trị thời gian hệ thống hiện tại.

*Ctimer Library API:*

```C
void ctimer_set(struct ctimer *c, clock_time_t t, void(*f)(void *), void *ptr); // Start the timer.
void ctimer_reset(struct ctimer *t); // Restart the timer from the previous expiration time.
void ctimer_restart(struct ctimer *t); // Restart the timer from current time.
void ctimer_stop(struct ctimer *t); // Stop the timer.
int ctimer_expired(struct ctimer *t); // Check if the timer has expired.
```

*Setup một ctimer để gọi một hàm mỗi 1 giây:*

```C
#include "sys/ctimer.h"
static struct ctimer timer;
static void callback(void *ptr)
{
    ctimer_reset(&timer);
    /* ... */
}
void init(void)
{
    ctimer_set(&timer, CLOCK_SECOND, callback, NULL);
}
```

#### Porting the Ctimer Library

Ctimer library được triển khai bằng cách sử dụng etimer library và không cần port vào thêm nữa.

## The Rtimer Library

Contiki rtimer library cung cấp scheduling và xử lý các real-time tasks (với thời gian xử lý được biết trước). Rtimer library sử dụng clock module của riêng nó để lập lịch và cho phép độ phân giải của clock cao. Hàm `RTIMER_NOW()` được sử dụng để lấy thời gian hiện tại theo ticks và hằng số `RTIMER_SECOND` sẽ chỉ định số tick mỗi giây.

Các real-time tasks sẽ xử lý theo dạng pre-empt cho việc xử lý task ngay lập tức. 

*Platform dependent functions for the rtimer library:*

```C
RTIMER_CLOCK_LT(a, b); // This should give TRUE if 'a' is less than 'b', otherwise false.
RTIMER_ARCH_SECOND; // The number of ticks per second.
void rtimer_arch_init(void); // Initialize the rtimer architecture code.
rtimer_clock_t rtimer_arch_now(); // Get the current time.
int rtimer_arch_schedule(rtimer_clock_t wakeup_time); // Schedule a call to 
<tt>rtimer_run_next()</tt>.
```
