//----------------------------------------
// Locals
//----------------------------------------

static QwiicCustomOLED *oled = nullptr;

// Fonts
#include <res/qw_fnt_5x7.h>
#include <res/qw_fnt_8x16.h>
#include <res/qw_fnt_largenum.h>

// Icons
#include "icons.h"

//----------------------------------------
// Routines
//----------------------------------------

void beginDisplay(TwoWire *i2cBus)
{
    if (displayType == DISPLAY_MAX_NONE)
        return;

    if (i2cBus == nullptr)
        reportFatalError("Illegal display i2cBus");

    uint8_t i2cAddress;
    uint16_t x;
    uint16_t y;

    // Setup the appropriate display

    if (displayType == DISPLAY_64x48)
    {
        i2cAddress = kOLEDMicroDefaultAddress;
        if (oled == nullptr)
            oled = new QwiicCustomOLED;
        if (!oled)
        {
            systemPrintln("ERROR: Failed to allocate oled data structure!\r\n");
            return;
        }

        // Set the display parameters
        oled->setXOffset(kOLEDMicroXOffset);
        oled->setYOffset(kOLEDMicroYOffset);
        oled->setDisplayWidth(kOLEDMicroWidth);
        oled->setDisplayHeight(kOLEDMicroHeight);
        oled->setPinConfig(kOLEDMicroPinConfig);
        oled->setPreCharge(kOLEDMicroPreCharge);
        oled->setVcomDeselect(kOLEDMicroVCOM);
    }

    if (displayType == DISPLAY_128x64)
    {
        i2cAddress = kOLEDMicroDefaultAddress;
        if (oled == nullptr)
            oled = new QwiicCustomOLED;
        if (!oled)
        {
            systemPrintln("ERROR: Failed to allocate oled data structure!\r\n");
            return;
        }

        oled->setXOffset(0);         // Set the active area X offset
        oled->setYOffset(0);         // Set the active area Y offset
        oled->setDisplayWidth(128);  // Set the active area width
        oled->setDisplayHeight(64);  // Set the active area height
        oled->setPinConfig(0x12);    // Set COM Pins Hardware Configuration (DAh)
        oled->setPreCharge(0xF1);    // Set Pre-charge Period (D9h)
        oled->setVcomDeselect(0x40); // Set VCOMH Deselect Level (DBh)
        oled->setContrast(0xCF);     // Set Contrast Control for BANK0 (81h)
    }

    // Display may still be powering up
    // Try multiple times to communicate then display logo
    int maxTries = 3;
    for (int tries = 0; tries < maxTries; tries++)
    {
        if (oled->begin(*i2cBus, i2cAddress) == true)
        {
            online.display = true;

            systemPrintln("Display started");

            // Display the SparkFun LOGO
            oled->erase();
            x = (oled->getWidth() - logoSparkFun_Width) / 2;
            y = (oled->getHeight() - logoSparkFun_Height) / 2;
            displayBitmap(x, y, logoSparkFun_Width, logoSparkFun_Height, logoSparkFun);
            oled->display();
            splashStart = millis();
            return;
        }

        delay(50); // Give display time to startup before attempting again
    }
}

// Given the system state, display the appropriate information
void updateDisplay()
{
    // Update the display if connected
    if (online.display == true)
    {
        if (millis() - lastDisplayUpdate > 500 || forceDisplayUpdate == true) // Update display at 2Hz
        {
            lastDisplayUpdate = millis();
            forceDisplayUpdate = false;

            oled->reset(false); // Incase of previous corruption, force re-alignment of CGRAM. Do not init buffers as it
                               // takes time and causes screen to blink.

            oled->erase();

            char textLine[22];
            snprintf(textLine, sizeof(textLine), "IP    %s",
                    gnssIP.toString().c_str());
            oled->setFont(QW_FONT_5X7); // Set font to smallest
            oled->setCursor(0, 0);
            oled->print(textLine);

            snprintf(textLine, sizeof(textLine), "%04d/%02d/%02d   %02d:%02d:%02d",
                    gnssYear, gnssMonth, gnssDay, gnssHour, gnssMinute, gnssSecond);
            oled->setCursor(0, 8);
            oled->print(textLine);

            snprintf(textLine, sizeof(textLine), "Lat   %.7f",
                    gnssLatitude_d);
            oled->setCursor(0, 16);
            oled->print(textLine);

            snprintf(textLine, sizeof(textLine), "Long  %.7f",
                    gnssLongitude_d);
            oled->setCursor(0, 24);
            oled->print(textLine);

            snprintf(textLine, sizeof(textLine), "Sys   %s",
                    mosaicTimeSystemNameFromId(gnssTimeSys));
            oled->setCursor(0, 32);
            oled->print(textLine);

            snprintf(textLine, sizeof(textLine), "Error %s",
                    mosaicPVTErrorNameFromId(gnssError));
            oled->setCursor(0, 40);
            oled->print(textLine);

            snprintf(textLine, sizeof(textLine), "Fine  %s",
                    gnssFineTime ? "True" : "False");
            oled->setCursor(0, 48);
            oled->print(textLine);

            if ((gnssClockBias_ms >= 1.0) || (gnssClockBias_ms <= -1.0))
                snprintf(textLine, sizeof(textLine), "Bias  %.3fms",
                    (float)gnssClockBias_ms);
            else if ((gnssClockBias_ms >= 0.001) || (gnssClockBias_ms <= -0.001))
                snprintf(textLine, sizeof(textLine), "Bias  %.3f%cs",
                    (float)(gnssClockBias_ms * 1000.0, char(181)));
            else
                snprintf(textLine, sizeof(textLine), "Bias  %.3fns",
                    (float)(gnssClockBias_ms * 1000000.0));
            oled->setCursor(0, 56);
            oled->print(textLine);

            oled->display(); // Push internal buffer to display
        }
    } // End display online
}

void displaySplash()
{
    if (online.display == true)
    {
        // Display SparkFun Logo for at least 1/10 of a second
        unsigned long minSplashFor = 1000;
        while ((millis() - splashStart) < minSplashFor)
            delay(10);

        oled->erase();

        int yPos = 0;
        int fontHeight = 8;

        printTextCenter("SparkFun", yPos, QW_FONT_5X7, 1, false); // text, y, font type, kerning, inverted

        yPos = yPos + fontHeight + 2;
        printTextCenter("RTK", yPos, QW_FONT_8X16, 1, false);

        yPos = yPos + fontHeight + 5;
        printTextCenter(productDisplayNames[productVariant], yPos, QW_FONT_8X16, 1, false);

        yPos = yPos + fontHeight + 7;
        char unitFirmware[50];
        getFirmwareVersion(unitFirmware, sizeof(unitFirmware), false);
        printTextCenter(unitFirmware, yPos, QW_FONT_5X7, 1, false);

        oled->display();

        // Start the timer for the splash screen display
        splashStart = millis();
    }
}

void displayGNSSFail(uint16_t displayTime)
{
    displayMessage("GNSS Failed", displayTime);
}

void displayNoRingBuffer(uint16_t displayTime)
{
    if (online.display == true)
    {
        oled->erase();

        uint8_t fontHeight = 8;
        uint8_t yPos = oled->getHeight() / 3 - fontHeight;

        printTextCenter("Fix GNSS", yPos, QW_FONT_5X7, 1, false); // text, y, font type, kerning, inverted
        yPos += fontHeight;
        printTextCenter("Handler", yPos, QW_FONT_5X7, 1, false); // text, y, font type, kerning, inverted
        yPos += fontHeight;
        printTextCenter("Buffer Sz", yPos, QW_FONT_5X7, 1, false); // text, y, font type, kerning, inverted

        oled->display();

        delay(displayTime);
    }
}

// When user enters serial config menu the display will freeze so show splash while config happens
void displaySerialConfig()
{
    displayMessage("Serial Config", 0);
}

// Displays a small error message then hard freeze
// Text wraps and is small but legible
void displayError(const char *errorMessage)
{
    if (online.display == true)
    {
        oled->erase(); // Clear the display's internal buffer

        oled->setCursor(0, 0);      // x, y
        printTextCenter("Error:", 0, QW_FONT_5X7, 1, true); // text, y, font type, kerning, inverted

        oled->setCursor(0, 10);
        // oled->setFont(QW_FONT_8X16);
        oled->print(errorMessage);

        oled->display(); // Push internal buffer to display
    }
}

// Given text, a position, and kerning, print text to display
// This is helpful for squishing or stretching a string to appropriately fill the display
void printTextwithKerning(const char *newText, uint8_t xPos, uint8_t yPos, uint8_t kerning)
{
    if (online.display == true)
    {
        for (int x = 0; x < strlen(newText); x++)
        {
            oled->setCursor(xPos, yPos);
            oled->print(newText[x]);
            xPos += kerning;
        }
    }
}

// Draw a frame at outside edge
void drawFrame()
{
    if (online.display == true)
    {
        // Init and draw box at edge to see screen alignment
        int xMax = 63;
        int yMax = 47;
        oled->line(0, 0, xMax, 0);       // Top
        oled->line(0, 0, 0, yMax);       // Left
        oled->line(0, yMax, xMax, yMax); // Bottom
        oled->line(xMax, 0, xMax, yMax); // Right
    }
}

// Given text, and location, print text center of the screen
void printTextCenter(const char *text, uint8_t yPos, QwiicFont &fontType, uint8_t kerning,
                     bool highlight) // text, y, font type, kearning, inverted
{
    if (online.display == true)
    {
        oled->setFont(fontType);
        oled->setDrawMode(grROPXOR);

        uint8_t fontWidth = fontType.width;
        if (fontWidth == 8)
            fontWidth = 7; // 8x16, but widest character is only 7 pixels.

        uint8_t xStart = (oled->getWidth() / 2) - ((strlen(text) * (fontWidth + kerning)) / 2) + 1;

        uint8_t xPos = xStart;
        for (int x = 0; x < strlen(text); x++)
        {
            oled->setCursor(xPos, yPos);
            oled->print(text[x]);
            xPos += fontWidth + kerning;
        }

        if (highlight) // Draw a box, inverted over text
        {
            uint8_t textPixelWidth = strlen(text) * (fontWidth + kerning);

            // Error check
            int xBoxStart = xStart - 5;
            if (xBoxStart < 0)
                xBoxStart = 0;
            int xBoxEnd = textPixelWidth + 9;
            if (xBoxEnd > oled->getWidth() - 1)
                xBoxEnd = oled->getWidth() - 1;

            oled->rectangleFill(xBoxStart, yPos, xBoxEnd, 12, 1); // x, y, width, height, color
        }
    }
}

// Given a message (one or two words) display centered
void displayMessage(const char *message, uint16_t displayTime)
{
    if (online.display == true)
    {
        char temp[21];
        uint8_t fontHeight = 15; // Assume fontsize 1

        // Count words based on spaces
        uint8_t wordCount = 0;
        strncpy(temp, message, sizeof(temp) - 1); // strtok modifies the message so make copy
        char *token = strtok(temp, " ");
        while (token != nullptr)
        {
            wordCount++;
            token = strtok(nullptr, " ");
        }

        uint8_t yPos = (oled->getHeight() / 2) - (fontHeight / 2);
        if (wordCount == 2)
            yPos -= (fontHeight / 2);

        oled->erase();

        // drawFrame();

        strncpy(temp, message, sizeof(temp) - 1);
        token = strtok(temp, " ");
        while (token != nullptr)
        {
            printTextCenter(token, yPos, QW_FONT_8X16, 1, false); // text, y, font type, kerning, inverted
            token = strtok(nullptr, " ");
            yPos += fontHeight;
        }

        oled->display();

        delay(displayTime);
    }
}

// Wrapper to avoid needing to pass width/height data twice
void displayBitmap(uint8_t x, uint8_t y, uint8_t imageWidth, uint8_t imageHeight, const uint8_t *imageData)
{
    if (online.display == true)
        oled->bitmap(x, y, x + imageWidth, y + imageHeight, (uint8_t *)imageData, imageWidth, imageHeight);
}
