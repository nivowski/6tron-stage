import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import base64
import json
import os

CONDUIT_BROKER_IP = "127.0.0.1"
CONDUIT_SUBSCRIBE_TOPIC = "lora/+/up"
THINGSBOARD_BROKER_DOMAIN = "demo.thingsboard.io"
THINGSBOARD_PUBLISH_TELEMETRY_TOPIC = "v1/gateway/telemetry"

APPEUI = "fe-ff-ff-ff-fd-ff-12-34"

THINGSBOARD_GATEWAY_ACCESS_TOKEN = "HTfr13dE822RTmkZZGsw"

def on_connect(client, userdata, flags, rc):
    print("Device connected with result code: " + str(rc))
    print("Error is ", mqtt.error_string(rc))

    if rc == mqtt.MQTT_ERR_SUCCESS:
        client.subscribe(CONDUIT_SUBSCRIBE_TOPIC)

        status = open("./status.json", "w")
        statusString = (
            '{"pid":'
            + str(os.getpid())
            + ',"AppInfo": "'
            + str(client._client_id)
            + ' client connected"}'
        )
        status.write(statusString)


def on_message(client, userdata, msg):
    json_data = json.loads(msg.payload)
    app_eui = json_data["appeui"]
    if app_eui == APPEUI:
        publish_measurements(json_data)    

def publish_measurements(json_frame):
    deveui = json_frame["deveui"]
    data = '{"' + deveui + '": [' + base64.b64decode(json_frame["data"]).decode("utf-8") + "]}"
    print(data)
    publish.single(
                topic=THINGSBOARD_PUBLISH_TELEMETRY_TOPIC,
                payload=data,
                hostname=THINGSBOARD_BROKER_DOMAIN,
                port=1883,
                auth={'username': THINGSBOARD_GATEWAY_ACCESS_TOKEN},
            )


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(CONDUIT_BROKER_IP, 1883, 60)

client.loop_forever()
