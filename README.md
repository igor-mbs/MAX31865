# IGORs_MAX31865_library
My take on the MAX31865 RTD ADC

This library is based on a lookup table generated via a python script that utilyzes numerical methods to reverse and pre-process the Callendar-Van Dusen equation. The only thing the MCU is required is to read the ADC register and lookup the equivalent temperature. This method is extremely precise and fast. The included table was made with a temperature ranging from -30ºC to 200ºC simply because that's the sensor's range that I was using. If desired, new tables can be generated from the python script either to embrace a bigger range or to save memory. The current range (-30 to 200 ºC) requires 28,720 bytes if stored as 32 bit float, though it is possible to reduce size by performing simple mathematical operations (e.g.: storing the values multiplied by 100 as integers).