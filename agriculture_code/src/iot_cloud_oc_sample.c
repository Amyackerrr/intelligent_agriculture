#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>

#include <unistd.h>
#include "cmsis_os2.h"
#include "ohos_init.h"
#include "oc_mqtt.h"

#include "E53_1.h"
#include "cloud.h"
#include "wifi_connect.h"

#define MSGQUEUE_COUNT 16
#define MSGQUEUE_SIZE 10
#define CLOUD_TASK_STACK_SIZE (1024 * 10)
#define CLOUD_TASK_PRIO 24
#define SENSOR_TASK_STACK_SIZE (1024 * 4)
#define SENSOR_TASK_PRIO 25
#define TASK_DELAY 3

#define CLIENT_ID "64856e7dec46a256bca60f0d_PF3VYHHK_0_0_2023061108"
#define USERNAME "64856e7dec46a256bca60f0d_PF3VYHHK"
#define PASSWORD "1d64b8a158a1ebe865642eb5b2695e0072cdc0fc73de8db30c88493666b177a9"

MSGQUEUE_OBJ_t msg;
app_cb_t g_app_cb;

int flag1;
int flag2;
int flag3;


void ReportMsg(report_t *report)
{
    oc_mqtt_profile_service_t service;
    oc_mqtt_profile_kv_t temperature;
    oc_mqtt_profile_kv_t humidity;
    oc_mqtt_profile_kv_t luminance;
    oc_mqtt_profile_kv_t led;
    oc_mqtt_profile_kv_t motor;

    service.event_time = NULL;
    service.service_id = "Agriculture";
    service.service_property = &temperature;
    service.nxt = NULL;

    temperature.key = "Temperature";
    temperature.value = &report->temp;
    temperature.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    temperature.nxt = &humidity;

    humidity.key = "Humidity";
    humidity.value = &report->hum;
    humidity.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    humidity.nxt = &luminance;

    luminance.key = "Luminance";
    luminance.value = &report->lum;
    luminance.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    luminance.nxt = &led;

    led.key = "LightStatus";
    led.value = g_app_cb.led ? "ON" : "OFF";
    led.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    led.nxt = &motor;

    motor.key = "MotorStatus";
    motor.value = g_app_cb.motor ? "ON" : "OFF";
    motor.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    motor.nxt = NULL;

    oc_mqtt_profile_propertyreport(USERNAME, &service);
    return;
}

void oc_cmdresp(cmd_t *cmd, int cmdret)
{
    oc_mqtt_profile_cmdresp_t cmdresp;
    ///< do the response
    cmdresp.paras = NULL;
    cmdresp.request_id = cmd->request_id;
    cmdresp.ret_code = cmdret;
    cmdresp.ret_name = NULL;
    (void)oc_mqtt_profile_cmdresp(NULL, &cmdresp);
}
///< COMMAND DEAL

void DealCmdMsg(cmd_t *cmd)
{
    cJSON *obj_root, *obj_cmdname, *obj_paras, *obj_para;

    int cmdret = 1;

    obj_root = cJSON_Parse(cmd->payload);
    if (obj_root == NULL) {
        oc_cmdresp(cmd, cmdret);
    }

    obj_cmdname = cJSON_GetObjectItem(obj_root, "command_name");
    if (obj_cmdname == NULL) {
        cJSON_Delete(obj_root);
    }
    if (strcmp(cJSON_GetStringValue(obj_cmdname), "Agriculture_Control_light") == 0) {
        obj_paras = cJSON_GetObjectItem(obj_root, "paras");
        if (obj_paras == NULL) {
            cJSON_Delete(obj_root);
        }
        obj_para = cJSON_GetObjectItem(obj_paras, "Light");
        if (obj_para == NULL) {
            cJSON_Delete(obj_root);
        }
        ///< operate the LED here
        if (strcmp(cJSON_GetStringValue(obj_para), "ON") == 0) {
            flag1=1;
            g_app_cb.led = 1;
            while(flag1){
                LightStatusSet(ON);

            }
            printf("Light On!");
        } else {
            flag1=0;
            g_app_cb.led = 0;
            LightStatusSet(OFF);
            printf("Light Off!");
        }
        cmdret = 0;
    } else if (strcmp(cJSON_GetStringValue(obj_cmdname), "Agriculture_Control_Motor") == 0) {
        obj_paras = cJSON_GetObjectItem(obj_root, "Paras");
        if (obj_paras == NULL) {
            cJSON_Delete(obj_root);
        }
        obj_para = cJSON_GetObjectItem(obj_paras, "Motor");
        if (obj_para == NULL) {
            cJSON_Delete(obj_root);
        }
        ///< operate the Motor here
        if (strcmp(cJSON_GetStringValue(obj_para), "ON") == 0) {
            flag2=1;
            g_app_cb.motor = 1;
            while(flag2){
                MotorStatusSet(ON);
            }
            printf("Motor On!");
        } else {
            flag2=0;
            g_app_cb.motor = 0;
            MotorStatusSet(OFF);
            printf("Motor Off!");
        }
        cmdret = 0;
    } else if (strcmp(cJSON_GetStringValue(obj_cmdname), "Agriculture_Control_Hum") == 0) {
        obj_paras = cJSON_GetObjectItem(obj_root, "Paras");
        if (obj_paras == NULL) {
            cJSON_Delete(obj_root);
        }
        obj_para = cJSON_GetObjectItem(obj_paras, "Hum");
        if (obj_para == NULL) {
            cJSON_Delete(obj_root);
        }
        ///< operate the Motor here
        if (strcmp(cJSON_GetStringValue(obj_para), "ON") == 0) {
            flag3=1;
            g_app_cb.motor = 1;
            while(flag2){
                HumaddStatusSet(ON);
            }
            printf("Hum+ On!");
        } else {
            flag3=0;
            g_app_cb.motor = 0;
            HumaddStatusSet(OFF);
            printf("Hum+ Off!");
        }
        cmdret = 0;
    }

    cJSON_Delete(obj_root);
}
