#include <SoftwareSerial.h>

// constants
#define TX_PIN    4
#define RX_PIN    3
#define PWM_PIN   0
#define SPD_PIN   1
#define DEFAULT_PWM 90
#define CHANGE_CMD "CH"
#define RESET_CMD  "RE"

// Variables
volatile int ticks = 0;
int rpm = 0;
int pwmValue = DEFAULT_PWM;
byte availableBytes = 0;
SoftwareSerial mySerial(RX_PIN, TX_PIN); // RX, TX

/**
 * Initialize speed control pin - PWM
 * Attach an interrupt to pin 2 to monitor fan speed
 * Initialize serial communications
 */
void setup() 
{
  // PWM   
  pinMode(SPD_PIN, INPUT_PULLUP);
  analogWrite(PWM_PIN, pwmValue);

  // FAN RPM interrupt
  attachInterrupt(0, tick, FALLING);

  // Serial comm
  mySerial.begin(9600);
} 

/**
 * Listen for incoming PWM value changes, enables interrupts
 * and monitors the fan tachometer for 5s. Later reports that
 * value on the serial port.
 */
void loop() 
{
  parseSerialInput();
  ticks = 0;
  interrupts();
  delay(5000);
  noInterrupts();
  rpm = (ticks/2)*12;
  mySerial.print(pwmValue);
  mySerial.print(",");
  mySerial.println(rpm);
}

/**  
 *   Read input serial data, trying to find one of the two commands,
 *   if found, execute, otherwise dump serial buffer data.
 */
void parseSerialInput()
{
  availableBytes = mySerial.available();   
  if (availableBytes >= 2)
  {
    char cmd[3];
    cmd[0] = mySerial.read();
    cmd[1] = mySerial.read();
    cmd[2] = '\0';

    if (!strcmp(cmd, CHANGE_CMD))
    {
      char c   = mySerial.read();
      byte nv  = (byte)c;
      analogWrite(PWM_PIN, nv);
      mySerial.print("Changing PWM to ");
      mySerial.println(nv);
      pwmValue = nv;

    } else if (!strcmp(cmd, RESET_CMD))
    {
      byte nv  = 90;
      analogWrite(PWM_PIN, nv);
      mySerial.println("Reseting PWM to default.");
      pwmValue = nv;

    } else {
      mySerial.print("read this: ");
      mySerial.print(cmd);
      for (int i = 0; i < availableBytes-2; i++)
      {
        char c = mySerial.read();
        mySerial.print(c);
      }
      mySerial.println();
    }
  }
}

void tick()
{
  ticks++;
}

