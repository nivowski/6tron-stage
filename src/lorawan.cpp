/*
 * Copyright (c) 2022, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#include "lorawan.h"

namespace {
#define CONFIRMED_MSG_RETRY_COUNTER 3

}

Lorawan::Lorawan(EventQueue *event_queue):
        _evt_queue(event_queue),
        _radio(MBED_CONF_SX1272_LORA_DRIVER_SPI_MOSI,
                MBED_CONF_SX1272_LORA_DRIVER_SPI_MISO,
                MBED_CONF_SX1272_LORA_DRIVER_SPI_SCLK,
                MBED_CONF_SX1272_LORA_DRIVER_SPI_CS,
                MBED_CONF_SX1272_LORA_DRIVER_RESET,
                MBED_CONF_SX1272_LORA_DRIVER_DIO0,
                MBED_CONF_SX1272_LORA_DRIVER_DIO1,
                MBED_CONF_SX1272_LORA_DRIVER_DIO2,
                MBED_CONF_SX1272_LORA_DRIVER_DIO3,
                MBED_CONF_SX1272_LORA_DRIVER_DIO4,
                MBED_CONF_SX1272_LORA_DRIVER_DIO5,
                MBED_CONF_SX1272_LORA_DRIVER_RF_SWITCH_CTL1,
                MBED_CONF_SX1272_LORA_DRIVER_RF_SWITCH_CTL2,
                MBED_CONF_SX1272_LORA_DRIVER_TXCTL,
                MBED_CONF_SX1272_LORA_DRIVER_RXCTL,
                MBED_CONF_SX1272_LORA_DRIVER_ANT_SWITCH,
                MBED_CONF_SX1272_LORA_DRIVER_PWR_AMP_CTL,
                MBED_CONF_SX1272_LORA_DRIVER_TCXO),
        _lorawan(new LoRaWANInterface(_radio)),
        _lorawan_status(LORAWAN_STATUS_OK),
        _buffer(new LoRaWANBuffer()),
        _spif(P1_SPI_MOSI, P1_SPI_MISO, P1_SPI_SCK, PB_13)

{
    init();

    initialize_external_flash();
}

void Lorawan::init()
{

    // Initialize LoRaWAN stack
    if (_lorawan->initialize(_evt_queue) != LORAWAN_STATUS_OK) {
        printf("\n LoRa initialization failed!\n");
        return;
    }

    printf("\n Mbed LoRaWANStack initialized\n");

    // prepare application callbacks
    _lorawan_callbacks.events = mbed::callback(this, &Lorawan::lora_event_handler);
    _lorawan->add_app_callbacks(&_lorawan_callbacks);

    // Set number of retries in case of CONFIRMED messages
    if (_lorawan->set_confirmed_msg_retries(CONFIRMED_MSG_RETRY_COUNTER) != LORAWAN_STATUS_OK) {
        printf("\n set_confirmed_msg_retries failed!\n");
        return;
    }

    printf("\n CONFIRMED message retries : %d \n", CONFIRMED_MSG_RETRY_COUNTER);

    // Enable adaptive data rate
    if (_lorawan->enable_adaptive_datarate() != LORAWAN_STATUS_OK) {
        printf("\n enable_adaptive_datarate failed!\n");
        return;
    }

    printf("\n Adaptive data  rate (ADR) - Enabled\n");

    // _lorawan_status = _lorawan->connect();

    // if (_lorawan_status == LORAWAN_STATUS_OK
    //         || _lorawan_status == LORAWAN_STATUS_CONNECT_IN_PROGRESS) {
    // } else {
    //     printf("\n Connection error, code = %d\n", _lorawan_status);
    //     return;
    // }

    // printf("\n Connection - In Progress ...\n");
}

void Lorawan::run(uint8_t *dev_eui, uint8_t *app_eui, uint8_t *app_key)
{
    _connect_params.connect_type = LORAWAN_CONNECTION_OTAA;
    _connect_params.connection_u.otaa.dev_eui = dev_eui;
    _connect_params.connection_u.otaa.app_eui = app_eui;
    _connect_params.connection_u.otaa.app_key = app_key;
    _connect_params.connection_u.otaa.nb_trials = MBED_CONF_LORA_NB_TRIALS;

    connect();
}
void Lorawan::configure_otaa_credentials(char *buffer, uint64_t addr)
{
    _spif.erase(addr * 4096, _spif.get_erase_size());
    _spif.program(buffer, addr * 4096, _spif.get_erase_size() / 8);
}

void Lorawan::connect()
{
    _lorawan_status = _lorawan->connect(_connect_params);

    if (_lorawan_status == LORAWAN_STATUS_OK
            || _lorawan_status == LORAWAN_STATUS_CONNECT_IN_PROGRESS) {
        printf("\n Connection - In Progress ...\n");
    } else if (_lorawan_status == LORAWAN_STATUS_ALREADY_CONNECTED) {
        printf("\n Connection - Already connected! \n");
    } else {
        printf("\n Connection error, code = %d \n", _lorawan_status);
    }
}

void Lorawan::lora_event_handler(lorawan_event_t event)
{
    switch (event) {
        case CONNECTED:
            printf("\r\n Connection - Successful \r\n");
            break;
        case DISCONNECTED:
            _evt_queue->break_dispatch();
            printf("\r\n Disconnected Successfully \r\n");
            break;
        case TX_DONE:
            printf("\r\n Message Sent to Network Server \r\n");
            break;
        case TX_TIMEOUT:
        case TX_ERROR:
        case TX_CRYPTO_ERROR:
        case TX_SCHEDULING_ERROR:
            printf("\r\n Transmission Error - EventCode = %d \r\n", event);
            break;
        case RX_DONE:
            printf("\r\n Received message from Network Server \r\n");
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
            printf("\r\n Error in reception - Code = %d \r\n", event);
            break;
        case JOIN_FAILURE:
            printf("\r\n OTAA Failed - Check Keys \r\n");
            break;
        case UPLINK_REQUIRED:
            printf("\r\n Uplink required by NS \r\n");
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }
}

void Lorawan::send_message(string message, uint16_t length, int port)
{
    _buffer->port = port;
    memcpy(_buffer->data, message.c_str(), length);
    _buffer->data_length = length;

    _evt_queue->call(this, &Lorawan::transmit);
}

void Lorawan::transmit()
{
    int16_t retcode;

    retcode = _lorawan->send(
            _buffer->port, _buffer->data, _buffer->data_length, MSG_CONFIRMED_FLAG);

    if (retcode < 0) {
        retcode == LORAWAN_STATUS_WOULD_BLOCK ? printf("\nsend - WOULD BLOCK\n")
                                              : printf("\n send() - Error code %d \n", retcode);
        return;
    }

    printf("\n %d bytes scheduled for transmission \n", retcode);
}

void Lorawan::dispatch_with_period(std::chrono::duration<int, std::milli> period)
{
    _evt_queue->dispatch_for(period);
}

void Lorawan::initialize_external_flash()
{
    _spif.init();
}
