# Processes

<img src="./image/3-scheduling_context.png">

Code trong Contiki OS chạy ở một trong hai bối cảnh (contexts): song hành (cooperative) hoặc ưu tiên (preemptive). Ở bối cản chạy song hành, code sẽ chạy tuần tự từng process. Ở bối cảnh có ưu tiên, các code ưu tiên cao hơn (ngắt, timer thời gian thực, ...) sẽ dừng chạy code trong các process cho đến khi thực thi xong code có mức ưu tiên cao hơn.
Tất cả chương trình trong Contiki OS đều là process. Một process sẽ được thực thi liên tục bởi Contiki OS, process sẽ được bắt đầu khi hệ thống boot, hoặc khi module chứa process được load vào hệ thống. Các process sẽ chạy khi có một event xảy ra, ví dụ như timer được kích hoạt hoặc sự kiện từ bên ngoài.

### The structure of Process

Process của Contiki OS bao gồm 2 phần: `process control block` và `process thread`. 

- `Process control block` được lưu trong RAM, chứa thông tin vận hành của process (run-time information): tên của process, trạng thái của process, con trỏ đến process thread của process.
- `Process thread` là code của process được lưu trong ROM.

#### The Process Control Block

*Structure bên trong của một process, người dùng không thể truy cập trực tiếp đến struct này của process:*
```C
struct process 
{
    struct process *next;
    const char *name;
    int (* thread)(struct pt *, process_event_t,process_data_t);
    struct pt pt;
    unsigned char state, needspoll;
};
```

Process Control Block có dung lượng rất nhỏ vì chỉ có một vài byte của bộ nhớ:
- `struct process *next;`: con trỏ trỏ đến structure của Process Control Block tiếp theo.
- `const char *name;`: tên văn bản của process.
- `int (* thread)(struct pt *, process_event_t,process_data_t);`: là một function pointer, trỏ đến process thread của process.
- `struct pt pt;`: lưu trạng thái của protothread trong process thread.
- `unsigned char state, needspoll;`: là internal flags giữ trạng thái của process, `needspoll` flag được set bởi hàm `process_poll()` khi một process ở trạng thái polled.

Process Control Block được định nghĩa thông qua `PROCESS()` macro. Macro này gồm 2 thông số: 
- `variable name of the process control block`: sử dụng để truy cập đến process.
- `textual name of process`: sử dụng trong quá trình debug và in ra danh sách các process đang hoạt động đến với user.

*Ví dụ:* 
```C
PROCESS(hello_world_process, "Hello world process");
```

#### The Process Thread

Process thread chứa code thực thi của process. Một Process thread là một Single Protothread được gọi từ scheduler.

*Ví dụ:*
```C
PROCESS_THREAD(hello_world_process, ev, data)
{
 PROCESS_BEGIN();
 printf("Hello, world\n");
 PROCESS_END();
}
```

### Protothread

Protothread là một cách để cấu trúc code cho phép hệ thống chạy các hoạt động khác khi code đang chờ điều gì đó xảy ra.

Protothread là một hàm C thông thường, được bắt đầu và kết thúc bằng hai macro `PT_BEGIN()` và `PT_END()`. Giữa hai macro có thể sử dụng các protothread khác đã được khai báo.

*Preprocessor C của các hoạt động chính của protothread:*
```C
struct pt { lc_t lc };
#define PT_WAITING      0
#define PT_EXITED       1
#define PT_ENDED        2
#define PT_INIT(pt)     LC_INIT(pt->lc)
#define PT_BEGIN(pt)    LC_RESUME(pt->lc)
#define PT_END(pt)      LC_END(pt->lc); \
                        return PT_ENDED
#define PT_WAIT_UNTIL   (pt, c) LC_SET(pt->lc); \
                        if(!(c)) \
                        return PT_WAITING
#define PT_EXIT(pt)     return PT_EXITED
```
*Khai báo local continuations với cấu trúc switch trong C:*
```C
typedef unsigned short lc_t;
#define LC_INIT(c)      c = 0
#define LC_RESUME(c)    switch(c) { case 0:
#define LC_SET(c)       c = __LINE__; case __LINE__:
#define LC_END(c)       }
```
`struct pt { lc_t lc}`: bao gồm một biến `lc` kiểu `unsigned short`, viết tắt của `local continuation`. Vậy mỗi protothread chiếm 2 byte dung lượng.

Protothread sử dụng macro thay vì dùng hàm là vì phù hợp với mục đích làm thay đổi luồng điều khiển. Nếu dùng hàm thì khi biên dịch cần thêm các đoạn hợp ngữ khi gọi hàm, vì vậy dùng macro để thay đổi luồng điều khiển bằng các cấu trúc C tiêu chuẩn sẽ tối ưu hơn.

*Ví dụ: (giả sử `__LINE__` đang ở dòng thứ 12 của code)*

```C
static PT_THREAD(example(struct pt *pt))
{
  PT_BEGIN(pt);
  
  while(1) {
    PT_WAIT_UNTIL(pt,
      counter == 1000);
    printf("Threshold reached\n");
    counter = 0;
  }
  
  PT_END(pt);
}
```
```C
static char example(struct pt *pt)
{
  switch(pt->lc) { case 0:
 
  while(1) {
    pt->lc = 12; case 12:
    if(!(counter == 1000)) return 0;
    printf("Threshold reached\n");
    counter = 0;
  }
 
  } pt->lc = 0; return 2;
}
```

### Protothread in Process

Process trong Contiki OS được viết dựa trên protothread của Dunkel (http://dunkels.com/adam/pt/). Vì vậy Contiki có phiên bản protothread riêng của nó, cho phép chờ đợi các sự kiện đến.

Process trong Contiki OS có những đặc điểm sau:
- Các process hoạt động theo dạng `cooperative multithreading`, các source code cho biết chính xác điểm nào sẽ được thực thi. Không phải ở hình thức `preemptive multithreading` nên các process sẽ không chuyển đổi tự động giữa các thread, mà là thực hiện xoay vòng tuần tự.
- Các process cùng sử dụng chung stack, vì vậy không có biến thread-local. Thay vào đó dùng các static variable để bảo toàn các giá trị qua nhiều lệnh gọi process.

*Protothread macro được sử dụng trong Contiki OS:*

```C
PROCESS_BEGIN();            // Declares the beginning of a process' protothread.
PROCESS_END();              // Declares the end of a process' protothread.
PROCESS_EXIT();             // Exit the process.
PROCESS_WAIT_EVENT();       // Wait for any event.
PROCESS_WAIT_EVENT_UNTIL(); // Wait for an event, but with a condition.
PROCESS_YIELD();            // Wait for any event, equivalent to PROCESS_WAIT_EVENT().
PROCESS_WAIT_UNTIL();       // Wait for a given condition; may not yield the process.
PROCESS_PAUSE();            // Temporarily yield the process.
```

*Ví dụ:*
```C
PROCESS_THREAD(hello_world_process, ev, data)
{
    static int i;
    PROCESS_BEGIN();
    for (i=0;i<10;i++)
    {
        printf("%d from 1st process\n",i);  
        process_poll(&hello_world_process2);
        PROCESS_YIELD();
    }
    PROCESS_END();
}

PROCESS_THREAD(hello_world_process2, ev, data)
{
    static int i;
    PROCESS_BEGIN();
    for (i=0;i<10;i++)
    {
        printf("%d from IIund process\n",i);
        process_poll(&hello_world_process);
        PROCESS_YIELD();
    }
    PROCESS_END();
}
```

### Process Scheduler

Process Scheduler có công việc chính là gọi để khởi chạy process khi nó tới thời điểm cần chạy. Process Scheduler gọi một process bằng cách gọi hàm thực hiện process thread.

Tất cả việc gọi process trong Contiki OS được thực hiện để phản hồi một sự kiện đã được post đến process đó, hoặc đáp ứng poll request đến process đó. Process Scheduler sẽ chuyển event identifer đến process được gọi. Một con trỏ mờ (opaque pointer) sẽ được truyền, con trỏ này được người gọi cung cấp và có thể là NULL cho biết không có dữ liệu nào được truyền bằng sự kiện. Một poll request không có dữ liệu nào được chuyển qua.

#### Starting Process

Process được bắt đầu với việc gọi hàm `process_start()`, thực hiện setup process control structure, đặt process vào danh sách các process đang hoạt động trong kernel và gọi code khởi tạo process thread.

#### Exiting and Killing Processes

Process Exit ở một trong hai cách: process tự exit (process itself exit) và process được một process khác kết thúc (killed by another process).

- Một process có thể tự exit bằng cách gọi hàm `PROCESS_EXIT()` hoặc khi chương trình chạy tới câu lệnh `PROCESS_END()`.
- Một process có thể được kết thúc khi process khác gọi hàm `process_exit()`.

Khi một process kết thúc, Contiki kernel sẽ gửi một event đến tất cả các process khác để thông báo process đã kết thúc. Các process nhận được event này có thể thực hiện giải phóng mọi phân bổ tài nguyên được thực hiện bởi process đã kết thúc. Sự kiện `PROCESS_EVENT_EXITED` sẽ được gửi dưới dạng asynchronous event đến tất cả các process đang hoạt động.


