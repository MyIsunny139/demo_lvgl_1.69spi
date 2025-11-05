#include "lvgl.h"
#include "esp_log.h"
#include "lv_port.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "st7789_driver.h"
#include "cst816t_driver.h"
#include "driver/gpio.h"
#include "esp_timer.h"

static lv_disp_drv_t disp_drv;

#define TAG "LVGL"

#define LCD_WIDTH 240
#define LCD_HEIGHT 280

void lv_flush_done_cb(void * parram)
{
    lv_disp_flush_ready(&disp_drv);  //写入完成通知LVGL刷新完成

}




void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    st7789_flush(area->x1,area->x2+1,area->y1+20,area->y2+20+1,color_p); //刷新屏幕

}



void lv_disp_init(void)
{
    
    static lv_disp_draw_buf_t disp_buf;
    const size_t disp_buf_size = LCD_WIDTH * (LCD_HEIGHT/7);
    lv_color_t *buf1 = heap_caps_malloc(disp_buf_size * sizeof(lv_color_t), MALLOC_CAP_DMA|MALLOC_CAP_INTERNAL);
    lv_color_t *buf2 = heap_caps_malloc(disp_buf_size * sizeof(lv_color_t), MALLOC_CAP_DMA|MALLOC_CAP_INTERNAL);
    if(!buf1 || !buf2) {
        ESP_LOGE(TAG, "Failed to allocate memory for display buffer");
        return;
    }
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, disp_buf_size);

    
    lv_disp_drv_init(&disp_drv);
    
    disp_drv.hor_res = LCD_WIDTH;
    disp_drv.ver_res = LCD_HEIGHT;
    
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = my_disp_flush;
    lv_disp_drv_register(&disp_drv);
}



void IRAM_ATTR touch_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    int16_t x, y;
    int state;
    cst816t_read(&x, &y, &state);
    data->point.x = x;
    data->point.y = y;
    data->state = state ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
}

/**
 * 初始化LVGL输入设备
 * 1. 初始化输入设备驱动结构体
 * 2. 设置设备类型为指针设备(触摸屏)
 * 3. 设置触摸读取回调函数
 * 4. 注册输入设备驱动
 */
void lv_indev_init(void)
{
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touch_read;
    lv_indev_drv_register(&indev_drv);
}

void st7789_hw_init(void)
{
    st7789_cfg_t st7789_config = 
    {
        .bl = GPIO_NUM_35,
        .clk = GPIO_NUM_47,
        .cs = GPIO_NUM_48,
        .dc = GPIO_NUM_45,
        .rst = GPIO_NUM_20,
        .mosi = GPIO_NUM_21,
        .spi_fre = 40*1000*1000,
        .width = LCD_WIDTH,
        .height = LCD_HEIGHT,
        .spin = 0,
        .done_cb = lv_flush_done_cb, //回调函数，用于通知LVGL刷新完成
        .cb_param = &disp_drv,
    };
    st7789_driver_hw_init(&st7789_config);

}

void cst816t_hw_init(void)
{
    cst816t_cfg_t cst816_config = 
    {
        .sda = GPIO_NUM_5,
        .scl = GPIO_NUM_4,
        .fre = 300*1000,
        .x_limit = LCD_WIDTH,
        .y_limit = LCD_HEIGHT,
    };
    cst816t_init(&cst816_config);
}


void lv_timer_callback(void *arg)
{
    uint32_t tick_interval = *((uint32_t*)arg);
    lv_tick_inc(tick_interval); //增加tick计数器
}

void lv_tick_init(void)
{
    static uint32_t tick_interval = 5;  //设置tick间隔为5ms

    const esp_timer_create_args_t arg = 
    {
        .arg = &tick_interval,
        .callback = lv_timer_callback,
        .name = "",
        .dispatch_method = ESP_TIMER_TASK,  //在任务中调度回调函数
        .skip_unhandled_events = true,  //跳过未处理的事件

    };
    esp_timer_handle_t timer_handle;
    esp_timer_create(&arg, &timer_handle);
    esp_timer_start_periodic(timer_handle, 5000);  //每5ms触发一次回调函数


}

void lv_port_init(void)
{
    lv_init();  //初始化LVGL库
    st7789_hw_init(); //初始化屏幕驱动硬件接口
    cst816t_hw_init(); //初始化触摸屏驱动硬件接口
    
    lv_disp_init(); //初始化显示设备驱动
    lv_indev_init(); //初始化输入设备驱动
    lv_tick_init();  //初始化LVGL的tick计时器
}
