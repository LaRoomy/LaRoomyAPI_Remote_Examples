#include <Arduino.h>
#include <FlashStorage.h>

typedef struct _NETWORK_CREDENTIALS {
    uint8_t isValid = 0;
    char _ssid[256];
    char _passwd[256];
}NETWORK_CREDENTIALS, *PNETWORK_CREDENTIALS;

FlashStorage(network_credentials, _NETWORK_CREDENTIALS);

class NetworkDataStorage
{
public:
    NetworkDataStorage() {}

    bool writeData(const char* ssid, const char* passwd)
    {
        if(ssid != nullptr && passwd != nullptr)
        {
            if(strlen(ssid) < 256 && strlen(passwd) < 256)
            {
                NETWORK_CREDENTIALS nw_cr;
                memset(&nw_cr._ssid, 0, sizeof(nw_cr._ssid));
                memset(&nw_cr._passwd, 0, sizeof(nw_cr._passwd));
                strcpy(nw_cr._ssid, ssid);
                strcpy(nw_cr._passwd, passwd);
                nw_cr.isValid = 7;

                network_credentials.write(nw_cr);

                return true;
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
    }

    bool readData(String& ssid_out, String& passwd_out)
    {
        NETWORK_CREDENTIALS nw_cr;
        network_credentials.read(&nw_cr);
        if(nw_cr.isValid == 7)
        {
            ssid_out = nw_cr._ssid;
            passwd_out = nw_cr._passwd;
            return true;
        }
        return false;
    }

    void eraseData()
    {
        NETWORK_CREDENTIALS nw_cr;
        memset(&nw_cr._ssid, 0, sizeof(nw_cr._ssid));
        memset(&nw_cr._passwd, 0, sizeof(nw_cr._passwd));
        nw_cr.isValid = 0;

        network_credentials.write(nw_cr);
    }
};
