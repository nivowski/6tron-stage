#include "lwshell/lwshell.h"
#include <string.h>

#include "EventQueue.h"
#include "mbed.h"

#include "lorawan.h"
#include "measurement.h"

#include "SPIFBlockDevice.h"

static SPIFBlockDevice spif(P1_SPI_MOSI, P1_SPI_MISO, P1_SPI_SCK, PB_13);

static UnbufferedSerial console(CONSOLE_TX, CONSOLE_RX);

static events::EventQueue event_queue(10 * EVENTS_EVENT_SIZE);

static Measurement measurement;
static Lorawan lorawan(&event_queue);

typedef struct LoRaWANParams {
    uint8_t dev_eui[8] = { 0xFE, 0xFF, 0xFF, 0xFF, 0xFD, 0xFF, 0x12, 0x34 };
    uint8_t app_eui[8] = { 0xFE, 0xFF, 0xFF, 0xFF, 0xFD, 0xFF, 0x12, 0x34 };
    uint8_t app_key[16] = { 0xA7,
        0xE0,
        0xB9,
        0x04,
        0xF3,
        0xA4,
        0xE1,
        0xFC,
        0x23,
        0x64,
        0xD1,
        0xFA,
        0xC4,
        0x6B,
        0xDB,
        0x7B };
} LoRaWANParams_t;

LoRaWANParams otaa_credentials;

static void on_console_receive()
{
    char c;
    if (console.read(&c, 1)) {
        lwshell_input(&c, 1);
    }
}

bool hex_string_to_bytes(char *string, uint8_t *buffer, size_t size)
{
    memset(buffer, 0, size);

    for (size_t index = 0; index < size * 2; index++) {
        char c = string[index];
        int value = 0;
        if (c >= '0' && c <= '9')
            value = (c - '0');
        else if (c >= 'A' && c <= 'F')
            value = (10 + (c - 'A'));
        else if (c >= 'a' && c <= 'f')
            value = (10 + (c - 'a'));
        else
            return false;

        buffer[(index / 2)] += value << (((index + 1) % 2) * 4);
    }
    return true;
}

int serial_print(const char *str)
{
    int sent = console.write(str, strlen(str));

    return sent;
}

int serial_printf(const char *fmt, ...)
{
    char output[100];
    va_list args;
    va_start(args, fmt);
    vsprintf(output, fmt, args);
    va_end(args);

    return serial_print(output);
}

static void shell_output(const char *str, struct lwshell *lw)
{
    serial_print(str);
}

int32_t cmd_configure_deveui(int32_t argc, char **argv)
{

    if (strlen(argv[2]) != 16) {
        serial_print("\nIncorrect format of DevEUI\n");
        return 0;
    }

    if (!hex_string_to_bytes(argv[2], otaa_credentials.dev_eui, sizeof(otaa_credentials.dev_eui))) {
        serial_print("\nIncorrect format of DevEUI\n");
        return 0;
    }

    event_queue.call(callback(&lorawan, &Lorawan::configure_otaa_credentials),
            (char *)otaa_credentials.dev_eui,
            lwshell_parse_int(argv[1]));

    serial_print("\nDevEUI OK\n");

    return 0;
}

int32_t cmd_configure_appkey(int32_t argc, char **argv)
{
    if (strlen(argv[2]) != 32) {
        serial_print("\nIncorrect format of AppKEY\n");
        return 0;
    }

    if (!hex_string_to_bytes(argv[2], otaa_credentials.app_key, sizeof(otaa_credentials.app_key))) {
        serial_print("\nIncorrect format of AppKEY\n");
        return 0;
    }

    event_queue.call(callback(&lorawan, &Lorawan::configure_otaa_credentials),
            (char *)otaa_credentials.app_key,
            lwshell_parse_int(argv[1]));

    serial_print("\nAppKEY OK\n");

    return 0;
}

int32_t cmd_configure_appeui(int32_t argc, char **argv)
{
    if (strlen(argv[2]) != 16) {
        serial_print("\nIncorrect format of AppEUI\n");
        return 0;
    }

    if (!hex_string_to_bytes(argv[2], otaa_credentials.app_key, sizeof(otaa_credentials.app_eui))) {
        serial_print("\nIncorrect format of AppEUI\n");
        return 0;
    }

    event_queue.call(callback(&lorawan, &Lorawan::configure_otaa_credentials),
            (char *)otaa_credentials.app_eui,
            lwshell_parse_int(argv[1]));

    serial_print("\nAppEUI OK\n");

    return 0;
}

int main()
{
    lwshell_init();

    lwshell_set_output_fn(shell_output);

    lwshell_register_cmd(
            "deveui", cmd_configure_deveui, "Configure DevEUI as 16 hexadecimal characters");

    lwshell_register_cmd(
            "appkey", cmd_configure_appkey, "Configure AppKEY as 32 hexadecimal characters");

    lwshell_register_cmd(
            "appeui", cmd_configure_appeui, "Configure AppEUI as 16 hexadecimal characters");

    lorawan.run(otaa_credentials.dev_eui, otaa_credentials.app_eui, otaa_credentials.app_key);

    while (true) {
        lorawan.dispatch_with_period(60s);

        string json = "{\"temperature\": " + std::to_string(measurement.get_temperature(2))
                + ", \"pressure\": " + std::to_string(measurement.get_pressure(2))
                + ", \"humidity\": " + std::to_string(measurement.get_humidity(2))
                + ", \"co2\": " + std::to_string(measurement.get_co2()) + "}";

        printf("%s", json.c_str());

        lorawan.send_message(json, json.length(), MBED_CONF_LORA_APP_PORT);
    }

    return 0;
}
