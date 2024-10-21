SparkFun GPSDO
========================================

[![SparkFun GPSDO]()]()

[*SparkFun GPSDO (GPS-26289)*](https://www.sparkfun.com/products/26289)

Designed and manufactured in Boulder, Colorado, USA, the SparkFun GPS Disciplined Oscillator (GPSDO) is the perfect solution for your high-precision timing needs. Based around the multi-constellation, multi-frequency, L5-ready mosaic-T from Septentrio, this is our most accurate GNSS timing product to date. It features a disciplined 10MHz Digitally-Controlled Temperature-Controlled Crystal Oscillator (DCTCXO). The mosaic-T also has built-in on-module support for the Fugro AtomiChron L-band timing service.

* SMA Connections:
    * GNSS Antenna (L1/L2/L5/L-Band) - provides 5V for an active antenna
    * 10MHz Output - disciplined, configurable for 5V / 3.3V / 2.8V / 1.8V and 50 Ohm
    * 10MHz Input - configurable
    * Pulse-Per-Second Square Wave - configurable for 5V / 3.3V / 2.8V / 1.8V and 50 Ohm
    * EventA Input - configurable for 5V / 3.3V / 2.8V / 1.8V
* 3.5mm Screw Cage Connections:
    * 9V-36V DC input
    * GND
    * mosaic-T COM2 (TX/RX/CTS/RTS) - 3.3V / 5V switchable
    * CTS can be configured as a 3.3V / 5V power output via a solder jumper
    * EventB input - 3.3V / 5V switchable
    * Secondary I<sup>2</sup>C bus (SCL and SDA) - 3.3V / 5V switchable
* Power Options:
    * USB-C
    * Power-over-Ethernet (PoE)
    * 9V-36V DC input (fully isolated)

Under the hood, the GPSDO is based on the mosaic-T GNSS module from Septentrio, plus the Espressif ESP32-WROVER processor (16MB flash, 8MB PSRAM). The mosaic-T has USB-C connectivity (with Ethernet-over-USB), multiple UARTs and supports full Ethernet connectivity. You can connect the mosaic-T directly to your Ethernet network - our product supports Power-over-Ethernet too. The ESP32 rides shotgun: disciplining the DCTCXO and controlling the OLED display. Remote configuration is possible, the ESP32 console can be accessed via TCP. The GPSDO has multiple power options including USB-C, PoE and 9V-36V DC, making it easy to connect it to a battery-backed supply.

RTK Base
--------------
The mosaic-T is a superb GNSS module designed for highly accurate timing applications. It can be configured as a RTK Rover and will make use of incoming RTCM corrections. **But it can not be configured as a RTK Base. RTCM output is not supported.**

Documentation
--------------

* **[Product Manual](http://docs.sparkfun.com/SparkFun_GPSDO/)** - Product manual for the GPSDO hosted by GitHub pages.<br>
  [![Built with Material for MkDocs](https://img.shields.io/badge/Material_for_MkDocs-526CFE?logo=MaterialForMkDocs&logoColor=white)](https://squidfunk.github.io/mkdocs-material/) [![GitHub Pages Deploy](https://github.com/sparkfun/SparkFun_RTK_mosaic-T/actions/workflows/mkdocs.yml/badge.svg)](https://github.com/sparkfun/SparkFun_RTK_mosaic-T/actions/workflows/mkdocs.yml)


*Need to download or print our hookup guide?*

* [Print *(Print to PDF)* from Single-Page View](http://docs.sparkfun.com/SparkFun_RTK_mosaic-T/print_view)

Firmware Update
-------------------

The **[/Firmware/Binaries](https://github.com/sparkfun/SparkFun_RTK_mosaic-T/tree/main/Firmware/Binaries)** folder contains the firmware binaries.

You can update or reload the firmware using the [SparkFun RTK Firmware Uploader](https://github.com/sparkfun/SparkFun_RTK_Firmware_Uploader).

Repository Contents
-------------------

* **[/docs](/docs/)** - Online documentation files
    * [assets](/docs/assets/) - Assets files
        * [board_files](/docs/assets/board_files/) - Files for the product design
            * [Schematic](/docs/assets/board_files/schematic.pdf) (.pdf)
            * [Dimensions](/docs/assets/board_files/dimensions.png) (.png)
            * [KiCad files](/docs/assets/board_files/kicad_files.zip) (.zip)
        * [img/hookup_guide/](/docs/assets/img/hookup_guide/) - Images for hookup guide documentation
* **[/Hardware](/Hardware/)** - KiCad design files
* **[/Hardware/Production](/Hardware/Production/)** - PCB panel production files
* **[/Front_Sticker](/Front_Sticker/)** - DXF and PDF files for the front sticker
* **[/Rear_Sticker](/Rear_Sticker/)** - DXF and PDF files for the rear sticker
* **[/Firmware](/Firmware/)** - Arduino source code and binaries for the firmware which runs on the ESP32-WROVER
* **[/Test_Sketches](/Test_Sketches/)** - Additional code used to validate and test the GPSDO
* **[/Documents](/Documents/)** - Component datasheets etc.

Product Variants
----------------

* [GPS-26289](https://www.sparkfun.com/products/26289)- v1.0, Initial Release

Version History
---------------

* [v10](https://github.com/sparkfun/SparkFun_RTK_mosaic-T/releases/tag/v10) - Initial Release


License Information
-------------------

This product is ***open source***!

Please review the [LICENSE.md](./LICENSE.md) file for license information.

If you have any questions or concerns about licensing, please contact technical support on our [SparkFun forums](https://forum.sparkfun.com/viewforum.php?f=152).

Distributed as-is; no warranty is given.

- Your friends at SparkFun.
