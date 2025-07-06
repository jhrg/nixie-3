See the scope trace.

Yellow is the output of the 555, with Vcc = 15v

Purple is Vm - input to the 2N2222 connected to the 555 Control pin.

Added a RC Snubber between the inductor (L1) and MOSFET (Q2) and the diode (CR1).
This reduced output ripple a bit. With this supply there is still has a little ghosting
of the digits. See https://www.digikey.com/en/articles/resistor-capacitor-rc-snubber-design-for-power-switches
