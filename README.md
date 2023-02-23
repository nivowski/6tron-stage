# FMLR Forwarder

Application integrated into the Multitech Conduit LoRaWAN gateway.

## General

- Application subscribes to the local MQTT broker topic `lora/+/up`
- When a message is received, it parses the AppEUI to determine the origin of
  the message:
    - CATIE measurement system payload is directly published to ThingsBoard

## Configuration

- `CONDUIT_BROKER_IP` represents the IP of the Conduit broker: it should be
  `127.0.0.1` if deployed on the gateway but can be changed if ran on another
  system on the same network
- `THINGSBOARD_GATEWAY_ACCESS_TOKEN` represents the token to access the device running as the gateway

## Deployment

Generate archive of release `x.y.y` with:
```shell
git archive --format=tar.gz -o 3SqAir_LoRaWAN_forwarder-vx.y.z.tar.gz x.y.z
```

Upload app on DeviceHQ in the Developer tab.

Schedule the install app task in on DeviceHQ in the Devices tab.
