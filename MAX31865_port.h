/*
 * Created  : 25/11/2021
 * Author	: Igor M B Silva
 * e-mail	: igor-mbs@outlook.com
 * GitHub	: Angry-Robot
 */ 

#ifndef _MAX31865_PORT_H
    #define _MAX31865_PORT_H

    #ifndef FALSE
        #define FALSE 0;         // Boolean false
    #endif

    #ifndef TRUE
        #define TRUE 1;          // Boolean true
    #endif

    typedef uint8_t BOOL;      // Boolean

    typedef uint8_t U8;        // Unsigned 8 bits
    typedef int8_t S8;         // Signed 8 bits

    typedef uint16_t U16;      // Unsigned 16 bits
    typedef int16_t S16;       // signed 16 bits

    typedef uint32_t U32;     // Unsigned 32 bits
    typedef int32_t S32;      // Signed 32 bits

    typedef uint64_t U64;     // Unsigned 64 bits
    typedef int64_t S64;      // Signed 64 bits

    typedef float F32;        // Signed 32 bit floating point
    typedef double F64;       // Signed 64 bit floating point

    #define ENTER_CRITICAL_SECTION()   CRITICAL_SECTION_ENTER()	    // Disable Interrupts 
    #define LEAVE_CRITICAL_SECTION()    CRITICAL_SECTION_LEAVE()	// Enable Interrupts

    void SPI_SEND(U8 spi_message, U16 message_size)     // Function to send SPI message
    {        
        // gpio_set_pin_level(SPI_CS0, FALSE);
        // io_write(SPI_io, spi_message, message_size);
        // delay_us(5);
        // gpio_set_pin_level(SPI_CS0, TRUE);
        // delay_us(5);        
    };

    void SPI_GET(U8 spi_message, U16 message_size)     // Function to receive SPI message
    {        
        // gpio_set_pin_level(SPI_CS0, FALSE);
        // io_read(SPI_io, spi_message, message_size);
        // delay_us(5);
        // gpio_set_pin_level(SPI_CS0, TRUE);
        // delay_us(5);        
    };

    BOOL max31865_read_DRDY(void)
    {
        return //gpio_get_pin_level(DRDY_0);
    };

    void max31865_DELAY_MS(U64 time)
    {
        // delay_ms(time);
    };

#endif
