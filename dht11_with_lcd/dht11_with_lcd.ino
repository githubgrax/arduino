/*
 * A combination of the DHT11 example code from http://www.dfrobot.com/wiki/index.php?title=DHT11_Temperature_and_Humidity_Sensor_(SKU:_DFR0067) (unknown license)
 * and the LCD 16x2 tutorial/example code from http://www.arduino.cc/en/Tutorial/LiquidCrystal (Public Domain)
 *
 * Measures humidity and temperature with the DHT11 every 2 seconds,
 * and outputs the readings on the serial port, and a 16x2 LCD panel.
 *
 * Compiled by Johan Herland <johan@herland.net>
 */

#include <LiquidCrystal.h>

// Initialize LCD library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// DHT11 signal pin is connected to this analog port
#define DHT11_PIN 0

byte read_dht11_dat()
{
	byte i = 0;
	byte result = 0;
	for (i = 0; i < 8; i++)
	{
		// wait forever until analog input port 0 is '1'
		// (NOTICE: PINC reads all the analog input ports
		// and _BV(X) is the macro operation which pull up
		// positon 'X' to '1' and the rest positions to '0'.
		// It is equivalent to 1 << X.)
		while(!(PINC & _BV(DHT11_PIN)));
		// if analog input port 0 is still '1' after 30 us
		// this position is 1.
		delayMicroseconds(30);
		if(PINC & _BV(DHT11_PIN))
			result |= (1 << (7 - i));
		// wait '1' finish
		while((PINC & _BV(DHT11_PIN)));
	}
	return result;
}


void setup()
{
	// Set DHT11 port as output port with initial value '1'
	DDRC |= _BV(DHT11_PIN);
	PORTC |= _BV(DHT11_PIN);

	// Initialize serial port
	Serial.begin(9600);
	Serial.println("Ready");

	// Set up 16x2 LCD panel
	lcd.begin(16, 2);
}


void loop()
{
	byte dht11_dat[5];
	byte dht11_in;
	byte i; // start condition

	PORTC &= ~_BV(DHT11_PIN); // 1. pull-down i/o pin for 18ms
	delay(18);
	PORTC |= _BV(DHT11_PIN);  // 2. pull-up i/o pin for 40ms
	delayMicroseconds(1);
	DDRC &= ~_BV(DHT11_PIN);  // let analog port 0 be input port
	delayMicroseconds(40);

	dht11_in = PINC & _BV(DHT11_PIN); // read only the input port 0
	if (dht11_in)
	{
		// wait for DHT response signal: LOW
		Serial.println("dht11 start condition 1 not met");
		delay(1000);
		return;
	}
	delayMicroseconds(80);
	dht11_in = PINC & _BV(DHT11_PIN);
	if (!dht11_in) {
		// wait for second response signal: HIGH
		Serial.println("dht11 start condition 2 not met");
		return;
	}

	delayMicroseconds(80); // now ready for data reception
	// receive 40 bits data. Details are described in datasheet
	for (i = 0; i < 5; i++)
		dht11_dat[i] = read_dht11_dat();

	// set DHT11 port to output value '1', after data is received
	DDRC |= _BV(DHT11_PIN);
	PORTC |= _BV(DHT11_PIN);

	// verify checksum
	byte dht11_check_sum =
		dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3];
	if (dht11_dat[4] != dht11_check_sum)
		Serial.println("DHT11 checksum error");

	// report humidity and temperature on serial port
	Serial.print("Current humdity = ");
	Serial.print(dht11_dat[0], DEC);
	Serial.print(".");
	Serial.print(dht11_dat[1], DEC);
	Serial.print("%  ");
	Serial.print("temperature = ");
	Serial.print(dht11_dat[2], DEC);
	Serial.print(".");
	Serial.print(dht11_dat[3], DEC);
	Serial.println("C  ");

	// print humidity on lcd
	lcd.setCursor(0, 0);
	lcd.print("Humd.: ");
	if (dht11_dat[0] < 10)
		lcd.print(" ");
	lcd.print(dht11_dat[0]);
	lcd.print("%");

	// print runtime on lcd
	int minutes = millis() / 60000;
	int seconds = (millis() / 1000) % 60;
	if (minutes < 100) {
		lcd.print(" ");
		if (minutes < 10)
			lcd.print(0);
	}
	lcd.print(minutes);
	lcd.print(":");
	if (seconds < 10)
		lcd.print(0);
	lcd.print(seconds);

	// print temperature on lcd
	lcd.setCursor(0, 1);
	lcd.print("Temp.: ");
	if (dht11_dat[2] < 10)
		lcd.print(" ");
	lcd.print(dht11_dat[2]);
	lcd.print("C ");

	// wait 2 seconds until next reading
	delay(2000);
}