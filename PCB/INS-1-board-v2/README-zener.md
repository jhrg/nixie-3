Zener diode power sources for INS-1 Nixie Tubes
July 1, 2025
Asking ChatGPT about a Power supply for INS-1 Nixie tubes using a zener diode.
Here’s how you size the series resistor (and check its power) step-by-step:

1. List your parameters
Supply voltage: $VV_\text{in}=180\text{ V}$

Zener voltage: $VV_Z=91\text{ V}$ (measured at its test current $I_{Zt}=4\text{ mA}$)

Tube load current: two tubes × 0.5 mA each ⇒ $I_L=1\text{ mA}$

Desired Zener bias: use the datasheet test current, $I_{Z,\text{bias}}=I_{Zt}=4\text{ mA}$

2. Compute resistor value
You need to supply both load and Zener bias current through RR.
$R  =  Vin−VZIL+IZ,bias=180−911 mA+4 mA=89 V5 mA=17.8 kΩR \;=\;\frac{V_\text{in}-V_Z}{I_L + I_{Z,\text{bias}}} =\frac{180-91}{1\,\text{mA}+4\,\text{mA}} =\frac{89\text{ V}}{5\text{ mA}} =17.8\text{ kΩ}$
Use the nearest standard: 18 kΩ.

3. Check currents (at any Vin variation)
At nominal 180 V:
 $IR=180−9118 kΩ≈4.94 mA⟹IZ=IR−IL≈4.94−1=3.94 mA I_R = \frac{180-91}{18\,\text{kΩ}}\approx4.94\text{ mA} \quad\Longrightarrow\quad I_Z = I_R - I_L \approx 4.94 - 1 = 3.94\text{ mA}$
 That’s essentially your 4 mA bias—right at the knee.


If your supply dipped to, say, 175 V:
 $IR=175−9118 kΩ≈4.67 mA⟹IZ≈3.67 mA I_R = \frac{175-91}{18\,\text{kΩ}}\approx4.67\text{ mA} \quad\Longrightarrow\quad I_Z\approx3.67\text{ mA}$
 Still above the knee current, so regulation holds.


If it rose to 185 V:
 $IR=185−9118 kΩ≈5.22 mA⟹IZ≈4.22 mA I_R = \frac{185-91}{18\,\text{kΩ}}\approx5.22\text{ mA} \quad\Longrightarrow\quad I_Z\approx4.22\text{ mA}$
 Well below any damaging level.



4. Check power dissipation
Resistor:
 $PR=(Vin−VZ)×IR≈89 V×0.005 A=0.445 W P_R = (V_\text{in}-V_Z)\times I_R \approx 89\text{ V}\times0.005\text{ A} =0.445\text{ W}$
 ⇒ use at least a 1 W resistor (or a ½ W with good ventilation, but 1 W is safer).


Zener:
 $PZ=VZ×IZ,bias=91 V×0.004 A=0.364 W P_Z = V_Z \times I_{Z,\text{bias}} =91\text{ V}\times0.004\text{ A} =0.364\text{ W}$
 ⇒ choose a Zener rated ≥ ½ W (or 1 W for head-room).



5. Final parts choice
Series resistor: 18 kΩ, 1 W


Zener diode: 91 V, ≥ 0.5 W (1 W preferred)


Optional: a small (0.1 µF–1 µF) bypass cap across the Zener if you ever see switching noise—though you said your supply is very clean, so it’s not strictly needed.


With that in place, your two Nixies (1 mA total) will see a rock-steady 91 V clamp, fed from your 180 V rail via the 18 kΩ/1 W resistor.
Asking about using the common emitter-follower + zener configuration

Adding an emitter–follower (i.e. a high-voltage NPN or N-channel device acting as a buffer) behind your Zener lets the Zener only source the transistor’s base (or gate) current instead of the entire tube load.  In practice that means:

1. **Much lower Zener bias current**

   * In our straight Zener clamp, the series resistor had to supply $I_L + I_{Z}$, e.g. 1 mA for the tubes plus 4 mA for the Zener → 5 mA total.
   * With an emitter-follower, the Zener only needs to supply the transistor’s base current:

     $$
       I_B \approx \frac{I_E}{\beta}
     $$

     If your transistor has a DC current gain ($\beta$) of, say, 100, then for 1 mA emitter current you only need \~10 µA of base current.  You’d typically pick a few tens of µA of Zener bias (say 50 µA) to keep it in regulation—two orders of magnitude less than before.

2. **Series-resistor power drops drastically**

   * Old design:

     $$
       P_R = (180 - 91)\,\text{V} \times 5\,\text{mA} \approx 0.445\,\text{W}
     $$

     ⇒ needed a ≥1 W resistor.
   * Emitter-follower design: if you set $I_{Z}+I_B\approx50\,\mu\text{A}$,

     $$
       P_R = 89\,\text{V} \times 50\,\mu\text{A} \approx 4.5\,\text{mW}
     $$

     ⇒ a tiny 1/10 W (or even 1/20 W) part is plenty.

3. **Zener diode dissipation plunges**

   * Previously $P_Z = 91\,\text{V}\times4\,\text{mA}\approx0.36\,\text{W}$.
   * Now $P_Z = 91\,\text{V}\times50\,\mu\text{A}\approx4.5\,\text{mW}$.

4. **Where the power goes**

   * The transistor now drops roughly the same 89 V at 1 mA, so $P_{\!Q}\approx89\,\text{mW}$.  That’s still small—any small HV-rated transistor (or MOSFET source follower) can handle it without a heat-sink.

5. **Voltage-drop considerations**

   * The emitter sits one V\_BE (\~0.6–0.8 V) below the Zener.  So if you clamp the base at 91 V, your tubes see \~90.2 V.  Make sure that still exceeds their strike voltage (91 V) or bump the Zener up a notch (e.g. 92 V).

---

### Bottom line

* **Yes**, with an emitter-follower you can reduce the Zener’s resistor and the Zener itself to wattages down in the milliwatt range.
* The trade-off is added complexity and the need for a high-voltage transistor (or MOSFET) rated above your 180 V rail.
* Regulation will also be **stiffer** (lower noise and better load-regulation) than a bare Zener clamp.

