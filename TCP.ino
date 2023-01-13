/**
 * @file TCP.ino
 * @author yewan
 * @brief 测试TCP功能
 * @version 1.0.0
 * @date 2022-11-23
 *
 * @copyright North China University of Science and Technology
 *            Embedded Systems and Internet of Things Applications Lab
 *
 */

#include "config.h"

#define SERIAL_BOUNDRATE        (115200)

/*==============================================
    WiFi
==============================================*/

#define STASSID             "your ssid"
#define STAPSK              "your password"

#define APSSID              "ESP666"
#define APPSK               "ESP8266_WiFi"

/* WiFi AP */
typedef struct{
    const char* ssid;
    const char* password;
}WiFi_AP_Infor;

/*==============================================
    TCP
==============================================*/

#define SERVICE_PORT                            (8266)
#define MAX_TCPSERVICE_CLIENTS_NUM              (10)         // 可连接的客户端数量最大值

typedef enum{
    START_LED_PWM = 1,          // 开始执行PWM控制的LED任务
    STOP_LED_PWM,               // 停止执行PWM控制的LED任务
    DIST_LED_PWM,               // 熄灭PWM控制的LED
}TCP_Instructions;

/*==============================================
    函数声明
==============================================*/

void WiFi_AP_Init(WiFi_AP_Infor* AP);

void start_LedPwm(void);
void stop_LedPwm(void);
void dist_LedPwm(void);
void set_LedPwmDuty(uint16_t newPwmDuty);
uint32_t Data_Handle(String str);

/*==============================================
    全局变量
==============================================*/

// LED
led_GPIOStruct __led = {
    .pin = LED_PIN,
    .state = LED_OFF,
    .GPIOmode = OUTPUT
};

// LED PWM
bool startPWM = false;
ledPwmDutyDir led = {
    .pwmFreq = LED_PWM_FREQ,
    .pwmStep = LED_PWM_STEP,
    .pin = LED_PIN,
    .GPIOmode = OUTPUT,
    .dir = 0,
    .pwmDuty = LED_PWM_DUTY_DIST
};

// WiFi STA
WiFi_STA_Infor STA = {
    .ssid = STASSID,
    .password = STAPSK
};

// WiFi AP
WiFi_AP_Infor AP = {
    .ssid = APSSID,
    .password = APPSK
};

// TCP
WiFiServer server(SERVICE_PORT);                            // 绑定服务器端口号
WiFiClient serverClient;
WiFiClient serverClients[MAX_TCPSERVICE_CLIENTS_NUM];       // 记录最大数量的客户端
/* 状态列表 */
const TCP_StateMachine TCP_State_Tab[] = {
    { "start LED PWM",      START_LED_PWM,      start_LedPwm },         // 开始执行PWM控制的LED任务
    { "stop LED PWM",       STOP_LED_PWM,       stop_LedPwm },          // 停止执行PWM控制的LED任务
    { "dist LED PWM",       DIST_LED_PWM,       dist_LedPwm },          // 熄灭PWM控制的LED
    { "",                   NULL,               NULL }                  // 标记状态机列表末尾
};

/*==============================================
    启动程序
==============================================*/

void setup() {
    Serial.begin(SERIAL_BOUNDRATE);
    ledPwmInit(&led);
    WiFi_STA_Init(&STA);
    TCP_Init(&server);
}

void loop() {
    uint8_t i;
    String rec_str = "";
    uint32_t TCP_Act[MAX_TCPSERVICE_CLIENTS_NUM];

    while(true) {
        distribute_Seat(&server, (WiFiClient*)&serverClients, &serverClient, MAX_TCPSERVICE_CLIENTS_NUM);
        receive_Request((WiFiClient*)&serverClients, (uint32_t*)&TCP_Act, MAX_TCPSERVICE_CLIENTS_NUM, Data_Handle);
        Polling_ClientInstruction((uint32_t*)&TCP_Act, MAX_TCPSERVICE_CLIENTS_NUM, (TCP_StateMachine*)TCP_State_Tab);

        if(startPWM) {
            writeLedPwmDuty(&led);
            delay(LED_PWM_FLASH_STEP);
        }
    }
}

/*==============================================
    协作程序
==============================================*/

void WiFi_AP_Init(WiFi_AP_Infor* AP)
{
    delay(1000);
    Serial.begin(SERIAL_BOUNDRATE);
    Serial.println();
    Serial.println("============================");
    Serial.println(" Configuring access point...");
    Serial.println("============================");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP->ssid, AP->password);

    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
}

/**
 * @fn uint32_t Data_Handle(String str)
 * @brief 处理客户端请求
 *
 * @param [str] 待处理的字符串
 * @return uint32_t 解码后的客户端请求
 */
uint32_t Data_Handle(String str)
{
    TCP_StateMachine* machine;

    Serial.println("开始数据处理");
    for(machine = (TCP_StateMachine*)TCP_State_Tab; machine->ack != NULL; machine++) {
        if(str == machine->instruction) return machine->state;
    }

    return NO_STATE;
}

/* 启动模拟(PEM)LED */
void start_LedPwm(void)
{
    startPWM = true;
}

/* 停止模拟(PEM)LED */
void stop_LedPwm(void)
{
    startPWM = false;
}

/* 熄灭模拟(PEM)LED */
void dist_LedPwm(void)
{
    led.pwmDuty = LED_PWM_DUTY_DIST;
    writeLedPwmDuty(&led);
}

/* 设置模拟(PEM)LED占空比 */
void set_LedPwmDuty(uint16_t newPwmDuty)
{
    led.pwmDuty = newPwmDuty;
    writeLedPwmDuty(&led);
}
