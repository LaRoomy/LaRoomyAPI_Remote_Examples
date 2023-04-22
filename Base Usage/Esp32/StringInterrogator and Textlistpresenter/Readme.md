# StringInterrogator and TextListPresenter

This example demonstrates the usage of the StringInterrogator and TextListPresenter property.

## Hardware

- One LED is used to indicate the connection status
- Two hardware buttons are used to add some items to the TextListPresenter

The example schematic shows the common test circuit - not all elements are used.

![Test Circuit](TestCircuit_Esp32_Common.png)

## How the program works

Both properties are added. When the user has confirmed the input data on the StringInterrogator, the respective callback method of the remote handler implementation
