# Radio duty cycling

Contiki có 3 cơ chế duty cycle: `ContikiMAC`, `X-MAX` và `LPP` là giao thức dựa trên các nguyên tắc của low-power listening (hiểu ngầm) nhưng tiết kiệm công suất hơn.

`X-MAX` dựa trên giao thức `X-MAX` gốc nhưng được cải tiến để giảm mức năng lượng tiêu thụ và duy trì mạng tốt.

`LPP` của Contiki dựa trên giao thức LowPower Probing nhưng được cải tiến mức tiêu tốn công suất như cung cấp các cơ chế broadcast data.

### Table of Contents
- [Radio duty cycling](#radio-duty-cycling)
    - [Table of Contents](#table-of-contents)
  - [Background on Radio Duty Cycling Protocols](#background-on-radio-duty-cycling-protocols)
    - [Low-Power Listening](#low-power-listening)
    - [Low-Power Probing](#low-power-probing)
  - [The Contiki Radio Duty Cycling Protocols](#the-contiki-radio-duty-cycling-protocols)
    - [The ContikiMAC Radio Duty Cycling Mechanism](#the-contikimac-radio-duty-cycling-mechanism)
    - [The Contiki X-MAC](#the-contiki-x-mac)
      - [Broadcast](#broadcast)
      - [Phase Awareness](#phase-awareness)
      - [Reliable Packet Transmissions](#reliable-packet-transmissions)
      - [812.15.4 Compatibility](#812154-compatibility)
  - [Implementation](#implementation)


## Background on Radio Duty Cycling Protocols

Năng lượng tiêu tốn là vấn đề rất quan trọng với WNS node ảnh hưởng đến thời gian vận hành của network. Để đạt được thời gian vận hành lâu dài, phần cứng tiết kiệm năng lượng khi phát radio là chưa đủ, việc truyền nhận dữ liệu qua sóng radio sử dụng rất nhiều năng lượng và chiếm phần lớn năng lượng trong quá trình hoạt động của node.

Để có thời gian hoạt động lâu dài, sóng radio phải được tắt nhiều nhất có thể. Nhưng khi sóng radio bị tắt thì node không thể gửi hay nhận bất kỳ tin nhắn nào. Vì vậy phải có phương pháp quản lý sóng radio sao cho node có thể nhận tin nhắn nhưng vẫn nhận được message giữa việc tiếp nhận và truyền tải message.

Mục đích của giao thức power-saving dutycycling là để giữ sóng radio tắt trong khi đó vẫn cung cấp đủ điểm hẹn để hai node có thể liên lạc được với nhau. Nếu các node được đồng bộ về mặt thời gian, giao thức duty cycling có thể thiết lập và duy trì lịch trình khi các node có thể giao tiếp. Giao thức sẽ bật sóng radio vào một thời điểm xác định và các node lân cận có thể gửi packet đúng với thời gian dự kiến đã được đồng bộ. Nếu không có thời gian đồng bộ, các node phải sử dụng các phương tiện khác để cung cấp điểm hẹn.

Trong giao tiếp mạng cảm biến không dây, giao thức radio duty cycling thường được gọi là MAC protocols vì giao thức này thường được triển khai ở lớp MAC. Ở Contiki, giao thức duty cycling được tách khỏi lớp MAC và được gọi là lớp RDC.

### Low-Power Listening

Low-Power Listening là một kỹ thuật duty cycling mà node nhận sẽ bật sóng radio theo chu kỳ để lắng nghe các node xung quanh. Nếu có node đang gửi, sóng radio sẽ được giữ thêm một khoảng thời gian để nhận packet.

Trước khi gửi một packet, node gửi sẽ gửi một số strobe packet với mục đích thông báo cho có node lân cận biết rằng đang có packet muốn được gửi. Bởi vì tất cả các node đều sẽ định kỳ lấy mẫu sóng radio nên sẽ thấy được strobe packet và tiếp tục bật sóng radio để dự đoán packet. Khoảng thời gian gửi đi strobe packet bằng với thời gian ngủ của những node lân cận.

Bằng cách đặt địa chỉ của node nhận vào các strobe packet, các node lân cận có thể quyết định xem có nhận packet không dựa vào địa chỉ đó.

`B_MAC` là được phát triển cho các bộ thu phát radio không tự động đóng gói các byte mà nó gửi đi, không sử dụng các strobe packet mà sử dụng các tone dài để đánh thức các node lân cận.

`WiseMAC` cũng tương tự như `B-MAC`, nhưng tối ưu hóa hơn bằng cách cho phép tất cả các node ghi lại các mẫu radio phase của các node lân cận. Khi gửi một packet đến một node nhất định, node gửi sẽ đợi cho đến khi node lân cận nhận biết được đường truyền thay đổi trước khi gửi tone báo thức. Do đó tone báo thức sẽ được gửi ngay trước khi node nhận thức dậy, giúp tiết kiệm năng lượng hơn nữa.

`X-MAC` là giao thức low-power listening đầu tiên được phát triển cho packetizing radio, và là giao thức đầu tiên xử dụng strobe packet thay vì wake-up tone. Trong X-MAC, strobes chứa địa chỉ của node nhận và những node và những node lân cận có thể không bật radio khi nghe thấy strobe của một node khác. Ngoài ra, `X-MAC` còn tối ưu hóa cơ chế hơn nữa bằng cách thêm một strobe acknowledgedment packet. Packet này được bởi node nhận để xác nhận đã nhận được strobe packet chứa địa chỉ của nó. Node gửi sẽ lắng nghe gói tin xác nhận đó và bắt đầu gửi dữ liệu.

### Low-Power Probing

## The Contiki Radio Duty Cycling Protocols

###  The ContikiMAC Radio Duty Cycling Mechanism

### The Contiki X-MAC

#### Broadcast

#### Phase Awareness

#### Reliable Packet Transmissions

#### 812.15.4 Compatibility

## Implementation