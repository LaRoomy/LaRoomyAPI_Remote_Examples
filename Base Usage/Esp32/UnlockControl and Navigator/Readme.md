# UnlockControl and Navigator

This is a demonstration program for the UnlockControl and the Navigator property.

## Hardware

- Three LEDs are used in this example. One to indicate the connection status. The other LEDs are to indicate the lock/unlock status
- One hardware button is used to reset the UnlockControl property

The example schematic shows the common test circuit - not all elements are used.

![Test Circuit](TestCircuit_Esp32_Common.png)

## What the program does

In setup, the properties are added. Note that the pin (if it was previously set) must be loaded and initially set to the state of the UnlockControl property.

If the UnlockControl property is in locked condition, the first LED is on. 
When the user unlocks the property with the correct pin, the second LED will be switched on and the first LED will be switched off. 
When the property is locked, the same is done vice versa.

The remote event callback implementation has two methods. The [onUnlockControlStateChanged](https://api.laroomy.com/p/laroomy-app-callback.html#laroomyApiRefMIDUCCtrlStateCh) is called when a lock or unlock action occurs or if a pin change action is done.
