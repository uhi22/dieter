
/* DIETER
 * DeviceInterfaceElecTronicEspeciallynotlimitedtoRaspberry
 *  
 * An Input/Output device controlled by serial line. Works on Raspberry and WindowsPC in the same way.
 * 
 * Features:
 *   - measure on ADC pins and print the values to serial line.
 *   - listen on serial line and control digital outputs
 *   - serial baud rate 19200, which can be transmitted over PC900 "MIDI" optocoupler.
 *   
 * Hardware: Arduino Pro Mini (ATmega328P, 16MHz)
 * 
 */

#define PIN_BUILTIN_LED 13
#define PIN_STATE_C 12
#define PIN_RELAY_1 11
#define PIN_RELAY_2 10

uint32_t lasttime_500ms;
uint32_t lasttime_5ms;
uint16_t n5ms;
uint16_t n_Adc0;
uint16_t n_Adc1;


void readTheInputs(void) {
  n_Adc0 = analogRead(A0);
  n_Adc1 = analogRead(A1);
}


void task500ms(void) {
  char s[30];
  sprintf(s, "A0=%04d", n_Adc0);
  Serial.println(s);
  sprintf(s, "A1=%04d", n_Adc1);
  Serial.println(s);
}


void setup() {
    pinMode(PIN_BUILTIN_LED, OUTPUT);
    digitalWrite(PIN_BUILTIN_LED, 0);

    pinMode(PIN_STATE_C, OUTPUT);
    pinMode(PIN_RELAY_1, OUTPUT);
    pinMode(PIN_RELAY_2, OUTPUT);
    digitalWrite(PIN_STATE_C    , 0);
    digitalWrite(PIN_RELAY_1    , 1);
    digitalWrite(PIN_RELAY_2    , 1);
    
    analogReference(INTERNAL); /* internal 1.1V reference voltage of the ATMega 328 */
    Serial.begin(19200);
    Serial.println(F("Starte..."));  
}

/*------------------------------------------------------------------------------*/
#define SERIAL_FIFO_LEN 7 /* example: "AT1234\n" */
char serialFifo[SERIAL_FIFO_LEN];
uint8_t incomingByte;
uint8_t serial_debug_value;
uint16_t serial_debug_value16;

void handleRemoteRequests(void) {
  uint16_t x;
  uint8_t i;
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    /* fill it in a FIFO */
    for (i=0; i<SERIAL_FIFO_LEN-1; i++) {
      serialFifo[i] = serialFifo[i+1];
    }
    serialFifo[SERIAL_FIFO_LEN-1] = incomingByte;
    if (serialFifo[SERIAL_FIFO_LEN-1]==0x0A)  { /* if the last character is a newline */
        if ((serialFifo[0]=='d') && (serialFifo[1]=='o')) { /* if the string starts with "do" */
          serial_debug_value++;
          /* we interpret the characters between the "do" and the newline as 4-digit-decimal-number. */
          serialFifo[SERIAL_FIFO_LEN-1]=0; /* end of string */
          x = atoi(&serialFifo[2]);
          serial_debug_value16 = x;
          Serial.print("I received value: ");
          Serial.println(serial_debug_value16, DEC);
          if (serial_debug_value16<256) {
            digitalWrite(PIN_BUILTIN_LED, (serial_debug_value16 & 1)!=0);
            digitalWrite(PIN_STATE_C    , (serial_debug_value16 & 1)!=0);
            digitalWrite(PIN_RELAY_1    , (serial_debug_value16 & 2)==0);
            digitalWrite(PIN_RELAY_2    , (serial_debug_value16 & 4)==0);

          }
        }    
        for (i=0; i<SERIAL_FIFO_LEN; i++) { /* clean the FIFO */
          serialFifo[i] = 0x20;
        }
    }
  }
}


void loop() {
  uint32_t mytime;
  readTheInputs();
  handleRemoteRequests();
  mytime = millis();
  if ((mytime-lasttime_500ms)>500) {
    lasttime_500ms+=500;
    task500ms();
  }
  if ((mytime-lasttime_5ms)>5) {
    lasttime_5ms+=5;
    //task5ms();
  }
}
