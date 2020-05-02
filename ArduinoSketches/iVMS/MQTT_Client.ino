WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server = "192.168.0.127";
long lastMsg = 0;
char msg[50];
int value = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void mqtt_client_publish(enumSensorDataType dataType, char* msg)
{
  char topicName[200];
  sprintf(topicName,"%s/%s/%s/%s/%s",g_devCfg.hospitalName,g_devCfg.buildingNum,g_devCfg.floorNum,g_devCfg.roomNum,g_devCfg.bedNum);
  switch(dataType)
  {
    case MAX30102_HEART_RATE:
      sprintf(topicName,"%s/heartRate",topicName);
      break;
    case MAX30102_SP02:
      sprintf(topicName,"%s/sp02",topicName);
      break;
    default:
      break;
  }

  Serial.print("Updating topic:");
  Serial.println(topicName);
  client.publish(topicName, msg);
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a =client ID with mac address
    String clientId = "ESP8266Client-"; 
    clientId += macToStr(g_devCfg.mac);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqtt_client_publish(MAX30102_HEART_RATE,"0");
      mqtt_client_publish(MAX30102_SP02,"0");
      //client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqtt_setup() {
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void mqtt_loop(){
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  char val[4];
  sprintf (val, "%03i", max30102_getHeartRate());
  mqtt_client_publish(MAX30102_HEART_RATE,val);
  sprintf (val, "%03i", max30102_getSpO2());
  mqtt_client_publish(MAX30102_SP02,val);
}
