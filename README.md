# nanoP9813FastLEDMySensors
P9813 RGB LED strip controller with MySensors and FastLED

I built a 1m x 1,5m accoustic absorber with Basotect for the ceiling in my living room. To generate ambient lighting, I installed a non-smart analog 5m LED strip on the top side. My first attempt was to use PWM from arduino which resultet in visual steps when do slow fading. Next approcach was a 12bit PWM motor shield, resulting in 4096 steps which were smooth but I couldn't use the real good FastLED library. So I ended up with a P9813 RGB MosFET Board from ebay which represents a single pixel. (But can easily chained for several strips in the room.)
FastLED is dithering between the 256 steps of the arduino and produces very smooth transitions.

### Hardware
- Arduino Nano (~5€)
- NRF24L01 (~2€)
- P9813 RGB MosFet Board from ebay (Full Color RGB LED Strip Driver Sensor v1.1 for Arduino STM32 AVR P9813) (4,29€)
- LM2596S DC-DC Step-Down Converter vor 12V->5V (~1€)
- 5m RGB LED Strip ( < 10€)
- old 12V laptop power supply 

### Software
My setup is a mqttClientGateway and a raspi with node-red. 

To change color, you can send a RRGGBB string to `mysensors-in/node-id/0/1/0/40`

To change the fade time you can send a integer from 0 to 255 to `mysensors-in/node-id/1/1/0/24`

To persist the current settings to eeprom you can send a "1" (meaning bool "true") to `mysensors-in/node-id/2/1/0/2`

To set the brightness you can send 0 to 100 (V_PERCENTAGE) to `mysensors-in/node-id/3/1/0/3`
  
This code is working. Nevertheless it could be a lot better. 
It is posted, to help people, take code lines or inspirations out of it.
