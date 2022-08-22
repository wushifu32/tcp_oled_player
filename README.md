# TCP OLED Player

Use ESP32 to driver a small OLED screen.

### Config
Config your wifi ssid & passwd in sdkconfig file:
```
CONFIG_EXAMPLE_WIFI_SSID="bigone"
CONFIG_EXAMPLE_WIFI_PASSWORD="bigbrother"
```

### Build
Follow the esp32 idf build instructions

### Others
Transfer a image file into C code array:
```bash
# build format tool first
gcc -o format format.c

python img2data.py xxxx.jpg a.out
cat a.out | format > a.c
```

Start to send video frame to OLED:
1. Find the ip address of ESP32
2. Change the 'HOST' value in `tools/tcp_video_player.py`
```
HOST = "192.168.2.7"
```
3. Play & send video
```bash
./tools/tcp_video_player.py file_to_video.mp4
```
