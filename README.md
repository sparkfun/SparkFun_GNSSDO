SparkFun RTK mosaic-T
========================================

[![SparkFun RTK mosaic-T](https://cdn.sparkfun.com/r/600-600/assets/parts/2/4/0/7/2/23748-RTK-Mosaic-X5-Kit-All-Feature.jpg)](https://cdn.sparkfun.com/assets/parts/2/4/0/7/2/23748-RTK-Mosaic-X5-Kit-All-Feature.jpg)

[*SparkFun RTK mosaic-T (GPS-23748)*](https://www.sparkfun.com/products/23748)

Designed and manufactured in Boulder, Colorado, USA, the SparkFun RTK mosaic-T is the perfect solution for your high-precision timing needs. Based around the multi-constellation, multi-frequency, L5-ready mosaic-T from Septentrio, this is our most accurate GNSS timing product to date. It features a disciplined 10MHz Digitally-Controlled Temperature-Controlled Crystal Oscillator (DCTCXO). The mosaic-T also has built-in on-module support for the Fugro AtomiChron L-band timing service.

* SMA Connections:
    * GNSS Antenna (L1/L2/L5/L-Band) - provides 5V for an active antenna
    * 10MHz Sine Wave - disciplined, 50Ohm, 3.3V / 5V switchable
    * Pulse-Per-Second Square Wave - 50Ohm, 3.3V / 5V switchable
    * ESP32 WiFi/BT - reverse polarity (RP), unused by the current firmware
* 3.5mm Screw Cage Connections:
    * 9V-36V DC input
    * GND
    * mosaic-T COM2 (TX/RX/CTS/RTS) - 3.3V / 5V switchable
    * CTS can be configured as a 3.3V / 5V power output via a solder jumper
    * EventA & EventB inputs - 3.3V / 5V switchable
    * Pulse-Per-Second output - 3.3V / 5V switchable
* Power Options:
    * USB-C
    * Power-over-Ethernet (PoE)
    * 9V-36V DC input (fully isolated)

Under the hood, the RTK mosaic-T is based on the mosaic-T GNSS module from Septentrio, plus the Espressif ESP32-WROVER processor (16MB flash, 8MB PSRAM). The mosaic-T has USB-C connectivity (with Ethernet-over-USB), multiple UARTs and supports full Ethernet connectivity. You can connect the mosaic-T directly to your Ethernet network - our product supports Power-over-Ethernet too. The ESP32 rides shotgun: disciplining the DCTCXO and controlling the OLED display. The RTK mosaic-T has multiple power options including USB-C, PoE and 9V-36V DC, making it easy to connect it to a battery-backed supply.

Documentation
--------------

* **[Product Manual](http://docs.sparkfun.com/SparkFun_RTK_mosaic-T/)** - Product manual for the RTK mosaic-T hosted by GitHub pages.<br>
  [![Built with Material for MkDocs](https://img.shields.io/badge/Material_for_MkDocs-526CFE?logo=MaterialForMkDocs&logoColor=white)](https://squidfunk.github.io/mkdocs-material/) [![GitHub Pages Deploy](https://github.com/sparkfun/SparkFun_RTK_mosaic-T/actions/workflows/mkdocs.yml/badge.svg)](https://github.com/sparkfun/SparkFun_RTK_mosaic-T/actions/workflows/mkdocs.yml)


*Need to download or print our hookup guide?*

* [Print *(Print to PDF)* from Single-Page View](http://docs.sparkfun.com/SparkFun_RTK_mosaic-T/print_view)

Repository Contents
-------------------

* **[/docs](/docs/)** - Online documentation files
    * [assets](/docs/assets/) - Assets files
        * [board_files](/docs/assets/board_files/) - Files for the product design
            * [Schematic](/docs/assets/board_files/schematic.pdf) (.pdf)
            * [Dimensions](/docs/assets/board_files/dimensions.png) (.png)
            * [Eagle files](/docs/assets/board_files/eagle_files.zip) (.zip)
        * [img/hookup_guide/](/docs/assets/img/hookup_guide/) - Images for hookup guide documentation
* **[/Hardware](/Hardware/)** - Eagle design files (.brd, .sch)
* **[/Production](/Production/)** - PCB panel production files
* **[/Front_Panel](/Front_Panel/)** - Eagle design files (.brd, .sch) for the prototype (PCB) enclosure panel used to validate the dimensions
* **[/Rear_Panel](/Rear_Panel/)** - Eagle design files (.brd, .sch) for the prototype (PCB) enclosure panel used to validate the dimensions
* **[/Front_Sticker](/Front_Sticker/)** - DXF and PDF files for the front sticker
* **[/Rear_Sticker](/Rear_Sticker/)** - DXF and PDF files for the rear sticker
* **[/Firmware](/Firmware/)** - ESP IDF source code and binaries for the firmware which runs on the ESP32-WROVER
* **[/Test_Sketches](/Test_Sketches/)** - Additional code used to validate and test the RTK mosaic-T
* **[/Documents](/Documents/)** - Component datasheets etc.

Product Variants
----------------

* [GPS-23748](https://www.sparkfun.com/products/23748)- v1.0, Initial Release

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
