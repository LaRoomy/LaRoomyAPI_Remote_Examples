# Custom Descriptor Implementation
This example demonstrates how descriptors for properties could be set dynamically in relation to the language of the app the device is connected to.

# Hardware
- Two LEDs are used in this example. The LEDs can be toggled by the two button properties. The built-in LED indicates the connection status.

The schematic shows the common test circuit - not all elements are used.

![Test Circuit](TestCircuit_NanoIot_Common.png)

# What the program does

The properties are added to the [LaRoomy API](https://api.laroomy.com/), but all text elements are omitted. They will be requested dynamically later.
To get this working a subclass of the IElementDescriptionCallback interface must be defined and added to the LaRoomy API.
Later when the app requests the device properties a request is raised in this callback class and in a response to this the appropriate element text must be
provided.

# Resources
- [IElementDescriptionCallback](https://api.laroomy.com/p/descriptor-callback.html)
- [Property descriptor syntax](https://api.laroomy.com/2023/03/property-descriptor-syntax.html)


