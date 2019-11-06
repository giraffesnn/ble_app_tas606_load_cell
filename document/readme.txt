0. 程序烧写
0.1 烧写SoftDevice蓝牙协议栈
    SoftDevice协议栈不开源, 只有hex文件. 通过nRFgo Studio工具
烧写SoftDevice.

Programming SoftDevices:
https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v12.0.0%2Fgetting_started_softdevice.html

0.1 通过SEGGER Embedded Studio for ARM烧写Application程序.

1. 长按开机, 长按关机

2. 电源指示灯为红绿双色灯:
(1) 电池电量高于90%, 电源指示灯变为绿色, 常亮;
(2) 电池电量低于5% , 电源指示灯变为红色, 闪烁;
(3) 电池电量高于5%且低于90%时, 电源指示灯变为红色, 常亮;
3. 蓝牙模块

(1) 蓝牙模块处于广播状态, 蓝牙指示灯闪烁;
(2) 蓝牙模块处于连接状态, 蓝牙指示灯常亮;
(3) 蓝牙广播间隔设置为500ms;
(4) TAS606 Load Cell模块为从设备, 需要树莓派或者手机搜索连接;

4. TAS606 Load Cell
(1) 设备开机后, 需要先使用树莓派或者手机建立蓝牙连接;
(2) 单击按键, 传感器指示灯点亮, 设备后端开始采集数据;
(3) 压力数据稳定后, 传感器指示灯熄灭, 设备会给树莓派或者手机发送通知, 请求更新数据;
(4) TAS606 Load Cell和ADS1231平时处于关断状态, 单击按键时, 才使能供电, 数据稳定后, 关断电源;

5. 功耗
(1) 广播状态: 蓝牙发送广播包(6mA, 时间很短), 不发送广播包(500 ~ 800uA)
(2) 连接状态: 不收发数据时(500 ~ 800uA), 点击按键, 设备后端采集发送压力数据时(10 ~ 30mA, 时间很短)