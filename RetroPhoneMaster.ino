#include <SoftwareSerial.h>

#define PIN_TX 11
#define PIN_TX_ A1
#define PIN_RST 10
#define PIN_RST_ A2
#define PIN_RX -1 // Disables recieving software serial data ?
#define PIN_RING 13
#define PIN_RING_ A0

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
boolean doRing = false;
boolean prevDoRing = doRing;


void setup()
{
  Serial.begin(9600);

  // reserve 200 bytes for the inputString:
  inputString.reserve(200);

  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("RetroPhoneMaster: testfase: Test RingerControler");

  pinMode(PIN_RST, OUTPUT);
  pinMode(PIN_RST_, OUTPUT);
  digitalWrite(PIN_RST, HIGH);
  digitalWrite(PIN_RST_, HIGH);

  pinMode(PIN_RING, OUTPUT);
  pinMode(PIN_RING_, OUTPUT);
  digitalWrite(PIN_RING, HIGH);
  digitalWrite(PIN_RING_, HIGH);
}

void loop()
{
  // print the string when a newline arrives:
  if (stringComplete)
  {
    Serial.println(inputString);

    // Doe iets
    if (inputString == "A")
    {
      doRing = true;
    }
    if (inputString == "0")
    {
      doRing = false;
    }
    if (inputString == "RST")
    {
      digitalWrite(PIN_RST, LOW);
      digitalWrite(PIN_RST_, LOW);
      delay(100);
      digitalWrite(PIN_RST, HIGH);
      digitalWrite(PIN_RST_, HIGH);
    }

    if (prevDoRing != doRing)
    {
      if (doRing)
      {
        Serial.println("DO RING");
      }
      else
      {
        Serial.println("STOP RING");
      }
      digitalWrite(PIN_RING, doRing ? LOW : HIGH);
      digitalWrite(PIN_RING_, doRing ? LOW : HIGH);
    }
    prevDoRing = doRing;

    // clear the string:
    inputString = "";
    stringComplete = false;
  }

} // void loop ()

/*
  SerialEvent occurs whenever a new data comes in the
  hardware serial RX.  This routine is run between each
  time loop() runs, so using delay inside loop can delay
  response.  Multiple bytes of data may be available.
*/
void serialEvent()
{
  while (Serial.available())
  {
    // get the new byte:
    char inChar = (char)Serial.read();
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n')
    {
      stringComplete = true;
    }
    else
    {
      // add inChar to the inputString:
      inputString += inChar;
    }
  }
}




