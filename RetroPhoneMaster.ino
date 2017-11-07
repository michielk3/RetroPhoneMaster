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

boolean doMeasure = false;

unsigned long delta = 0;
unsigned long startMicros = 0;
volatile unsigned short data[ndata];
volatile unsigned short idata = 0;
volatile long duration = 0;

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
    if (inputString == "M")
    {
      doMeasure = true;
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
  } // END if (stringComplete)


  // print the freq when a newline arrives:
  if (doMeasure)
  {
    noInterrupts();
    idataCopy = idata;
    interrupts();
    
    if (idataCopy < ndata)
    { // measure data
      if (idataCopy == 0)
      { // Measure data
    
        // Start ringing
        digitalWrite(ringingPin, HIGH);

        // Start measuring
        Timer1.attachInterrupt(timerIsr,delta);
      
      }
      //digitalWrite(ring1Pin,HIGH);
    }
    else
    { // print data
      // Stop measuring
      Timer1.detachInterrupt();
    
      Serial.print("duration=");Serial.print(duration);Serial.println(" (us)");
      Serial.println();

      //digitalWrite(ring1Pin,LOW);

      // Stop ringing
      digitalWrite(ringingPin, LOW);
      
      // Bepaal min en max
      unsigned short min = 1023;
      unsigned short max = 0;
      for (int i=0; i<ndata; i++)
      {
        if (data[i] > max)
          max = data[i];
        if (data[i] < min)
          min = data[i];
      }

      // print graphic
      Serial.print("max: "); Serial.println(5.*max/1023);
      for (int inivo=(nnivo-1); inivo>=0; inivo--)
      {
        for (int i=0; i<ndata; i++)
        {
          int nivo = int(.5 + (nnivo-1) * ((float) data[i] - min)/(max - min));
          if (nivo == inivo)
          {
            Serial.print("*");
          }
          else
          {
            Serial.print(" ");
          }
        }
        Serial.println();
      }
      Serial.print("min: "); Serial.println(5.*min/1023);
      Serial.print("0                       ---->             ");
      Serial.print(ndata*delta);
      Serial.println("    microseconds");
      
      // clear data
      for (int i=0; i < ndata; i++)
        data[i] = 0;
        
      noInterrupts();
      idata = 0;
      interrupts();
      
      doMeasure = false;
    }

    askParams = true;
  } // END if (doMeasure)
  else
  {
    // Bepaalopname parameters
    if (askParams)
    {
      Serial.println("\nGeef opname tijdstap (microseconds): ");
      askParams = false;
    }
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
     if the incoming character is a newline, set a flag/
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



void timerIsr()
{
  duration = micros();
//  if (isAnalog)/
    data[idata] = analogRead(measurePin);
//  else
//    data[idata] = digitalRead(measurePin);
  idata++;
  duration = micros() - duration;
}

