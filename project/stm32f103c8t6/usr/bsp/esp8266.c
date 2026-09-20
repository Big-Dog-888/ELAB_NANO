#include "esp8266.h"
#include "debug_uart.h"
#include "../../../elab/common/elab_log.h"
#include "../../../elab/elib/elib_queue.h"
#include "../../../elab/common/elab_export.h"
#include <string.h>
#include <stdio.h>

ELAB_TAG("ESP8266");

static uint8_t esp8266_data[256];
static char esp8266_cmd[128];
static char esp8266_line[128];

void send_to_esp8266_cmd(uint8_t *cmd, uint16_t len)
{
    uart_send(cmd, len);
    uart_send("\r\n", 2);
}

void send_esp8266_data(uint8_t *data, uint16_t len)
{
    uart_send(data, len);
}

int receive_from_esp8266_data(uint8_t *data, uint16_t len)
{
    int n = uart_receive(data, len);
    return n;
}

static void esp8266_dump_hex(const uint8_t *buf, uint16_t n)
{
    uint16_t i;
    int p = 0;

    p += snprintf(esp8266_line + p, sizeof(esp8266_line), "HEX(%d): ", n);
    for (i = 0; i < n && p < (int)sizeof(esp8266_line) - 4; i++)
        p += snprintf(esp8266_line + p, sizeof(esp8266_line) - p, "%02X ", buf[i]);

    elog_info("%s", esp8266_line);
}

static int esp8266_send_cmd_wait_ok(const char *cmd, uint16_t cmd_len,
                                    uint32_t timeout_ms)
{
    int16_t n;

    uart_buffer_clear();
    send_to_esp8266_cmd((uint8_t *)cmd, cmd_len);
    n = uart_receive_timeout(esp8266_data, sizeof(esp8266_data) - 1,
                             timeout_ms, 80);
    esp8266_data[n] = '\0';

    elog_info("CMD: %s -> got %d bytes", cmd, n);
    esp8266_dump_hex(esp8266_data, n);

    if (n <= 0)
        return -1;
    if (strstr((char *)esp8266_data, "OK") != NULL)
        return 0;
    if (strstr((char *)esp8266_data, "ERROR") != NULL)
        return -2;

    return -3;
}

static int esp8266_tcp_send_raw(const uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
    int16_t n;

    uart_buffer_clear();
    snprintf(esp8266_cmd, sizeof(esp8266_cmd), "AT+CIPSEND=%d", len);
    send_to_esp8266_cmd((uint8_t *)esp8266_cmd, strlen(esp8266_cmd));
    HAL_Delay(100);

    uart_buffer_clear();
    uart_send((void *)data, len);
    n = uart_receive_timeout(esp8266_data, sizeof(esp8266_data) - 1,
                             timeout_ms, 100);
    esp8266_data[n] = '\0';

    if (n <= 0)
        return -1;
    if (strstr((char *)esp8266_data, "SEND OK") != NULL)
        return 0;
    if (strstr((char *)esp8266_data, "SEND FAIL") != NULL)
        return -2;
    if (strstr((char *)esp8266_data, "CLOSED") != NULL)
        return -3;

    return 0;
}

static uint8_t mqtt_packet[128];

static int esp8266_mqtt_connect(const char *client_id, const char *username,
                                const char *password, uint16_t keepalive)
{
    uint8_t *p = mqtt_packet;
    uint16_t plen = 0;
    uint16_t cid_len = strlen(client_id);
    uint16_t usr_len = username ? strlen(username) : 0;
    uint16_t pwd_len = password ? strlen(password) : 0;
    uint8_t flags = 0xC2;

    *p++ = 0x10;
    uint16_t var_header = 2 + 4 + 1 + 1 + 2 + 2 + cid_len;
    if (usr_len) var_header += 2 + usr_len;
    if (pwd_len) var_header += 2 + pwd_len;
    *p++ = var_header;

    *p++ = 0x00; *p++ = 0x04;
    *p++ = 'M'; *p++ = 'Q'; *p++ = 'T'; *p++ = 'T';
    *p++ = 0x04;
    *p++ = flags;
    *p++ = (keepalive >> 8) & 0xFF;
    *p++ = keepalive & 0xFF;

    *p++ = (cid_len >> 8) & 0xFF;
    *p++ = cid_len & 0xFF;
    memcpy(p, client_id, cid_len); p += cid_len;

    if (usr_len) {
        *p++ = (usr_len >> 8) & 0xFF;
        *p++ = usr_len & 0xFF;
        memcpy(p, username, usr_len); p += usr_len;
    }
    if (pwd_len) {
        *p++ = (pwd_len >> 8) & 0xFF;
        *p++ = pwd_len & 0xFF;
        memcpy(p, password, pwd_len); p += pwd_len;
    }

    plen = p - mqtt_packet;
    return esp8266_tcp_send_raw(mqtt_packet, plen, 2000);
}

static int esp8266_mqtt_subscribe(uint16_t pkt_id, const char *topic, uint8_t qos)
{
    uint8_t *p = mqtt_packet;
    uint16_t tlen = strlen(topic);

    *p++ = 0x82;
    *p++ = 2 + 2 + tlen + 1;
    *p++ = (pkt_id >> 8) & 0xFF;
    *p++ = pkt_id & 0xFF;
    *p++ = (tlen >> 8) & 0xFF;
    *p++ = tlen & 0xFF;
    memcpy(p, topic, tlen); p += tlen;
    *p++ = qos;

    uint16_t plen = p - mqtt_packet;
    return esp8266_tcp_send_raw(mqtt_packet, plen, 2000);
}

static int esp8266_mqtt_publish(const char *topic, const char *payload)
{
    uint8_t *p = mqtt_packet;
    uint16_t tlen = strlen(topic);
    uint16_t plen_payload = strlen(payload);

    *p++ = 0x30;
    *p++ = 2 + tlen + plen_payload;
    *p++ = (tlen >> 8) & 0xFF;
    *p++ = tlen & 0xFF;
    memcpy(p, topic, tlen); p += tlen;
    memcpy(p, payload, plen_payload); p += plen_payload;

    uint16_t plen = p - mqtt_packet;
    return esp8266_tcp_send_raw(mqtt_packet, plen, 2000);
}

void ESP_ConnectWiFi(char *ssid, char *pwd)
{
    int ret;
    int i;

    HAL_Delay(2500);
    uart_buffer_clear();
    HAL_Delay(200);
    uart_buffer_clear();

    for (i = 0; i < 5; i++)
    {
        ret = esp8266_send_cmd_wait_ok("AT", 2, 1000);
        if (ret == 0)
            break;
        HAL_Delay(500);
    }
    if (ret != 0)
    {
        elog_error("AT 通信异常，ret=%d", ret);
        return;
    }

    ret = esp8266_send_cmd_wait_ok("AT+RST", 6, 2000);
    HAL_Delay(3000);
    uart_buffer_clear();
    HAL_Delay(200);
    uart_buffer_clear();

    ret = esp8266_send_cmd_wait_ok("AT+CWMODE=1", 12, 1000);
    if (ret != 0)
    {
        elog_error("CWMODE 失败，ret=%d", ret);
        return;
    }

    snprintf(esp8266_cmd, sizeof(esp8266_cmd),
             "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);
    elog_info("正在连接 WiFi: %s", ssid);
    ret = esp8266_send_cmd_wait_ok(esp8266_cmd, strlen(esp8266_cmd), 8000);
    if (ret == 0)
    {
        elog_info("WiFi 连接成功");
    }
    else
    {
        elog_error("WiFi 连接失败 (ret=%d)", ret);
        return;
    }
}

void ESP_ConnectMQTT(void)
{
    int ret;

    HAL_Delay(500);
    uart_buffer_clear();
    HAL_Delay(200);
    uart_buffer_clear();

    ret = esp8266_send_cmd_wait_ok("AT+CIPMUX=0", 10, 1000);
    if (ret != 0)
    {
        elog_warn("CIPMUX=0 跳过 (NonOS AT 默认单连接), ret=%d, raw: %s", ret, esp8266_data);
    }

    ret = esp8266_send_cmd_wait_ok("AT+CIPMODE=0", 11, 1000);
    if (ret != 0)
    {
        elog_warn("CIPMODE=0 跳过 (默认非透传), ret=%d, raw: %s", ret, esp8266_data);
    }

    snprintf(esp8266_cmd, sizeof(esp8266_cmd),
             "AT+CIPSTART=\"TCP\",\"192.168.100.36\",1883");
    ret = esp8266_send_cmd_wait_ok(esp8266_cmd, strlen(esp8266_cmd), 5000);
    if (ret != 0)
    {
        elog_error("TCP 连接 Broker 失败，ret=%d", ret);
        return;
    }
    elog_info("TCP Broker 连接成功");

    ret = esp8266_mqtt_connect("esp01s_001", "esp", "123456", 60);
    if (ret != 0)
    {
        elog_error("MQTT CONNECT 失败，ret=%d", ret);
        return;
    }
    elog_info("MQTT CONNECT 成功");

    HAL_Delay(200);

    ret = esp8266_mqtt_subscribe(1, "topic/ctrl", 0);
    if (ret != 0)
    {
        elog_error("MQTT SUBSCRIBE 失败，ret=%d", ret);
        return;
    }
    elog_info("MQTT SUBSCRIBE topic/ctrl 成功");

    HAL_Delay(200);

    ret = esp8266_mqtt_publish("topic/esp01s", "hello");
    if (ret != 0)
    {
        elog_error("MQTT PUBLISH 失败，ret=%d", ret);
        return;
    }
    elog_info("MQTT PUBLISH hello 成功");
}

static uint8_t esp_init_done = 0;

void Esp_Init(void)
{
    if (esp_init_done) return;
    esp_init_done = 1;

    ESP_ConnectWiFi("K60", "232232232");
    ESP_ConnectMQTT();
}

POLL_EXPORT(Esp_Init, 1000);


void esp8266_test(void)
{
    int16_t n;

    uart_buffer_clear();
    send_to_esp8266_cmd("AT", 2);
    n = uart_receive_timeout(esp8266_data, sizeof(esp8266_data) - 1,
                             1000, 80);
    esp8266_data[n] = '\0';
    elog_info("ESP8266 reply(%d): %s", n, esp8266_data);
}