#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "lv_port.h"
#include "lv_demos.h"
#include "st7789_driver.h"
#include "esp_heap_caps.h"  // 用于内存监控
#include "App_Init.h"






void led_run_task(void *pvParameters)
{
    while(1) {
        uint32_t duty = 0;
        if(xQueueReceive(ledc_queue, &duty, portMAX_DELAY) == pdTRUE) {
            // printf("LEDC Queue Received Duty: %ld\n", duty);
        }

        if(xEventGroupWaitBits(ledc_event_handle, FULL_EVENT_BIT0, pdTRUE, pdFALSE, portMAX_DELAY) & FULL_EVENT_BIT0) {
            // printf("LEDC Full Event Detected: Duty Cycle is 4095\n");
            gpio_set_level(GPIO_OUTPUT_IO_0, 1); // Turn on LED
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0, 2000); // Set fade to 0
            ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT); // Start fade
            
        } 
        if(xEventGroupWaitBits(ledc_event_handle, EMPTY_EVENT_BIT1, pdTRUE, pdFALSE, portMAX_DELAY) & EMPTY_EVENT_BIT1) {
            // printf("LEDC Empty Event Detected: Duty Cycle is 0\n");
            gpio_set_level(GPIO_OUTPUT_IO_0, 0); // Turn off LED
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 4095, 2000); // Set fade to 8191
            ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT); // Start fade
            
        }
        /* 回调已在 app_main 中注册，避免在循环中重复注册 */
        vTaskDelay(pdMS_TO_TICKS(500)); // Delay for visibility
    }
    return;
}

void Lcd_Show_Task(void *pvParameters)
{
    
    while (1)
    {
        /* code */
        lv_task_handler();
        
        vTaskDelay(pdMS_TO_TICKS(50)); // 延迟一段时间以减少CPU占用率
    }
    
}

void app_main(void)
{

    app_init();
    xTaskCreatePinnedToCore(led_run_task, "led_run_task", 2048, NULL, 10, NULL, 0);
    lv_port_init();
    st7789_lcd_backlight(1);
    lv_demo_widgets();
    xTaskCreatePinnedToCore(Lcd_Show_Task, "Lcd_Show_Task", 8192, NULL, 10, NULL, 0);  //增加LVGL任务栈大小

}


