FROM ubuntu:latest AS upstream

ARG DEBIAN_FRONTEND=noninteractive

ARG CORE_VERSION=3.0.7

ARG FIRMWARE_VERSION_MAJOR=9
ARG FIRMWARE_VERSION_MINOR=9

# ESP32 Core Debug Level
# We use "none" for releases and "error" for release_candidate
# You may find "verbose" useful while you are debugging your changes
ARG DEBUG_LEVEL=error

# arduino-cli warnings: none default more all
ARG WARNINGS=default

# Get curl and python3
RUN apt-get update \
    && apt-get install -y curl python3 python3-pip python3-venv \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Avoid the externally managed environment constraint
RUN PYTHON_VER=$(ls /usr/lib | grep python3.) \
    && echo "Python version: ${PYTHON_VER}" \
    && rm /usr/lib/${PYTHON_VER}/EXTERNALLY-MANAGED

# Install Python dependencies - esptool needs pyserial
#RUN python3 -m pip install --upgrade pip && \
RUN pip install pyserial

# Setup Arduino CLI
#RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=/usr/local/bin sh
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

# Start config file
RUN arduino-cli config init --additional-urls https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json,https://espressif.github.io/arduino-esp32/package_esp32_dev_index.json

# Update core index
RUN arduino-cli core update-index

# Update library index
RUN arduino-cli lib update-index

# Install platform
RUN arduino-cli core install "esp32:esp32@${CORE_VERSION}"

# Get Known Libraries
RUN arduino-cli lib install ESP32Time@2.0.0
RUN arduino-cli lib install JC_Button@2.1.2
RUN arduino-cli lib install "SparkFun Qwiic OLED Arduino Library"@1.0.13
RUN arduino-cli lib install "SparkFun Toolkit"@0.9.2
#RUN arduino-cli lib install "SparkFun SiT5358 DCTCXO Arduino Library"@1.0.1
#RUN arduino-cli lib install "SparkFun SiT5811 OCXO Arduino Library"@1.0.1
#RUN arduino-cli lib install "SparkFun STP3593LF OCXO Arduino Library"@1.0.1

# Enable external libs
RUN arduino-cli config set library.enable_unsafe_install true

# Get external libs
RUN arduino-cli lib install --git-url https://github.com/sparkfun/SparkFun_SiT5358_DCTCXO_Arduino_Library.git
RUN arduino-cli lib install --git-url https://github.com/sparkfun/SparkFun_SiT5811_OCXO_Arduino_Library.git
RUN arduino-cli lib install --git-url https://github.com/sparkfun/SparkFun_STP3593LF_OCXO_Arduino_Library.git

# Copy GNSSDO_Firmware and build deployment image
FROM upstream AS deployment

# Add the source files
ADD . .

# Copy custom app3M_fat9M_16MB.csv
RUN cp app3M_fat9M_16MB.csv "/root/.arduino15/packages/esp32/hardware/esp32/${CORE_VERSION}/tools/partitions/app3M_fat9M_16MB.csv"

# Compile Sketch
RUN cd ./GNSSDO_Firmware \
    && arduino-cli compile --fqbn "esp32:esp32:esp32":DebugLevel=${DEBUG_LEVEL} \
    --warnings ${WARNINGS} \
    ./GNSSDO_Firmware.ino \
    --build-property build.partitions=app3M_fat9M_16MB \
    --build-property upload.maximum_size=3145728 \
    --build-property "compiler.cpp.extra_flags=-MMD -c \
    \"-DFIRMWARE_VERSION_MAJOR=${FIRMWARE_VERSION_MAJOR}\" \
    \"-DFIRMWARE_VERSION_MINOR=${FIRMWARE_VERSION_MINOR}\"" \
    --export-binaries

# Copy the compile output. List the files
FROM deployment AS output
COPY --from=deployment ./GNSSDO_Firmware/build/esp32.esp32.esp32 /
CMD echo $(ls /*.*)
