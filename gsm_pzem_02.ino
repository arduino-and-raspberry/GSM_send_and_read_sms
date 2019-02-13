#include <Wire.h>
#include <SoftwareSerial.h>
#include <PZEM004T.h>

SoftwareSerial mySerial(7, 8); //GSM Shield Serial
boolean flag = false;
PZEM004T pzem(2,3);  // (RX,TX) connect to TX,RX of PZEM
IPAddress ip(192,168,1,1);

const static int bufferLength = 200;
char responseBuffer[ bufferLength ] = { '\0' };
int responseLength;

void setup() {
  Wire.begin(); // WireOne interface init
  Serial.begin(9600);
  pzem.setAddress(ip);
  mySerial.begin(9600);               // the GPRS baud rate
  checkGsmShield();  


  Serial.println("Initializing...");
  delay(1000);

  mySerial.println("AT"); //Handshaking with SIM900
  updateSerial();
  mySerial.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  mySerial.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  mySerial.println("AT+CREG?"); //Check whether it has registered in the network
  updateSerial();

  mySerial.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  updateSerial();
   
}

boolean sendAT(const char* command, const char* expectedResponse,  char *responseBuffer, int* bufferLength)
{
    mySerial.print(command);

    uint16_t i = 0;
    memset(responseBuffer, '\0', *bufferLength);
    unsigned long start = millis();

    while (i < *bufferLength && ((millis() - start) < 10000))
    {
        if (mySerial.available())
        {
            responseBuffer[i++] = (char)mySerial.read();
        }
        if (strstr(responseBuffer, expectedResponse) != NULL || strstr(responseBuffer, "ERROR") != NULL)
        {
            responseBuffer[i] = '\0';
            *bufferLength = i;
            return true;
        }
    }

    return false;
}


void sendSMS() {

  mySerial.begin(19200);               // the GPRS/GSM baud rate
  mySerial.print("\r");
  delay(1000);                    //Wait for a second while the modem sends an "OK"
  mySerial.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  delay(1000);
  mySerial.print("AT+CMGS=\"+79161206481\"\r");    //Start accepting the text for the message
  delay(1000);
  mySerial.print("No AC\r");   //The text for the message
  mySerial.write(0x1A);  //Equivalent to sending Ctrl+Z

}


void power_ON_Down() { // включение GSM/GPRS Shield

  pinMode(9, OUTPUT);
  digitalWrite(9, LOW);
  delay(1000);
  digitalWrite(9, HIGH);
  delay(2000);
  digitalWrite(9, LOW);
  delay(3000);

}


void checkGsmShield() // выключение GSM/GPRS Shield
{
char currSymb;
 delay(8000);  //wait for a 8 sec
 mySerial.println("ATD89161206481;"); // xxxxxxxxx is the number you want to dial.  
 
 if(mySerial.available())
  Serial.print((unsigned char)mySerial.read());
  mySerial.println("ATH"); //End the call.
 if(mySerial.available())
    Serial.print((unsigned char)mySerial.read()); 


 delay(1000);
     
// Считываем очередной символ с платы
currSymb = mySerial.read(); 
Serial.println(currSymb);
Serial.print((unsigned char)mySerial.read());

 if ('\r' == currSymb) { 
  Serial.println("mySerial.available = true");
  Serial.println(currSymb);
       
  
  } else {
    power_ON_Down();
    Serial.println("mySerial.available = false. Turning GSM Shield on");
    delay(9000);  //wait for a 2 sec
    Serial.println("GSM Shield is ON");
 }     
}


void checkVoltage() {
 

 float v = pzem.voltage(ip);
 Serial.print(v);Serial.print("V; ");

   if ((v < 0.0) && (flag == false)) {
    Serial.print(v);Serial.print("Alert; "); 
   
   //  power_ON_Down(); // включаем GSM/GPRS Shield
   //  delay(7000); // wait for a 7 sec
     sendSMS(); 

 //    power_ON_Down(); // включаем GSM/GPRS Shield
   //  delay(7000); // wait for a 7 sec
  //   sendSMS(); 

     
     delay(4000); 
     flag = true;
     return;
  }

  
}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}

void loop() {


  // responseLength = bufferLength;
 
 //   boolean i = sendAT("AT\r", "OK",  responseBuffer, &responseLength);
  //  Serial.println(responseBuffer);
//    Serial.println(i);
//    delay(9000);

updateSerial();

   // checkVoltage();
  //  sendSMS(); 


}
