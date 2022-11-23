/**
 * @file TCP.h
 * @author yewan
 * @brief 用于设置TCP服务器
 * @version 1.0.0
 * @date 2022-11-23
 *
 * @copyright North China University of Science and Technology
 *            Embedded Systems and Internet of Things Applications Lab
 *
 */

#ifndef __TCP_H
#define __TCP_H

#include "Arduino.h"
#include <ESP8266WiFi.h>

/*==============================================
    TCP
==============================================*/

/* WiFi STA */
typedef struct{
    const char* ssid;
    const char* password;
}WiFi_STA_Infor;

/*==============================================
    指令
==============================================*/

#define NO_STATE            (0)             // 无目标

// TCP状态机
typedef void (*TCP_StateMachine_Act)(void);
typedef struct{
    String instruction;         // 指令语句
    uint32_t state;             // 状态
    TCP_StateMachine_Act ack;   // 动作
}TCP_StateMachine;

/*==============================================
    函数声明
==============================================*/

/**
 * @fn WiFi_STA_Init(WiFi_STA_Infor* STA, uint32_t Serial_Boundrate)
 * @brief 初始化STA功能
 *
 * @param [STA] WiFi_STA_Infor*
 */
void WiFi_STA_Init(WiFi_STA_Infor* STA);

/**
 * @fn TCP_Init(WiFiServer* server)
 * @brief 初始化TCP服务器功能
 *
 * @param [server] WiFiServer*
 */
void TCP_Init(WiFiServer* server);

/**
 * @fn void distribute_Seat(
 *         WiFiServer* server, WiFiClient* serverClients,
 *         WiFiClient* serverClient, uint8_t Max_TCPservice_Clients_Num
 *     )
 * @brief 为客户端分配位置，以处理客户端任务
 *
 * @param [server] WiFiServer*
 * @param [serverClients] WiFiClient(*)[]
 * @param [serverClient] WiFiClient*
 * @param [Max_TCPservice_Clients_Num] uint8_t 最大客户端扫描数
 */
void distribute_Seat(
    WiFiServer* server, WiFiClient* serverClients,
    WiFiClient* serverClient, uint8_t Max_TCPservice_Clients_Num
);

/**
 * @fn void receive_Request(
 *         WiFiClient* serverClients, uint32_t* TCP_Act,
 *         uint8_t Max_TCPservice_Clients_Num,
 *         uint32_t (*Data_Handle)(String)
 *     )
 * @brief 接收并处理客户端请求
 *
 * @param [serverClients] WiFiClient(*)[]
 * @param [TCP_Act] uint32_t(*)[] 客户请求列表
 * @param [Max_TCPservice_Clients_Num] uint8_t 最大客户端扫描数
 * @param [Data_Handle] 请求处理回调函数，由用户编写
 */
void receive_Request(
    WiFiClient* serverClients, uint32_t* TCP_Act,
    uint8_t Max_TCPservice_Clients_Num,
    uint32_t (*Data_Handle)(String)
);

/**
 * @fn void Polling_ClientInstruction(
 *         uint32_t* TCP_Act, uint8_t Max_TCPservice_Clients_Num,
 *         const TCP_StateMachine* tab
 *     )
 * @brief 轮询并执行用户请求，执行完毕后释放请求
 *
 * @param [TCP_Act] uint32_t(*)[] 客户请求列表
 * @param [Max_TCPservice_Clients_Num] uint8_t 最大客户端扫描数
 * @param [tab] const TCP_StateMachine(*)[] 请求状态机
 */
void Polling_ClientInstruction(
    uint32_t* TCP_Act, uint8_t Max_TCPservice_Clients_Num,
    const TCP_StateMachine* tab
);

#endif /* __TCP_H */
