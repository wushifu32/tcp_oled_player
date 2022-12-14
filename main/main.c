#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "freertos/stream_buffer.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "lvgl.h"
#include "lvgl_helpers.h"

#define TAG "oled"
#define LV_TICK_PERIOD_MS           (1000)
#define PORT                        (3333)
#define KEEPALIVE_IDLE              (5)
#define KEEPALIVE_INTERVAL          (5)
#define KEEPALIVE_COUNT             (3)
#define FRAME_SIZE                  (128*64/8)
#define STREAM_BUF_SIZE             (4*FRAME_SIZE)

const LV_ATTRIBUTE_MEM_ALIGN uint8_t cover_map[] = {
0xFF,0xFF,0xFF,0xFF,0xFF,0xC0,0x3F,0xFF,0xFF,0xFB,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xE0,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xFB,0xFF,0xFF,0xFF,0xFC,0x1F,0xFF,0xFF,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x1F,0xFF,0xFF,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0xFF,0xFF,0xFF,0xFF,0x07,0xF7,0xFF,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0xFF,0xFF,0xFF,0xFF,0x83,0xFB,0xFF,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xE6,0xFD,0xFF,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,0xFF,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xF8,0x1F,0xFF,0xFF,0xFF,0xE7,0xFF,0x3F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,0x07,0xFF,0xFF,0xFF,0xDF,0xFE,0x3F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xCF,0xE3,0xFF,0xFF,0xFF,0x87,0xFF,0x7F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,0xF1,0xFF,0xFF,0xFF,0x0F,0x2F,0x7F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xBF,0xFF,0xFF,0xFF,0xFF,0x07,0xE7,0xEF,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFB,0x7F,0xFF,0xFF,0xFB,0xFF,0x87,0xEF,0x97,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xE3,0x7F,0xFF,0xFF,0xF0,0x0F,0xCD,0xC7,0x47,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFB,0x7F,0xFF,0xFF,0xF0,0x07,0x87,0x86,0xC7,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xF3,0x07,0xFF,0xFF,0xFF,0xC3,0x83,0x93,0x3F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0x22,0x80,0x1F,0xFF,0xFF,0xF9,0x83,0x38,0xFF,0x3F,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFA,0x47,0x80,0x0F,0xFF,0xFF,0xFC,0x00,0xBB,0xFF,0xBF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFB,0x87,0xFF,0xF7,0xFE,0xFF,0xFC,0x00,0xF9,0xFE,0xBF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xF0,0x7F,0xFF,0xFF,0xFF,0xFF,0xFC,0x00,0x38,0xFC,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xF0,0x1F,0xFF,0xFF,0xFF,0xFF,0xFC,0x00,0x07,0xFE,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFB,0xFF,0xFF,0xFF,0xFF,0x07,0xFC,0x00,0x0F,0xFF,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFD,0xFF,0xFF,0xFF,0xFF,0xF0,0x7C,0x00,0x1F,0xFF,0xDF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xFF,0xFF,0xF0,0x1C,0x00,0x07,0xFF,0xDF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xCF,0xFC,0x0C,0x00,0x17,0xFF,0xBF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x8F,0xFF,0x04,0x00,0x7F,0xFF,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFE,0xFF,0xFF,0xC6,0x0F,0xFF,0x80,0x00,0x0F,0xFF,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x10,0x1F,0xFF,0xF2,0x01,0x07,0xF7,0x1F,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF8,0x1B,0xFF,0xFE,0x01,0x0F,0xFE,0x9F,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xE1,0xFF,0xFE,0x07,0x0F,0xF8,0xBF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xE1,0xFF,0xFC,0x06,0x0F,0x7A,0xBF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xBF,0xFF,0xFC,0x3F,0xFF,0xFF,0xFC,0x06,0x0F,0xFC,0x7F,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,0x0F,0xFF,0xFF,0xF0,0x04,0x0F,0x9C,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFE,0xFF,0xFF,0xE0,0x1C,0x7F,0xFF,0xE0,0x00,0x1F,0xFD,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFE,0x7F,0xFF,0xE2,0xE0,0x1F,0xFF,0xC0,0x00,0x0E,0xED,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xC7,0xFC,0x0F,0xFF,0x80,0x00,0x07,0xDD,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0xCF,0xFF,0x07,0xDF,0x00,0x00,0x0B,0xC7,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFA,0x7F,0xC8,0x3F,0x83,0xFF,0x00,0x00,0x13,0xC7,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xF2,0x7F,0xC0,0x07,0xC3,0xFE,0x00,0x00,0x9F,0xCC,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xE0,0xFF,0xC0,0x00,0xF1,0xFC,0x00,0x00,0x9F,0xC1,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xC0,0x7F,0xC0,0x00,0x29,0xFC,0x00,0x00,0xDF,0xEB,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xC0,0x3F,0x80,0x00,0x31,0xFC,0x00,0x00,0xEF,0x87,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xC2,0x7F,0x9E,0x00,0x01,0xFF,0x00,0x03,0xE7,0x83,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xC4,0xBF,0xC3,0xC0,0x03,0xFE,0x00,0x03,0xE1,0x01,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xC0,0x3F,0xC2,0xF8,0x03,0xFE,0x00,0x07,0xE0,0x03,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0x00,0x7F,0xC1,0xE3,0x07,0xFE,0x00,0x03,0xC0,0x07,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0x00,0x3F,0xE1,0xC0,0x0F,0xFE,0x00,0x01,0xC0,0x07,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0x80,0x3F,0xFE,0x00,0x1F,0xFE,0x00,0x83,0xE0,0x07,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0x78,0x1F,0xFF,0xC0,0x7F,0xFE,0x00,0x03,0x82,0x0F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0x78,0x1F,0xFF,0xFF,0xFF,0xFE,0x01,0x01,0xF0,0x1F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xBC,0x1F,0xFF,0xFF,0xFF,0xFC,0x00,0x40,0x04,0x1F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0x1E,0x1F,0xFF,0xFF,0xFF,0xF8,0x00,0x00,0x0C,0x3F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFE,0x18,0x1F,0xFF,0xFF,0xFF,0xF8,0x00,0x00,0x18,0x3F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0x1E,0x1F,0xFF,0xFF,0xFF,0xF0,0x00,0x80,0x00,0x7F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xBC,0x1F,0xFF,0xFF,0xFF,0xE0,0x00,0x00,0x02,0x7F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0x7E,0x1B,0xFF,0xFF,0xFF,0xC0,0x00,0x80,0x01,0xFF,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0x7F,0x98,0x7F,0xFF,0xDF,0x80,0x08,0x80,0x00,0xFF,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0x3F,0x98,0x03,0xFE,0x04,0x00,0x02,0x00,0x00,0x7F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0x07,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xBE,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFE,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xBE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0xFF,0xFF,\
0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x03,0x80,0x00,0x01,0xFF,0xFF,0xFF,0xFF,\
};

lv_img_dsc_t mbin = {
  .header.cf = LV_IMG_CF_ALPHA_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 128,
  .header.h = 64,
  .data_size = FRAME_SIZE,
  .data = cover_map,
};

static void lv_tick_task(void *arg);
static void guiTask(void *pvParameter);
static void create_demo_application(void);
static uint8_t frame_buffer[FRAME_SIZE];
static int frame_cnt;
StreamBufferHandle_t stream_buf;
lv_obj_t *frame;
static volatile int play_start;

static void do_retransmit(const int sock)
{
    int len;
    char rx_buffer[2048];

    do {
        len = recv(sock, rx_buffer, sizeof(rx_buffer), 0);
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        } else if (len == 0) {
	   		play_start = 0;
            ESP_LOGW(TAG, "Connection closed");
        } else {
            int cnt = len;
            play_start = 1;
            while (cnt > 0) {
		    size_t result = xStreamBufferSend(stream_buf, &rx_buffer, cnt, portMAX_DELAY);
		    cnt -= result;
	    }
            //ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
        }
    } while (len > 0);
}

static void tcp_server_task(void *pvParameters)
{
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;

    struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family = AF_INET;
    dest_addr_ip4->sin_port = htons(PORT);
    ip_protocol = IPPROTO_IP;

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) {

        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
        // Convert ip address to string
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }

        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

void app_main() {

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());
	
    stream_buf = xStreamBufferCreate(STREAM_BUF_SIZE, FRAME_SIZE);
    xTaskCreate(tcp_server_task, "tcp_server", 4096*2, (void*)AF_INET, 5, NULL);

    /* Pinned task to avoid memory corruption problems */
    xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 0, NULL, 1);
}

SemaphoreHandle_t xGuiSemaphore;

static void guiTask(void *pvParameter) {

    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();
    lvgl_driver_init();

    lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);
    static lv_color_t *buf2 = NULL;
    static lv_disp_buf_t disp_buf;
    uint32_t size_in_px = DISP_BUF_SIZE;
    /* Actual size in pixels, not bytes. */
    size_in_px *= 8;
    /* Initialize the working buffer depending on the selected display.
     * NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;
    /* When using a monochrome display we need to register the callbacks:
     * - rounder_cb
     * - set_px_cb */
    disp_drv.rounder_cb = disp_driver_rounder;
    disp_drv.set_px_cb = disp_driver_set_px;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 40));

    /* Create the demo application */
    create_demo_application();

    while (1) {
        //vTaskDelay(pdMS_TO_TICKS(30));
        size_t result = xStreamBufferReceive(stream_buf, frame_buffer+frame_cnt ,
                        FRAME_SIZE-frame_cnt, pdMS_TO_TICKS(40));
        frame_cnt += result;
        if (play_start && frame_cnt < FRAME_SIZE) {
            ESP_LOGW(TAG, "frame lost");
        }
        assert(frame_cnt <= FRAME_SIZE);

        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            if (play_start && frame_cnt == FRAME_SIZE) {
                 mbin.data = (const uint8_t *)frame_buffer;
                 frame_cnt = 0;
                 lv_img_set_src(frame, &mbin);
            } else if (!play_start) {
                 mbin.data = (const uint8_t *)cover_map;
                 lv_img_set_src(frame, &mbin);
            }
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
       }
    }

    /* A task should NEVER return */
    free(buf1);
    vTaskDelete(NULL);
}

static void create_demo_application(void)
{
    lv_obj_t * scr = lv_disp_get_scr_act(NULL);
    frame =  lv_img_create(scr, NULL);
    lv_img_set_src(frame, &mbin);
}

static void lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}
