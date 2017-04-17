// Measure the voltage supplied to the microcontroller and output via the built-in LED.

// This Arduino sketch uses the internal bandgap reference of the ATmega328 as an 
// input to the ADC; the (analog) supply voltage can be determined from the resulting ratio.


/* 
  supported microcontrollers:
  ATmega328P
  ATmega32U4

  boards tested:
  SainSmart's clone of the Arduino Uno
  Sparkfun RedBoard
  Pololu A-Star Prime SV
*/

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega32U4__)
// microcontroller is supported
#else
#error ERROR unsupported or unrecognized CPU
#endif

const float bandgap_voltage = 1.1;  // nominally 1.1V, but may vary
// NOTE: while the ATmega32U4 features a 2.56 voltage reference, 
// the ADC input options only include the 1.1V bandgap.

// blinking parameters; durations in ms

// duration to light the LED
const unsigned ON_TIME = 375;

// time between blinks within one symbol
const unsigned INTRA_SYMBOL_TIME = 375;

// special handling of 0
const unsigned ZERO_ON_TIME = ON_TIME/4;
const unsigned ZERO_OFF_TIME = ON_TIME - ZERO_ON_TIME;

// time between symbols
const unsigned INTER_SYMBOL_TIME = 1500;

// minimum time between messages
const unsigned INTER_MESSAGE_TIME = 3000;


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
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

  // delay for a short time so the blink of the TX light doesn't 
  // distract from reading the output LED.
  delay(500);
  
  blink_unsigned(voltage_code);
  
  // delay for some more time before getting another reading
  delay(4000);

  Serial.println();
}


void blink_unsigned(unsigned val) {
  String string = String(val);

  for (int i = 0; i < string.length(); i++) {
    byte digit = string.charAt(i) - 48;
    blink_digit(digit);

    delay(INTER_SYMBOL_TIME);
  }
}


void blink_digit(byte digit) {
  if (digit == 0) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(ZERO_ON_TIME);
    digitalWrite(LED_BUILTIN, LOW);
    delay(ZERO_OFF_TIME);
    delay(INTRA_SYMBOL_TIME);
  } else {
    for (int i = 0; i < digit; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(ON_TIME);
      digitalWrite(LED_BUILTIN, LOW);
      delay(INTRA_SYMBOL_TIME);
    }
  }
}


// the following is derived from Arduino's analogRead implementation 
// in wiring_analog.c, which is licensed under the GNU LGPL v2.1.
unsigned read_bandgap() {
  // WARNING: requires no conflicting external AREF
  
  uint8_t low, high;

#if defined(__AVR_ATmega328P__)
  ADMUX = 78; // Vcc reference, and measure internal bandgap
#elif defined(__AVR_ATmega32U4__)
  ADMUX = 94;
  // clear MUX5
  ADCSRB &= ~(_BV(MUX5));
#else
  // FIXME: unknown microcontroller
#endif

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

