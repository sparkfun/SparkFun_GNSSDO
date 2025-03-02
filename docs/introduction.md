---
icon: material/book-open-page-variant
---

# Introduction
<div class="grid cards desc" markdown>

-   <a href="https://www.sparkfun.com/sparkpnt-gnss-disciplined-oscillator.html">
	**GNSSDO**<br>
	**SKU:** GPS-26289

	---

	<figure markdown>
	![Product Thumbnail](https://cdn.sparkfun.com/assets/parts/2/7/2/0/7/26289-GNSS-Disciplined-Oscillator-Front-Lit.jpg)
	</figure></a>


	<center>
	<article class="video-500px">
	<iframe src="https://www.youtube.com/embed/WI0NbXBxeUo" title="Product Showcase Video" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>
	</article>
	![QR code to product video](./assets/img/qr_code/product_video.png){ .qr width=100 }
	</center>


	<center>
	[Purchase from SparkFun :fontawesome-solid-cart-plus:{ .heart } ![QR code to product page](./assets/img/qr_code/product-low.png){ .tinyqr }](https://www.sparkfun.com/sparkpnt-gnss-disciplined-oscillator.html){ .md-button .md-button--primary }
	</center>


-	Designed and manufactured in Boulder, Colorado, USA, the SparkPNT GNSSDO is the perfect solution for your high-precision timing needs. Based around the multi-constellation, multi-frequency, L5-ready mosaic-T from Septentrio, this is our most accurate GNSS timing product to date. It features a SiTime SiT5358 disciplined 10MHz Digitally-Controlled Temperature-Controlled Crystal Oscillator (DCTCXO) providing excellent clock accuracy, stability and holdover. The mosaic-T also has built-in on-module support for the Fugro AtomiChron L-band timing service.

	Under the hood, the GNSSDO is based on the mosaic-T GNSS module from Septentrio, plus the Espressif ESP32-WROVER processor (16MB flash, 8MB PSRAM). The mosaic-T has USB-C connectivity (with Ethernet-over-USB), multiple UARTs and supports full Ethernet connectivity. You can connect the mosaic-T directly to your Ethernet network - our product supports Power-over-Ethernet too. The ESP32 rides shotgun, disciplining the DCTCXO and controlling the OLED display. The GNSSDO has multiple power options including USB-C, PoE and 9V-36V DC, making it easy to connect it to a battery-backed supply. Robust SMA connections and screw cage terminals provide access to the Pulse-Per-Second and 10MHz clock signals.


	??? info "Features"
		- Septentrio mosaic-T multi-constellation, multi-frequency GNSS timing receiver
			- Accurate and resilient with dedicated timing features
			- Highly secure against jamming and spoofing with AIM+ unique interference mitigation technology combined with Fugro AtomiChron services
			- Update rate of 10 Hz
			- 448 hardware channels for simultaneous tracking of all visible supported satellite signals:
				- GPS: L1C/A, L1PY, L2C, L2P, L5
				- GLONASS: L1CA, L2CA, L2P, L3 CDMA
				- Beidou: B1I, B1C, B2a, B2b, B2I, B3
				- Galileo: E1, E5a, E5b, E5 AltBoc, E6
				- QZSS: L1C/A, L1 C/B, L2C, L5
				- NavIC: L5
				- SBAS: Egnos, WAAS, GAGAN, MSAS, SDCM (L1, L5)
				- On-module L-band
			- Supports RTCM input for RTK Rover. **RTCM output (RTK Base) is not supported.**
			- Time pulse precision: 5ns
			- Time pulse precision with AtomiChron (L-Band or IP): < 1ns
			- Event accuracy: < 20ns
			- Operating temperature: -40 to 85 °C
			- USB-C interface (UART and Ethernet-over-USB)
		- ESP32-WROVER processor (16MB flash, 8MB PSRAM)
			- USB-C interface (UART via CH340)
		- SiT5358 disciplined 10MHz oscillator
			- ±50ppb stability
			- ±1ppb/°C frequency slope
			- ±58ppb typical 20-year aging
			- Digital frequency pulling via I²C
			- [Allan Deviation](./assets/img/hookup_guide/Allan_Deviation.jpg) approaches 1E-14 at 10000 seconds with AtomiChron enabled
			- Operating temperature: -40 to 85 °C (Industrial)
		- microSD socket
			- Connected directly to the mosaic-T for fast data logging
		- OLED display
			- 128x64 pixels
		- Status LEDs


	??? info "Connectivity Options"
		- SMA Connections:
			- GNSS Antenna (L1/L2/L5/L-Band) - provides 3.3V for an active antenna
			- 10MHz Output - disciplined, configurable for 5V / 3.3V / 2.8V / 1.8V and 50 Ohm
			- 10MHz Input - switchable, input impedance 50Ω, detection level -14dBm, max supported level +12dBm
			- Pulse-Per-Second Square Wave - configurable for 5V / 3.3V / 2.8V / 1.8V and 50 Ohm
			- EventA Input - configurable for 5V / 3.3V / 2.8V / 1.8V and 50 Ohm
		-	3.5mm Screw Cage Connections:
			- 9V-36V DC input (isolated)
			- GND
			- mosaic-T COM2 (TX/RX/CTS/RTS) - 3.3V / 5V switchable
			- CTS can be configured as a 3.3V / 5V power output via a solder jumper
			- EventB input - 3.3V / 5V switchable
			- I<sup>2</sup>C (SCL2 / SDA2) for an external TCXO / OCXO - 3.3V / 5V switchable
		-	Ethernet:
			- KSZ8041NLI Ethernet PHY interface
			- 10Base-T / 100Base-TX with auto-negotiate and Auto MDI/MDI-X
		-	Power Options:
			- USB-C
			- Power-over-Ethernet (PoE)
			- 9V-36V DC input (fully isolated)


	!!! warning "RTK Base"
		The mosaic-T is a superb GNSS module designed for highly accurate timing applications. It can be configured as a RTK Rover and will make use of incoming RTCM corrections. **However, it can <span style="color:red;">NOT</span> be configured as a <span style="color:red;">RTK Base</span> because the RTCM output is not supported.**

</div>


??? question "Product Comparison"
	Below is a simple comparison table between our breakout board and Septentrio's development and evaluation kits:

	<div class="annotate" markdown align="center">
	<table markdown>
	<tr markdown>
	<td></td>
	<th markdown style="text-align:center">
		mosaic-T Development Kit<br>
		<hr>
		<figure markdown>
		![Product Thumbnail](./assets/img/product_comparison/mosaic-development-kit.png)
		</figure>
	</th>
	<th markdown style="text-align:center">
		mosaic-go Evaluation Kit<br>
		<hr>
		<figure markdown>
		![Product Thumbnail](./assets/img/product_comparison/mosaic-go-evaluation-kit.png)
		</figure>
	</th>
	<th markdown style="text-align:center">
		mosaic-X5 GNSS Breakout<br>
		<hr>
		<figure markdown>
		![Product Thumbnail](./assets/img/product_comparison/tri-band_gnss_breakout.png)
		</figure>
	</th>
	<th markdown style="text-align:center">
		RTK mosaic-X5<br>
		<hr>
		<figure markdown>
		![Product Thumbnail](./assets/img/product_comparison/rtk_mosaic-x5.png)
		</figure>
	</th>
	<th markdown style="text-align:center">
		GNSSDO<br>
		<hr>
		<figure markdown>
		![Product Thumbnail](./assets/img/product_comparison/rtk_mosaic-x5.png)
		</figure>
	</th>
	</tr>
	<tr>
		<td>RTK Rover</td>
		<td style="text-align:center">Yes</td>
		<td style="text-align:center">Yes</td>
		<td style="text-align:center">Yes</td>
		<td style="text-align:center">Yes</td>
		<td style="text-align:center">Yes</td>
	</tr>
	<tr>
		<td>RTK Base</td>
		<td style="text-align:center">Yes</td>
		<td style="text-align:center">Yes</td>
		<td style="text-align:center">Yes</td>
		<td style="text-align:center">Yes</td>
		<td style="text-align:center">**No**</td>
	</tr>
	<tr>
		<td style="vertical-align:middle;">GNSS Antenna</td>
		<td style="text-align:center; vertical-align:middle;">Dual</td>
		<td style="text-align:center">
			Single (mosaic-X5)<br>
			Dual (mosaic-H)
		</td>
		<td style="text-align:center; vertical-align:middle;">Single</td>
		<td style="text-align:center; vertical-align:middle;">Single</td>
		<td style="text-align:center; vertical-align:middle;">Single</td>
	</tr>
	<tr>
		<td>USB Connector</td>
		<td style="text-align:center">micro-B</td>
		<td style="text-align:center">micro-B</td>
		<td style="text-align:center">Type-C</td>
		<td style="text-align:center">Type-C</td>
		<td style="text-align:center">Type-C</td>
	</tr>
	<tr>
		<td style="vertical-align:middle;">Ethernet</td>
		<td style="text-align:center; vertical-align:middle;">
			Yes<br>
			<i>10/100 Base-T</i>
		</td>
		<td style="text-align:center; vertical-align:middle;">No</td>
		<td style="text-align:center; vertical-align:middle;">No</td>
		<td style="text-align:center">
			Yes<br>
			<i>10/100 Base-T</i>
		</td>
		<td style="text-align:center">
			Yes<br>
			<i>10/100 Base-T</i>
		</td>
	</tr>
	<tr>
		<td style="vertical-align:middle;">WiFi</td>
		<td style="text-align:center; vertical-align:middle;">No</td>
		<td style="text-align:center; vertical-align:middle;">No</td>
		<td style="text-align:center; vertical-align:middle;">No</td>
		<td style="text-align:center">
			Yes - Network Bridge<br>
			<i>10 Base-T</i>
		</td>
		<td style="text-align:center; vertical-align:middle;">No</td>
	</tr>
	<tr>
		<td style="vertical-align:middle;">COM Ports</td>
		<td style="text-align:center">4</td>
		<td style="text-align:center">2</td>
		<td style="text-align:center">4</td>
		<td style="text-align:center">
			1 - mosaic-X5<br>
			1 - ESP32
		</td>
		<td style="text-align:center">
			1 - mosaic-T<br>
			1 - ESP32
		</td>
	</tr>
	<tr>
		<td>&micro;SD Card Slot</td>
		<td style="text-align:center">Yes</td>
		<td style="text-align:center">Yes</td>
		<td style="text-align:center">Yes</td>
		<td style="text-align:center">Yes</td>
		<td style="text-align:center">Yes</td>
	</tr>
	<tr>
		<td style="vertical-align:middle;">Reset/Log Buttons</td>
		<td style="text-align:center; vertical-align:middle;">Yes</td>
		<td style="text-align:center; vertical-align:middle;">No*</td>
		<td style="text-align:center; vertical-align:middle;">Yes</td>
		<td style="text-align:center; vertical-align:middle;">Yes</td>
		<td style="text-align:center; vertical-align:middle;">Yes</td>
	</tr>
	<tr>
		<td style="vertical-align:middle;">Logic-Level</td>
		<td style="text-align:center">
			1.8V<br>
			3.3V
		</td>
		<td style="text-align:center; vertical-align:middle;">3.3V</td>
		<td style="text-align:center; vertical-align:middle;">3.3V</td>
		<td style="text-align:center">
			3.3V<br>
			5V
		</td>
		<td style="text-align:center">
			3.3V<br>
			5V
		</td>
	</tr>
	<tr>
		<td>PPS Signal</td>
		<td style="text-align:center">Header Pin</td>
		<td style="text-align:center">6-Pin JST Connector</td>
		<td style="text-align:center">SMA Connector</td>
		<td style="text-align:center">Screw Terminal</td>
		<td style="text-align:center">SMA Connector</td>
	</tr>
	<tr>
		<td style="vertical-align:middle;">10MHz Signal</td>
		<td style="text-align:center">
			SMA Connector<br>
			<i>In Only</i>
		</td>
		<td style="text-align:center; vertical-align:middle;">No</td>
		<td style="text-align:center; vertical-align:middle;">No</td>
		<td style="text-align:center; vertical-align:middle;">No</td>
		<td style="text-align:center">
			SMA Connector<br>
			<i>In/Out</i>
		</td>
	</tr>
	<tr>
		<td>Enclosure Material</td>
		<td style="text-align:center; vertical-align:middle;">N/A</td>
		<td style="text-align:center; vertical-align:middle;">Metal</td>
		<td style="text-align:center; vertical-align:middle;">N/A</td>
		<td style="text-align:center; vertical-align:middle;">Aluminum</td>
		<td style="text-align:center; vertical-align:middle;">Aluminum</td>
	</tr>
	<tr>
		<td style="vertical-align:middle;">Dimensions</td>
		<td style="text-align:center; vertical-align:middle;">N/A</td>
		<td style="text-align:center; vertical-align:middle;">71 x 59 x 12mm ± 1mm</td>
		<td style="text-align:center; vertical-align:middle;">70.9 x 50.8 x 8mm</td>
		<td style="text-align:center">
			180.6 x 101.8 x 41mm<br>
			<i>Enclosure Only</i>
		</td>
		<td style="text-align:center">
			180.6 x 101.8 x 41mm<br>
			<i>Enclosure Only</i>
		</td>
	</tr>
	<tr>
		<td style="vertical-align:middle;">Weight</td>
		<td style="text-align:center; vertical-align:middle;">N/A</td>
		<td style="text-align:center; vertical-align:middle;">58g  ± 1g</td>
		<td style="text-align:center; vertical-align:middle;">22.6g</td>
		<td style="text-align:center">
			415.15g<br>
			<i>Enclosure Only</i>
		</td>
		<td style="text-align:center">
			415.15g<br>
			<i>Enclosure Only</i>
		</td>
	</tr>
	
	</table>
	
	</div>

	!!! note "mosaic-go Evaluation Kit"
		The reset pin is exposed on 4-pin JST connector and the log pin is connected to the latch pin of the SD card slot.



## :fontawesome-solid-list-check: Required Materials
The GNSSDO comes with everything you need to get up and running.

**Kit Contents**

---

<div class="grid" markdown>

<div markdown>

<figure markdown>

[![Kit contents](https://cdn.sparkfun.com/assets/parts/2/7/2/0/7/26289-GNSS-Disciplined-Oscillator-Kit-Feature.jpg){ width="300" }](https://cdn.sparkfun.com/assets/parts/2/7/2/0/7/26289-GNSS-Disciplined-Oscillator-Kit-Feature.jpg "Click to enlarge")
<figcaption markdown>
Everything that is included in the GNSSDO kit.
</figcaption>
</figure>

</div>

<div markdown>

<div class="annotate" markdown>

- [Quick Start Guide](./assets/quick_start_guide-v10.pdf)
- Cased GNSS Receiver
	- [Aluminum Enclosure](https://www.sparkfun.com/metal-enclosure-custom-aluminum-extrusion-6in-x-4in-pcb.html) (1)
	- [10-Way Terminal Blocks](https://www.sparkfun.com/10-way-terminal-socket.html)
	- [Qwiic 1.3" OLED Display](https://www.sparkfun.com/sparkfun-qwiic-oled-1-3in-128x64.html)
- [L1/L2/L5 GNSS Surveying Antenna](https://www.sparkfun.com/gnss-multi-band-l1-l2-l5-surveying-antenna-tnc-spk6618h.html)
- [Reinforced RG58 TNC-SMA Cable (10m)](https://www.sparkfun.com/reinforced-interface-cable-sma-male-to-tnc-male-10m.html)
- [32GB microSD Card (Class 10)](https://www.sparkfun.com/microsd-card-32gb-class-10.html)
- [USB A and C Power Delivery (PD) Wall Adapter - 65W](https://www.sparkfun.com/usb-a-and-c-power-delivery-pd-wall-adapter-65w.html)
- [USB-C to USB-C Cable (Flexible Silicone, 3m)](https://www.sparkfun.com/usb-c-to-usb-c-silicone-power-charging-cable-3m.html)
- [Ethernet Cable (CAT-6, 1m)](https://www.sparkfun.com/cat-6-cable-3ft.html)
- [SMA to BNC Cable (RG316, 1m)](https://www.sparkfun.com/sma-to-bnc-male-cable-rg316-1m.html)
- [Silicone Bumpers - 5x11mm (4 Pack)](https://www.sparkfun.com/silicone-bumpers-5x11mm-4-pack.html)

</div>

1. The linked product does not include the front/rear panels and stickers for the GNSSDO. Additionally, the product only features a red anodized plating and is not powder coated in the Septentrio orange.

</div>

</div>


???+ note "Mounting Hardware"
	This kit does not include any mounting hardware for the antenna. If you wish to permanently mount the antenna outside, we recommend the following products:

	<div class="grid cards" markdown>

	-   <a href="https://www.sparkfun.com/gnss-antenna-mounting-hardware-kit.html">
		<figure markdown>
		![GNSS Antenna Mounting Hardware Kit](https://cdn.sparkfun.com//assets/parts/2/2/0/9/7/22197-_01.jpg)
		</figure>

		---

		**GNSS Antenna Mounting Hardware Kit**<br>
		KIT-22197</a>

	-   <a href="https://www.sparkfun.com/gnss-magnetic-antenna-mount-5-8-11-tpi.html">
		<figure markdown>
		![GNSS Magnetic Antenna Mount - 5/8" 11-TPI](https://cdn.sparkfun.com//assets/parts/2/1/0/2/7/SparkFun-GNSS-Antenna-Magnetic-Mount-21257-1.jpg)
		</figure>

		---

		**GNSS Magnetic Antenna Mount - 5/8" 11-TPI**<br>
		PRT-21257</a>

	</div>

	!!! tip
		If needed, use an extension cable to reach your installation location. We have daisy chained up to three RG58 10m cables with a marginal loss in signal strength.


??? note "Extension Cables"
	Your GNSSDO is equally at home on your desk, lab bench, or in a server rack. But you're still going to want to put the GNSS antenna outdoors, so it will have the best view of the sky. Some extra SMA extension cables may be useful. The `GNSS` SMA antenna connection from the SparkPNT GNSSDO has a standard polarity.

	For shorter extensions, we have **RG178** cables in 1m and 25cm lengths. For longer extensions, we have higher quality, **low-loss RG58** 10m cables available in our catalog.

	<div class="grid cards" markdown>

	-   <a href="https://www.sparkfun.com/interface-cable-sma-female-to-sma-male-10m-rg58.html">
		<figure markdown>
		![Interface Cable - SMA Female to SMA Male (10m, RG58)](https://cdn.sparkfun.com//assets/parts/2/1/0/6/5/21281-_CAB-_01.jpg)
		</figure>

		---

		**Interface Cable - SMA Female to SMA Male (10m, RG58)**<br>
		CAB-21281</a>

	-   <a href="https://www.sparkfun.com/interface-cable-sma-male-to-sma-female-cable-1m-rg174.html">
		<figure markdown>
		![Interface Cable - SMA Male to SMA Female Cable (1M, RG174)](https://cdn.sparkfun.com/assets/parts/2/1/9/0/2/22035-_CAB-_01.jpg)
		</figure>

		---

		**Interface Cable - SMA Male to SMA Female Cable (1M, RG174)**<br>
		CAB-22035</a>

	-   <a href="https://www.sparkfun.com/interface-cable-sma-male-to-sma-female-25cm-rg174.html">
		<figure markdown>
		![Interface Cable - SMA Male to SMA Female (25cm, RG174)](https://cdn.sparkfun.com/assets/parts/2/1/9/0/1/22034-_CAB-_01.jpg)
		</figure>

		---

		**Interface Cable - SMA Male to SMA Female (25cm, RG174)**<br>
		CAB-22034</a>

	</div>


??? note "SMA Adapters"
	We have several adapters for users who need to connect to the `EVENTA`, `PPS`, and/or `10MHz` signals for their server or test equipment. These SMA connections from the SparkPNT GNSSDO have a standard polarity.

	<div class="grid cards" markdown>

	-   <a href="https://www.sparkfun.com/sma-male-to-rpsma-male-adapter.html">
		<figure markdown>
		![SMA Male to RPSMA Male Adapter](https://cdn.sparkfun.com/assets/parts/2/7/0/6/09233-5.jpg)
		</figure>

		---

		**SMA Male to RPSMA Male Adapter**<br>
		WRL-09233</a>

	-   <a href="https://www.sparkfun.com/sma-male-to-sma-male-adapter.html">
		<figure markdown>
		![SMA Male to SMA Male Adapter](https://cdn.sparkfun.com/assets/parts/2/0/9/9/4/WRL-21225.jpg)
		</figure>

		---

		**SMA Male to SMA Male Adapter**<br>
		WRL-21225</a>

	-   <a href="https://www.sparkfun.com/sma-to-u-fl-cable-150mm.html">
		<figure markdown>
		![SMA to U.FL Cable - 150mm](https://cdn.sparkfun.com/assets/parts/1/8/0/2/9/18568-Scale.jpg)
		</figure>

		---

		**SMA to U.FL Cable - 150mm**<br>
		WRL-18568</a>

	-   <a href="https://www.sparkfun.com/sma-to-bnc-male-cable-rg316-1m.html">
		<figure markdown>
		![SMA to BNC Male Cable - 1m (RG316)](https://cdn.sparkfun.com/assets/parts/2/8/3/6/7/27480-SMA-to-BNC-Male-Cable-RG316-FEature.jpg)
		</figure>

		---

		**SMA to BNC Male Cable - 1m (RG316)**<br>
		CAB-27480</a>

	</div>


??? note ":material-weather-pouring: Selecting an Outdoor Enclosure"

	The GNSSDO comes in a beautiful custom extruded aluminum enclosure, with machined end panels and matching stickers. The slotted flanges make it easy to install and secure the enclosure in many locations. But the enclosure only provides limited protection against the ingress of dust and water; it is designed to IP42. So, if you are going to permanently install it up on the roof too, you're going to need a suitable weatherproof box. We found a good one - the [Orbit 57095](https://www.orbitonline.com/products/gray-outdoor-timer-cabinet) - also available from [Amazon](https://www.amazon.com/Orbit-57095-Weather-Resistant-Outdoor-Mounted-Controller/dp/B000VYGMF2) - back when we put together our very first [DIY GNSS Reference Station](https://learn.sparkfun.com/tutorials/how-to-build-a-diy-gnss-reference-station#mini-computer-setup-option-1).

	<div class="grid" markdown>

	<div class="grid cards" markdown align="center">

	-   <a href="https://learn.sparkfun.com/tutorials/1363">
		<figure markdown>
		![Tutorial Thumbnail](https://cdn.sparkfun.com/c/178-100/assets/learn_tutorials/1/3/6/3/Roof_Enclosure.jpg)
		</figure>

		---

		**How to Build a DIY GNSS Reference Station**</a>

	</div>

	<div markdown>

	!!! info "AC Not Required!"
		The Orbit enclosure comes with a built-in power outlet, but you don't need it! The GNSSDO can be powered by [Power-over-Ethernet (PoE)](https://en.wikipedia.org/wiki/Power_over_Ethernet), meaning all you really need to run up to the roof is a standard 8-core CAT-6 Ethernet cable. Choose a PoE Ethernet Switch that meets your needs. We have had good experiences with the [TP-Link TL-SG1005P](https://www.tp-link.com/us/business-networking/poe-switch/tl-sg1005p/) - available from many retailers including [Amazon](https://www.amazon.com/TP-Link-Compliant-Shielded-Optimization-TL-SG1005P/dp/B076HZFY3F).

	</div>

	</div>


## :material-bookshelf: Suggested Reading

As a more sophisticated product, we will skip over the more fundamental tutorials (i.e. [**Ohm's Law**](https://learn.sparkfun.com/tutorials/voltage-current-resistance-and-ohms-law) and [**What is Electricity?**](https://learn.sparkfun.com/tutorials/what-is-electricity)). However, below are a few tutorials that may help users familiarize themselves with various aspects of the board.


<div class="grid cards" markdown align="center">

-   <a href="https://learn.sparkfun.com/tutorials/9">
	<figure markdown>
	![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/parts/5/9/7/4/10890-01.jpg)
	</figure>

	---

	**GPS Basics**</a>

-   <a href="https://learn.sparkfun.com/tutorials/813">
	<figure markdown>
	![Tutorial Thumbnail](https://cdn.sparkfun.com/c/178-100/assets/learn_tutorials/8/1/3/Location-Wandering-GPS-combined.jpg)
	</figure>

	---

	**What is GPS RTK?**</a>

-   <a href="https://learn.sparkfun.com/tutorials/1363">
	<figure markdown>
	![Tutorial Thumbnail](https://cdn.sparkfun.com/c/178-100/assets/learn_tutorials/1/3/6/3/Roof_Enclosure.jpg)
	</figure>

	---

	**How to Build a DIY GNSS Reference Station**</a>

-   <a href="https://learn.sparkfun.com/tutorials/1362">
	<figure markdown>
	![Tutorial Thumbnail](https://cdn.sparkfun.com/c/178-100/assets/learn_tutorials/1/3/6/2/GNSS_RTK_DIY_Surveying_Tutorial.jpg)
	</figure>

	---

	**Setting up a Rover Base RTK System**</a>

-   <a href="https://docs.sparkfun.com/SparkFun_RTK_mosaic-X5">
	<figure markdown>
	![Tutorial Thumbnail](https://docs.sparkfun.com/SparkFun_RTK_mosaic-X5/assets/img/thumbnail.jpg)
	</figure>

	---

	**RTK mosaic-X5 Hookup Guide**</a>

-   <a href="https://docs.sparkfun.com/SparkFun_GNSS_mosaic-X5">
	<figure markdown>
	![Tutorial Thumbnail](https://docs.sparkfun.com/SparkFun_GNSS_mosaic-X5/assets/img/thumbnail.jpg)
	</figure>

	---

	**mosaic-X5 GNSS Breakout Board Hookup Guide**</a>

-   <a href="https://learn.sparkfun.com/tutorials/908">
	<figure markdown>
	![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/learn_tutorials/9/0/8/USB-to-serial_converter_CH340-closeup.jpg)
	</figure>

	---

	**How to Install CH340 Drivers**</a>

-   <a href="https://learn.sparkfun.com/tutorials/112">
	<figure markdown>
	![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/learn_tutorials/1/1/2/thumb.jpg)
	</figure>

	---

	**Serial Terminal Basics**</a>

-   <a href="https://learn.sparkfun.com/tutorials/8">
	<figure markdown>
	![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/7/d/f/9/9/50d24be7ce395f1f6c000000.jpg)
	</figure>

	---

	**Serial Communication**</a>

-   <a href="https://docs.sparkfun.com/SparkFun_Qwiic_OLED_1.3in">
	<figure markdown>
	![Tutorial Thumbnail](https://docs.sparkfun.com/SparkFun_Qwiic_OLED_1.3in/assets/img/thumbnail.jpg)
	</figure>

	---

	**Qwiic OLED 1.3" Hookup Guide**</a>

-   <a href="https://learn.sparkfun.com/tutorials/82">
	<figure markdown>
	![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/learn_tutorials/8/2/I2C-Block-Diagram.jpg)
	</figure>

	---

	**I2C**</a>

-   <a href="https://learn.sparkfun.com/tutorials/62">
	<figure markdown>
	![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/learn_tutorials/6/2/Input_Output_Logic_Level_Tolerances_tutorial_tile.png)
	</figure>

	---

	**Logic Levels**</a>

-   <a href="https://learn.sparkfun.com/tutorials/664">
	<figure markdown>
	![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/learn_tutorials/6/6/4/PCB_TraceCutLumenati.jpg)
	</figure>

	---

	**How to Work with Jumper Pads and PCB Traces**</a>

</div>

??? info "Related Blog Posts"
	Additionally, users may be interested in these blog post articles on GNSS technologies:

	<div class="grid cards col-4" markdown align="center">

	-   <a href="https://www.sparkfun.com/news/4276">
		<figure markdown>
		![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/home_page_posts/4/2/7/6/GPSvGNSSHomepageImage4.png)
		</figure>

		---

		**GPS vs GNSS**</a>

	-   <a href="https://www.sparkfun.com/news/7138">
		<figure markdown>
		![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/home_page_posts/7/1/3/8/SparkFun_RTK_Facet_-_Surveying_Monopod.jpg)
		</figure>

		---

		**What is Correction Data?**</a>

	-   <a href="https://www.sparkfun.com/news/7533">
		<figure markdown>
		![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/home_page_posts/7/5/3/3/rtk-blog-thumb.png)
		</figure>

		---

		**Real-Time Kinematics Explained**</a>

	-   <a href="https://www.sparkfun.com/news/9514">
		<figure markdown>
		![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/home_page_posts/9/5/1/4/DIY-Surveying-Blog__1_.jpg)
		</figure>

		---

		**DIY RTK Surveying**</a>

	-   <a href="https://www.sparkfun.com/news/7401">
		<figure markdown>
		![Tutorial Thumbnail](https://cdn.sparkfun.com/c/264-148/assets/home_page_posts/7/4/0/1/Screen_Shot_2023-06-26_at_8.30.22_PM.png)
		</figure>

		---

		**New Video: Unlocking High-Precision RTK Positioning**</a>

	</div>
