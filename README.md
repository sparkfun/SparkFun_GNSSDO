SparkPNT GNSSDO
========================================

[![SparkPNT GNSSDO](https://cdn.sparkfun.com/r/600-600/assets/parts/2/7/2/0/7/26289-GNSS-Disciplined-Oscillator-Front-Lit.jpg)](www.sparkfun/sparkpnt-gnss-disciplined-oscillator.html)

[*SparkPNT GNSSDO (GPS-26289)*](www.sparkfun/sparkpnt-gnss-disciplined-oscillator.html)

Designed and manufactured in Boulder, Colorado, USA, the SparkPNT GNSS Disciplined Oscillator (GNSSDO) is the perfect solution for your high-precision timing needs. Based around the multi-constellation, multi-frequency, L5-ready mosaic-T from Septentrio, this is our most accurate GNSS timing product to date. It features a disciplined 10MHz Digitally-Controlled Temperature-Controlled Crystal Oscillator (DCTCXO). The mosaic-T also has built-in on-module support for the Fugro AtomiChron L-band timing service.

Under the hood, the GNSSDO is based on the mosaic-T GNSS module from Septentrio, plus the Espressif ESP32-WROVER processor (16MB flash, 8MB PSRAM). The mosaic-T has USB-C connectivity (with Ethernet-over-USB), multiple UARTs and supports full Ethernet connectivity. You can connect the mosaic-T directly to your Ethernet network - our product supports Power-over-Ethernet too. The ESP32 rides shotgun: disciplining the DCTCXO and controlling the OLED display. Remote configuration is possible, the ESP32 console can be accessed via TCP. The GNSSDO has multiple power options including USB-C, PoE and 9V-36V DC, making it easy to connect it to a battery-backed supply.

RTK Base
--------------
The mosaic-T is a superb GNSS module designed for highly accurate timing applications. It can be configured as a RTK Rover and will make use of incoming RTCM corrections. **But it can not be configured as a RTK Base. RTCM output is not supported.**

Documentation
--------------

- **[Product Manual](http://docs.sparkfun.com/SparkFun_GNSSDO/)** - Product manual for the GNSSDO hosted by GitHub pages.<br>
  [![Built with Material for MkDocs](https://img.shields.io/badge/Material_for_MkDocs-526CFE?logo=MaterialForMkDocs&logoColor=white)](https://squidfunk.github.io/mkdocs-material/) [![GitHub Pages Deploy](https://github.com/sparkfun/SparkFun_GNSSDO/actions/workflows/mkdocs.yml/badge.svg)](https://github.com/sparkfun/SparkFun_GNSSDO/actions/workflows/mkdocs.yml)


*Need to download or print our hookup guide?*

- [Print *(Print to PDF)* from Single-Page View](http://docs.sparkfun.com/SparkFun_GNSSDO/print_view)

Firmware Update
-------------------

The **[/Firmware/Binaries](https://github.com/sparkfun/SparkFun_GNSSDO/tree/main/Firmware/Binaries)** folder contains the firmware binaries.

You can update or reload the firmware using the [SparkFun RTK Firmware Uploader](https://github.com/sparkfun/SparkFun_RTK_Firmware_Uploader).

Repository Contents
-------------------

- **[/docs](/docs/)** - Online documentation files
    - [assets](/docs/assets/) - Assets files
        - [board_files](/docs/assets/board_files/) - Files for the product design
            - [Schematic](/docs/assets/board_files/schematic.pdf) (.pdf)
            - [Dimensions](/docs/assets/board_files/dimensions.png) (.png)
            - [KiCad files](/docs/assets/board_files/kicad_files.zip) (.zip)
        - [img/hookup_guide/](/docs/assets/img/hookup_guide/) - Images for hookup guide documentation
- **[/Hardware](/Hardware/)** - KiCad design files
- **[/Hardware/Production](/Hardware/Production/)** - PCB panel production files
- **[/Front_Sticker](/Front_Sticker/)** - DXF and PDF files for the front sticker
- **[/Rear_Sticker](/Rear_Sticker/)** - DXF and PDF files for the rear sticker
- **[/Firmware](/Firmware/)** - Arduino source code and binaries for the firmware which runs on the ESP32-WROVER
- **[/Test_Sketches](/Test_Sketches/)** - Additional code used to validate and test the GNSSDO
- **[/Documents](/Documents/)** - Component datasheets etc.

Product Variants
----------------

- [GPS-26289](www.sparkfun/sparkpnt-gnss-disciplined-oscillator.html)- v1.0, Initial Release

Version History
---------------

- [v10](https://github.com/sparkfun/SparkFun_GNSSDO/releases/tag/v10) - Initial Release


License Information
-------------------

This product is ***open source***!

Please review the [LICENSE.md](./LICENSE.md) file for license information.

If you have any questions or concerns about licensing, please contact technical support on our [SparkFun forums](https://forum.sparkfun.com/viewforum.php?f=152).

Distributed as-is; no warranty is given.

- Your friends at SparkPNT.
