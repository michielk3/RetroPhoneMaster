//#include <SoftwareSerial.h>
#include <TimerOne.h>

#define PIN_TX 11
#define PIN_TX_ A1
#define PIN_RST 10
#define PIN_RST_ A2
#define PIN_RX -1 // Disables recieving software serial data ?
#define PIN_RING 13
#define PIN_RING_ A0

#define PIN_VOLT A5

#define NDATA 500
#define MAX_MEASURE_DURATION 10000                 // 10 s

String  inputString                      = "";     // a string to hold incoming data
boolean stringComplete                   = false;  // whether the string is complete
boolean doRing                           = false;
boolean prevDoRing                       = doRing;

volatile boolean        doMeasure        = false;
volatile boolean        isAnalog         = false;
unsigned int            delta            = 1E3; // 1 ms

volatile unsigned short measurement[NDATA];
volatile unsigned short idata            = 0;

volatile unsigned long  iCycle           = 0;

void setup()
{
  
  Timer1.initialize(); // 1 second = 1 Hz
  Timer1.stop();

  pinMode(PIN_VOLT, INPUT);
  
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

  for(int i=0; i<NDATA; i++)
  {
    measurement[i] = 0;
  }

}

void loop()
{
  // print the string when a newline arrives:
  if (stringComplete)
  {
    Serial.println(inputString);

    // Doe iets
    if (inputString == "?" || inputString == "h" || inputString == "H")
    {
       Serial.println("A     : start ringing");
       Serial.println("0     : stop ringing");
       Serial.println("RST   : reset ATTiny ringer controller");
       Serial.println("MA100 : measure analog at 100 ms intervals");
       Serial.println("MA10  : measure analog at  10 ms intervals");
       Serial.println("MA1   : measure analog at   1 ms intervals");
       Serial.println("MD100 : measure digital at 100 us intervals");
       Serial.println("MD10  : measure digital at  10 us intervals");
       Serial.println("");
       Serial.println("pins:");
       Serial.println("PIN_TX    : 11 (niet gebruikt)");
       Serial.println("PIN_TX_  .: A1 (niet gebruikt)");
       Serial.println("PIN_RST   : 10");
       Serial.println("PIN_RST_  : A2");
       Serial.println("PIN_RING  : 13");
       Serial.println("PIN_RING_ : A0");
       Serial.println("PIN_VOLT  : A5");

    }
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
    if (inputString == "MA100")
    {
      // Init measuring
      doMeasure = true;
      isAnalog  = true;
      delta = 100E3; // 100 ms
    }
    if (inputString == "MA10")
    {
      // Init measuring
      doMeasure = true;
      isAnalog  = true;
      delta = 10E3; // 10 ms
    }
    if (inputString == "MA1")
    {
      // Init measuring
      doMeasure = true;
      isAnalog  = true;
      delta = 1E3; // 1 ms
    }
    if (inputString == "MD100")
    {
      // Init measuring
      doMeasure = true;
      isAnalog  = false;
      delta = 100; // 100 us
    }
    if (inputString == "MD10")
    {
      // Init measuring
      doMeasure = true;
      isAnalog  = false;
      delta = 10; // 10 us
    }
    if (inputString == "N")
    {
      // measure Volts
      Serial.println("volt");
      for (int i = 0; i < 10; i++)
      {
        Serial.println(5.*analogRead(PIN_VOLT)/1023.);
        delay(500);
      }
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
      Timer1.detachInterrupt();
      doMeasure        = false;

      // ... en reset
      idata            = 0;
      iCycle           = 0;

      // Toon data
      Serial.println("measurement");
      for (int i = 0; i < NDATA; i++)
      {
        if (isAnalog)
          Serial.println(5. * measurement[i] / 1023.);
        else
          Serial.println(measurement[i] == HIGH ? "high" : "low");
      }
      
      // Bereken gemiddelde period en spreiding ervan
      /*
      unsigned long mean = 0;
      unsigned long prev = changeTime[0];
      for (int i = 0; i < NDATA; i++)
      {
        if (isToHigh[i] == true)
        {
          mean += changeTime[i] - prev;
          prev = changeTime[i];
        }
      }
      mean = long (mean / NDATA);

      unsigned long sdev = 0;
      prev = changeTime[0];
      for (int i = 1; i < NDATA; i++)
      {
        if (isToHigh[i] == true)
        {
          sdev = mean - changeTime[i] + prev;
          sdev += sdev * sdev;
          prev = changeTime[i];
        }
      }
      sdev = long(sqrt(sdev / NDATA));

      Serial.print("duration = ");
      Serial.print(mean);
      Serial.print(" +/- ");
      Serial.print(sdev);
      Serial.println(" (us)");
      Serial.print("freq = ");
      double freq = 1E6 * 1. / mean;
      if (freq < 0.001 )
      {
        Serial.print(freq/1000000.);
        Serial.print(" (MHz)");
      }
      else if (freq < 1.)
      {
        Serial.print(freq/1000.);
        Serial.print(" (KHz)");
      }
      else
      {
        Serial.print(1E6 * 1./mean);
        Serial.println(" (Hz)");
      }
      Serial.println();
*/
    } // if (idata >= NDATA)
    else if (idata == 0 && digitalRead(PIN_VOLT) == HIGH)
    {
      
      // Start measuring
      Timer1.attachInterrupt(timerIsr,delta); // Sample time 0.1 ms
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


void timerIsr()
{
  if (iCycle++ > 2)
  {
    if (isAnalog)
      measurement[idata] = analogRead(PIN_VOLT);
    else
      measurement[idata] = digitalRead(PIN_VOLT);
    idata++;
  }
}

