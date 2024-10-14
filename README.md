# ESP32 Fake Wi-Fi Network Rickroll

Just a simple ESP32 script that uses the ESP32's onboard Wi-Fi to broadcast a fake network, which rickrolls unsuspecting users upon joining.

## Video Tutorial

<a href="https://www.youtube.com/watch?v=er-FVC-X14Y">
  <img src="https://img.youtube.com/vi/er-FVC-X14Y/maxresdefault.jpg" alt="Tutorial" style="width:100%; max-width:800px;">
</a>


## SD Card Version

Wire the SD card module as shown in the diagram below:

<img src="https://i0.wp.com/randomnerdtutorials.com/wp-content/uploads/2021/02/ESP32-microSD-Card-Wiring-Diagram.png" alt="Tutorial" style="width:60%; max-width:800px;">

Once wired, be sure to format your SD card to FAT32. Upload the video file you'd like to play upon joining and name it `your-video-file.mp4`. After that, upload the SD card version of the sketch to your ESP32 and insert the SD card into the reader.

## Non-SD Card Version

For the non-SD card version, you’ll need to flash a shortened audio-only file to the ESP32 using [SPIFFS](https://www.programmingelectronics.com/spiffs-esp32/) and name it `your-audio-file.mp4`. Then, upload the non-SD card version of the sketch to the ESP32.
