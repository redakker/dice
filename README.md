# Dice

This is a code for a Dice light project. We are making dice-shaped light which has built-in LED light at each and every point on its sides. It makes the possibility to create creative light with that and has functions which are also useful if you want to play board games. Of course, this is not the primary goal. :)

If you want to follow the (hardware making) project in pictures then please check the instagram page: https://www.instagram.com/dicelightproject/

## Code

You can build this code in VS code with a PlatformIO plugin in it.

## Hardware (electrocics)

You will need:
 - ESP32 board
 - WS2812B LED strip with 21 leds in it
 - Power source (5V)
 
 ## Control
 
At the first start the program offers a wifi Access Point which has name Dice. Connect to this AP and you will end up on a captive portal (phone offers it). If not you can reach the configuration page in the IP address 192.168.4.1

On this webpage with filling the form with the right values you can set up the device and connect it to your WiFi network and MQTT server.

### MQTT

Once is connected to your network and MQTT server you can send commands to the dice to control its light.

Default MQTT topic is: ```/dice/in```

Template for the command (JSON object) with all available properties. Can be sent but not all applied at the same time.

```
{
command:"singleColor",
number: 3,
speed: 1000,
count: 5,
infinity: false,
color: "#FF0000"
}
```

#### Properties and values

Possible values of the command property (features):

- showNumber -> Show a number on a dice 1-6
- singleColor -> set all pixels to actual color
- rollTheDice -> display a random number on the dice
- rollTheDiceAnimatedToSpinUp -> random dice numbers, spin up and finally stop at a random number
- rollTheDiceAnimatedToSlowDown -> random dice numbers, slow down and finally stop at a random number
- orderRun -> count up the dice numbers
- reverseOrderRun -> count down the dice numbers

Some commands are ignore properties from the following list. For example rollTheDice will skip the number property.

Other properties:

 - number -> show a specific number on a dice
 - speed -> animation speed
 - count -> animation count after this number of animation it stops and show the actual light setting
 - infinity -> infinity animation (animation count will be skipped)
 - color -> color of the lit leds


### Bluetooth

This function is not fully tested yet.
