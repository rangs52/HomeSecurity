#include <RCSwitch.h>
RCSwitch radioReciever = RCSwitch();
int debugCounter = 0;

int deviceAddress[] = {
  8214352,  // Front Door
  14114128, // Back door
  5592405,  // IR Motion sensor
  6100304   // First floor door
  //Add the new device address here
};


char *names[] = { "Front door opened",
                  "Back door oopened" ,
                  "IR Motion sensor activated",
                  "First floor door opened"
}; 

#define RECENT_PAST 100
const int NUM_OF_DEVICES = sizeof(deviceAddress)/sizeof(int); //The number of devices that we currently have
unsigned long lastTriggerTime[NUM_OF_DEVICES] = {0};        //This keeps track of the last trigger time [obtained via micros()] for each device
boolean  firstTimeTriggerStatus[NUM_OF_DEVICES] = {false}; //This keeps track whether the device has atleast been triggered once


void setup() {
  radioReciever.enableReceive(0);  // Receiver on inerrupt 0 => that is pin #2
  //Set up the GSM modem
  Serial.begin(9600);  
  Serial.println("AT"); //To check if GSM module is working
  delay(2000);
  Serial.println("AT+CMGF=1"); // set the SMS mode to text
  delay(1500);
}

void loop() {
  if (radioReciever.available()) {
    long currentTime = micros();
    int currentDeviceAddress = radioReciever.getReceivedValue();
    printDebug(); 
    int currentDeviceIndex = getDeviceId(currentDeviceAddress);
    if (currentDeviceAddress != -1) {
      //Check if this is the first time this device is triggered
      //if so we just send out the alert SMS
      if(!firstTimeTriggerStatus[currentDeviceIndex]) {
        sendAlertSMS(currentDeviceIndex);
        callPhone();
        firstTimeTriggerStatus[currentDeviceIndex] = true;
      } else {
        //Check if the device is not triggered the first time, then check if the 
        //device has been triggered in the recent past,if not we need to send out 
        //SMS
        int timeDiff = currentTime - lastTriggerTime[currentDeviceIndex];
        //timeDiff might be negative in case the micros overflows
        if(abs(timeDiff) > RECENT_PAST) {
          sendAlertSMS(currentDeviceIndex);
          lastTriggerTime[currentDeviceIndex] = currentTime;
        } else {
          //Seems like we have sent the SMS only a few seconds back , lets 
          //not do anything
        }
      }
    } else {
      //Seems like we have recieved the data from an unkown device
    }
    radioReciever.resetAvailable();
  }
}

int getDeviceId(int currentDeviceAddress) {
    for(int i =0;i<NUM_OF_DEVICES;i++) {
      if(deviceAddress[i] == currentDeviceAddress) {
          return i;
      }
    }
    return -1;
}

void sendAlertSMS(int deviceId) {
  delay(2500);
  Serial.print("AT+CMGS=");
  Serial.write(34); //ASCII of "
  Serial.print("+919443505195");
  Serial.write(34); //ASCII of "
  Serial.print("\r\n");
  Serial.print(names[deviceId]);  
  Serial.write(26); //ASCII of [ctrl + z]
  Serial.print("\r\n");
  delay(2500);  
}

void callPhone() {
  delay(2500);  
  Serial.print("ATD9443505195;\r\n");
  delay(20000);
  Serial.print("ATH");
  delay(1000);
  
}

void printDebug() {
  Serial.print(debugCounter++);
  Serial.print(":Received ");
  Serial.print( radioReciever.getReceivedValue() );
  Serial.print(" / ");
  Serial.print( radioReciever.getReceivedBitlength() );
  Serial.print("bit ");
  Serial.print("Protocol: ");
  Serial.println( radioReciever.getReceivedProtocol() );
  Serial.print("Address: ");
  Serial.println(radioReciever.getReceivedValue());
}
