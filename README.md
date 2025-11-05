基于esp32s3n16r8主控，移植lvgl嵌入式图形显示库，并驱动spi屏幕
屏幕参数：240*280
显示芯片：st7789
触摸芯片：cst816t
引脚连接：
屏幕触摸部分
INT -> PIN7
TRST -> PIN6
SDA -> PIN5
SCL -> PIN4
屏幕显示部分
RST -> PIN20
MOSI ->PIN21 
SCLK -> PIN47
CS -> PIN48
DC -> PIN45
BLK -> PIN0

