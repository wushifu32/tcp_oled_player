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
