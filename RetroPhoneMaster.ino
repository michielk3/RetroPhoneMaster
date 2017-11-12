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

#define NDATA 20
#define MAX_MEASURE_DURATION 10000                 // 10 s

String  inputString                      = "";     // a string to hold incoming data
boolean stringComplete                   = false;  // whether the string is complete
boolean doRing                           = false;
boolean prevDoRing                       = doRing;

volatile boolean        doMeasure        = false;
unsigned long           startTimeMeasure = 0;

volatile unsigned long  changeTime[NDATA];
volatile boolean        isToHigh[NDATA];
volatile unsigned short idata            = 0;
volatile unsigned short jdata            = 0;

volatile unsigned long  iCycle           = 0;
volatile boolean        measuringStarted = false;

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

  for(int i=0; i<NDATA; i++)
  {
    changeTime[i] = 0;
    isToHigh[i] = false;
  }

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
      startTimeMeasure = millis();
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
    if (idata >= NDATA || millis() - startTimeMeasure > MAX_MEASURE_DURATION)
    { // print data
      
      // Stop measuring
      detachInterrupt(digitalPinToInterrupt(PIN_TO_HIGH));
      detachInterrupt(digitalPinToInterrupt(PIN_TO_LOW));
      doMeasure        = false;

      // ... en reset
      idata            = 0;
      jdata            = 0;
      iCycle           = 0;
      measuringStarted = false;

      // Toon data
      Serial.println("duration | durationHigh");
      for (int i = 1; i < NDATA; i++)
      {
        Serial.print(changeTime[i] - changeTime[i-1]);
        Serial.print(" | ");
        if (isToHigh[i-1] && !isToHigh[i])
          Serial.print("toHigh toLow");
        if (isToHigh[i-1] && isToHigh[i])
          Serial.print("toHigh toHigh");
        if (!isToHigh[i-1] && isToHigh[i])
          Serial.print("toLow toHigh");
        if (!isToHigh[i-1] && !isToHigh[i])
          Serial.print("toLow toLow");
        Serial.println();
      }
      
      // Bereken gemiddelde period en spreiding ervan
      /*
      unsigned long mean = 0;
      for (int i = 1; i < NDATA; i++)
        mean = mean + toHighTime[i] - toHighTime[i-1];
      mean = long (mean / NDATA);

      unsigned long sdev = 0;
      for (int i = 0; i < NDATA; i++)
      {
        sdev = mean - toHighTime[i] - toHighTime[i-1];
        sdev = sdev * sdev;
      }
      sdev = long(sqrt(sdev / NDATA));

      Serial.print("duration = ");
      Serial.print(mean);
      Serial.print(" +/- ");
      Serial.print(sdev);
      Serial.println(" (us)");
      Serial.print("freq = ");
      Serial.print(1E6 * 1./mean);
      Serial.println(" (Hz)");
      Serial.println();
      
      // Bereken gemiddelde duty cycle en spreiding ervan
      /*mean = 0;
      for (int i = 1; i < NDATA; i++)
      {
        if (toHighTime[i] - toHighTime[i-1] != 0)
          mean += (100 * (toLowTime[i] - toHighTime[i-1)] / (toHighTime[i] - toHighTime[i-1]));
      }
      mean = long (mean / NDATA);
/*
      sdev = 0;
      for (int i = 1; i < NDATA; i++)
      {
        if (toHighTime[i] - toHighTime[i-1] != 0)
        {
          sdev = (mean - long(100 * (toLowTime[i] - toHighTime[i-1)] / (toHighTime[i] - toHighTime[i-1])));
          sdev = sdev * sdev;
        }
      }
      sdev = long(sqrt(sdev / NDATA));

      Serial.print("duty = ");
      Serial.print(mean);
      Serial.print(" +/- ");
      Serial.print(sdev);
      Serial.println(" (%)");
      Serial.println();
*/
 
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
  if (iCycle++ == 2)
    measuringStarted = true;
  if (measuringStarted && idata < NDATA)
  {
    changeTime[idata] = micros();
    isToHigh[idata++] = true;
  }
}

// Interrupt routine voor als pin naar laag gaat
void toLowIsr()
{
  // Bepaal duty cycle
  unsigned long changeMicros = micros();
  if (measuringStarted && idata < NDATA)
  {
    changeTime[idata] = micros();
    isToHigh[idata++] = false;
  }
}

