#ifndef DICE
#define DICE

#include <cstddef>
#include "Callback.h"
#include "definitions.h"
#include "log.cpp"
#include "mqtt.cpp"
#include <FastLED.h>

// Number of the dice's points
#define NUM_LEDS 21
#define MAX_POWER_MILLIAMPS 1500
#define LED_TYPE            WS2812B
#define COLOR_ORDER         GRB

class Dice {

    Log* rlog;
    Signal<MQTTMessage>* message;
    String log_prefix = "[DICE] ";

    static const uint8_t DATA_PIN = LED_STRIP_PIN;

    // Create array for the dice points
    CRGB leds[NUM_LEDS];

    public:
        Dice(Log &rlog) {
            this -> rlog = &rlog;
        }

        void setup (Signal<MQTTMessage> &message) {

            this -> message = &message;

            delay( 3000); // 3 second delay for boot recovery, and a moment of silence
            FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
            FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MILLIAMPS);
          
            this -> rlog -> log(log_prefix, "Dice is ready");
        }

        void loop() {   

            leds[0] = CRGB::White; FastLED.show(); delay(30); 
            leds[0] = CRGB::Black; FastLED.show(); delay(30);

        }

        void receiveCommand(String command) {
           
        }

    private:
     
};

#endif