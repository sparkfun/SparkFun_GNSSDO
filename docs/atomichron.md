---
icon: material/tools
---

Fugro AtomiChron^®^ is a global, real-time, precise clock synchronisation and authentication service based on GNSS, for applications that require resilient, accurate and reliable time and frequency. This innovative technology eliminates time drift caused by clocks counting time at slightly different rates and gives users access to extreme frequency stability as well as extreme accurate time references, with levels that surpass high-calibre industrial standard caesium clocks and approaching hydrogen maser performance.

<figure markdown>
[![Fugro AtomiChron overview](./assets/img/hookup_guide/AtomiChron.png){ width="600" }](./assets/img/hookup_guide/AtomiChron.png "Click to enlarge")
<figcaption markdown>Fugro AtomiChron overview.</figcaption>
</figure>

## :fontawesome-solid-screwdriver-wrench:&nbsp;Specifications

* Timing accuracy: <5 ns UTC (95% of the time)

* Accuracy: <1 ns to Fugro AtomiChron^®^ timescale (independent of world-wide location)

* Performance improvement: >10x better than existing high-end GNSS receivers

* NMA status: per satellite status for all 4 GNSS constellations concurrently

## Performance

With AtomiChron enabled, the GNSSDO achieves a GPS clock bias of better than 1 ns:

<figure markdown>
[![Clock bias performance](./assets/img/hookup_guide/clock_bias.png){ width="600" }](./assets/img/hookup_guide/clock_bias.png "Click to enlarge")
<figcaption markdown>GPS clock bias with AtomiChron vs GPS Pulse-Per-Second offset (without AtomiChron).</figcaption>
</figure>

With AtomiChron enabled, the Pulse-Per-Second Allan Deviation approaches 1E-14 at 10000 seconds:

<figure markdown>
[![Allan Deviation with and without AtomiChron](./assets/img/hookup_guide/Allan_Deviation.jpg){ width="400" }](./assets/img/hookup_guide/Allan_Deviation.jpg "Click to enlarge")
<figcaption markdown>Pulse-Per-Second Allan Deviation with and without AtomiChron.</figcaption>
</figure>

## Firmware configuration

Without an AtomiChron subscription, the firmware will use the composite GNSS **RxClkBias** from **PVTGeodetic** to discipline the TCXO frequency.

<figure markdown>
[![Fugro AtomiChron expired](./assets/img/hookup_guide/AtomiChron-Expired.png){ width="400" }](./assets/img/hookup_guide/AtomiChron-Expired.png "Click to enlarge")
<figcaption markdown>By default, the Fugro AtomiChron subscription shows as "expired".</figcaption>
</figure>

With AtomiChron, the firmware will use the Fugro composite bias reported in **FugroTimeOffset** to discipline the TCXO frequency. You also have the option to prefer the individual GPS or Galileo clock bias over the Fugro composite bias. Please refer to the [ESP32 firmware settings](./software_overview.md#configure-operation) for more details.

<figure markdown>
[![Fugro AtomiChron active](./assets/img/hookup_guide/AtomiChron-Active.png){ width="400" }](./assets/img/hookup_guide/AtomiChron-Active.png "Click to enlarge")
<figcaption markdown>The mosaic-T with an active Fugro AtomiChron subscription.</figcaption>
</figure>

