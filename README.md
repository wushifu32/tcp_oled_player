# TCP OLED Player

Use ESP32 to driver a small OLED screen.

### Config
Config your wifi ssid & passwd in sdkconfig file:
```
CONFIG_EXAMPLE_WIFI_SSID="bigone"
CONFIG_EXAMPLE_WIFI_PASSWORD="bigbrother"
```

### Build
Add **lvgl** component to esp-idf: [Use LVGL in your ESP-IDF project](https://github.com/lvgl/lv_port_esp32#use-lvgl-in-your-esp-idf-project)
Add **SDD1306 driver** componet to esp-idf: [Use lvgl_esp32_drivers in your project](https://github.com/lvgl/lv_port_esp32#use-lvgl_esp32_drivers-in-your-project)
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

### Issues
1. I2c bus may deadlock to cause ESP32 exception
To fix it, apply the following patch:
```diff
diff --git a/lvgl_tft/ssd1306.c b/lvgl_tft/ssd1306.c
index c845eac..3eb82d3 100644
--- a/lvgl_tft/ssd1306.c
+++ b/lvgl_tft/ssd1306.c
@@ -13,7 +13,8 @@
 /*********************
  *      INCLUDES
  *********************/
-#include "assert.h"
+//#include "assert.h"
+#define assert(x)

 #include "lvgl_i2c/i2c_manager.h"
```
The i2c_write function can recover from deadlock state, so do need to kill self when transmision failed.
