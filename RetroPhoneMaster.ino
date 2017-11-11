//#include <SoftwareSerial.h>

#define PIN_TX 11
#define PIN_TX_ A1
#define PIN_RST 10
#define PIN_RST_ A2
#define PIN_RX -1 // Disables recieving software serial data ?
#define PIN_RING 13
#define PIN_RING_ A0

#define PIN_TO_HIGH 2
#define PIN_TO_LOW  3

#define NDATA 10

String  inputString                      = "";     // a string to hold incoming data
boolean stringComplete                   = false;  // whether the string is complete
boolean doRing                           = false;
boolean prevDoRing                       = doRing;

volatile boolean        doMeasure        = false;

volatile unsigned int   duration[NDATA];
volatile unsigned int   durationHigh[NDATA];
volatile unsigned short idata            = 0;
volatile unsigned long  lastToHighMicros = 0;

void setup()
{

  pinMode(PIN_TO_HIGH, INPUT_PULLUP);
  pinMode(PIN_TO_LOW,  INPUT_PULLUP);
  
  Serial.begin(9600);

  // reserve 200 bytes for the inputString:
  inputString.reserve(200);

  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("RetroPhoneMaster: testfase: Test RingerControler");

  pinMode(PIN_RST,  OUTPUT);
  pinMode(PIN_RST_, OUTPUT);
  digitalWrite(PIN_RST,  HIGH);
  digitalWrite(PIN_RST_, HIGH);

  pinMode(PIN_RING,  OUTPUT);
  pinMode(PIN_RING_, OUTPUT);
  digitalWrite(PIN_RING,  HIGH);
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
    if (inputString == "M")
    {
      // Start measuring
      doMeasure = true;
      attachInterrupt(digitalPinToInterrupt(PIN_TO_HIGH), toHighIsr, RISING);
      attachInterrupt(digitalPinToInterrupt(PIN_TO_LOW),  toLowIsr,  FALLING);
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
    inputString    = "";
    stringComplete = false;
    
  } // END if (stringComplete)


  if (doMeasure)
  {
    if (idata >= NDATA)
    { // print data
      
      // Stop measuring
      detachInterrupt(digitalPinToInterrupt(PIN_TO_HIGH));
      detachInterrupt(digitalPinToInterrupt(PIN_TO_LOW));
      doMeasure        = false;

      // ... en reset
      idata            = 0;
      lastToHighMicros = 0;

      // Toon data
      Serial.println("duration | durationHigh");
      for (int i = 0; i < NDATA; i++)
      {
        Serial.print(duration[i]);
        Serial.print(" | ");
        Serial.print(durationHigh[i]);
        Serial.println();
      }
      
      // Bereken gemiddelde period en spreiding ervan
      unsigned int mean = 0;
      for (int i = 0; i < NDATA; i++)
        mean += duration[i];
      mean = int (mean / NDATA);

      unsigned int sdev = 0;
      for (int i = 0; i < NDATA; i++)
      {
        sdev = mean - duration[i];
        sdev = sdev * sdev;
      }
      sdev = int(sqrt(sdev / NDATA));

      Serial.print("duration = ");
      Serial.print(mean);
      Serial.println(" +/- ");
      Serial.print(sdev);
      Serial.println(" (us)");
      Serial.println();
      
      // Bereken gemiddelde duty cycle en spreiding ervan
      mean = 0;
      for (int i = 0; i < NDATA; i++)
        mean += int(100 * durationHigh[i] / duration[i]);
      mean = int (mean / NDATA);

      sdev = 0;
      for (int i = 0; i < NDATA; i++)
      {
        sdev = (mean - int(100 * durationHigh[i] / duration[i]));
        sdev = sdev * sdev;
      }
      sdev = int(sqrt(sdev / NDATA));

      Serial.print("duty = ");
      Serial.print(mean);
      Serial.println(" +/- ");
      Serial.print(sdev);
      Serial.println(" (%)");
      Serial.println();

 
    }

  } // END if (doMeasure)

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

// Interrupt routine voor als pin naar hoog gaat
void toHighIsr()
{
  // Bepaal periode
  unsigned long changeMicros = micros();
  if (lastToHighMicros != 0 && idata < NDATA)
    duration[idata] = int(changeMicros - lastToHighMicros);
  lastToHighMicros = changeMicros;
}

// Interrupt routine voor als pin naar laag gaat
void toLowIsr()
{
  // Bepaal duty cycle
  if (lastToHighMicros != 0 && idata < NDATA)
  {
    durationHigh[idata] = int(micros() - lastToHighMicros);
    idata++;
  }
}

