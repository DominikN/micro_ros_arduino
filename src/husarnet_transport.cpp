#if defined(ESP32) && defined(HUSARNET)
#include <Arduino.h>
#include <Husarnet.h>
#include <HusarnetClient.h>
#include <micro_ros_arduino.h>

#include <AsyncUDP.h>

extern "C" {

static AsyncUDP udp;

QueueHandle_t rx_queue;

bool arduino_husarnet_transport_open(struct uxrCustomTransport *transport) {
  struct micro_ros_agent_locator *locator =
      (struct micro_ros_agent_locator *)transport->args;

  Serial1.printf("Connecting to \"%s:%d\"... ", locator->hostname,
                 locator->port);

  rx_queue = xQueueCreate(1000, sizeof(uint8_t));


  Serial1.printf("%s ... ", locator->addr.toString().c_str());

  if (!udp.connect(locator->addr, locator->port)) {
    Serial1.printf("failed\r\n");
    return false;
  }

  if(udp.connected()) {
    Serial1.printf("ok...");
  } else {
    Serial1.printf("fail...");
  }

  udp.onPacket([](AsyncUDPPacket packet) {
    Serial1.printf("rcv msg\r\n");
    for (int i = 0; i < packet.length(); i++) {
      xQueueSendFromISR(rx_queue, packet.data() + i, NULL);
    }
  });

  Serial1.printf("done\r\n");

  return true;
}

bool arduino_husarnet_transport_close(struct uxrCustomTransport *transport) {
  Serial1.printf("udp close\r\n");
  udp.close();
  return true;
}

size_t arduino_husarnet_transport_write(struct uxrCustomTransport *transport,
                                        const uint8_t *buf, size_t len,
                                        uint8_t *errcode) {
  (void)errcode;
  struct micro_ros_agent_locator *locator =
      (struct micro_ros_agent_locator *)transport->args;

  Serial1.printf("send msg...");
  // size_t sent = udp.write(buf, len);
  size_t sent = udp.writeTo(buf, len, locator->addr, locator->port);
  Serial1.printf("%d\r\n", sent);
  delay(100);
  return sent;
}


size_t arduino_husarnet_transport_read(struct uxrCustomTransport *transport,
                                       uint8_t *buf, size_t len, int timeout,
                                       uint8_t *errcode) {
  (void)errcode;

  int retval = 0;
  Serial1.printf("read msg\r\n");

  for (int i = 0; i < len; i++) {
    if (pdTRUE == xQueueReceive(rx_queue, buf + i, 0)) {
      retval++;
    }
  }

  return retval;
}
}

#endif
