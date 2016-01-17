# i2c_frsky_receiver

A sketch for an Arduino Mini Pro to forward telemetry data received from MultiWii 
via I2C over a serial connection to FrSky D8R-ii plus receiver. The serial data
needs to be inverted before being fed into the digital RX port of the FrSky
receiver using a simple transistor circuit (see inverter_schematic.png).

Requires telemetry code with I2C support for MultiWii (see dosenflieger/multiwii 
repository, coming soon).


## Author

Lars Wessels, Karlsruhe, GERMANY  
Contact: github@dosenflieger.de


## License

This Software is licensed under GPLv3, with all the conditions the license implies.

This program is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software Foundation, 
either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY 
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR 
A PARTICULAR PURPOSE.