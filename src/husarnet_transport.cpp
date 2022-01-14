#if defined(ESP32) && defined(HUSARNET)
#include <Arduino.h>
#include <Husarnet.h>
#include <HusarnetClient.h>
#include <micro_ros_arduino.h>

extern "C" {

static HusarnetClient client;

bool arduino_husarnet_transport_open(struct uxrCustomTransport *transport) {
  struct micro_ros_agent_locator *locator =
      (struct micro_ros_agent_locator *)transport->args;

  Serial1.printf("Connecting to \"%s:%d\"... ", locator->hostname,
                 locator->port);

  /* Try to connect to a server on port 8002 on your laptop */
  if (!client.connect(locator->hostname, locator->port)) {
    Serial1.printf("failed\r\n");
    return false;
  }

  Serial1.printf("done\r\n");

  return true;
}

bool arduino_husarnet_transport_close(struct uxrCustomTransport *transport) {
  client.stop();
  return true;
}

size_t arduino_husarnet_transport_write(struct uxrCustomTransport *transport,
                                        const uint8_t *buf, size_t len,
                                        uint8_t *errcode) {
  (void)errcode;
  struct micro_ros_agent_locator *locator =
      (struct micro_ros_agent_locator *)transport->args;

  Serial1.printf("transport_write:\r\n\tbuf:%s\r\n\tlen:%d\r\n", buf, len);

  size_t sent = client.write(buf, len);

  return sent;
}

size_t arduino_husarnet_transport_read(struct uxrCustomTransport *transport,
                                       uint8_t *buf, size_t len, int timeout,
                                       uint8_t *errcode) {
  (void)errcode;
  client.setTimeout(timeout);
  return client.read(buf, len);
}
}

#endif
