## Use the [LaRoomy App](https://www.laroomy.com/) to enter network credentials on your Esp32

When programming an Esp32 to log into a wifi network, there is often the problem that the device has no interface to enter the network credentials. Ok, the credentials could be hardcoded, but what if the wifi password or ssid has changed. Or what if the device must be moved between networks. This is where the [LaRoomy API](https://api.laroomy.com/) comes into play. With the LaRoomy API and the LaRoomy App, an minimum effort solution exists - Use it to add a Bluetooth interface quickly.

> To use Bluetooth and Wifi together on an Esp32 the partition sizes must be changed, due to the large size of the Bluetooth Library. Otherwise the binary size will not fit on the partition of the chip.
