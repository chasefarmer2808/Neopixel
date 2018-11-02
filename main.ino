/************************************************************
 *      ADAFRUIT NEOPIXEL PROJECT HALLOWEEN 2018
 * 
 * Hardware:
 *  - Adafruit Metro Mini 5V Microcontroller
 *  - 1 meter of mini skinny RGB Neopixels
 *  - 300 Ohm resistor
 *  - 1 Farad capacitor
 *  - 13000 mAh DC5V rechargable battery pack with 2 USB-A ports
 *************************************************************/


#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
    #include <avr/power.h>
#endif

#define DATA_PIN 12    // Connects to the data pin on the Neopixels.
#define BUTTON_PIN 3   // Digital input to the push button acting as an external interrupt.
#define LED_PIN 13     // Debug LED pin build into the Adafruit Metro Mini.
#define NUM_PIXELS 48  // Number of pixels you want to activate
#define BRIGHTNESS 50  // Overall brightness level of the pixels.  0 - 255

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, DATA_PIN, NEO_GRB + NEO_KHZ800);

int START = 0;
bool flipFlag = true;
volatile int patternSelect = 0;
volatile bool interruptFlag = false;

uint32_t red = strip.Color(255, 0, 0);
uint32_t green = strip.Color(0, 255, 0);
uint32_t blue = strip.Color(0, 0, 255);
uint32_t magenta = strip.Color(255, 0, 255);
uint32_t cyan = strip.Color(0, 255, 255);
uint32_t yellow = strip.Color(255, 255, 0);
uint32_t halloweenOrange = strip.Color(255, 100, 0);
uint32_t halloweenPurple = strip.Color(70, 6, 69);
uint32_t gatorBlue = strip.Color(0, 33, 180);
uint32_t gatorOrange = strip.Color(255, 50, 10);

uint32_t colors[] = {red, yellow, green, cyan, blue, magenta};

void setup() 
{
    pinMode(LED_PIN, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), nextPattern, RISING);
    strip.setBrightness(BRIGHTNESS);
    strip.begin();
    allOff();
    strip.show();
}

void loop() 
{
    switch (patternSelect) {
        case 0:
            allOn(halloweenPurple);
            break;
        case 1:
            alternate(halloweenOrange, halloweenPurple, 12);
            break;
        case 2:
            alternate(halloweenOrange, halloweenPurple, 6);
            break;
        case 3:
            alternate(halloweenOrange, halloweenPurple, 4);
            break;
        case 4:
            delayAlternate(halloweenOrange, halloweenPurple, 4, 60);
            break;
        case 5:
            alternate(gatorBlue, gatorOrange, 4);
            break;
        case 6:
            rainbowWipe(70);
            break;
        case 7:
            rainbowCycle(20);
            break;
        case 8:
            doublePixelStackCycle(0, 23, magenta, 20);
            break;
        case 9:
            allOff();
            break;
        default:
            allOn(red);
    }

}

// ISR
void nextPattern() {
    patternSelect++;
    patternSelect %= 10; // Always one more than number of case statements
    interruptFlag = true;
}

/*
    Slowley alternate between ca and cb on the strips.  The width value is how many pixels
    to set to one color before switching to the second color.

    Demo with width = 4:
    ****----****----
*/
void alternate(uint32_t ca, uint32_t cb, int width) {
    flipFlag = true;
    uint32_t currColor = ca;

    for (int i = 1; i <= strip.numPixels(); i++) {
        if (flipFlag) {
            currColor = ca;
        } else {
            currColor = cb;
        }

        strip.setPixelColor(i-1, currColor);


        /*
            These if-statements are spattered all over this program to increase responseiveness
            of the button pressing action.  The low responsiveness is due to all the delays.
        */
        if (i % width == 0) {
            flipFlag = !flipFlag;
        }

        strip.show();

        if (interruptFlag) {
            interruptFlag = false;
            return;
        }
    }
}

/*
    Like alternate, but with a delay so a human can see the transitions.
*/
void delayAlternate(uint32_t ca, uint32_t cb, int width, uint8_t wait) {
    flipFlag = !flipFlag;
    uint32_t currColor = ca;

    for (int i = 1; i <= strip.numPixels(); i++) {
        if (flipFlag) {
            currColor = ca;
        } else {
            currColor = cb;
        }

        strip.setPixelColor(i-1, currColor);

        if (i % width == 0) {
            flipFlag = !flipFlag;
        }

        strip.show();
        delay(wait);

        if (interruptFlag) {
            interruptFlag = false;
            return;
        }
    }
}

/*
    This pattern sends out a rainbow of colors within the colors array defined above.  The 
    colors are sent out in a way that they "crawl" along the strip.

    Demo with colors[] = {r, g, b}:
    r
    gr
    bgr
    rbgr
*/
void rainbowWipe(uint8_t wait) {
    for (int i = 0; i < 6; i++) {
        uint32_t currColor = colors[i];
        if (START == 0) {
            strip.setPixelColor(START, currColor);
        } else {
            for (int k = START; k > -1; k--) {

                if (k == 0) {
                    strip.setPixelColor(k, currColor);
                } else {
                    strip.setPixelColor(k, strip.getPixelColor(k-1));
                }

                if (interruptFlag) {
                    interruptFlag = false;
                    return;
                }
            }

        }

        START++;

        strip.show();
        delay(wait);

        if (interruptFlag) {
            interruptFlag = false;
            return;
        }
    }
    
    if (START > strip.numPixels()) {
        START = 0;
    } 
}

/*
    This was stolen from the strandtest.ino program provided by the Neopixel 
    library.
*/
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));

        if (interruptFlag) {
            interruptFlag = false;
            return;
        }
    }

    strip.show();
    delay(wait);

    if (interruptFlag) {
        interruptFlag = false;
        return;
    }
  }
}

/*
    Sends out one pixel every wait duration.  The bottom parameter represents a pointer to 
    the bottom of a theoretical stack on the led strip.  The pixels stack up on top of eachother
    until reaching the top.

    Demo:
    ----
    ---*
    --*-
    -*--
    *---
    *--*
    *-*-
    **--
    **-*
    ***-
    ****
*/
void pixelStack(uint8_t top, uint8_t bottom, uint8_t wait) {
    for (int i = bottom; i > -1; i--) {
        for (int j = top; j < i; j++) {
            strip.setPixelColor(j, blue);
            allOffExcept(j, top, i);
            strip.show();
            delay(wait);
        }
    }
}

/*
    Start with a section of lit pixels, and every wait delay, take one off
    the top and send it out.  This would be like emptying the stack that 
    was created by pixelStack.

    Demo:
    ****
    ***-
    **-*
    **--
    *-*-
    *--*
    *---
    -*--
    --*-
    ---*
    ----
*/
void reversePixelStack(uint8_t top, uint8_t bottom, uint8_t wait) {
    for (int i = top; i < bottom; i++) {
        strip.setPixelColor(i, 0);
        for (int j = (i-1); j > -1; j--) {
            if (j >= 0) {
                strip.setPixelColor(j, blue);
                allOffExcept(j, top, i);
            }

            strip.show();
            delay(wait);
        }
    }
}

/*
    Like pixelStack, but mirrored accross the other end of the strip.
*/
void doublePixelStack(uint8_t top, uint8_t bottom, uint32_t c, uint8_t wait) {
    uint8_t mirrTop = bottom + (bottom - top) + 1;
    uint8_t mirrBottom = bottom + 1;
    uint8_t mirrJ;


    for (int i = bottom; i > -1; i--) {
        for (int j = top; j <= i; j++) {
            strip.setPixelColor(mirrTop, 0);
            mirrJ = mirrTop - j;
            strip.setPixelColor(mirrJ, c);
            strip.setPixelColor(j, c);
            allOffExcept(j, top, (i+1));
            allOffExcept(mirrJ, mirrBottom, mirrTop);
            strip.show();
            delay(wait);

            if (interruptFlag) {
                return;
            }
        }
        mirrBottom++;

        if (interruptFlag) {
            return;
        }
    }
}

/*
    Like reversePixelStack but mirrored accross the other end of the strip.
*/
void doubleReversePixelStack(uint8_t top, uint8_t bottom, uint32_t c, uint8_t wait) {
    uint8_t mirrTop = bottom + (bottom - top) + 1;
    uint8_t mirrBottom = bottom + 1;
    uint8_t mirrJ;
    uint8_t mirrI;

    for (int i = top; i <= bottom; i++) {
        mirrI = mirrTop - i;
        strip.setPixelColor(i, 0);
        strip.setPixelColor(mirrI, 0);

        for (int j = (i-1); j > -1; j--) {
            strip.setPixelColor(mirrTop, 0);
            mirrJ = mirrTop - j;

            if (j >= 0) {
                strip.setPixelColor(j, c);
                strip.setPixelColor(mirrJ, c);
                allOffExcept(j, top, i);
                allOffExcept(mirrJ, mirrI, mirrTop);
            }

            strip.show();
            delay(wait);

            if (interruptFlag) {
                return;
            }
        }
    }

    if (interruptFlag) {
        return;
    }
}

void doublePixelStackCycle(uint8_t top, uint8_t bottom, uint32_t c, uint8_t wait) {
    doublePixelStack(top, bottom, c, wait);
    if (interruptFlag) {
        interruptFlag = false;
        return;
    }
    doubleReversePixelStack(top, bottom, c, wait);
    if (interruptFlag) {
        interruptFlag = false;
        return;
    }
}

uint32_t Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85) {
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if(WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;    
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void allOffExcept(int index, int start_ind, int end_ind) {
    for (int i = start_ind; i < end_ind; i++) {
        if (i != index) {
            strip.setPixelColor(i, 0);
        }
    }
    strip.show();
}

void allOff() {
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, 0);
    }
    strip.show();
}

void allOn(uint32_t c) {
    for (int i = 0; i < strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
    }
    strip.show();
}

void rangeOn(uint8_t start_ind, uint8_t end_ind, uint32_t c) {
    for (int i = start_ind; i < end_ind; i++) {
        strip.setPixelColor(i, c);
    }
    strip.show();
} 