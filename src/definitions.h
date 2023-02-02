// VERSION
#define MAJOR_VERSION 0.01

// Board specific setings
// board name which will be verified in its database. If does not match with the database, db will be reset
#define BOARD_NAME "dice"
// What is this board for? Bed sensor? Relay? Give any kind of name here which will apeear on the web interface
#define APP_NAME "Dice"
#define LED_BUILTIN 2
#define EEPROM_SIZE 1024

// Business values
#define LED_STRIP_PIN 16 // Dice uses WS2812B ledstrip to show light and effects. This pin which the LED strip connected to.

// Software settings
#define SERVER_PORT 80
#define MQTT_MAX_TRY 10 // give the connect up after this amount of tries
#define MQTT_TOPIC "/dice"
#define MQTT_IN_POSTFIX "/in"
#define MQTT_STATUS_ON "{\"status\": \"on\"}"
#define MQTT_STATUS_OFF "{\"status\": \"off\"}"

// DICE COMMNANDS
#define PROPERTY_COMMAND "command"
#define PROPERTY_SPEED "speed"
#define PROPERTY_COUNT "count"
#define PROPERTY_INFINITY "infinity"
#define PROPERTY_COLOR "color"

// Brightness control is skipped under this animation speed
#define BRIGHTNESS_CONTROL_TIME_LIMIT 800


// Network
#define WIFI_MAX_TRY 10

// ERRORS
#define ERROR_NO_ERROR 0
#define ERROR_UNKNOWN 1
#define ERROR_WIFI 2
#define ERROR_MQTT 8

#define COMMAND_CONFIG "config"

// DATABASE PROPERTIES
#define DB_WIFI_NAME "ssid"
#define DB_WIFI_PASSWORD "pw"
#define DB_MQTT_SERVER "mqttserver"
#define DB_MQTT_PORT "mqttport"
#define DB_MQTT_USER "mqttuser"
#define DB_MQTT_PW "mqttpw"
#define DB_MQTT_TOPIC_PREFIX "mqttprefix"
#define DB_VERSION "version"
#define DB_DETAILED_REPORT "detailed"
#define DB_REBOOT_TIMEOUT "reboot"
