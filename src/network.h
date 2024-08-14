#include <ArduinoJson.h>
#include "device_state.h"

void init_wifi();
String create_session_token();
DeviceState get_device_state(const String &sessionToken);