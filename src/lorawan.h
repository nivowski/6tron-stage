/*
 * Copyright (c) 2022, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LORAWAN_H
#define LORAWAN_H

#include "mbed.h"

#include "EventQueue.h"
#include "SX1272_LoRaRadio.h"
#include "lorawan/LoRaRadio.h"
#include "lorawan/LoRaWANInterface.h"

#include "SPIFBlockDevice.h"

using namespace events;

class Lorawan {

public:
    Lorawan(EventQueue *event_queue);

    typedef struct LoRaWANBuffer {
        uint8_t data[100];
        uint16_t data_length;
        uint8_t port;
    } LoRaWANBuffer_t;

    void lora_event_handler(lorawan_event_t event);

    void connect();
    void send_message(string message, uint16_t length, int port);

    void dispatch_with_period(std::chrono::duration<int, std::milli> period);

    void configure_otaa_credentials(char *buffer, uint64_t addr);

    void run(uint8_t *dev_eui, uint8_t *app_eui, uint8_t *app_key);

private:
    void init();

    void transmit();

    void initialize_external_flash();

    EventQueue *_evt_queue;
    SX1272_LoRaRadio _radio;
    LoRaWANInterface *_lorawan;
    lorawan_app_callbacks_t _lorawan_callbacks;
    lorawan_status_t _lorawan_status;
    lorawan_connect_t _connect_params;
    LoRaWANBuffer_t *_buffer;

    SPIFBlockDevice _spif;
};

#endif // MEASUREMENT_H
