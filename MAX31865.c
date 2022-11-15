/*
 * Created	: 25/11/2021
 * Author	: Igor M B Silva
 * e-mail	: igor-mbs@outlook.com
 * GitHub	: Angry-Robot
 */ 

#include "../LUT/lut.h"
#include "../MAX31865.h"

/*	// MAX31865 abstract //
	
	~DRDY:
		The DRDY output goes low when a new conversion result is available in the RTD Data Registers. When a read operation of the RTD Data Registers completes, DRDY returns high.
	
	MAX31865 registers organization:
										READ	,	WRITE
		Configuration register		:	x00		,	x80
		RTD Resistance MSB			:	x01		,	--
		RTD Resistance LSB			:	x02		,	--
		High fault threshold MSB	:	x03		,	x83
		High fault threshold LSB	:	x04		,	x84
		Low fault threshold MSB		:	x05		,	x85
		Low fault threshold LSB		:	x06		,	x86
		Fault Status Register		:	x07		,	--
		
	Converting RTD data register values to temperature:
		Rrtd [Ohm] = (ADCreg * Rref) / (2^15)
			Rref = Resistance of the reference resistor (usually four times the platinum RTD resistance at 0�C)
			ADCreg = 15-bit ADC results from RTD Data registers (01h�02h)
		R [Ohm] = R0 * ( 1 + A*T + B*(T^2) + C*(T-100)*(T^3) )
			T = Temperature
			R0 = Resistance at 0�C
			A = 3.9083*(10^-3)
			B = -5.775*(10^-7)
			C = -4.18301*(10^-12) for -200<T<0 and 0 for 0=<T<850
		Rough approximation:
			T [�C] = (ADCreg / 32) - 256
*/

//---------------// Declarations //---------------//
static MAX31865CFG CONFIGURATION;
static RTDREG RTD_REGISTER;

static MODE_ENUM max31865_operation_mode;
static MODE_ENUM max31865_hz_mode;
static MODE_ENUM max31865_wire_mode;

static U8 const REFERENCE_RESISTOR = 400;
static U8 const RTD_RESISTANCE = 100;

//---------------// Read addresses //---------------//				//---------------// Write addresses //---------------//
static U8 const CONFIGURATION_READ		    	= 0x00;			    static U8 const CONFIGURATION_WRITE 			= 0x80; 
static U8 const RTD_MS					    	= 0x01;		    	// -
static U8 const RTD_LS					    	= 0x02;		    	// -
static U8 const HIGH_FAULT_THRESHOLD_MS_READ 	= 0x03;		    	static U8 const HIGH_FAULT_THRESHOLD_MS_WRITE   = 0x83;
static U8 const HIGH_FAULT_THRESHOLD_LS_READ 	= 0x04;			    static U8 const HIGH_FAULT_THRESHOLD_LS_WRITE	= 0x84;
static U8 const LOW_FAULT_THRESHOLD_MS_READ 	= 0x05;			    static U8 const LOW_FAULT_THRESHOLD_MS_WRITE 	= 0x85;
static U8 const LOW_FAULT_THRESHOLD_LS_READ 	= 0x06;		    	static U8 const LOW_FAULT_THRESHOLD_LS_WRITE	= 0x86;
static U8 const FAULT_STATUS 					= 0x07;		    	// -

//---------------// Functions //---------------//
STATUS_ENUM max31865_start(MODE_ENUM mode, U8 wire, U8 hz)
{
	STATUS_ENUM STATUS = OK;
	U8 spi_buffer[8];
	max31865_operation_mode = mode;
	max31865_wire_mode = wire;
	max31865_hz_mode = hz;

	switch (wire)
	{
	case 2:
		CONFIGURATION.bit.WIRE = 0;	// Bit 4	:	two or four wires mode	:	Changes the RTD wiring mode (1: three wires mode, 0: two or four wires mode)
		break;
	case 3:
		CONFIGURATION.bit.WIRE = 1;	// Bit 4	:	Three wires mode		:	Changes the RTD wiring mode (1: three wires mode, 0: two or four wires mode)
		break;
	case 4:
		CONFIGURATION.bit.WIRE = 0;	// Bit 4	:	two or four wires mode	:	Changes the RTD wiring mode (1: three wires mode, 0: two or four wires mode)
		break;
	default:
		STATUS = WIRE_INPUT_ERROR;
		break;
	}

	switch (hz)
	{
	case 50:
		CONFIGURATION.bit.HZFILTER 	= 1;	// Bit 0	:	50Hz	:	Noise rejection filter frequency select (do not change frequency while in auto reading mode) (0: 60Hz, 1: 50Hz)
		break;
	case 60:
		CONFIGURATION.bit.HZFILTER 	= 0;	// Bit 0	:	60Hz	:	Noise rejection filter frequency select (do not change frequency while in auto reading mode) (0: 60Hz, 1: 50Hz)
		break;
	default:
		STATUS = HZ_INPUT_ERROR;
		break;
	}

	if (STATUS == OK)
	{
		// First turn on VBIAS. 
		CONFIGURATION.bit.FAULTCLR 	 = 1;	// 1	:	Fault status clear	:	Write a 1 to this bit while writing 0 to bits D5, D3, and D2 to return all fault status bits (D[7:2]) in the "Fault Status Register" to 0.
		CONFIGURATION.bit.FAULTCYCLE = 0;	// 2-3	:	Fault detection		:	Confusing AF, go read the data sheet : https://datasheets.maximintegrated.com/en/ds/MAX31865.pdf
		CONFIGURATION.bit.ONESHOT 	 = 0;	// 5	:	One shot conversion	:	Reads sensor a single time, after a pulse on CS pin (requires "CONVMODE" off and "VBIAS" on; each conversion takes about 53ms in 60Hz and 63ms in 50HZ) (1: on, 0: off)
		CONFIGURATION.bit.CONVMODE 	 = 0;	// 6	:	Conversion mode		:	Turn on/off continuous reading at 50/60Hz rate (1: on, 0: off)
		CONFIGURATION.bit.VBIAS 	 = 1;	// 7	:	Voltage bias		:	Turn on/off voltage bias on the platinum thermistor (requires 11 time constants of the RC net + 1ms before starting the next reading) (1: on, 0: off)
		ENTER_CRITICAL_SECTION();
		spi_buffer[0] = CONFIGURATION_WRITE;
		spi_buffer[1] = CONFIGURATION.reg;
		SPI_SEND(spi_buffer, 2);
		spi_buffer[0] = CONFIGURATION_READ;
		SPI_SEND(spi_buffer, 1);
		SPI_GET(spi_buffer, 1);
		LEAVE_CRITICAL_SECTION();
		if (spi_buffer[1] =! CONFIGURATION.reg)
		{
			STATUS = ERROR_PRECONFIGURING;				// changes not getting effect !
		}

		if (STATUS == OK)
		{		
			switch (mode)
			{
				case ONESHOT:  
					CONFIGURATION.bit.ONESHOT 	= 1;	// Bit 5	:	One shot conversion	:	Reads sensor a single time, after a pulse on CS pin (requires "CONVMODE" off and "VBIAS" on; each conversion takes about 53ms in 60Hz and 63ms in 50HZ) (1: on, 0: off)
					CONFIGURATION.bit.CONVMODE 	= 0;	// Bit 6	:	One shot conversion	:	Turn on/off continuous reading at 50/60Hz rate (1: on, 0: off)
					break;
				case AUTOMATIC:  
					CONFIGURATION.bit.ONESHOT 	= 0;	// Bit 5	:	Automatic conversion	:	Reads sensor a single time, after a pulse on CS pin (requires "CONVMODE" off and "VBIAS" on; each conversion takes about 53ms in 60Hz and 63ms in 50HZ) (1: on, 0: off)
					CONFIGURATION.bit.CONVMODE 	= 1;	// Bit 6	:	Automatic conversion	:	Turn on/off continuous reading at 50/60Hz rate (1: on, 0: off)
					break;
				default:
					STATUS = MODE_INPUT_ERROR;
					break;
			}

			if (STATUS == OK)
			{
				/*	Wait 10.5 time constants + 1ms.
					
					From datasheet:  "Note 4: For 15-bit settling, a wait of at least 10.5 time constants of the input RC network is required. Max startup time is calculated
											with a 10kOhm reference resistor and a 0.1µF capacitor across the RTD inputs.";
										"*CI = 10nF FOR 1kΩ RTD and 100nF FOR 100Ω RTD";
										"Therefore, a PT100 uses a 400Ohm reference resistor, and a PT1000 uses a 4kOhm reference resistor.".
					
					For 400Ohm res plus a00nF cap tau = 40us.
					Maximum tau = 1ms (10kOhm res and 0.1uF cap).

					Made it double the maximum scenario to prevent errors in different circuits.
				*/
				max31865_DELAY_MS(4);

				// Then arm conversion.
				ENTER_CRITICAL_SECTION();
				spi_buffer[0] = CONFIGURATION_WRITE;
				spi_buffer[1] = CONFIGURATION.reg;
				SPI_SEND(spi_buffer, 2);
				spi_buffer[0] = CONFIGURATION_READ;
				SPI_SEND(spi_buffer, 1);
				SPI_GET(spi_buffer, 1);
				LEAVE_CRITICAL_SECTION();
				if (spi_buffer[1] =! CONFIGURATION.reg)
				{
					STATUS = ERROR_CONFIGURING;				// changes not getting effect !
				}
			}
		}
	}

	return STATUS;
}

STATUS_ENUM max31865_read(TEMPERATURE_STRUCT temperature)
{
	STATUS_ENUM STATUS = OK;
	U8 spi_buffer[8];

	if ( max31865_read_DRDY() )
	{
		STATUS = OLD_READING;
	}

	spi_buffer[0] = RTD_MS;
	SPI_SEND(spi_buffer, 1);
	SPI_GET(spi_buffer, 2);
	RTD_REGISTER.reg = (spi_buffer[0] << 8) + spi_buffer[1];
	
	if (RTD_REGISTER.val.faultFlag)	// Checks if and error ocurred first
	{
		spi_buffer[0] = FAULT_STATUS;
		SPI_SEND(spi_buffer, 1);
		SPI_GET(spi_buffer, 1);
		/*	Fault status register (0x7):	
			
			Bit	:	Description								:	Possible cause																											:	Resulting data
			----:-------------------------------------------:---------------------------------------------------------------------------------------------------------------------------:-----------------
			7	:	RTD high threshold						:	Open RTD element, RTDIN+ or Force+ shorted high and not connected to RTD												:	Full scale
			6	:	RTD low threshold						:	RTDIN+ and RTDIN- shorted together, RTDIN+ or RTDIN- shorted low and not connected to RTD, Force+ shorted low			:	Near zero
			5	:	REFIN > 0.85*Vbias						:	Open RTD element, Force+ or Force- shorted high, Force- shorted low and not connected, Force- or Force+ unconnected		:	Full scale (first case) or indeterminate
			4	:	REFIN < 0.85*Vbias, (FORCE- is open)	:	Force- or RTDIN- shorted low and connected																				:	Indeterminate (first case) or apparently valid
			3	:	RTDIN- < 0.85*Vbias, (FORCE- is open)	:	Force+ or Force- shorted low, RTDIN- shorted low and connected to RTD													:	Near zero or apparently valid
			2	:	IC over/under voltage					:	Over voltage or under voltage fault																						:	Indeterminate
			1	:	Reserved
			0	:	Reserved
		
			Writing 1 to the Fault Status Clear bit in the Configuration Register returns all fault status bits to 0.
		*/
		if ((spi_buffer[0] & 0b11111000) >= 0b00001000)
		{
			STATUS = BROKEN_RTD_WIRE;				// Broken RTD wire	
		} 
		else if ((spi_buffer[0] & 0b00000111) < 0b00001000)
		{
			STATUS = CIRCUIT_MALFUNCTION;			// Circuit malfunction	
		}
		else
		{
			STATUS = UNKNOWN_ERROR;					// Unknown error
		}
		
		max31865_start(max31865_operation_mode, max31865_wire_mode, max31865_hz_mode);	// Skip this reading and re-configure sensor to try again later
	} 
	else											// If there is no flags, then process data
	{
		temperature.celsius = tArray[RTD_REGISTER.val.rtd - 7227];
		temperature.kelvin = temperature.celsius + 273.15;				// 0ºC is -273.15K, according to OIML, at < https://www.oiml.org/en/files/pdf_g/g008-e91.pdf >, and NIST, at < https://www.nist.gov/pml/weights-and-measures/si-units-temperature > (links last accessed at 13/03/22 00:18 GMT-3).
		temperature.fahrenheit = (temperature.celsius * 1.8) + 32.0;	// This Fahrenheit conversion follows NIST convention, at < https://www.nist.gov/pml/weights-and-measures/si-units-temperature > (links last accessed at 13/03/22 00:18 GMT-3).
	}

	return STATUS;
}

