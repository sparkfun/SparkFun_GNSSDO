//----------------------------------------
// Locals
//----------------------------------------

static QwiicMicroOLED oled;

// Fonts
#include <res/qw_fnt_5x7.h>
#include <res/qw_fnt_8x16.h>
#include <res/qw_fnt_largenum.h>

// Icons
#include "icons.h"

//----------------------------------------
// Routines
//----------------------------------------

void beginDisplay()
{
    // At this point we have not identified the RTK platform
    int maxTries = 3;
    for (int x = 0; x < maxTries; x++)
    {
        if (oled.begin() == true)
        {
            online.display = true;

            systemPrintln("Display started");

            oled.erase();
            return;
        }

        delay(50); // Give display time to startup before attempting again
    }

    systemPrintln("Display not detected");
}

// Display the SparkFun logo
void displaySfeFlame()
{
    if (online.display == true)
    {
        oled.erase();
        displayBitmap(0, 0, logoSparkFun_Width, logoSparkFun_Height, logoSparkFun);
        oled.display();
        splashStart = millis();
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

            oled.reset(false); // Incase of previous corruption, force re-alignment of CGRAM. Do not init buffers as it
                               // takes time and causes screen to blink.

            oled.erase();


            oled.display(); // Push internal buffer to display
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

        oled.erase();

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

        oled.display();

        // Start the timer for the splash screen display
        splashStart = millis();
    }
}

// Displays a small error message then hard freeze
// Text wraps and is small but legible
void displayError(const char *errorMessage)
{
    if (online.display == true)
    {
        oled.erase(); // Clear the display's internal buffer

        oled.setCursor(0, 0);      // x, y
        oled.setFont(QW_FONT_5X7); // Set font to smallest
        oled.print("Error:");

        oled.setCursor(2, 10);
        // oled.setFont(QW_FONT_8X16);
        oled.print(errorMessage);

        oled.display(); // Push internal buffer to display

        while (1)
            delay(10); // Hard freeze
    }
}

// Given text, a position, and kerning, print text to display
// This is helpful for squishing or stretching a string to appropriately fill the display
void printTextwithKerning(const char *newText, uint8_t xPos, uint8_t yPos, uint8_t kerning)
{
    for (int x = 0; x < strlen(newText); x++)
    {
        oled.setCursor(xPos, yPos);
        oled.print(newText[x]);
        xPos += kerning;
    }
}

// Scroll through IP address. Wipe with spaces both ends.
void paintIPAddress()
{
    char ipAddress[22];
    snprintf(ipAddress, sizeof(ipAddress), "IP    %d.%d.%d.%d",
             localIP[0], localIP[1], localIP[2], localIP[3]);

    oled.setFont(QW_FONT_5X7); // Set font to smallest
    oled.setCursor(0, 0);
    oled.print(ipAddress);
}

// Draw a frame at outside edge
void drawFrame()
{
    // Init and draw box at edge to see screen alignment
    int xMax = 63;
    int yMax = 47;
    oled.line(0, 0, xMax, 0);       // Top
    oled.line(0, 0, 0, yMax);       // Left
    oled.line(0, yMax, xMax, yMax); // Bottom
    oled.line(xMax, 0, xMax, yMax); // Right
}

// Given text, and location, print text center of the screen
void printTextCenter(const char *text, uint8_t yPos, QwiicFont &fontType, uint8_t kerning,
                     bool highlight) // text, y, font type, kearning, inverted
{
    oled.setFont(fontType);
    oled.setDrawMode(grROPXOR);

    uint8_t fontWidth = fontType.width;
    if (fontWidth == 8)
        fontWidth = 7; // 8x16, but widest character is only 7 pixels.

    uint8_t xStart = (oled.getWidth() / 2) - ((strlen(text) * (fontWidth + kerning)) / 2) + 1;

    uint8_t xPos = xStart;
    for (int x = 0; x < strlen(text); x++)
    {
        oled.setCursor(xPos, yPos);
        oled.print(text[x]);
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
        if (xBoxEnd > oled.getWidth() - 1)
            xBoxEnd = oled.getWidth() - 1;

        oled.rectangleFill(xBoxStart, yPos, xBoxEnd, 12, 1); // x, y, width, height, color
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

        uint8_t yPos = (oled.getHeight() / 2) - (fontHeight / 2);
        if (wordCount == 2)
            yPos -= (fontHeight / 2);

        oled.erase();

        // drawFrame();

        strncpy(temp, message, sizeof(temp) - 1);
        token = strtok(temp, " ");
        while (token != nullptr)
        {
            printTextCenter(token, yPos, QW_FONT_8X16, 1, false); // text, y, font type, kerning, inverted
            token = strtok(nullptr, " ");
            yPos += fontHeight;
        }

        oled.display();

        delay(displayTime);
    }
}

// Wrapper to avoid needing to pass width/height data twice
void displayBitmap(uint8_t x, uint8_t y, uint8_t imageWidth, uint8_t imageHeight, const uint8_t *imageData)
{
    oled.bitmap(x, y, x + imageWidth, y + imageHeight, (uint8_t *)imageData, imageWidth, imageHeight);
}
