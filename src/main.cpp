/*
 * Copyright (c) 2022, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mbed.h"

#include "lorawan.h"
#include "measurement.h"

int main()
{
    Measurement meas;
    Lorawan lora;

    while (true) {
        lora.dispatch_with_period(20s);

        string json = "{\"temperature\": " + std::to_string(meas.get_temperature(2))
                + ", \"pressure\": " + std::to_string(meas.get_pressure(2))
                + ", \"humidity\": " + std::to_string(meas.get_humidity(2))
                + ", \"co2\": " + std::to_string(meas.get_co2()) + "}";

        printf("%s", json.c_str());

        lora.send_message(json, json.length(), MBED_CONF_LORA_APP_PORT);
    }

    return 0;
}
