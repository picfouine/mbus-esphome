# mbus-esphome

## What is this all about?
This repository contains an integration for Mbus in ESPHome.
It allows a user to configure the mbus_controller component in /components as an external component.

### Mbus
Mbus, short for Meter-Bus, is a protocol to read out meters like water, gas and electricity meters.
Originally it was meant for remote reading of these utility meters in homes.
For practical (and obvious) reasons the physical setup is a bus system. The bus has no Chip Select kind of wire, so each meter has an address.

#### Mbus - physical
Bit of basics on the physical protocol: The bus wires have 36V across them. When the controller wants to send a byte to a meter, it indicates it wants to start transmitting by decreasing the voltage across the bus lines by 12V. Then it can switch between -12V or the original voltage to indicate a '0' or a '1'. The meter sends data back to the controller in a completely different way: it increases the current drawn on the bus by 11 to 20 milli Ampères.

Because of the (somewhat) higher voltage on the bus: be careful with it.

#### Mbus - protocol
The Mbus protocol is built on top of the OSI model. It mainly implements the Physical, Data link and Application layers. It also supports the Network layer, but this is NOT supported by this integration.

The Data link layer has multiple features like retries in case of no or incorrect responses.
It allows the Application layer to request so-called user data, which is just binary data to the Data link layer, and is interpreted in the Application layer.

The Application layer in the integration only supports a generic read out of the meter.
While the Mbus protocol supports much more, this is the basis that is required for the integration.

Meter data is sent in so-called data blocks. Each data block contains meta data about the value it contains. The data block indicates the size of the actual data, and its data type (e.g. 16-bit binary integer). Next to that it contains information on the semantics of the data. E.g. whether the value is an instantaneous value or some maximum or minimum value, whether the value is the most recent one or an older value. Even more important: it contains the type of physical quantity (e.g. energy, volume, volume flow, etc.) with the unit (e.g. Wh or J, m3, m3/h or m3/min or m3/s) and the power of ten with which the raw value has to be multiplied (e.g.: a volume with unit m3, and ten-power -3, is in 0.001 cubic meters, which is liters).

See [Meter-Bus on Wikipedia](https://en.wikipedia.org/wiki/Meter-Bus), and for much more in-depth information see [m-bus.com](https://m-bus.com/).