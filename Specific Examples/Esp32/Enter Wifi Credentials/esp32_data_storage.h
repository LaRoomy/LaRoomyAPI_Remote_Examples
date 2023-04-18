#include <Arduino.h>
#include <nvs.h>

#define WIFI_DATA_STORAGE_NAMESPACE "ds_nmspce_gbl_"
#define DATA_STORAGE_SSID_KEY "ds_ssid_dkey_"
#define DATA_STORAGE_PSWD_KEY "ds_pswd_dkey_"

class EspDataStorage
{
public:
    EspDataStorage() {}

    bool writeData(const char* dNameSpace, const char* dKey, const String& data){
        return this->writeData(dNameSpace, dKey, data.c_str());
    }

    bool writeData(const char* dNameSpace, const char* dKey, const char* data)
    {
        esp_err_t res;
        bool vRes = false;

        // open
        res = nvs_open(dNameSpace, NVS_READWRITE, &this->nvsDataHandle);
        if (res == ESP_OK)
        {

            // write
            res = nvs_set_str(this->nvsDataHandle, dKey, data);
            if (res == ESP_OK)
            {

                // commit
                res = nvs_commit(this->nvsDataHandle);
                if (res == ESP_OK)
                {
                    vRes = true;
                }
            }
            // close & return
            nvs_close(this->nvsDataHandle);
        }
        return vRes;
    }

    bool readData(const char* dNameSpace, const char* dKey, String& data_out)
    {
        esp_err_t res;
        bool vRes = false;

        // open
        res = nvs_open(dNameSpace, NVS_READWRITE, &this->nvsDataHandle);
        if (res == ESP_OK)
        {
            size_t required_size = 0;

            // read size of string
            res = nvs_get_str(this->nvsDataHandle, dKey, nullptr, &required_size);
            if (res == ESP_OK)
            {
                // only proceed if size is above zero
                if (required_size > 0)
                {
                    // allocate mem
                    char *data = new char[required_size + (size_t)1];
                    if (data != nullptr)
                    {
                        // read data
                        res = nvs_get_str(this->nvsDataHandle, dKey, data, &required_size);
                        if (res == ESP_OK)
                        {
                            data_out = data;
                            vRes = true;
                        }
                        delete[] data;
                    }
                }
            }
            nvs_close(this->nvsDataHandle);
        }
        return vRes;
    }

    bool eraseData(const char* dNamespace, const char* dKey)
    {

        esp_err_t res;
        bool vRes = false;

        // open
        res = nvs_open(dNamespace, NVS_READWRITE, &this->nvsDataHandle);
        if (res == ESP_OK)
        {
            // erase
            res = nvs_erase_key(this->nvsDataHandle, dKey);
            if (res == ESP_OK)
            { // res == ESP_ERR_NVS_NOT_FOUND if key does not exist

                // commit
                res = nvs_commit(this->nvsDataHandle);
                if (res == ESP_OK)
                {
                    vRes = true;
                }
            }
            // close & return
            nvs_close(this->nvsDataHandle);
        }
        return vRes;
    }

private:
    nvs_handle nvsDataHandle;
};
