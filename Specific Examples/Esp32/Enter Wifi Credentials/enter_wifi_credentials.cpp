#include <LaRoomyApi_Esp32.h>
#include <WiFi.h>

#include "esp32_data_storage.h"

/*
 * please note that this example will only work if the partition table is modified,
 * because the esp bluetooth library takes a lot of memory space
 * set the max partion table in your platformio.ini file by adding:
 * 
 *      board_build.partitions = huge_app.csv
*/

// define the pin names
#define CON_LED 2    // connection indicator led
#define HBUTTON_1 18 // hardware button 1
#define HBUTTON_2 19 // hardware button 2
#define LED_1 4      // led 1
#define LED_2 5      // led 2

// define the control IDs - IMPORTANT: do not use zero as ID value and do not use an ID value more than once !!
#define CREDENTIAL_PROPERTY 1

bool credentials_are_set = false;
bool startWifiConnection = false;

// data storage helper object
EspDataStorage dataStorage;

void laroomyApiSetupAndStart();

// define the callback for the remote app-user events - https://api.laroomy.com/p/laroomy-app-callback.html
class RemoteEvents : public ILaroomyAppCallback
{
public:
    // receive connection state change info
    void onConnectionStateChanged(bool newState) override
    {
        if (newState)
        {
            Serial.println("Bluetooth Connected!");
            digitalWrite(CON_LED, HIGH);
        }
        else
        {
            Serial.println("Bluetooth Disconnected!");
            digitalWrite(CON_LED, LOW);

            if(credentials_are_set){
                credentials_are_set = false;
                startWifiConnection = true;
                LaRoomyApi.end();
            }
        }
    }

    // receive string-interrogator data transmissions
    void onStringInterrogatorDataReceived(cID stringInterrogatorID, String &fieldOneContent, String &fieldTwoContent) override
    {
        // if a login attempt is executed, this callback method is called with the entered contents

        // monitor
        Serial.print("Credentials received. SSID: ");
        Serial.print(fieldOneContent);
        Serial.print("  Password: ");
        Serial.println(fieldTwoContent);

        // only process if both values are set - otherwise notify user
        if(fieldOneContent.length() > 0 && fieldTwoContent.length() > 0)
        {
            // save credentials
            dataStorage.writeData(WIFI_DATA_STORAGE_NAMESPACE, DATA_STORAGE_SSID_KEY, fieldOneContent);
            dataStorage.writeData(WIFI_DATA_STORAGE_NAMESPACE, DATA_STORAGE_PSWD_KEY, fieldTwoContent);
            // mark as set
            credentials_are_set = true;

            // both values are set - terminate bluetooth connection to try to log into wifi
            LaRoomyApi.sendCloseDeviceCommand();

            // now wait for the connection state to be set to false...
        }
        else {
            if(fieldOneContent.length() == 0){
                LaRoomyApi.sendUserMessage(UserMessageType::Warning, UserMessageHoldingPeriod::INFINITE, "SSID must not be empty");
            }
            else if(fieldTwoContent.length() == 0){
                LaRoomyApi.sendUserMessage(UserMessageType::Warning, UserMessageHoldingPeriod::INFINITE, "Password must not be empty");
            }
        }
    }
};

void setup()
{
    // put your setup code here, to run once:

    // monitor output for evaluation
    Serial.begin(115200);

    pinMode(CON_LED, OUTPUT);
    pinMode(HBUTTON_1, INPUT);
    pinMode(HBUTTON_2, INPUT);
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);

    digitalWrite(CON_LED, LOW);
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);

    bool wifi_success = false;

    Serial.println("First check if credentials do exist..");

    // check if credentials were saved before
    String ssid;
    dataStorage.readData(WIFI_DATA_STORAGE_NAMESPACE, DATA_STORAGE_SSID_KEY, ssid);

    String passwd;
    dataStorage.readData(WIFI_DATA_STORAGE_NAMESPACE, DATA_STORAGE_PSWD_KEY, passwd);

    if(ssid.length() > 0 && passwd.length() > 0)
    {
        Serial.print("Data found ->  ");

        int counter = 30;

        // credentials do exist, try to connect
        Serial.print("Connecting to wifi..");
        WiFi.begin(ssid.c_str(), passwd.c_str());

        // wait for a connection with a 15 sec timeout
        while(1){
            auto status = WiFi.status();
            if(status == WL_CONNECTED){
                wifi_success = true;
                digitalWrite(LED_2, HIGH);
                digitalWrite(LED_1, LOW);
                Serial.println("  Connected!");
                break;
            }
            else if(status == WL_CONNECT_FAILED){
                Serial.println("  Connection failed!");
                dataStorage.eraseData(WIFI_DATA_STORAGE_NAMESPACE, DATA_STORAGE_SSID_KEY);
                dataStorage.eraseData(WIFI_DATA_STORAGE_NAMESPACE, DATA_STORAGE_PSWD_KEY);
                ESP.restart();
                break;
            }
            delay(500);
            digitalWrite(LED_1, digitalRead(LED_1) ? LOW : HIGH);
            Serial.print(".");

            counter--;

            if(counter == 0){
                // timeout !
                Serial.println("Connection timeout !");
                dataStorage.eraseData(WIFI_DATA_STORAGE_NAMESPACE, DATA_STORAGE_SSID_KEY);
                dataStorage.eraseData(WIFI_DATA_STORAGE_NAMESPACE, DATA_STORAGE_PSWD_KEY);
                ESP.restart();
                break;
            }
        }
    }

    // when the wifi connection succeeds, proceed, otherwise start the laroomy api
    if(!wifi_success){
        Serial.println("No network credentials found -> starting LaRoomy Api");
        laroomyApiSetupAndStart();
    }
}

void loop()
{
    // put your main code here, to run repeatedly:

    if(startWifiConnection){
        startWifiConnection = false;

        Serial.print("Connecting to wifi..");

        String ssid;
        dataStorage.readData(WIFI_DATA_STORAGE_NAMESPACE, DATA_STORAGE_SSID_KEY, ssid);

        String passwd;
        dataStorage.readData(WIFI_DATA_STORAGE_NAMESPACE, DATA_STORAGE_PSWD_KEY, passwd);

        WiFi.begin(ssid.c_str(), passwd.c_str());

        while(1){
            auto status = WiFi.status();
            if(status == WL_CONNECTED){
                digitalWrite(LED_2, HIGH);
                digitalWrite(LED_1, LOW);
                Serial.println("  Connected!");
                break;
            }
            else if(status == WL_CONNECT_FAILED){
                Serial.println("  Connection failed!");
                break;
            }
            delay(500);
            digitalWrite(LED_1, digitalRead(LED_1) ? LOW : HIGH);
            Serial.print(".");
        }
    }

    // first hardware button is pressed
    if (digitalRead(HBUTTON_1) == LOW)
    {
        delay(10);

        // disconnect and clear wifi credentials, after that restart the controller

        Serial.print("Disconnecting from network..");

        digitalWrite(LED_1, LOW);
        digitalWrite(LED_2, LOW);

        // stop wifi
        WiFi.disconnect();
        while(WiFi.status() == WL_CONNECTED){
            delay(300);
            Serial.print(".");
        }

        // delete the ssid and password to invalidate the network
        dataStorage.eraseData(WIFI_DATA_STORAGE_NAMESPACE, DATA_STORAGE_SSID_KEY);
        dataStorage.eraseData(WIFI_DATA_STORAGE_NAMESPACE, DATA_STORAGE_PSWD_KEY);

        // restart esp
        Serial.println("Restarting..");
        ESP.restart();

        while (digitalRead(HBUTTON_1) == LOW)
            ;
        delay(50);
    }
}

// Check out the full documentation at: https://api.laroomy.com/
void laroomyApiSetupAndStart()
{
    // begin - https://api.laroomy.com/p/laroomy-api-class.html
    LaRoomyApi.begin();

    // set the bluetooth name
    LaRoomyApi.setBluetoothName("Network Access");

    // set the image for the device
    LaRoomyApi.setDeviceImage(LaRoomyImages::CONNECTION_SYNC_173);

    // enable device-binding auto handler
    LaRoomyApi.enableInternalBindingHandler(true);

    // enable standalone mode
    LaRoomyApi.setStandAloneMode(true);

    // set the callback handler for remote events
    LaRoomyApi.setCallbackInterface(
        dynamic_cast<ILaroomyAppCallback *>(
            new RemoteEvents()));

    // create the string interrogator
    StringInterrogator si;
    si.imageID = LaRoomyImages::STRING_REQUEST_166;
    si.stringInterrogatorDescription = "Network Credentials";
    si.stringInterrogatorID = CREDENTIAL_PROPERTY;

    // define the field attributes for the string-interrogator
    // https://api.laroomy.com/p/property-state-classes.html#laroomyApiRefMIDStringIS

    // set the text of the confirm-button
    si.stringInterrogatorState.buttonText = "Connect";

    // prevent back navigation
    si.stringInterrogatorState.navigateBackOnButtonPress = false;

    // set the attributes for field 1
    si.stringInterrogatorState.fieldOneDescriptor = "SSID";
    si.stringInterrogatorState.fieldOneHint = "Please enter ssid";
    si.stringInterrogatorState.fieldOneInputType = StringInterrogatorFieldInputType::SI_INPUT_TEXT; // this is the default

    // set the attributes for field 2
    si.stringInterrogatorState.fieldTwoDescriptor = "Password";
    si.stringInterrogatorState.fieldTwoHint = "Please enter password";
    si.stringInterrogatorState.fieldTwoInputType = StringInterrogatorFieldInputType::SI_INPUT_TEXT_PASSWORD;

    // add it
    LaRoomyApi.addDeviceProperty(si);

    // at last, call run to apply the setup and start bluetooth advertising
    LaRoomyApi.run();
}
