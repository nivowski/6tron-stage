# IAQ Sensor Demo
Mbed OS application measuring interior air quality.

## Requirements
### Hardware requirements
The following boards are required:
- Zest_Core_FMLR-72
- Zest_Sensor_T-RH
- Zest_Sensor_CO2

### Software requirements
IAQ Sensor Demo makes use of the following libraries (automatically
imported by `mbed deploy` or `mbed import`):
- zest-core-fmlr-72
- 2smpb-02e
- ams-as621x
- htu21d
- scd4x

## Usage
To clone **and** deploy the project in one command, use `mbed import` and skip to the
target enabling instructions:
```shell
mbed import https://gitlab.com/catie_6tron/iaq-sensor-demo.git iaq-sensor-demo
```

Alternatively:

- Clone to "iaq-sensor-demo" and enter it:
  ```shell
  git clone https://gitlab.com/catie_6tron/iaq-sensor-demo.git iaq-sensor-demo
  cd iaq-sensor-demo
  ```

- Deploy software requirements with:
  ```shell
  mbed deploy
  ```

Enable the custom target:
```shell
cp zest-core-fmlr-72/custom_targets.json .
```

Compile the project:
```shell
mbed compile
```

Program the target device with a Segger J-Link debug probe and
[`sixtron_flash`](https://gitlab.com/catie_6tron/6tron-flash) tool:
```shell
sixtron_flash stm32l071rz BUILD/ZEST_CORE_FMLR-72/GCC_ARM/iaq-sensor-demo.elf
```

Debug on the target device with the probe and Segger
[Ozone](https://www.segger.com/products/development-tools/ozone-j-link-debugger)
software.
