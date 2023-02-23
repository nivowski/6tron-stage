/*
 * Copyright (c) 2022, CATIE
 * SPDX-License-Identifier: Apache-2.0
 */

#include "measurement.h"

inline double round_up(double value, int num_decimal)
{
    return ceil(value * pow(10.0, num_decimal)) / pow(10.0, num_decimal);
}

Measurement::Measurement():
        _o2smpb02e(P1_I2C_SDA, P1_I2C_SCL),
        _as621x(P1_I2C_SDA,
                P1_I2C_SCL,
                sixtron::AS621X::Add1Pin::PullUp_VDD,
                sixtron::AS621X::Add0Pin::VSS),
        _htu21d(P1_I2C_SDA, P1_I2C_SCL),
        _scd4x(P1_I2C_SDA, P1_I2C_SCL),
        _sensor_error((uint8_t)SensorError::none)

{
    init();
}

Measurement::Measurement(PinName i2c_sda, PinName i2c_scl):
        _o2smpb02e(i2c_sda, i2c_scl),
        _as621x(i2c_sda,
                i2c_scl,
                sixtron::AS621X::Add1Pin::PullUp_VDD,
                sixtron::AS621X::Add0Pin::VSS),
        _htu21d(i2c_sda, i2c_scl),
        _scd4x(i2c_sda, i2c_scl)

{
    init();
}

void Measurement::init()
{
    SCD4X::ErrorType scd4x_err;
    HTU21D::ErrorType htu21d_err;
    htud21d_user_reg_t htu21d_reg;
    AS621X::ErrorType as621x_err;
    AS621X::Config_t as621x_config;

    ThisThread::sleep_for(500ms);
    if (!_o2smpb02e.init()) {
        _sensor_error |= (uint8_t)SensorError::o2smpb02e;
    } else {
        _o2smpb02e.start_periodic_measurement();
    }

    as621x_err = _as621x.read_config(&as621x_config);
    if (as621x_err != AS621X::ErrorType::Ok) {
        _sensor_error |= (uint8_t)SensorError::as621x;
    }

    _htu21d.soft_reset();
    ThisThread::sleep_for(15ms);
    htu21d_reg.resolution = MeasurementResolution::RH_12_Temp_14;
    htu21d_err = _htu21d.write_user_register(&htu21d_reg);
    if (htu21d_err != HTU21D::ErrorType::Ok) {
        _sensor_error |= (uint8_t)SensorError::htu21d;
    }

    scd4x_err = _scd4x.stop_periodic_measurement();
    ThisThread::sleep_for(500ms);
    if (scd4x_err != SCD4X::ErrorType::Ok) {
        _sensor_error |= (uint8_t)SensorError::scd4x;
    } else {
        scd4x_err = _scd4x.set_automatic_calibration_enabled(true);
        ThisThread::sleep_for(1ms);
        scd4x_err = _scd4x.start_periodic_measurement();
    }

    if (_sensor_error != (uint8_t)SensorError::none) {
        printf("Sensor Error: %d\n", _sensor_error);
    }
}

float Measurement::get_pressure(int decimal_num)
{
    float pressure = _o2smpb02e.pressure();

    if (pressure >= 0) {
        pressure = pressure / 100.0F;
        printf("Pressure: %.0f hPa\n", pressure);
        return round_up((double)pressure, decimal_num);
    }
}

double Measurement::get_temperature(int decimal_num)
{
    double temperature = 0.0f;

    if (_as621x.read_temperature(AS621X::RegisterAddress::Tval, &temperature)
            == AS621X::ErrorType::Ok) {
        printf("Temperature: %.1f°C\n", temperature);
        return round_up(temperature, decimal_num);
    }
}

double Measurement::get_humidity(int decimal_num)
{
    double humidity = 0.0f;

    if (_htu21d.read_humidity(&humidity) == HTU21D::ErrorType::Ok) {
        printf("Humidity: %.0f %%RH\n", humidity);
        return round_up(humidity, decimal_num);
    }
}

int Measurement::get_co2()
{
    static scd4x_measurement_t scd4x_data;

    if (_scd4x.read_measurement(&scd4x_data) == SCD4X::ErrorType::Ok) {
        printf("CO2: %d ppm\n", scd4x_data.co2);
    }
    return scd4x_data.co2;
}
