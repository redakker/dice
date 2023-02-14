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
    float animationSpeed = 200.0;
    int animationCount = 1;
    bool infinityAnimation = false;
    String currentColor = "#FF8000";

    // Command set which is available from outside
    enum Command {
        showNumber, // Show a number on a dice 1-6
        singleColor, // set all pixels to actual color
        rollTheDice, // display a random number on the dice
        rollTheDiceAnimatedToSpinUp, // random dice numbers, spin up and finally stop at a random number
        rollTheDiceAnimatedToSlowDown, // random dice numbers, slow down and finally stop at a random number
        orderRun, // count up the dice numbers
        reverseOrderRun, // count down the dice numbers
        error // in case of error, show some noticable light
    };

    Command commandConvert(const String& str)
    {
        if(str == "showNumber") { return showNumber; }
        if(str == "singleColor") { return singleColor; }
        if(str == "rollTheDice") { return rollTheDice; }
        if(str == "rollTheDiceAnimatedToSpinUp") { return rollTheDiceAnimatedToSpinUp; }
        if(str == "rollTheDiceAnimatedToSlowDown") { return rollTheDiceAnimatedToSlowDown; }
        if(str == "orderRun") { return orderRun; }
        if(str == "reverseOrderRun") { return reverseOrderRun; }

        // else
        return error;
    }

    // Helper variables
    Command currentCommand = singleColor;
    int currentDiceNumber = 1; // Always shows the real number (not an array style)
    int lastAnimationTime = 0; // Handle the delay with this (avoid delay() function)
    int lastFadeTime = 0; // Fade up light animation timer
    float brightness = 0; //Use this value to "spin up" slow the light (increase brightness during the loop)
    int ceilBrightness = 255; // This value is for control, the user brightness. The maximum value of amount of light
    
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

            // Fade up light
            // But if we want to have speed motion, skip the fade up mechanism
            if (animationSpeed > BRIGHTNESS_CONTROL_TIME_LIMIT) {
                if (millis() - lastFadeTime > 20) {
                    if (brightness == 0) {
                        brightness = 1.2;
                    }

                    lastFadeTime = millis();
                    brightness = brightness * 1.2;
                    if (brightness > ceilBrightness) {
                        brightness = ceilBrightness;
                    }
                }
            } else {
                brightness = ceilBrightness;
            }

            if ((animationCount > 0 || infinityAnimation) && brightness == 255) {
                if (millis() - lastAnimationTime > animationSpeed) {                    
                    lastAnimationTime = millis();

                    // Strip does the animation which is in the command
                    switch (currentCommand)
                    {
                        case showNumber:
                            setDiceNumber(currentDiceNumber);
                            break;
                        case singleColor:
                            colorizeLedStrip();
                            break;
                        case rollTheDice:
                            rollDice();
                            break;
                        case rollTheDiceAnimatedToSpinUp:
                            rollDiceAnimatedToSpinUp();
                            break;
                        case rollTheDiceAnimatedToSlowDown:
                            rollDiceAnimatedToSlowDown();
                            break;
                        case orderRun:
                            countUp();
                            setDiceNumber(currentDiceNumber);
                            break;
                        case reverseOrderRun:
                            countDown();
                            setDiceNumber(currentDiceNumber);
                            break;
                        case error:
                            errorLight();                            
                            break;
                        
                        default:
                            resetLedStrip();
                            break;
                    }
                    
                    // Reduce the animation count after animation
                    if (!infinityAnimation) {
                        animationCount--;
                    }
                    // Need for the fade up to star the animation from "black"
                    brightness = 0;                   
                }
            }
            
            FastLED.setBrightness(brightness);
            FastLED.show();
        }
  

        void receiveCommand(String message) {

            StaticJsonDocument<1000> tempJson; 
            DeserializationError Derror = deserializeJson(tempJson, message);

            this -> rlog -> log(log_prefix, "Message reseived: " + message);

            if (Derror) {
                rlog -> log(log_prefix + (String) "DeserializationError: " + Derror.c_str() + " (receiveCommand) " + message);
                currentCommand = error;
            } else {

                if (tempJson.containsKey(PROPERTY_COMMAND)) {

                    // Try to get a command even if that is a number or string value (both valid)
                    String c = tempJson[PROPERTY_COMMAND].as<String>();
                    if (is_number(c)) {
                        currentCommand = (Command) tempJson[PROPERTY_COMMAND].as<int>();
                    } else {
                        currentCommand = commandConvert(c);
                    }
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

                if (tempJson.containsKey(PROPERTY_NUMBER)) {
                    currentDiceNumber = tempJson[PROPERTY_NUMBER].as<int>();
                    this -> rlog -> log(log_prefix, "Number: " + (String) currentDiceNumber);
                }

                if (tempJson.containsKey(PROPERTY_NUMBER)) {
                    currentDiceNumber = tempJson[PROPERTY_BRIGHTNESS].as<int>();
                    this -> rlog -> log(log_prefix, "Brightness: " + (String) ceilBrightness);
                }

                
            }
           
        }

    private:

    // Colorize the led to red and flash it
    void errorLight() {
        animationSpeed = 300;
        infinityAnimation = true;
        
        if (currentColor != "#FF0000") {
            currentColor = "#FF0000";
        } else {
            currentColor = "#000000";
        }
        colorizeLedStrip();
    }

    void colorizeLedStrip() {
        resetLedStrip();
        for (int i=0; i < NUM_LEDS; i++ ){
            leds[i].setColorCode(getColorAsNumber(currentColor));            
        }
    }

    void rollDiceAnimatedToSpinUp() {
        
        int randNum = random(1,7);
        while (currentDiceNumber == randNum) {
            randNum = random(1,7);
        }
        currentDiceNumber = randNum;
        animationSpeed = animationSpeed * 0.7;
        
        if (animationSpeed < 100) {
            animationSpeed = 100;
        }
        
        setDiceNumber(currentDiceNumber);        
    }

    void rollDiceAnimatedToSlowDown() {
        int randNum = random(1,7);
        while (currentDiceNumber == randNum) {
            randNum = random(1,7);
        }
        currentDiceNumber = randNum;
        animationSpeed = animationSpeed * 1.05;
        
        if (animationSpeed < 50) {
            animationSpeed = 50;
        }

        // We don't want to wait forever for the next number
        if (animationSpeed > 3000) {
            animationSpeed = 3000;
        }

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

    bool is_number(const String& s)
    {
        return !s.isEmpty() && std::find_if(s.begin(), 
            s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
    }
     
};

#endif