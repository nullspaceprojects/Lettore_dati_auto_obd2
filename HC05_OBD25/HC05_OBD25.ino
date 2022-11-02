#include <SoftwareSerial.h>
#include "ELMduino.h"
#include "EasyNextionLibrary.h"

SoftwareSerial swSerial(10, 11); // RX, TX
//#define swSerial Serial1

//COMMENTA QUESTO DEFINE SE SI VOGLIONO LEGGERE I DATI DA OBDII
//#define ATMODE 1 // IN AT MODE INSERIRE I SEGUENTI PARAMETRI DI SERIALE: BOTH NL & CR - 9600baud
EasyNex myNex(Serial);
ELM327 myELM327;
int state=0;

class DatiMacchina
{
  public:
    uint32_t rpm;
    float engineLoad;
    float oilTemp;
    float kmh;
    uint8_t manifoldPressure;
    float engineCoolantTemp;
    float intakeAirTemp;
    float mafRate;
    float fuelRailGuagePressure;
    uint8_t absBaroPressure;
    float fuelInjectTiming;
    uint32_t supportedPIDs_1_20;
};

DatiMacchina dati_macchina;

void setup()
{
  Serial.begin(9600);
 while(!Serial);
   myNex.begin(115200);

#ifdef ATMODE
  swSerial.begin(38400);
  Serial.println("HC-05 IN AT-MODE");
#else
  swSerial.begin(38400);
  if (!myELM327.begin(swSerial, false, 2000))
  {
    Serial.println("Couldn't connect to OBD scanner");
    while (1);
  }
  Serial.println("Connected to ELM327");
#endif

  //while(!swSerial);
  //swSerial.println("AT SP 0");
}

void loop()
{
 
  byte c;

 #ifdef ATMODE
  //BLUETOOTH AT-MODE
  if(Serial.available())
  {
    c = Serial.read();
    //Serial.write(c);
    swSerial.write(c);
  }

  if(swSerial.available())
  {
    c = swSerial.read();
    Serial.write(c);
  }
#else
  //LOGICA DI LETTURA DATI DA OBDII

  //DALLA LIBRERIA ELMDUINO: Do not try to query more than one PID at a time. 
  //                         You must wait for the current PID query to complete before starting the next one.
  
  switch(state)
  {
    case 0:
    {
        state=100;
        break;
    }
    case 100: //LETTURA RPM
    {
      float tempRPM = myELM327.rpm();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        dati_macchina.rpm = (uint32_t)tempRPM;
        //Serial.print("RPM: "); Serial.println(dati_macchina.rpm);
        state=200;
         myNex.writeStr("rpm.txt", String(dati_macchina.rpm));
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        state=200;
      }
      
      break;
    }
    case 200: //LETTURA kmh
    {
      float tempMPH = myELM327.mph();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        dati_macchina.kmh = tempMPH*1.60934;
        //Serial.print("kmh: "); Serial.println(dati_macchina.kmh);
        state=300;
        myNex.writeStr("kmh.txt", String(dati_macchina.kmh,0));
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        state=300;
      }
      break;
    }
     case 300: //LETTURA carico motore
    {
      float tempLOAD = myELM327.engineLoad();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        dati_macchina.engineLoad = tempLOAD;
        //Serial.print("Carico Motore %: "); Serial.println(dati_macchina.engineLoad);
        state=400;
        myNex.writeStr("load.txt", String(dati_macchina.engineLoad,0));
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        state=400;
      }
      break;
    }
    case 400: //LETTURA pressione aspirazione
    {
      uint8_t tempPresAsp = myELM327.manifoldPressure();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        dati_macchina.manifoldPressure = tempPresAsp;
        //Serial.print("pressione aspirazione kPa: "); Serial.println(dati_macchina.manifoldPressure);
        state=500;
        myNex.writeStr("pres.txt", String(dati_macchina.manifoldPressure));
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        state=500;
      }
      break;
    }
    case 500: //LETTURA Temperatura Motore
    {
      float tempMotore = myELM327.engineCoolantTemp();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        dati_macchina.engineCoolantTemp = tempMotore;
        //Serial.print("Temperatura Motore °C: "); Serial.println(dati_macchina.engineCoolantTemp);
        state=600;
        myNex.writeStr("temp.txt", String(dati_macchina.engineCoolantTemp,0));
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        state=600;
      }
      break;
    }
    case 600: //LETTURA Temperatura aspirazione
    {
      float tempAriaAsp = myELM327.intakeAirTemp();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        dati_macchina.intakeAirTemp = tempAriaAsp;
        //Serial.print("Temperatura Aria Aspirazione °C: "); Serial.println(dati_macchina.intakeAirTemp);
        state=700;
        myNex.writeStr("airtemp.txt", String(dati_macchina.intakeAirTemp,0));
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        state=700;
      }
      break;
    }
    case 700: //LETTURA Portata d'aria debimetro
    {
      float tempMAF = myELM327.mafRate();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        dati_macchina.mafRate = tempMAF;
        //Serial.print("Portata Aria MAF g/s: "); Serial.println(dati_macchina.mafRate);
        state=800;
        myNex.writeStr("air.txt", String(dati_macchina.mafRate,0));
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        state=800;
      }
      break;
    }
    case 800: //LETTURA Pressione Carburante
    {
      float tempPresCarb = myELM327.fuelRailGuagePressure();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        dati_macchina.fuelRailGuagePressure = tempPresCarb;
        //Serial.print("Pressione Carburante kPa: "); Serial.println(dati_macchina.fuelRailGuagePressure);
        state=900;
        myNex.writeStr("presfuel.txt", String(dati_macchina.fuelRailGuagePressure,0));
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        state=900;
      }
      break;
    }
     case 900: //LETTURA Pressione Barometrica
    {
      uint8_t tempBarometro = myELM327.absBaroPressure();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        dati_macchina.absBaroPressure = tempBarometro;
        //Serial.print("Pressione Barometrica kPa: "); Serial.println(dati_macchina.absBaroPressure);
        state=1000;
        myNex.writeStr("baro.txt", String(dati_macchina.absBaroPressure));
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        state=1000;
      }
      break;
    }
    case 1000: //LETTURA TEnsione Modulo Controllo
    {
      float tempInie = myELM327.fuelInjectTiming();

      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        dati_macchina.fuelInjectTiming = tempInie;
        //Serial.print("Fasatura Iniezione Carburante °: "); Serial.println(dati_macchina.fuelInjectTiming);
        state=100;
        myNex.writeStr("inje.txt", String(dati_macchina.fuelInjectTiming,0));
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        state=100;
      }
      break;
    }
      case 1100: //LETTURA PIDs supportati
    {
      uint32_t tempPID0120 = myELM327.supportedPIDs_1_20();
      //Serial.print("STATO: "); Serial.println(myELM327.nb_rx_state);
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        dati_macchina.supportedPIDs_1_20 = tempPID0120;
        //Serial.print("PID 01_20: CIAO "); Serial.println(dati_macchina.supportedPIDs_1_20, HEX);
        state=100;
      }
      else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
      {
        myELM327.printError();
        state=100;
      }
     break;
    }
  }

  //delay(5);
#endif
}

/*
If EGBT-045MS is powered ON with CMD pin (pin34) to
+Vcc, the UART is set to 38400bps, 8 data bits, 1
stop bit, no parity, no handshake.
*/

/*
https://forum.arduino.cc/t/hc-05-error-1f/517071/7
//slow flashing leds (spegnere, tenere premuto tasto S1 e togliere la pressione al tasto dopo accensione)
AT                        //Test UART Connection deve ritornare OK
AT+UART=38400,0,0        //setta baud a 38400 quando non è in CMD mode (default 9600) 
AT+PSWD="0000"
AT+CMODE=0                //Connect to a specified Bluetooth device only (default)
AT+ROLE=1                 //1=master 0=slave
AT+RESET
AT+INQM=0,5,48             //Inquire Access Mode=0(standard),  Maximum number of devices response(5), Inquire timeout(9)sec
AT+INIT                   //Initialize SPP Profile (potrebbe non servire perche sempre abilitato = errore 17 = SPP already initialized)
//fast flashing led (spegnere, tenere premuto S1 e tenerlo sempre premuto dopo accensione)
AT+INQ                    //Query Nearby Discoverable Devices
(discovered the address of the OBD-II scanner is 1D:A5:1F16BB - yours might be different)
AT+BIND=1D,A5,0D4552      //Query/Set Binding Bluetooth Address
AT+PAIR=1D,A5,0D4552,20   //Device Pairing, timeout(20sec)
AT+LINK=1D,A5,0D4552      //Connect to a Remote Device
AT+RESET
ATRV       //VOLTAGGIO OBDII
ATZ     //RESET OBD
*/
