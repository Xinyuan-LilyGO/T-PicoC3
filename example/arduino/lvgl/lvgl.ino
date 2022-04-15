#include "Arduino.h"
#include "pin_config.h"
#include "picoImage.h"
#include "TFT_eSPI.h"
#include <lvgl.h>
#include <lv_demo.h>

#define SCREEN_WIDTH 240
#define SCREEN_HIGH 135
#define SCREEN_BUF_SIZE (SCREEN_WIDTH * SCREEN_HIGH)
TFT_eSPI tft;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[SCREEN_BUF_SIZE];
static bool btn1_pressed, btn2_pressed;
static uint32_t btn1_delay, btn2_delay;
#define gradually_light()          \
    for (int i = 0; i < 0xff; i++) \
    {                              \
        delay(5);                  \
        analogWrite(TFT_BL, i);    \
    }

void setup()
{
    Serial.begin(115200);
    pinMode(PIN_PWR_ON, OUTPUT);
    digitalWrite(PIN_PWR_ON, HIGH);

    tft.begin();
    tft.initDMA();
    tft.setSwapBytes(true);
    tft.setRotation(1);
    tft.pushImage(0, 0, 240, 135, lilygo_logo);
    gradually_light();

    lv_init();
    lv_port_init();

    /* You need to cancel the corresponding comments in lv_example/lv_demo_conf.h. */
    // lv_demo_music();
    // lv_demo_benchmark();
    // lv_demo_stress();
    lv_demo_keypad_encoder();
}

void loop()
{
    lv_timer_handler();
    if (!digitalRead(PIN_BOTTON1))
    {
        if (millis() - btn1_delay > 200)
        {
            btn1_pressed = true;
            Serial.println("PIN_BOTTON1 PRESSED");
            btn1_delay = millis();
        }
    }

   if (!digitalRead(PIN_BOTTON2))
    {
        if (millis() - btn2_delay > 200)
        {
            btn2_pressed = true;
            Serial.println("PIN_BOTTON1 PRESSED");
            btn2_delay = millis();
        }
    }
}

void lv_port_init()
{
    pinMode(PIN_BOTTON1, INPUT);
    pinMode(PIN_BOTTON2, INPUT);

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, SCREEN_BUF_SIZE);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HIGH;
    disp_drv.flush_cb = dispaly_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = key_read;
    lv_indev_drv_register(&indev_drv);
}

void dispaly_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushPixelsDMA((uint16_t *)color_p, w * h);
    tft.endWrite();
    lv_disp_flush_ready(disp);
}
void key_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    if (btn1_pressed)
    {
        data->enc_diff = 1;
        btn1_pressed = false;
    }
    else if (btn2_pressed)
    {
        data->enc_diff = -1;
        btn2_pressed = false;
    }

    if (BOOTSEL)
        data->state = LV_INDEV_STATE_PR;
    else
        data->state = LV_INDEV_STATE_REL;
}