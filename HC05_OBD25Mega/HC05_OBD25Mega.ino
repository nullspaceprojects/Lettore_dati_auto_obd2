#include <SoftwareSerial.h>
#include "ELMduino.h"
#include "EasyNextionLibrary.h"

//SoftwareSerial swSerial(10, 11); // RX, TX
#define swSerial Serial //used by ELM DUINO beacause it uses Serial to print Error e Debug

//COMMENTA QUESTO DEFINE SE SI VOGLIONO LEGGERE I DATI DA OBDII
//#define ATMODE 1 // IN AT MODE INSERIRE I SEGUENTI PARAMETRI DI SERIALE: BOTH NL & CR - 9600baud
EasyNex myNex(Serial1);
ELM327 myELM327;
int state=0;

class DatiMacchina    //Classe che raggruppa i valori che dovrò leggere tramite lo scanner
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

DatiMacchina dati_macchina; //Istanza della classe DatiMacchina 

void setup()
{
  //Serial.begin(9600);
  //while(!Serial);
  myNex.begin(9600); //testa con 9600

#ifdef ATMODE
  swSerial.begin(38400);
  Serial.println("HC-05 in modalità AT");
#else
  swSerial.begin(38400);
  while(!swSerial);
  if (!myELM327.begin(swSerial, false, 2000)) //se imposto true al parametro il monitor serial va in modalità debug visualizzando tutti i dati scambiati.
  {
    Serial.println("Impossbile connettersi allo scanner OBD");
    while (1);
  }
  Serial.println("Connesso all'ELM327");
#endif

}

void loop()
{
 
  byte c;
//il blocco ifdef inizializza le seriali per la comunicazione tra Arduino ed il modulo HC-05
//permettendo nella modalità AT di configurare i parametri tramite monitor seriale
 #ifdef ATMODE              
  //BLUETOOTH AT-MODE
  if(Serial.available())
  {
    c = Serial.read();
    swSerial.write(c);
  }

  if(swSerial.available())
  {
    c = swSerial.read();
    Serial.write(c);
  }
#else
  //LOGICA DI LETTURA DATI DA OBDII
  
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
      if (myELM327.nb_rx_state == ELM_SUCCESS)
      {
        dati_macchina.supportedPIDs_1_20 = tempPID0120;
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

  //myNex.NextionListen();

#endif
}
/*
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
