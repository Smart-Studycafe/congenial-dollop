import paho.mqtt.client as mqtt
from dotenv import load_dotenv
import os, time, sys
load_dotenv()

class MqttClient():
    def __init__(self):
        self.host = os.getenv("CORE_ENDPOINT")
        self.ca = os.getenv("CORE_CA")
        self.cert = os.getenv("CORE_CERT")
        self.key = os.getenv("CORE_KEY")
        self.client = mqtt.Client()
        self.client.tls_set(self.ca, self.cert, self.key)
    
    def publish(self, device_id):
        topic = f"smartstudy/buzzer/{device_id}"
        self.client.connect(host=self.host, port=8883)
        self.client.loop_start()
        while True:
            time.sleep(1)
            message = '{"buzzflag": 1}'
            result = self.client.publish(topic, message)
            status = result[0]
            if status == 0:
                print(f"Send `{message}` to topic `{topic}`")
                break
            else:
                print(f"Failed to send message to topic {topic}")
        self.client.loop_stop()
        self.client.disconnect()