#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_NODE_ID 15
#define MY_PARENT_NODE_ID 0
#define MY_PARENT_NODE_IS_STATIC
#define SKETCH_NAME "InsertSketchNameHere"

#define CHILD_SET_RGB_COLOR 0
#define CHILD_SET_FADE_TIME 1
#define CHILD_PERSIST_STATE 2
#define CHILD_SETBRIGHTNESS 3

#define ADDRESS_RED 1
#define ADDRESS_GREEN 2
#define ADDRESS_BLUE 3
#define ADDRESS_DURATION 4
#define ADDRESS_BRIGHTNESS 5

#define FASTLED_NUM_LEDS 1
#define FASTLED_DATA_PIN 4
#define FASTLED_CLOCK_PIN 3

#include <FastLED.h>
#include <MySensors.h>

MyMessage msgSetRgbColor( CHILD_SET_RGB_COLOR , V_RGB );
MyMessage msgSetBrightness( CHILD_SETBRIGHTNESS, V_PERCENTAGE);
MyMessage msgSetStatus( CHILD_SETBRIGHTNESS, V_STATUS);
MyMessage msgSetFadeTime( CHILD_SET_FADE_TIME , V_VAR1 );
MyMessage msgSaveCurrentState( CHILD_PERSIST_STATE , V_STATUS );

int16_t currentBrighntess;
int16_t fadeDuration; // seconds

CRGB leds[FASTLED_NUM_LEDS];
CRGB currentColor;

CRGB hexToCRGB(const char* hexstring)
{
    int32_t number = strtol( &hexstring[0], NULL, 16);
    return CRGB(number);
}

void fadeTo(CRGB newColor, uint32_t fadeTime)
{
    for (uint8_t amount = 0; amount < 255; amount++) {
        leds[0] = blend(currentColor, newColor, ease8InOutApprox(amount));
        // fadeTime[s] * delayTimeUnit / steps == 1000ms / 256 steps = 3.9
        //FastLED.delay((uint32_t) round(fadeTime*3.9));
        // To be quicker, accept a slightly longer fade (4*256=1024ms=1,024s)
        FastLED.delay(fadeTime * 4);
    }
    currentColor = newColor;
    return;
}

void changeBrightnessTo(uint8_t newBrightness)
{
        int16_t steps = currentBrighntess - newBrightness;
    	int8_t step = steps > 0 ? -1 : 1;
        int16_t delay = round( fadeDuration * 1000 / abs(steps) );
    	while ( currentBrighntess != newBrightness ) {
    		currentBrighntess += step;
            FastLED.setBrightness((uint8_t)(currentBrighntess));
    		FastLED.delay( delay );
    }
    return;
}

void loadStateFromEeprom()
{
    // https://www.mysensors.org/download/sensor_api_20#saving-state
    uint8_t r = loadState(ADDRESS_RED);
    uint8_t g = loadState(ADDRESS_GREEN);
    uint8_t b = loadState(ADDRESS_BLUE);
    currentColor = CRGB(r,g,b);
    fadeDuration = (int16_t) loadState(ADDRESS_DURATION);
    currentBrighntess = loadState(ADDRESS_BRIGHTNESS);

    return;
}

void blinkLeds(uint8_t blinkCount)
{
    for (uint8_t i = 0; i < blinkCount; i++)
    {
        leds[0] = -currentColor;
        FastLED.delay(200);
        leds[0] = currentColor;
        FastLED.delay(300);
    }
    return;
}

void before(){
    Serial.begin(115200);
    loadStateFromEeprom();
    FastLED.addLeds<P9813, FASTLED_DATA_PIN, FASTLED_CLOCK_PIN, RGB>(leds, FASTLED_NUM_LEDS);
    FastLED.setCorrection( Tungsten40W );
    FastLED.setBrightness( currentBrighntess );
    fadeTo(currentColor, 0);
}

void setup() {
}

void presentation()
{
    present( CHILD_SET_RGB_COLOR , S_RGB_LIGHT  , "Set color"       );
    present( CHILD_SET_FADE_TIME , S_CUSTOM     , "Set fading time" );
    present( CHILD_PERSIST_STATE , S_BINARY     , "Persist state"   );
    present( CHILD_SETBRIGHTNESS , S_DIMMER     , "Set brightness"  );
    sendSketchInfo(SKETCH_NAME, __DATE__);
}

void receive(const MyMessage &message)
{
    switch (message.sensor) {
        case CHILD_SET_RGB_COLOR:
        {
            if(message.type == V_RGB)
            {
                // MQTT publish: mysensors-in/15/0/1/0/40 with payload 'rrggbb' without '0x' or '#'
                const char* hex = message.getString();
                CRGB newColor = hexToCRGB(hex);
                //Serial.println( hex );
                fadeTo( newColor , fadeDuration );
            }
            break;
        }
        case CHILD_SET_FADE_TIME:
        {
            if(message.type == V_VAR1)
            {
                fadeDuration = constrain(message.getLong() , 0 , 255);
                Serial.print("set fade duration to ");
                Serial.println(fadeDuration);
            }
            break;
        }
        case CHILD_SETBRIGHTNESS:
        {
            if(message.type == V_PERCENTAGE)
            {
                uint8_t percentage = constrain(message.getByte(), 0, 100);
                uint8_t newBrightness = map(percentage, 0, 100, 0, 255);
                changeBrightnessTo( newBrightness );
            }
            if(message.type == V_STATUS)
            {
                bool lightShouldBeOn = message.getBool();
                bool lightIsOff = currentBrighntess == 0;
                if(lightIsOff && lightShouldBeOn)
                {
                    changeBrightnessTo(loadState(ADDRESS_BRIGHTNESS));
                }
                if(!lightIsOff && !lightShouldBeOn)
                {
                    changeBrightnessTo(0);
                }
            }
            break;
        }
        case CHILD_PERSIST_STATE:
        {
            // Only persist the state if received payload is 'true'
            if(message.type == V_STATUS && message.getBool() )
            {
                saveState( ADDRESS_RED          , currentColor.r    );
                saveState( ADDRESS_GREEN        , currentColor.g    );
                saveState( ADDRESS_BLUE         , currentColor.b    );
                saveState( ADDRESS_DURATION     , fadeDuration      );
                saveState( ADDRESS_BRIGHTNESS   , currentBrighntess );
                blinkLeds(1);
            }
            break;
        }
    }
    return;
}

void loop() {
    FastLED.delay(0);
}
