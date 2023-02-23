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

using namespace events;

class Lorawan {

public:
    Lorawan();

    typedef struct LoRaWANBuffer {
        uint8_t data[100];
        uint16_t data_length;
        uint8_t port;
    } LoRaWANBuffer_t;

    void lora_event_handler(lorawan_event_t event);
    void send_message(string message, uint16_t length, int port);

    void dispatch_with_period(std::chrono::duration<int, std::milli> period);

private:
    void init();

    void transmit();

    EventQueue *_evt_queue;
    LoRaWANInterface *_lorawan;
    lorawan_app_callbacks_t _lorawan_callbacks;
    lorawan_status_t _lorawan_status;
    LoRaWANBuffer_t *_buffer;
};

#endif // MEASUREMENT_H
