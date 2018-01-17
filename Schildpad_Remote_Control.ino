#include <Boards.h>
#include <Firmata.h>
#include <FirmataConstants.h>
#include <FirmataDefines.h>
#include <FirmataMarshaller.h>
#include <FirmataParser.h>
#include <SoftwareSerial.h>

SoftwareSerial BTserial(2,3); // RX | TX

// max length of bluetooth command is 20 chrs
const byte numChars = 20;
boolean newDataReceived = false;
char receivedChars[numChars];
char CurrentAction = 'S';

boolean debug = false;
boolean debug_bluetooth = true;

int Motor1_1 = 6;      // Digital poot5
int Motor1_2 = 5;      // Digital port6
int Motor2_1 = 11;      // Digital poot5
int Motor2_2 = 10;      // Digital port6
int StopSensor = 2;
int AfstandPin_rechts = 3;
int AfstandPin_links = 4;
int OmkeerSensor = 5;
int Afstand_links = 0;
int Afstand_rechts =0;
int dempel = 850;
int TopSensorDrempelWaarde = 50;
int max_snelheid_links=200;
int max_snelheid_rechts=220;

//int max_snelheid_links=0;
//int max_snelheid_rechts=0;

int snelheid = 50;
int huidige_snelheid=0;
int gem_afstand=0;
int new_afstand=0;
int afstands_array_rechts[4]={0,0,0,0};
int afstands_array_links[4]={0,0,0,0};
int afstands_array_pointer=0;

//void automatic_move();
//void motor(int nieuwe_snelheid_rechts, int nieuwe_snelheid_links);
//void gemiddelde_afstand(void);
//void recvWithStartEndMarkers();




void setup() {
  // put your setup code here, to run once:
  pinMode(Motor1_1, OUTPUT);  // declare the ledPin as an OUTPUT
  pinMode(Motor1_2, OUTPUT);  // declare the ledPin as an OUTPUT
  pinMode(Motor2_1, OUTPUT);  // declare the ledPin as an OUTPUT
  pinMode(Motor2_2, OUTPUT);  // declare the ledPin as an OUTPUT

  Serial.begin(9600);
  BTserial.begin(9600);

  motor(0,0);
  //while (analogRead(StopSensor) > TopSensorDrempelWaarde )
  //{
  //  if (debug) { Serial.println(analogRead(StopSensor)); }
  //}
  delay(500);
}

void loop() 
{
  
    //if (debug_bluetooth) {Serial.println("Bluetooth active");} 
    
    if (BTserial.available() > 0) 
    {
      recvWithStartEndMarkers();
      if (newDataReceived)
      {        
        newDataReceived = false;
        if (debug_bluetooth) 
        {
          Serial.print("receivedChars[0] = ");
          Serial.println(receivedChars[0]);
        }
        CurrentAction = receivedChars[0];
      }
    }
    if (debug_bluetooth) 
    {
      Serial.print("CurrentAction = ");
      Serial.println(CurrentAction);
    }
    if (CurrentAction == 'A')
    {
      automatic_move();
    }
    
    if (CurrentAction == 'L')
    {
      motor(max_snelheid_rechts,0);
    }
    
    if (CurrentAction == 'R')
    {
      motor(0,max_snelheid_rechts);
    }
    if (CurrentAction == 'F')
    {
      motor(max_snelheid_rechts,max_snelheid_rechts);
    }
    if (CurrentAction == 'B')
    {
      motor(-max_snelheid_rechts,-max_snelheid_links);
    }
    if (CurrentAction == 'S')
    {
      motor(0,0);
    }
}


void automatic_move() {
  
  //delay(1000);
  gemiddelde_afstand();

  //Serial.println(analogRead(StopSensor));

  if (debug) {Serial.print("R:");}
  if (debug) {Serial.print(Afstand_rechts);}
  if (debug) {Serial.print(" L:");}
  if (debug) {Serial.println(Afstand_links);}
  if (debug) {Serial.println();}
  
  if (analogRead(StopSensor) > TopSensorDrempelWaarde )
  {
    if (analogRead(OmkeerSensor) > TopSensorDrempelWaarde )
    {
      if ((Afstand_rechts < dempel) && (Afstand_links < dempel))
      {
        if (debug) {Serial.println("rechtdoor");}
        motor(max_snelheid_rechts,max_snelheid_links);
      }
      if ((Afstand_rechts > dempel) && (Afstand_links > dempel))
      {
        if (debug) {Serial.println("achteruit");}
        motor(-max_snelheid_rechts,-max_snelheid_links+100);
      }
      else
      {
        if ( Afstand_rechts > dempel)
        {
          if (debug) {Serial.println("rechts");}
          motor(max_snelheid_rechts,0);      
        }

        if ( Afstand_links > dempel)
        {
          if (debug) {Serial.println("links");}
          motor(0,max_snelheid_links);      
        }
      }
    } 
    else
    {
      motor(0,0); 
      delay(1000); 
      motor(-max_snelheid_rechts,-max_snelheid_links);
      delay(1000);
      motor(-max_snelheid_rechts,0);
      delay(1000);
    }
  }
  else
  {
    motor(0,0);
    delay(2000);
    while(analogRead(StopSensor) > 100)
    {
      if (debug) {Serial.print("Stoploop ");}
      if (debug) {Serial.println(analogRead(StopSensor));}
    }
    delay(1000);
  }
}

void motor(int nieuwe_snelheid_rechts, int nieuwe_snelheid_links) {
  //Serial.print("nieuwe_snelheid_rechts: ");
  //Serial.println(nieuwe_snelheid_rechts);
  //Serial.print("nieuwe_snelheid_links: ");
  //Serial.println(nieuwe_snelheid_links);

  //Vooruit
  if (nieuwe_snelheid_rechts >= 0)
  {
    digitalWrite(Motor1_1, LOW);
    analogWrite(Motor1_2, nieuwe_snelheid_rechts);
  }
  if (nieuwe_snelheid_links >= 0)
  {
    digitalWrite(Motor2_1, LOW);
    analogWrite(Motor2_2, nieuwe_snelheid_links);
  }
  //Achteruit 
  if (nieuwe_snelheid_rechts < 0)
  {
    nieuwe_snelheid_rechts=-nieuwe_snelheid_rechts;
    digitalWrite(Motor1_2, LOW);
    analogWrite(Motor1_1, nieuwe_snelheid_rechts);
  }
  if (nieuwe_snelheid_links < 0)
   {
    nieuwe_snelheid_links=-nieuwe_snelheid_links;
    digitalWrite(Motor2_2, LOW);
    analogWrite(Motor2_1, nieuwe_snelheid_links);
  } 
}

void gemiddelde_afstand(void) 
{
  int afstand_optelling=0;
  int gemiddelde_afstand=0;
  int nieuwe_afstand_rechts=analogRead(AfstandPin_rechts);
  int nieuwe_afstand_links=analogRead(AfstandPin_links);
  int oude_afstand_rechts=0;
  int oude_afstand_links=0;

  //Serial.print("R:");
  //Serial.print(nieuwe_afstand_rechts);
  //Serial.print("   L:");
  //Serial.println(nieuwe_afstand_links);
  
  oude_afstand_rechts=afstands_array_rechts[afstands_array_pointer];
  oude_afstand_links=afstands_array_links[afstands_array_pointer];
 
  //Serial.print("nieuwe_afstand_links: ");
  //   Serial.println(analogRead(AfstandPin_links));

  
 
  Afstand_rechts= Afstand_rechts + nieuwe_afstand_rechts - oude_afstand_rechts;
  Afstand_links = Afstand_links  + nieuwe_afstand_links  - oude_afstand_links;

  afstands_array_rechts[afstands_array_pointer]=nieuwe_afstand_rechts;
  afstands_array_links[afstands_array_pointer]=nieuwe_afstand_links;
    
  afstands_array_pointer++;
  if (afstands_array_pointer == 4 )
  {
    afstands_array_pointer=0;
  }
  
  //  Serial.print("totaal ");
  //  Serial.println(afstand_optelling);
  //  Serial.print("nieuwe afstand: ");
  //  Serial.println(nieuwe_afstand_rechts);
  //  Serial.print("pointer: ");
  //  Serial.println(afstands_array_pointer);
  
  //return afstand_optelling;
}

void recvWithStartEndMarkers() 
{
     
// function recvWithStartEndMarkers by Robin2 of the Arduino forums
// See  http://forum.arduino.cc/index.php?topic=288234.0

     static boolean recvInProgress = false;
     static byte ndx = 0;
     char startMarker = '<';
     char endMarker = '>';
     char rc;

     if (BTserial.available() > 0) 
     {
          rc = BTserial.read();
          if (recvInProgress == true) 
          {
               if (rc != endMarker) 
               {
                    receivedChars[ndx] = rc;
                    ndx++;
                    if (ndx >= numChars) { ndx = numChars - 1; }
               }
               else 
               {
                     receivedChars[ndx] = '\0'; // terminate the string
                     recvInProgress = false;
                     ndx = 0;
                     newDataReceived = true;
               }
          }

          else if (rc == startMarker) { recvInProgress = true; }
     }

}


