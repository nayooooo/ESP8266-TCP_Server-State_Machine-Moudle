/**
 * @file TCP.cpp
 * @author yewan
 * @brief 用于设置TCP服务器
 * @version 1.0.0
 * @date 2022-11-23
 *
 * @copyright North China University of Science and Technology
 *            Embedded Systems and Internet of Things Applications Lab
 *
 */

#include "Arduino.h"
#include "TCP.h"

/*==============================================
    控制函数
==============================================*/

void WiFi_STA_Init(WiFi_STA_Infor* STA)
{
    uint8_t errTimes = 0;

    delay(1000);
    Serial.println();
    Serial.println("============================");
    Serial.println("     Connecting WiFi...     ");
    Serial.println("============================");
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to ");
    Serial.println(STA->ssid);

    WiFi.begin(STA->ssid, STA->password);

    while(WiFi.status() != WL_CONNECTED && errTimes < 51) {
        errTimes++;
        delay(200);
        Serial.print(".");
    }
    if(errTimes >= 51) {  // 10s内未连接成功
        Serial.println();
        Serial.println("============================");
        Serial.println("       WiFi disconnect!     ");
        Serial.println("============================");
        while(true);  // 连接失败，阻塞
    }
    else {
        Serial.println();
        Serial.println("============================");
        Serial.println("       WiFi connected!      ");
        Serial.println("============================");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }
}

void TCP_Init(WiFiServer* server)
{
    server->begin();                  // 启动服务器
    server->setNoDelay(true);         // 禁用Nagle算法
    Serial.println();
    Serial.println("============================");
    Serial.println("       TCP Configured!      ");
    Serial.println("============================");
    Serial.println("等待客户端接入......");
}

void distribute_Seat(
    WiFiServer* server, WiFiClient* serverClients,
    WiFiClient* serverClient, uint8_t Max_TCPservice_Clients_Num
)
{
    uint8_t i;

    if(server->hasClient()) {  // 有客户端接入
        Serial.println("客户端接入");
        for(i = 0; i < Max_TCPservice_Clients_Num; i++) {
        if((!serverClients[i]) || (!serverClients[i].connected())) {  // 有空闲客户端
            if(serverClients[i]) serverClients[i].stop();
            serverClients[i] = server->available();
            serverClients[i].print("Connected!");  // 客户端连接成功提示
        }
        }
    }
    else {  // 无客户端接入
        *serverClient = server->available();
        serverClient->stop();
    }
}

void receive_Request(
    WiFiClient* serverClients, uint32_t* TCP_Act,
    uint8_t Max_TCPservice_Clients_Num,
    uint32_t (*Data_Handle)(String)
)
{
    uint8_t i;
    String rec_str = "";

    for(i = 0; i < Max_TCPservice_Clients_Num; i++) {
        TCP_Act[i] = NO_STATE;
        if(serverClients[i] && serverClients[i].connected()) {
            if(serverClients[i].available()) {
                rec_str = "";
                while(serverClients[i].available()) {
                    rec_str += char(serverClients[i].read());
                }
                Serial.print("接收到客户端的数据：");
                Serial.println(rec_str);
                serverClients[i].print(rec_str);  // 回传数据，以校验数据完整性

                if(rec_str == "Disconnect!") {  // 客户端发起“断开连接”
                    serverClients[i].stop();
                    Serial.println("客户端断开！");
                }
                else  // 处理客户数据
                    TCP_Act[i] = Data_Handle(rec_str);
                rec_str = "";
            }
        }
    }
}

void Polling_ClientInstruction(
    uint32_t* TCP_Act, uint8_t Max_TCPservice_Clients_Num,
    const TCP_StateMachine* tab
)
{
    uint8_t i;
    TCP_StateMachine* machine;

    for(i = 0; i < Max_TCPservice_Clients_Num; i++) {
        for(machine = (TCP_StateMachine*)tab; machine->ack != NULL; machine++) {
            if(TCP_Act[i] == machine->state) {
                machine->ack();
                TCP_Act[i] = NO_STATE;
            }
        }
    }
}
