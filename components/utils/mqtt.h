
void initializeMqttTasks();

int publishToMqttTopic(char* text, const char* topic, const int retained, const int qos);
