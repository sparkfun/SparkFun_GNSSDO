docker build -t gnssdo_firmware --no-cache-filter deployment .
docker create --name=gnssdo_image gnssdo_firmware:latest
docker cp gnssdo_image:/GNSSDO_Firmware.ino.bin .
docker cp gnssdo_image:/GNSSDO_Firmware.ino.elf .
docker container rm gnssdo_image
