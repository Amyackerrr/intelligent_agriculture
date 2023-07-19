#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "iot_errno.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "iot_i2c.h"

#include "cmsis_os2.h"
#include "ohos_init.h"
#include "lwip/sockets.h"
#include "wifi_connect.h"
#include "oc_mqtt.h"

#include "nfc.h"
#include "cloud.h"
#include "E53_1.h"
#include "iic_oled.h"

#define MSGQUEUE_COUNT 16 // number of Message Queue Objects
#define MSGQUEUE_SIZE 10
#define CLOUD_TASK_STACK_SIZE (1024 * 10)
#define CLOUD_TASK_PRIO 24
#define SENSOR_TASK_STACK_SIZE (1024 * 2)
#define SENSOR_TASK_PRIO 25
#define NFC_TASK_STACK_SIZE (1024 * 8)
#define NFC_TASK_PRIO 25
#define TASK_STACK_SIZE (1024 * 16)
#define TASK_PRIO 25

#define TASK_DELAY_1S 1
#define TASK_DELAY_3S 3
#define MIN_LUX 20
#define MIN_LUX_2 20
#define MAX_HUM 70
#define MIN_HUM 50
#define MAX_TEMP 35
#define BUTTON_F1_GPIO 11
#define BUTTON_F2_GPIO 12

#define CLIENT_ID "64856e7dec46a256bca60f0d_PF3VYHHK_0_0_2023061108"
#define USERNAME "64856e7dec46a256bca60f0d_PF3VYHHK"
#define PASSWORD "1d64b8a158a1ebe865642eb5b2695e0072cdc0fc73de8db30c88493666b177a9"
osMessageQueueId_t mid_MsgQueue; // message queue id

bool Enable=0;
typedef struct {
    int LED1;
    int LED2;
    int MOTOR;
    int Hum;
}device;


static void F1Pressed(char *arg)
{
    (void)arg;
    Enable=1;
}

static void F2Pressed(char *arg)
{
    (void)arg;
    Enable=0;
}

static void Voice1_ON(char *arg)
{
    (void)arg;
    while(1){
        BigLightStatusSet(ON);
        usleep(10000);
        if (IoTGpioGetInputVal(LED_1_GPIO,0))
            break;
    }
}

static void Voice2_ON(char *arg)
{
    (void)arg;
    while(1){
        LightStatusSet(ON);
        usleep(10000);
        if (IoTGpioGetInputVal(LED_2_GPIO,0))
            break;
    }
}

static void Voice3_ON(char *arg)
{
    (void)arg;
    while(1){
        MotorStatusSet(ON);
        usleep(10000);
        if (IoTGpioGetInputVal(MOTOR_GPIO,0))
            break;
    }
}

static void Interrupt_set(void)
{
    // init gpio of F1 key and set it as the falling edge to trigger interrupt
    IoTGpioInit(BUTTON_F1_GPIO);
    IoTGpioSetFunc(BUTTON_F1_GPIO, IOT_GPIO_FUNC_GPIO_11_GPIO);
    IoTGpioSetDir(BUTTON_F1_GPIO, IOT_GPIO_DIR_IN);
    IoTGpioSetPull(BUTTON_F1_GPIO, IOT_GPIO_PULL_UP);
    IoTGpioRegisterIsrFunc(BUTTON_F1_GPIO, IOT_INT_TYPE_EDGE, IOT_GPIO_EDGE_FALL_LEVEL_LOW, F1Pressed, NULL);

    // init gpio of F2 key and set it as the falling edge to trigger interrupt
    IoTGpioInit(BUTTON_F2_GPIO);
    IoTGpioSetFunc(BUTTON_F2_GPIO, IOT_GPIO_FUNC_GPIO_12_GPIO);
    IoTGpioSetDir(BUTTON_F2_GPIO, IOT_GPIO_DIR_IN);
    IoTGpioSetPull(BUTTON_F2_GPIO, IOT_GPIO_PULL_UP);
    IoTGpioRegisterIsrFunc(BUTTON_F2_GPIO, IOT_INT_TYPE_EDGE, IOT_GPIO_EDGE_FALL_LEVEL_LOW, F2Pressed, NULL);

    // init gpio of LED1 and set it as the falling edge to trigger interrupt
    IoTGpioInit(LED_1_GPIO);
    IoTGpioSetFunc(LED_1_GPIO, IOT_GPIO_FUNC_GPIO_3_GPIO);
    IoTGpioSetDir(LED_1_GPIO, IOT_GPIO_DIR_IN);
    IoTGpioSetPull(LED_1_GPIO, IOT_GPIO_PULL_DOWN);
    IoTGpioRegisterIsrFunc(LED_1_GPIO, IOT_INT_TYPE_LEVEL, IOT_GPIO_VALUE1, Voice1_ON, NULL);

    // init gpio of LED2 and set it as the falling edge to trigger interrupt
    IoTGpioInit(LED_2_GPIO);
    IoTGpioSetFunc(LED_2_GPIO, IOT_GPIO_FUNC_GPIO_4_GPIO);
    IoTGpioSetDir(LED_2_GPIO, IOT_GPIO_DIR_IN);
    IoTGpioSetPull(LED_2_GPIO, IOT_GPIO_PULL_DOWN);
    IoTGpioRegisterIsrFunc(LED_2_GPIO, IOT_INT_TYPE_LEVEL, IOT_GPIO_VALUE1, Voice2_ON, NULL);

    // init gpio of motor and set it as the falling edge to trigger interrupt
    IoTGpioInit(MOTOR_GPIO);
    IoTGpioSetFunc(MOTOR_GPIO, IOT_GPIO_FUNC_GPIO_10_GPIO);
    IoTGpioSetDir(MOTOR_GPIO, IOT_GPIO_DIR_IN);
    IoTGpioSetPull(MOTOR_GPIO, IOT_GPIO_PULL_DOWN);
    IoTGpioRegisterIsrFunc(MOTOR_GPIO, IOT_INT_TYPE_LEVEL, IOT_GPIO_VALUE1, Voice3_ON, NULL);
}

static void MsgRcvCallback(uint8_t *recv_data, size_t recv_size, uint8_t **resp_data, size_t *resp_size)
{
    app_msg_t *app_msg;

    int ret = 0;
    app_msg = malloc(sizeof(app_msg_t));
    app_msg->msg_type = en_msg_cmd;
    app_msg->msg.cmd.payload = (char *)recv_data;

    printf("recv data is %s\n", recv_data);
    ret = osMessageQueuePut(mid_MsgQueue, &app_msg, 0U, 0U);
    if (ret != 0) {
        free(recv_data);
    }
    *resp_data = NULL;
    *resp_size = 0;
}

// static void Judge_and_Show(void)
// {
//     if (IoTGpioGetOutputVal(E53_IA1_LIGHT_GPIO,0))
//         OLED_ShowString(0, 0, (uint8_t *)"LED1off", 8);
//     else
//         OLED_ShowString(0, 0, (uint8_t *)"LED1on ", 8);

//     if (IoTGpioGetOutputVal(E53_SC1_LIGHT_GPIO,0))
//         OLED_ShowString(64, 0, (uint8_t *)"LED2off", 8);
//     else
//         OLED_ShowString(64, 0, (uint8_t *)"LED2on ", 8);

//     if (IoTGpioGetOutputVal(E53_IA1_MOTOR_GPIO,0))
//         OLED_ShowString(0, 1, (uint8_t *)"Mt off", 8);
//     else
//         OLED_ShowString(0, 1, (uint8_t *)"Mt on ", 8);

//     if (IoTGpioGetOutputVal(HUMIDIFIER_GPIO,0))
//         OLED_ShowString(64, 1, (uint8_t *)"Hum+ off", 8);
//     else
//         OLED_ShowString(64, 1, (uint8_t *)"Hum+ on ", 8);
// }

static void ExampleTask(void)
{
    int ret;
    E53IA1Data data;

    ret = E53IA1Init();
    if (ret != 0) {
        printf("E53_IA1 Init failed!\r\n");
        return;
    }
    ret = E53SC1Init();
    if (ret != 0) {
        printf("E53_SC1 Init failed!\r\n");
        return;
    }

    IoTGpioInit(HUMIDIFIER_GPIO);
    IoTGpioSetFunc(HUMIDIFIER_GPIO, 0);
    IoTGpioSetDir(HUMIDIFIER_GPIO, IOT_GPIO_DIR_OUT);

    //init oled
    oled_init();   
    printf("OLED Init success\n");

    Interrupt_set();

    while (1) {
        printf("\r\n=======================================\r\n");
        printf("\r\n*************E53_IA1_example***********\r\n");
        printf("\r\n=======================================\r\n");

        ret = E53IA1ReadData(&data);
        if (ret != 0) {
            printf("E53_IA1 Read Data failed!\r\n");
            return;
        }
        printf("\r\n******************************Lux Value is  %.2f\r\n", data.Lux);
        printf("\r\n******************************Humidity is  %.2f\r\n", data.Humidity);
        printf("\r\n******************************Temperature is  %.2f\r\n", data.Temperature);
        OLED_ShowString(5, 2, (uint8_t *)"Lux Value:", 16);OLED_ShowFloat(5+8*10, 2, data.Lux, 16);
        OLED_ShowString(5, 4, (uint8_t *)"Humidity:", 16);OLED_ShowFloat(5+8*9, 4, data.Humidity, 16);
        OLED_ShowString(0, 6, (uint8_t *)"Temperature:", 16);OLED_ShowFloat(8*12, 6, data.Temperature, 16);
        if ((int)data.Lux < MIN_LUX) {
            LightStatusSet(ON);
        } else {
            LightStatusSet(OFF);
        }//补光灯

        if (((int)data.Humidity > MAX_HUM) | ((int)data.Temperature > MAX_TEMP)) {
            MotorStatusSet(ON);
        } else {
            MotorStatusSet(OFF);
        }//风扇电机

        if (((int)data.Humidity < MIN_HUM)) {
            HumaddStatusSet(ON);
        } else {
            HumaddStatusSet(OFF);
        }//加湿器

        if (((int)data.Lux < MIN_LUX_2 && Enable==1)) {
            BigLightStatusSet(ON);
        } 
        else {
            BigLightStatusSet(OFF);
        }//门灯

        sleep(TASK_DELAY_1S);
    }
}

static void NFC_Task(void)
{
    uint8_t ret;

    // GPIO_0 multiplexed to I2C1_SDA
    IoTGpioInit(0);
    IoTGpioSetFunc(0, WIFI_IOT_IO_FUNC_GPIO_0_I2C1_SDA);

    // GPIO_1 multiplexed to I2C1_SCL
    IoTGpioInit(1);
    IoTGpioSetFunc(1, WIFI_IOT_IO_FUNC_GPIO_1_I2C1_SCL);

    // baudrate: 400kbps
    IoTI2cInit(WIFI_IOT_I2C_IDX_1, WIFI_IOT_I2C_BAUDRATE);

    printf("I2C Test Start\n");

    ret = storeUrihttp(NDEFFirstPos, (uint8_t *)WEB);
    if (ret != 1) {
        printf("NFC Write Data Falied :%d \n", ret);
    }
    while (1) {
        printf("=======================================\n");
        printf("***********I2C_NFC_example**********\n");
        printf("=======================================\n");
        printf("Please use the mobile phone with NFC function close to the development board!\n");
        sleep(TASK_DELAY_1S);
    }
}

static int CloudMainTaskEntry(void)
{
    app_msg_t *app_msg;

    uint32_t ret = WifiConnect("华为c", "cwx54201");

    device_info_init(CLIENT_ID, USERNAME, PASSWORD);
    oc_mqtt_init();
    oc_set_cmd_rsp_cb(MsgRcvCallback);

    while (1) {
        app_msg = NULL;
        (void)osMessageQueueGet(mid_MsgQueue, (void **)&app_msg, NULL, 0U);
        if (app_msg != NULL) {
            switch (app_msg->msg_type) {
                case en_msg_cmd:
                    DealCmdMsg(&app_msg->msg.cmd);
                    break;
                case en_msg_report:
                    ReportMsg(&app_msg->msg.report);
                    break;
                default:
                    break;
            }
            free(app_msg);
        }
    }
    return 0;
}

static int SensorTaskEntry(void)
{
    int ret;
    app_msg_t *app_msg;
    E53IA1Data data;
    ret = E53IA1Init();
    if (ret != 0) {
        printf("E53_IA1 Init failed!\r\n");
        return;
    }
    while (1) {
        ret = E53IA1ReadData(&data);
        if (ret != 0) {
            printf("E53_IA1 Read Data failed!\r\n");
            return;
        }
        app_msg = malloc(sizeof(app_msg_t));
        printf("SENSOR:lum:%.2f temp:%.2f hum:%.2f\r\n", data.Lux, data.Temperature, data.Humidity);
        if (app_msg != NULL) {
            app_msg->msg_type = en_msg_report;
            app_msg->msg.report.hum = (int)data.Humidity;
            app_msg->msg.report.lum = (int)data.Lux;
            app_msg->msg.report.temp = (int)data.Temperature;
            if (osMessageQueuePut(mid_MsgQueue, &app_msg, 0U, 0U) != 0) {
                free(app_msg);
            }
        }
        sleep(TASK_DELAY_3S);
    }
    return 0;
}




static void IotMainTaskEntry(void)
{
    mid_MsgQueue = osMessageQueueNew(MSGQUEUE_COUNT, MSGQUEUE_SIZE, NULL);
    if (mid_MsgQueue == NULL) {
        printf("Failed to create Message Queue!\n");
    }

    osThreadAttr_t attr;
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;

    attr.name = "CloudMainTaskEntry";
    attr.stack_size = CLOUD_TASK_STACK_SIZE;
    attr.priority = CLOUD_TASK_PRIO;
    if (osThreadNew((osThreadFunc_t)CloudMainTaskEntry, NULL, &attr) == NULL) {
        printf("Failed to create CloudMainTaskEntry!\n");
    }
    attr.stack_size = SENSOR_TASK_STACK_SIZE;
    attr.priority = SENSOR_TASK_PRIO;
    attr.name = "SensorTaskEntry";
    if (osThreadNew((osThreadFunc_t)SensorTaskEntry, NULL, &attr) == NULL) {
        printf("Failed to create SensorTaskEntry!\n");
    }

    attr.stack_size = NFC_TASK_STACK_SIZE;
    attr.priority = NFC_TASK_PRIO;
    attr.name = "NFC_Task";
    if (osThreadNew((osThreadFunc_t)NFC_Task, NULL, &attr) == NULL) {
        printf("Falied to create NFC_Task!\n");
    }
    attr.stack_size = TASK_STACK_SIZE;
    attr.priority = TASK_PRIO;
    attr.name = "ExampleTask";
    if (osThreadNew((osThreadFunc_t)ExampleTask, NULL, &attr) == NULL) {
        printf("Failed to create ExampleTask!\n");
    }
}

APP_FEATURE_INIT(IotMainTaskEntry);