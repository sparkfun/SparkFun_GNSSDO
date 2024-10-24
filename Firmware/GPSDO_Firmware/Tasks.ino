/*------------------------------------------------------------------------------
Tasks.ino

  This module implements the high frequency tasks made by xTaskCreate() and any
  low frequency tasks that are called by Ticker.

                    GNSS
                    |
                    v
                    |
                    Serial
                    |
                    v
                    |
                    | gnssReadTask
                    |    gpsMessageParserFirstByte
                    |        ...
                    |    processUart1Message
                    |
                    v
                    Ring Buffer
                    |
                    | handleGnssDataTask
                    |
                    v
                    Consumers
------------------------------------------------------------------------------*/

//----------------------------------------
// Macros
//----------------------------------------

#define WRAP_OFFSET(offset, increment, arraySize)                                                                      \
    {                                                                                                                  \
        offset += increment;                                                                                           \
        if (offset >= arraySize)                                                                                       \
            offset -= arraySize;                                                                                       \
    }

//----------------------------------------
// Constants
//----------------------------------------

enum RingBufferConsumers
{
    RBC_PARSER = 0,
    // Insert new consumers here
    RBC_MAX
};

const char *const ringBufferConsumer[] = {
    "Parser",
};

const int ringBufferConsumerEntries = sizeof(ringBufferConsumer) / sizeof(ringBufferConsumer[0]);

//----------------------------------------
// Locals
//----------------------------------------

volatile static RING_BUFFER_OFFSET dataHead; // Head advances as data comes in from GNSS's UART
volatile int32_t availableHandlerSpace;      // settings.gnssHandlerBufferSize - usedSpace
volatile const char *slowConsumer;

// Ring buffer tails
static RING_BUFFER_OFFSET parserRingBufferTail; // Tail advances as data is parsed

// Ring buffer offsets
static uint16_t rbOffsetHead;

//----------------------------------------
// Task routines
//----------------------------------------

// Normally a delay(1) will feed the WDT but if we don't want to wait that long, this feeds the WDT without delay
void feedWdt()
{
    vTaskDelay(1);
}

//----------------------------------------------------------------------
// Ring buffer empty when all the tails == dataHead
//
//        +---------+
//        |         |
//        |         |
//        |         |
//        |         |
//        +---------+ <-- dataHead, parserRingBufferTail, etc.
//
// Ring buffer contains data when any tail != dataHead
//
//        +---------+
//        |         |
//        |         |
//        | yyyyyyy | <-- dataHead
//        | xxxxxxx | <-- btRingBufferTail (1 byte in buffer)
//        +---------+ <-- sdRingBufferTail (2 bytes in buffer)
//
//        +---------+
//        | yyyyyyy | <-- btRingBufferTail (1 byte in buffer)
//        | xxxxxxx | <-- sdRingBufferTail (2 bytes in buffer)
//        |         |
//        |         |
//        +---------+ <-- dataHead
//
// Maximum ring buffer fill is settings.gnssHandlerBufferSize - 1
//----------------------------------------------------------------------

// Read bytes from GNSS into ESP32 circular buffer
void gnssReadTask(void *e)
{
    static PARSE_STATE parse = {gpsMessageParserFirstByte, processUart1Message, "Log"};

    while (true)
    {
        if ((settings.enableTaskReports == true) && (!inMainMenu))
        {
            static unsigned long lastPrint = 0;
            if (millis() > (lastPrint + settings.periodicPrintInterval_ms))
            {
                systemPrintf("SerialReadTask High watermark: %d\r\n", uxTaskGetStackHighWaterMark(nullptr));
                lastPrint = millis();
            }
        }

        while (serialGNSS.available())
        {
            // Read the data from UART1
            uint8_t incomingData[500];
            int bytesIncoming = serialGNSS.read(incomingData, sizeof(incomingData));

            for (int x = 0; x < bytesIncoming; x++)
            {
                // Save the data byte
                parse.buffer[parse.length++] = incomingData[x];
                parse.length %= PARSE_BUFFER_LENGTH;

                // Update the parser state based on the incoming byte
                parse.state(&parse, incomingData[x]);
            }
        }

        feedWdt();
        taskYIELD();
    }
}

// Process a complete message incoming from parser
// If we get a complete NMEA/RTCM message, pass it on to the consumers
void processUart1Message(PARSE_STATE *parse, uint8_t type)
{
    int32_t bytesToCopy;
    const char *consumer;
    RING_BUFFER_OFFSET remainingBytes;
    int32_t space;
    int32_t use;

    // Display the message
    if ((settings.enablePrintGNSSMessages) && (!inMainMenu))
    {
        systemPrint("GNSS RX: ");
        switch (type)
        {
        case SENTENCE_TYPE_NMEA:
            systemPrintf("%s NMEA %s, %2d bytes\r\n", parse->parserName, parse->nmeaMessageName, parse->length);
            break;

        case SENTENCE_TYPE_RTCM:
            systemPrintf("%s RTCM %d, %2d bytes\r\n", parse->parserName, parse->message, parse->length);
            break;

        case SENTENCE_TYPE_SBF:
            systemPrintf("%s SBF %d, %2d bytes\r\n", parse->parserName, parse->message & 0x1FFF, parse->length);
            break;
        }
    }

    // Determine if this message will fit into the ring buffer
    bytesToCopy = parse->length;
    space = availableHandlerSpace;
    use = settings.gnssHandlerBufferSize - space;
    consumer = (char *)slowConsumer;
    if ((bytesToCopy > space) && (!inMainMenu))
    {
        int32_t bufferedData;
        int32_t bytesToDiscard;
        int32_t discardedBytes;
        int32_t listEnd;
        int32_t messageLength;
        int32_t previousTail;
        int32_t rbOffsetTail;

        // Determine the tail of the ring buffer
        previousTail = dataHead + space + 1;
        if (previousTail >= settings.gnssHandlerBufferSize)
            previousTail -= settings.gnssHandlerBufferSize;

        /*  The rbOffsetArray holds the offsets into the ring buffer of the
         *  start of each of the parsed messages.  A head (rbOffsetHead) and
         *  tail (rbOffsetTail) offsets are used for this array to insert and
         *  remove entries.  Typically this task only manipulates the head as
         *  new messages are placed into the ring buffer.  The handleGnssDataTask
         *  normally manipulates the tail as data is removed from the buffer.
         *  However this task will manipulate the tail under two conditions:
         *
         *  1.  The ring buffer gets full and data must be discarded
         *
         *  2.  The rbOffsetArray is too small to hold all of the message
         *      offsets for the data in the ring buffer.  The array is full
         *      when (Head + 1) == Tail
         *
         *  Notes:
         *      The rbOffsetArray is allocated along with the ring buffer in
         *      Begin.ino
         *
         *      The first entry rbOffsetArray[0] is initialized to zero (0)
         *      in Begin.ino
         *
         *      The array always has one entry in it containing the head offset
         *      which contains a valid offset into the ringBuffer, handled below
         *
         *      The empty condition is Tail == Head
         *
         *      The amount of data described by the rbOffsetArray is
         *      rbOffsetArray[Head] - rbOffsetArray[Tail]
         *
         *              rbOffsetArray                  ringBuffer
         *           .-----------------.           .-----------------.
         *           |                 |           |                 |
         *           +-----------------+           |                 |
         *  Tail --> |   Msg 1 Offset  |---------->+-----------------+ <-- Tail n
         *           +-----------------+           |      Msg 1      |
         *           |   Msg 2 Offset  |--------.  |                 |
         *           +-----------------+        |  |                 |
         *           |   Msg 3 Offset  |------. '->+-----------------+
         *           +-----------------+      |    |      Msg 2      |
         *  Head --> |   Head Offset   |--.   |    |                 |
         *           +-----------------+  |   |    |                 |
         *           |                 |  |   |    |                 |
         *           +-----------------+  |   |    |                 |
         *           |                 |  |   '--->+-----------------+
         *           +-----------------+  |        |      Msg 3      |
         *           |                 |  |        |                 |
         *           +-----------------+  '------->+-----------------+ <-- dataHead
         *           |                 |           |                 |
         */

        // Determine the index for the end of the circular ring buffer
        // offset list
        listEnd = rbOffsetHead;
        WRAP_OFFSET(listEnd, 1, rbOffsetEntries);

        // Update the tail, walk newest message to oldest message
        rbOffsetTail = rbOffsetHead;
        bufferedData = 0;
        messageLength = 0;
        while ((rbOffsetTail != listEnd) && (bufferedData < use))
        {
            // Determine the amount of data in the ring buffer up until
            // either the tail or the end of the rbOffsetArray
            //
            //                      |           |
            //                      |           | Valid, still in ring buffer
            //                      |  Newest   |
            //                      +-----------+ <-- rbOffsetHead
            //                      |           |
            //                      |           | free space
            //                      |           |
            //     rbOffsetTail --> +-----------+ <-- bufferedData
            //                      |   ring    |
            //                      |  buffer   | <-- used
            //                      |   data    |
            //                      +-----------+ Valid, still in ring buffer
            //                      |           |
            //
            messageLength = rbOffsetArray[rbOffsetTail];
            WRAP_OFFSET(rbOffsetTail, rbOffsetEntries - 1, rbOffsetEntries);
            messageLength -= rbOffsetArray[rbOffsetTail];
            if (messageLength < 0)
                messageLength += settings.gnssHandlerBufferSize;
            bufferedData += messageLength;
        }

        // Account for any data in the ring buffer not described by the array
        //
        //                      |           |
        //                      +-----------+
        //                      |  Oldest   |
        //                      |           |
        //                      |   ring    |
        //                      |  buffer   | <-- used
        //                      |   data    |
        //                      +-----------+ Valid, still in ring buffer
        //                      |           |
        //     rbOffsetTail --> +-----------+ <-- bufferedData
        //                      |           |
        //                      |  Newest   |
        //                      +-----------+ <-- rbOffsetHead
        //                      |           |
        //
        discardedBytes = 0;
        if (bufferedData < use)
            discardedBytes = use - bufferedData;

        // Writing to the SD card, the network or Bluetooth, a partial
        // message may be written leaving the tail pointer mid-message
        //
        //                      |           |
        //     rbOffsetTail --> +-----------+
        //                      |  Oldest   |
        //                      |           |
        //                      |   ring    |
        //                      |  buffer   | <-- used
        //                      |   data    | Valid, still in ring buffer
        //                      +-----------+ <--
        //                      |           |
        //                      +-----------+
        //                      |           |
        //                      |  Newest   |
        //                      +-----------+ <-- rbOffsetHead
        //                      |           |
        //
        else if (bufferedData > use)
        {
            // Remove the remaining portion of the oldest entry in the array
            discardedBytes = messageLength + use - bufferedData;
            WRAP_OFFSET(rbOffsetTail, 1, rbOffsetEntries);
        }

        // rbOffsetTail now points to the beginning of a message in the
        // ring buffer
        // Determine the amount of data to discard
        bytesToDiscard = discardedBytes;
        if (bytesToDiscard < bytesToCopy)
            bytesToDiscard = bytesToCopy;
        if (bytesToDiscard < AMOUNT_OF_RING_BUFFER_DATA_TO_DISCARD)
            bytesToDiscard = AMOUNT_OF_RING_BUFFER_DATA_TO_DISCARD;

        // Walk the ring buffer messages from oldest to newest
        while ((discardedBytes < bytesToDiscard) && (rbOffsetTail != rbOffsetHead))
        {
            // Determine the length of the oldest message
            WRAP_OFFSET(rbOffsetTail, 1, rbOffsetEntries);
            discardedBytes = rbOffsetArray[rbOffsetTail] - previousTail;
            if (discardedBytes < 0)
                discardedBytes += settings.gnssHandlerBufferSize;
        }

        // Discard the oldest data from the ring buffer
        if (consumer)
            systemPrintf("Ring buffer full: discarding %d bytes, %s is slow\r\n", discardedBytes, consumer);
        else
            systemPrintf("Ring buffer full: discarding %d bytes\r\n", discardedBytes);
        updateRingBufferTails(previousTail, rbOffsetArray[rbOffsetTail]);
        availableHandlerSpace = availableHandlerSpace + discardedBytes;
    }

    // Add another message to the ring buffer
    // Account for this message
    availableHandlerSpace = availableHandlerSpace - bytesToCopy;

    // Fill the buffer to the end and then start at the beginning
    if ((dataHead + bytesToCopy) > settings.gnssHandlerBufferSize)
        bytesToCopy = settings.gnssHandlerBufferSize - dataHead;

    // Display the dataHead offset
    if (settings.enablePrintRingBufferOffsets && (!inMainMenu))
        systemPrintf("DH: %4d --> ", dataHead);

    // Copy the data into the ring buffer
    memcpy(&ringBuffer[dataHead], parse->buffer, bytesToCopy);
    dataHead = dataHead + bytesToCopy;
    if (dataHead >= settings.gnssHandlerBufferSize)
        dataHead = dataHead - settings.gnssHandlerBufferSize;

    // Determine the remaining bytes
    remainingBytes = parse->length - bytesToCopy;
    if (remainingBytes)
    {
        // Copy the remaining bytes into the beginning of the ring buffer
        memcpy(ringBuffer, &parse->buffer[bytesToCopy], remainingBytes);
        dataHead = dataHead + remainingBytes;
        if (dataHead >= settings.gnssHandlerBufferSize)
            dataHead = dataHead - settings.gnssHandlerBufferSize;
    }

    // Add the head offset to the offset array
    WRAP_OFFSET(rbOffsetHead, 1, rbOffsetEntries);
    rbOffsetArray[rbOffsetHead] = dataHead;

    // Display the dataHead offset
    if (settings.enablePrintRingBufferOffsets && (!inMainMenu))
        systemPrintf("%4d\r\n", dataHead);
}

// Remove previous messages from the ring buffer
void updateRingBufferTails(RING_BUFFER_OFFSET previousTail, RING_BUFFER_OFFSET newTail)
{
    // Trim any long or medium tails
    discardRingBufferBytes(&parserRingBufferTail, previousTail, newTail);
}

// Remove previous messages from the ring buffer
void discardRingBufferBytes(RING_BUFFER_OFFSET *tail, RING_BUFFER_OFFSET previousTail, RING_BUFFER_OFFSET newTail)
{
    // The longest tail is being trimmed.  Medium length tails may contain
    // some data within the region begin trimmed.  The shortest tails will
    // be trimmed.
    //
    // Devices that get their tails trimmed, may output a partial message
    // prior to the buffer trimming.  After the trimming, the tail of the
    // ring buffer points to the beginning of a new message.
    //
    //                 previousTail                newTail
    //                      |                         |
    //  Before trimming     v         Discarded       v   After trimming
    //  ----+-----------------  ...  -----+--  ..  ---+-----------+------
    //      | Partial message             |           |           |
    //  ----+-----------------  ...  -----+--  ..  ---+-----------+------
    //                      ^          ^                     ^
    //                      |          |                     |
    //        long tail ----'          '--- medium tail      '-- short tail
    //
    // Determine if the trimmed data wraps the end of the buffer
    if (previousTail < newTail)
    {
        // No buffer wrap occurred
        // Only discard the data from long and medium tails
        if ((*tail >= previousTail) && (*tail < newTail))
            *tail = newTail;
    }
    else
    {
        // Buffer wrap occurred
        if ((*tail >= previousTail) || (*tail < newTail))
            *tail = newTail;
    }
}

// If new data is in the ringBuffer, dole it out to appropriate consumers
// Each consumer gets its own tail.
// If the consumer is running too slowly then data for that consumer is dropped.
// The usedSpace variable tracks the total space in use in the buffer.
void handleGnssDataTask(void *e)
{
    int32_t bytesToSend;
    uint32_t deltaMillis;
    int32_t freeSpace;
    static uint32_t maxMillis[RBC_MAX];
    uint32_t startMillis;
    int32_t usedSpace;

    // Initialize the tails
    parserRingBufferTail = 0;

    // Parser
    static PARSE_STATE consumer = {gpsMessageParserFirstByte, processConsumerMessage, "Consume"};

    while (true)
    {
        usedSpace = 0;

        startMillis = millis();

        {
            // Determine the amount of data in the buffer
            bytesToSend = dataHead - parserRingBufferTail;
            if (bytesToSend < 0)
                bytesToSend += settings.gnssHandlerBufferSize;
            if (bytesToSend > 0)
            {
                // Reduce bytes to send if we have more to send then the end of
                // the buffer, we'll wrap next loop
                if ((parserRingBufferTail + bytesToSend) > settings.gnssHandlerBufferSize)
                    bytesToSend = settings.gnssHandlerBufferSize - parserRingBufferTail;

                // Consume the data                
                for (int32_t x = 0; x < bytesToSend; x++)
                {
                    // Save the data byte
                    consumer.buffer[consumer.length++] = ringBuffer[parserRingBufferTail + x];
                    consumer.length %= PARSE_BUFFER_LENGTH;

                    // Update the parser state based on the incoming byte
                    consumer.state(&consumer, ringBuffer[parserRingBufferTail + x]);
                }

                // Account for the data that was sent
                if (bytesToSend > 0)
                {
                    // Account for the sent or dropped data
                    parserRingBufferTail += bytesToSend;
                    if (parserRingBufferTail >= settings.gnssHandlerBufferSize)
                        parserRingBufferTail -= settings.gnssHandlerBufferSize;

                    // Remember the maximum transfer time
                    deltaMillis = millis() - startMillis;
                    if (maxMillis[RBC_PARSER] < deltaMillis)
                        maxMillis[RBC_PARSER] = deltaMillis;
                }

                // Determine the amount of data that remains in the buffer
                bytesToSend = dataHead - parserRingBufferTail;
                if (bytesToSend < 0)
                    bytesToSend += settings.gnssHandlerBufferSize;
                if (usedSpace < bytesToSend)
                {
                    usedSpace = bytesToSend;
                    slowConsumer = "Parser";
                }
            }
        }

        //----------------------------------------------------------------------
        // Update the available space in the ring buffer
        //----------------------------------------------------------------------

        freeSpace = settings.gnssHandlerBufferSize - usedSpace;

        // Don't fill the last byte to prevent buffer overflow
        if (freeSpace)
            freeSpace -= 1;
        availableHandlerSpace = freeSpace;

        //----------------------------------------------------------------------
        // Display the millisecond values for the different ring buffer consumers
        //----------------------------------------------------------------------

        static uint32_t lastConsumerPrint = 0;

        if (settings.enablePrintConsumers && ((millis() - lastConsumerPrint) > settings.periodicPrintInterval_ms))
        {
            lastConsumerPrint = millis();

            int milliseconds;
            int seconds;

            for (int index = 0; index < RBC_MAX; index++)
            {
                milliseconds = maxMillis[index];
                if (milliseconds > 1)
                {
                    seconds = milliseconds / MILLISECONDS_IN_A_SECOND;
                    milliseconds %= MILLISECONDS_IN_A_SECOND;
                    systemPrintf("Consumer %s: %d:%03d Sec\r\n", ringBufferConsumer[index], seconds, milliseconds);
                }
            }
        }

        //----------------------------------------------------------------------
        // Let other tasks run, prevent watch dog timer (WDT) resets
        //----------------------------------------------------------------------

        delay(1);
        taskYIELD();
    }
}

// Consume messages
void processConsumerMessage(PARSE_STATE *parse, uint8_t type)
{
    if (type == SENTENCE_TYPE_SBF) // SBF
    {
        if ((parse->message & 0x1FFF) == 5914) // ReceiverTime
        {
            gnssTimeUpdated[0] = gnssTimeUpdated[1] = gnssTimeUpdated[2] = true;
            gnssTimeArrivalMillis = millis();
            
            gnssTOW_ms = ((uint32_t)parse->buffer[8]) << 0;
            gnssTOW_ms |= ((uint32_t)parse->buffer[9]) << 8;
            gnssTOW_ms |= ((uint32_t)parse->buffer[10]) << 16;
            gnssTOW_ms |= ((uint32_t)parse->buffer[11]) << 24;
            if (gnssTOW_ms > 700000000)
                gnssTOW_ms = 0;

            union {
                int8_t signed8;
                uint8_t unsigned8;
            } signedUnsigned8;

            signedUnsigned8.unsigned8 = parse->buffer[14];
            if (signedUnsigned8.signed8 >= 0)
                gnssYear = signedUnsigned8.signed8 + 2000;
            else
                gnssYear = 0;
            signedUnsigned8.unsigned8 = parse->buffer[15];
            if (signedUnsigned8.signed8 >= 0)
                gnssMonth = signedUnsigned8.signed8;
            else
                gnssMonth = 0;
            signedUnsigned8.unsigned8 = parse->buffer[16];
            if (signedUnsigned8.signed8 >= 0)
                gnssDay = signedUnsigned8.signed8;
            else
                gnssDay = 0;
            signedUnsigned8.unsigned8 = parse->buffer[17];
            if (signedUnsigned8.signed8 >= 0)
                gnssHour = signedUnsigned8.signed8;
            else
                gnssHour = 0;
            signedUnsigned8.unsigned8 = parse->buffer[18];
            if (signedUnsigned8.signed8 >= 0)
                gnssMinute = signedUnsigned8.signed8;
            else
                gnssMinute = 0;
            signedUnsigned8.unsigned8 = parse->buffer[19];
            if (signedUnsigned8.signed8 >= 0)
                gnssSecond = signedUnsigned8.signed8;
            else
                gnssSecond = 0;

            gnssWNSet = (parse->buffer[21] >> 0) & 0x01;
            gnssToWSet = (parse->buffer[21] >> 1) & 0x01;
            gnssFineTime = (parse->buffer[21] >> 2) & 0x01;

            forceDisplayUpdate = true;
        }
        else if ((parse->message & 0x1FFF) == 4007) // PVTGeodetic
        {
            gnssPVTUpdated = true;
            gnssPVTArrivalMillis = millis();

            gnssError = parse->buffer[15];

            union {
                double dbl;
                uint64_t unsigned64;
            } dblUnsigned64;

            dblUnsigned64.unsigned64 = 0;
            for (int i = 0; i < 8; i++)
                dblUnsigned64.unsigned64 |= ((uint64_t)parse->buffer[16 + i]) << (i * 8);
            gnssLatitude_d = dblUnsigned64.dbl * 90.0 / (PI / 2.0);
            if ((gnssLatitude_d < -91.0) || (gnssLatitude_d > 91.0))
                gnssLatitude_d = 0.0;

            dblUnsigned64.unsigned64 = 0;
            for (int i = 0; i < 8; i++)
                dblUnsigned64.unsigned64 |= ((uint64_t)parse->buffer[24 + i]) << (i * 8);
            gnssLongitude_d = dblUnsigned64.dbl * 180.0 / PI;
            if ((gnssLongitude_d < -181.0) || (gnssLongitude_d > 181.0))
                gnssLongitude_d = 0.0;

            dblUnsigned64.unsigned64 = 0;
            for (int i = 0; i < 8; i++)
                dblUnsigned64.unsigned64 |= ((uint64_t)parse->buffer[32 + i]) << (i * 8);
            gnssAltitude_m = dblUnsigned64.dbl;
            if ((gnssAltitude_m < -1000.0) || (gnssAltitude_m > 50000.0))
                gnssAltitude_m = 0.0;

            dblUnsigned64.unsigned64 = 0;
            for (int i = 0; i < 8; i++)
                dblUnsigned64.unsigned64 |= ((uint64_t)parse->buffer[60 + i]) << (i * 8);
            gnssClockBias_ms = dblUnsigned64.dbl;
            if (gnssClockBias_ms < -999.999)
                gnssClockBias_ms = -999.999;
            if (gnssClockBias_ms > 999.999)
                gnssClockBias_ms = 999.999;

            gnssTimeSys = parse->buffer[72];
        }
        else if ((parse->message & 0x1FFF) == 4058) // IPStatus
        {
            uint32_t theIP = ((uint32_t)parse->buffer[32]) << 0;
            theIP |= ((uint32_t)parse->buffer[33]) << 8;
            theIP |= ((uint32_t)parse->buffer[34]) << 16;
            theIP |= ((uint32_t)parse->buffer[35]) << 24;
            gnssIP = IPAddress(theIP);
        }
        else if ((parse->message & 0x1FFF) == 4255) // FugroTimeOffset
        {
            int N = parse->buffer[14]; // Number of FugroTOSub sub-blocks in this block
            int SBLength = parse->buffer[15]; // Length of a FugroTOSub sub-block

            for (int b = 0; b < N; b++) // For each block
            {
                uint8_t SysUsage = parse->buffer[20 + (b * SBLength) + 0];
                uint8_t TimeSystem = parse->buffer[20 + (b * SBLength) + 2];

                union {
                    double dbl;
                    uint64_t unsigned64;
                } dblUnsigned64;

                dblUnsigned64.unsigned64 = 0;
                for (int i = 0; i < 8; i++)
                    dblUnsigned64.unsigned64 |= ((uint64_t)parse->buffer[20 + (b * SBLength) + 4 + i]) << (i * 8);

                // If this block contains a non-composite clock indicator, store it
                for (int TS = 0; TS < NUM_FUGRO_CLK_BIASES; TS++)
                {
                    if (fugroTimeSystems[TS].SysUsage == SysUsage)
                    {
                        fugroTimeSystems[TS].RxClkBias_ms = dblUnsigned64.dbl;
                        fugroTimeSystems[TS].updated = true;
                        break;
                    }
                }
            }
        }
    }
}

// Monitor momentary buttons
void ButtonCheckTask(void *e)
{
    if (setupBtn != nullptr)
        setupBtn->begin();

    while (true)
    {
        if (productVariant == RTK_MOSAIC_T)
        {
            if (setupBtn &&
                (settings.disableSetupButton == false)) // Allow check of the setup button if not overridden by settings
            {
                setupBtn->read();

                if (setupBtn->isPressed())
                {
                    // Do stuff... Maybe change the display?
                }
                else if (setupBtn->wasReleased())
                {
                    // Do stuff... Maybe change the display?
                }
            }
        }

        delay(1); // Poor man's way of feeding WDT. Required to prevent Priority 1 tasks from causing WDT reset
        taskYIELD();
    }
}

void idleTask(void *e)
{
    int cpu = xPortGetCoreID();
    uint32_t idleCount = 0;
    uint32_t lastDisplayIdleTime = 0;
    uint32_t lastStackPrintTime = 0;

    while (1)
    {
        // Increment a count during the idle time
        idleCount++;

        // Determine if it is time to print the CPU idle times
        if ((millis() - lastDisplayIdleTime) >= (IDLE_TIME_DISPLAY_SECONDS * 1000) && !inMainMenu)
        {
            lastDisplayIdleTime = millis();

            // Get the idle time
            if (idleCount > max_idle_count)
                max_idle_count = idleCount;

            // Display the idle times
            if (settings.enablePrintIdleTime)
            {
                systemPrintf("CPU %d idle time: %d%% (%d/%d)\r\n", cpu, idleCount * 100 / max_idle_count, idleCount,
                             max_idle_count);

                // Print the task count
                if (cpu)
                    systemPrintf("%d Tasks\r\n", uxTaskGetNumberOfTasks());
            }

            // Restart the idle count for the next display time
            idleCount = 0;
        }

        // Display the high water mark if requested
        if ((settings.enableTaskReports == true) &&
            ((millis() - lastStackPrintTime) >= (IDLE_TIME_DISPLAY_SECONDS * 1000)))
        {
            lastStackPrintTime = millis();
            systemPrintf("idleTask %d High watermark: %d\r\n", xPortGetCoreID(), uxTaskGetStackHighWaterMark(nullptr));
        }

        // The idle task should NOT delay or yield
    }
}

// Serial Read/Write tasks for the F9P must be started after BT is up and running otherwise SerialBT->available will
// cause reboot
bool tasksStartUART1()
{
    // Verify that the ring buffer was successfully allocated
    if (!ringBuffer)
    {
        systemPrintln("ERROR: Ring buffer allocation failure!");
        systemPrintln("Decrease GNSS handler (ring) buffer size");
        displayNoRingBuffer(5000);
        return false;
    }

    availableHandlerSpace = settings.gnssHandlerBufferSize;

    // Reads data from GNSS and stores data into circular buffer
    if (gnssReadTaskHandle == nullptr)
        xTaskCreatePinnedToCore(gnssReadTask,                  // Function to call
                                "gnssRead",                    // Just for humans
                                gnssReadTaskStackSize,         // Stack Size
                                nullptr,                       // Task input parameter
                                settings.gnssReadTaskPriority, // Priority
                                &gnssReadTaskHandle,           // Task handle
                                settings.gnssReadTaskCore);    // Core where task should run, 0=core, 1=Arduino

    // Reads data from circular buffer and passes it to consumers
    if (handleGnssDataTaskHandle == nullptr)
        xTaskCreatePinnedToCore(handleGnssDataTask,                  // Function to call
                                "handleGNSSData",                    // Just for humans
                                handleGnssDataTaskStackSize,         // Stack Size
                                nullptr,                             // Task input parameter
                                settings.handleGnssDataTaskPriority, // Priority
                                &handleGnssDataTaskHandle,           // Task handle
                                settings.handleGnssDataTaskCore);    // Core where task should run, 0=core, 1=Arduino

    return true;
}

// Stop tasks - useful when running firmware update or WiFi AP is running
void tasksStopUART1()
{
    // Delete tasks if running
    if (gnssReadTaskHandle != nullptr)
    {
        vTaskDelete(gnssReadTaskHandle);
        gnssReadTaskHandle = nullptr;
    }
    if (handleGnssDataTaskHandle != nullptr)
    {
        vTaskDelete(handleGnssDataTaskHandle);
        handleGnssDataTaskHandle = nullptr;
    }

    // Give the other CPU time to finish
    // Eliminates CPU bus hang condition
    delay(100);
}

// Validate the task table lengths
void tasksValidateTables()
{
    if (ringBufferConsumerEntries != RBC_MAX)
        reportFatalError("Fix ringBufferConsumer table to match RingBufferConsumers");
}
