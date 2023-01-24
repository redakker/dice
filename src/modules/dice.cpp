#ifndef DICE
#define DICE

#include <cstddef>
#include "Callback.h"
#include "definitions.h"
#include "log.cpp"
#include "mqtt.cpp"
#include <FastLED.h>
#include <LinkedList.h>

// Number of the dice's points
#define NUM_LEDS 21
#define MAX_POWER_MILLIAMPS 10
#define LED_TYPE            WS2812B
#define COLOR_ORDER         GRB

class Dice {

    Log* rlog;
    Signal<MQTTMessage>* message;
    String log_prefix = "[DICE] ";

    static const uint8_t DATA_PIN = LED_STRIP_PIN;

    // Create array for the dice points
    CRGB leds[NUM_LEDS];    

    // Variables which can be modified from outside    
    int animationSpeed = 200;
    int animationCount = 5;
    bool infinityAnimation = false;
    String currentColor = "#FF1493"; // CRGB::DeepPink;


    // Command set which is available from outside
    enum command {
        singleColor, // set all pixels to actual color
        rollTheDice, // display a random number on the dice
        rollTheDiceAnimated, // count up the dice numbers, slow down and finally stop at a random number
        orderRun, // count up the dice numbers
        ReverseOrderRun, // count down the dice numbers
    };

    // Helper variables
    command currentCommand = singleColor;
    int currentDiceNumber = 1; // Always shows the real number (not an array style)
    int lastAnimationTime = 0; // Handle the delay with this (avoid delay() function)
    
    public:
        Dice(Log &rlog) {
            this -> rlog = &rlog;
        }

        void setup (Signal<MQTTMessage> &message) {

            this -> message = &message;

            delay(3000); // 3 second delay for boot recovery, and a moment of silence
            FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
            FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MILLIAMPS);

            this -> rlog -> log(log_prefix, "Dice is ready");
            
        }

        void loop() {

            // Always give some time to Fastled to show
            if (animationSpeed < 10) {
                animationSpeed = 10;
            }

            if (animationCount > 0 || infinityAnimation) {
                
                if (millis() - lastAnimationTime > animationSpeed) {
                    
                    lastAnimationTime = millis();

                    // Strip does the animation which is in the command
                    switch (currentCommand)
                    {
                    case singleColor:
                        colorizeLedStrip();
                        break;
                    case rollTheDice:
                        rollDice();
                        break;
                    case rollTheDiceAnimated:
                        rollDiceAnimated();
                        break;
                    case orderRun:
                        countUp();
                        setDiceNumber(currentDiceNumber);
                        break;
                    case ReverseOrderRun:
                        countDown();
                        setDiceNumber(currentDiceNumber);
                        break;
                    
                    default:
                        resetLedStrip();
                        break;
                    }

                    FastLED.show();                    
                    
                    // Reduce the animation count after animation
                    if (!infinityAnimation) {
                        animationCount--;
                    }
                }                
            }

        }

        void receiveCommand(String message) {

            StaticJsonDocument<1000> tempJson; 
            DeserializationError error = deserializeJson(tempJson, message);

            this -> rlog -> log(log_prefix, "Message reseived: " + message);

            if (error) {
                rlog -> log(log_prefix + (String) "DeserializationError: " + error.c_str() + " (receiveCommand) " + message);
            } else {

                if (tempJson.containsKey(PROPERTY_COMMAND)) {
                    currentCommand = (command) tempJson[PROPERTY_COMMAND].as<int>();
                    
                    resetLedStrip();
                    this -> rlog -> log(log_prefix, "New command: " + tempJson[PROPERTY_COMMAND].as<String>());
                } else {
                this -> rlog -> log(log_prefix, "Command was not valid");
                }

                if (tempJson.containsKey(PROPERTY_SPEED)) {
                    animationSpeed = tempJson[PROPERTY_SPEED].as<int>();
                    this -> rlog -> log(log_prefix, "New speed: " + (String) animationSpeed);
                }

                if (tempJson.containsKey(PROPERTY_COUNT)) {
                    animationCount = tempJson[PROPERTY_COUNT].as<int>();
                    this -> rlog -> log(log_prefix, "New count: " + (String) animationCount);
                }

                if (tempJson.containsKey(PROPERTY_INFINITY)) {
                    infinityAnimation = tempJson[PROPERTY_INFINITY].as<bool>();
                    this -> rlog -> log(log_prefix, "Infinity: " + (String) animationSpeed);
                }

                if (tempJson.containsKey(PROPERTY_COLOR)) {
                    currentColor = tempJson[PROPERTY_COLOR].as<String>();
                    this -> rlog -> log(log_prefix, "Color: " + (String) currentColor);
                }
            }
           
        }

    private:

    void colorizeLedStrip() {
        resetLedStrip();
        for (int i=0; i < NUM_LEDS; i++ ){
            leds[i].setColorCode(getColorAsNumber(currentColor));
            leds[i].fadeToBlackBy(10);
        }
    }

    void rollDiceAnimated() {
        
        int randNum = random(1,7);
        while (currentDiceNumber == randNum) {
            randNum = random(1,7);
        }
        currentDiceNumber = randNum;
        setDiceNumber(currentDiceNumber);        
    }

    // Random number of the dice
    void rollDice() {
        int randNum = random(1,7);
        setDiceNumber(randNum);
    }

    // Count the number of the dice UP
    void countUp() {
        if (currentDiceNumber >= 6) {
            currentDiceNumber = 1;
        } else {
            currentDiceNumber++;
        }
    }

    // Count the number of the dice UP
    void countDown() {
        if (currentDiceNumber <= 1) {
            currentDiceNumber = 6;
        } else {
            currentDiceNumber--;
        }
    }

    // Set the right LED(s) on the strip to show the dice numbers
    void setDiceNumber(int number) {

        if (number > 6) {
            number = 6;
        }

        if (number < 1) {
            number = 1;
        }

        resetLedStrip();

        switch (number) {
            case 1:
                leds[0].setColorCode(getColorAsNumber(currentColor));
            break;
            case 2:
                leds[1].setColorCode(getColorAsNumber(currentColor));
                leds[2].setColorCode(getColorAsNumber(currentColor));
            break;
            case 3:
                leds[3].setColorCode(getColorAsNumber(currentColor));
                leds[4].setColorCode(getColorAsNumber(currentColor));
                leds[5].setColorCode(getColorAsNumber(currentColor));
            break;
            case 4:
                leds[6].setColorCode(getColorAsNumber(currentColor));
                leds[7].setColorCode(getColorAsNumber(currentColor));
                leds[8].setColorCode(getColorAsNumber(currentColor));
                leds[9].setColorCode(getColorAsNumber(currentColor));
            break;
            case 5:
                leds[10].setColorCode(getColorAsNumber(currentColor));
                leds[11].setColorCode(getColorAsNumber(currentColor));
                leds[12].setColorCode(getColorAsNumber(currentColor));
                leds[13].setColorCode(getColorAsNumber(currentColor));
                leds[14].setColorCode(getColorAsNumber(currentColor));
            break;
            case 6:
                leds[15].setColorCode(getColorAsNumber(currentColor));
                leds[16].setColorCode(getColorAsNumber(currentColor));
                leds[17].setColorCode(getColorAsNumber(currentColor));
                leds[18].setColorCode(getColorAsNumber(currentColor));
                leds[19].setColorCode(getColorAsNumber(currentColor));
                leds[20].setColorCode(getColorAsNumber(currentColor));
            break;
        }
    }

    // Set all LEDs to blak (switch them off)
    void resetLedStrip() {
        for (int i=0; i < NUM_LEDS; i++ ){
            leds[i] = CRGB::Black;
        }
        // FastLED.clear();
    }

    uint32_t getColorAsNumber(String colorCode) { //incoming looks like this -> #00FF00       
        String colorString = "0x" + colorCode.substring(1); // remove #
        uint32_t color = strtol(colorString.c_str(), NULL, 16);
        return color;
    }
     
};

#endif