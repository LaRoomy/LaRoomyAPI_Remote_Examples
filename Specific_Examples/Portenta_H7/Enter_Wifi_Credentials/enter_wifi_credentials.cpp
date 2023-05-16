#include <LaRoomyApi_STM32.h>
#include <portentaLedColor.h>
#include <WiFi.h>

// define the pin names
#define LED_1 0
#define LED_2 1
#define HBUTTON_1 2
#define HBUTTON_2 3

// define the control IDs - IMPORTANT: do not use zero as ID value and do not use an ID value more than once !!
#define CREDENTIAL_PROPERTY 1

// data keys
#define DATA_KEY_WIFI_SSID  "dk_wf_ssid_key"
#define DATA_KEY_WIFI_PASSWD    "dk_wf_psswd_key"

// control params
bool credentials_are_set = false;
bool wifi_active = false;

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
            setRGBLedColorD(PortentaRGBColor::GREENCOL);
        }
        else
        {
            Serial.println("Bluetooth Disconnected!");
            setRGBLedColorD(PortentaRGBColor::REDCOL);

            if(credentials_are_set){
                NVIC_SystemReset();
            }
        }
    }

    // receive string-interrogator data transmissions
    void onStringInterrogatorDataReceived(cID stringInterrogatorID, String &fieldOneContent, String &fieldTwoContent) override
    {
        // if a login attempt is executed, this callback method is called with the entered contents

        // monitor
        Serial.print("Wifi Credentials received. SSID: ");
        Serial.print(fieldOneContent);
        Serial.print("  Password: ");
        Serial.println(fieldTwoContent);

        // only process if both values are set - otherwise notify user
        if(fieldOneContent.length() > 0 && fieldTwoContent.length() > 0)
        {
            // save credentials
            FlashStorageManager.writeString(DATA_KEY_WIFI_SSID, fieldOneContent);
            FlashStorageManager.writeString(DATA_KEY_WIFI_PASSWD, fieldTwoContent);

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

    // while (!Serial) {
    //    ; // wait for serial port to connect. Needed for native USB port only
    // }

    initRGBLed();
    setRGBLedColorD(PortentaRGBColor::REDCOL);

    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    pinMode(HBUTTON_1, INPUT);
    pinMode(HBUTTON_2, INPUT);

    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);

    bool wifi_success = false;

    Serial.println("First check if credentials do exist..");

    // check if credentials were saved before
    String ssid = FlashStorageManager.readString(DATA_KEY_WIFI_SSID);
    String passwd = FlashStorageManager.readString(DATA_KEY_WIFI_PASSWD);

    if(ssid.length() > 0 && passwd.length() > 0)
    {
        Serial.print("Data loaded ->  ");

        // credentials do exist, try to connect
        Serial.print("Connecting to wifi..");
        WiFi.begin(ssid.c_str(), passwd.c_str());


        auto status =
                WiFi.status();

        if(status == WL_CONNECTED){
            wifi_success = true;
            wifi_active = true;
            digitalWrite(LED_2, HIGH);
            digitalWrite(LED_1, LOW);
            Serial.println("  Connected!");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());
        }
        else if(status == WL_CONNECT_FAILED){
            Serial.println("  Connection failed!");
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

    if(!wifi_active)
    {
        LaRoomyApi.onLoop();
    }

    // first hardware button is pressed
    if (digitalRead(HBUTTON_1) == LOW)
    {
        delay(10);

        // disconnect and clear wifi credentials, after that restart the controller

        Serial.print("Disconnecting from network..");

        // stop wifi
        WiFi.disconnect();
        while(WiFi.status() == WL_CONNECTED){
            delay(300);
            Serial.print(".");
        }

        // delete the ssid and password to invalidate the network
        FlashStorageManager.eraseData(DATA_KEY_WIFI_PASSWD);
        FlashStorageManager.eraseData(DATA_KEY_WIFI_SSID);
        delay(100);

        // restart controller
        Serial.println("Restarting..");
        //Serial.end();
        NVIC_SystemReset();

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