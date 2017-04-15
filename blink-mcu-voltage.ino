// Measure the voltage supplied to the microcontroller and output via the built-in LED.

// This Arduino sketch uses the internal bandgap reference of the ATmega328 as an 
// input to the ADC; the (analog) supply voltage can be determined from the resulting ratio.


// tested on Sparkfun's RedBoard

// FIXME: require ATmega328
// TODO: add other microcontrollers

const float bandgap_voltage = 1.1;  // nominally 1.1V, but may vary 


void setup() {
  Serial.begin(9600);

  // collect and discard the first reading (expected to be erroneous)
  read_bandgap();

  delay(1000);
}


void loop() {
  unsigned reading = read_bandgap();

  Serial.print("reading: ");
  Serial.println(reading);

  float voltage_float = bandgap_voltage * 1023 / reading;

  Serial.print("voltage (float): ");
  Serial.println(voltage_float);

  unsigned voltage_code = voltage_float * 100 + .5;

  Serial.print("voltage (code): ");
  Serial.println(voltage_code);

  // delay for some time before getting another reading
  delay(4000);

  Serial.println();
}


// the following is derived from Arduino's analogRead code in 
// wiring_analog.c, which is licensed under the GNU LGPL v2.1.
unsigned read_bandgap() {
  // WARNING: requires no conflicting external AREF
  
  uint8_t low, high;

  ADMUX = 78; // Vcc reference, and measure internal bandgap

  // start the conversion by setting the ADSC bit of ADCSRA
  ADCSRA |= _BV(ADSC);

  // ADSC is cleared when the conversion finishes
  while (bit_is_set(ADCSRA, ADSC));

  // we have to read ADCL first; doing so locks both ADCL
  // and ADCH until ADCH is read.  reading ADCL second would
  // cause the results of each conversion to be discarded,
  // as ADCL and ADCH would be locked when it completed.
  low  = ADCL;
  high = ADCH;

  // combine the two bytes
  return (high << 8) | low;
}

