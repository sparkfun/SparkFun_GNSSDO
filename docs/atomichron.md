---
icon: material/tools
---

## Fugro AtomiChron<sup>Ⓡ</sup>

For the ultimate in clock accuracy, you can subscribe to Fugro's AtomiChron service. Corrections can be delivered via L-Band or IP (Ethernet). AtomiChron allows the mosaic-T to achieve better than 1ns timing accuracy. Activation takes place over L-Band.

Without an AtomiChron subscription, the firmware will use the composite GNSS **RxClkBias** from **PVTGeodetic** to discipline the TCXO frequency.

<figure markdown>
[![Fugro AtomiChron expired](./assets/img/hookup_guide/AtomiChron-Expired.png){ width="400" }](./assets/img/hookup_guide/AtomiChron-Expired.png "Click to enlarge")
<figcaption markdown>By default, the Fugro AtomiChron subscription shows as "expired".</figcaption>
</figure>

With AtomiChron, you have the option to use the individual GPS or Galileo clock bias from **FugroTimeOffset** over the Fugro composite bias.

<figure markdown>
[![Fugro AtomiChron active](./assets/img/hookup_guide/AtomiChron-Active.png){ width="400" }](./assets/img/hookup_guide/AtomiChron-Active.png "Click to enlarge")
<figcaption markdown>The mosaic-T with an active Fugro AtomiChron subscription.</figcaption>
</figure>

