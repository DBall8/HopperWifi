#ifndef CONFIG_HPP
#define CONFIG_HPP

const static uint8_t MAX_SSID_LEN = 32;
const static uint8_t MAX_PASSWORD_LEN = 64;
const static uint8_t ID_LEN = sizeof(uint16_t);

const static uint32_t RETRY_MS = 30 * 60 * 1000;
const static uint32_t SOCKET_RECONNECT_MS = 5 * 60 * 1000;
const static uint32_t HEARTBEAT_MS = 10 * 60 * 1000;
const static uint32_t HEARTBEAT_TIMEOUT_MS = 60 * 1000;
const static uint32_t DC_HEARTBEAT_COUNT = 3;
const static uint32_t CMD_TIMEOUT_MS = 15 * 1000;

#endif