/*
 * Copyright (c) 2022, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include "mbed.h"

#include "2SMPB02E.h"
#include "as621x.h"
#include "htu21d.h"
#include "scd4x.h"

#include "lorawan.h"

using namespace sixtron;

class Measurement {

public:
    enum class SensorError : uint8_t {
        none = 0,
        scd4x = (1 << 0),
        htu21d = (1 << 1),
        o2smpb02e = (1 << 2),
        as621x = (1 << 3)
    };

    Measurement();
    Measurement(PinName i2c_sda, PinName i2c_scl);
    float get_pressure(int decimal_num);
    double get_temperature(int decimal_num);
    double get_humidity(int decimal_num);
    int get_co2();

private:
    void init();

    O2SMPB02E _o2smpb02e;
    AS621X _as621x;
    HTU21D _htu21d;
    SCD4X _scd4x;

    uint8_t _sensor_error;
};

#endif // MEASUREMENT_H
