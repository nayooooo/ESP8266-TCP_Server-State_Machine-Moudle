# 用于测试TCP的Demo #

一个使用到了回调函数和状态机的TCP服务器模板。

## 想法的由来 ##

近期正在学习ESP系列的单片机，接触到WiFi时，脑袋里面冒出来一个想法，能不能自己做一个能够手动地、远程地调节LED状态的小夜灯，说干就干。

首先，ESP8266可以使用WiFi进行远程通信，我们可以连接WiFi，让ESP充当STA，然后以ESP建立TCP服务器，之后只需要手机连接TCP就行了。

## 设计思路 ##

### STA模式初始化 ###

STA的初始化很简单，我们只需要按照乐鑫官方提供的资料写就行了，下面是此demo中的STA初始化函数，值得注意的是，调用该函数之前需初始化串口。

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

大体步骤是

    选用`WIFI_STA`模式-->连接WiFi-->连接成功

如果连接WiFi失败，则阻塞进程，我没有在此处添加提示信息，可子兴添加。

### 建立TCP服务器 ###

当连接WiFi成功后，便可建立TCP服务器了，在此，我们只需要调用 `void TCP_Init(WiFiServer* server)` 函数就能建立好TCP服务器了。

    server->begin();                  // 启动服务器
    server->setNoDelay(true);         // 禁用Nagle算法

首先启动TCP服务器，然后禁用Nagle算法就行了。

### 设计TCP状态机 ###

TCP的状态机是这个demo的核心点，我在这里使用状态机是为了使整个例程更具可读性，当前(2022-11-23)的状态机如下：

    const TCP_StateMachine TCP_State_Tab[] = {
        { "start LED PWM",      START_LED_PWM,      start_LedPwm },         // 开始执行PWM控制的LED任务
        { "stop LED PWM",       STOP_LED_PWM,       stop_LedPwm },          // 停止执行PWM控制的LED任务
        { "dist LED PWM",       DIST_LED_PWM,       dist_LedPwm },          // 熄灭PWM控制的LED
        { "",                   NULL,               NULL }                  // 标记状态机列表末尾
    };

它拥有三个功能，分别是 `启动PWM-LED` `停止PWM-LED` `熄灭PWM-LED` ，要了解这三个功能首先需要知道状态机中的三个参数分别是什么，我们看到 `TCP_StateMachine` 的定义如下：

    typedef void (*TCP_StateMachine_Act)(void);
    typedef struct{
        String instruction;         // 指令语句
        uint32_t state;             // 状态
        TCP_StateMachine_Act ack;   // 动作
    }TCP_StateMachine;

> 第一个参数 `instruction` 表示客户端发送的指令语句；
>
> 第二个参数 `state` 表示该语句对应的状态；
>
> 第三个参数 `ack` 表示该状态需要执行的动作，通过回调函数实现。

现在，我们再看到状态机列表中的第一个状态，表示当客户端发送字符串 `start LED PWM` 时，应该对应 `启动PWM-LED` ，执行 `start_LedPwm` 动作，这个动作是一个回调函数，此处记录的是它的内存地址。

### 解码客户端数据 ###

客户端发送的数据是字符串，而状态机中需要对应到 `状态` 因此需要一个解码的过程。

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

该解码函数需要使用到客户端列表 `serverClients` 、客户端请求表 `TCP_Act`、最大客户端扫描数 `Max_TCPservice_Clients_Num`、用户编写的解码函数 `Data_Handle`，其中最重要的就是回调函数 `Data_Handle`，它需要我们自己编写，此处我提供了一个解码函数：

    uint32_t Data_Handle(String str)
    {
        TCP_StateMachine* machine;

        Serial.println("开始数据处理");
        for(machine = (TCP_StateMachine*)TCP_State_Tab; machine->ack != NULL; machine++) {
            if(str == machine->instruction) return machine->state;
        }

        return NO_STATE;
    }

它需要定义一个 `TCP_StateMachine*` 指针用于遍历状态机列表，当遍历完成时，即遍历到最后一个 `空状态机` 时，就结束遍历了。

如果客户端发送的字符串与状态机中的一个状态对应，将会直接返回该状态，否则返回空状态。

### 状态机的动作 ###

据前文可以知道，状态机的动作是一个 `typedef void (*TCP_StateMachine_Act)(void);` 类型的函数指针，由于本人能力有限，只能设计成参数为 `void` 的函数指针，而不是可变参数类型，因此有诸多不便之处，例如：若要设置PWM占空比，肯能需要使用到两次命令，或许还只能设置特定的占空比，有违初衷。

## 该模板的开发与使用 ##

### 模块简介 ###

该模板使用到了两个模块，分别是 `LED_PWM` 和 `TCP`，其中 `LWD_PWM` 只是简单地初始化LED和设置高低电平或占空比，在此不作过多介绍。

### TCP初始化 ###

`TCP` 模块设计有STA初始化和TCP服务器初始化，使用时只需先初始化串口，再依次初始化STA和TCP就行了，在烧录之前只需修改

    #define STASSID             "your ssid"
    #define STAPSK              "your password"

为自己所在地区的WiFi名称和密码，但是ESP8266无法连接5GWiFi。

### TCP状态机的编写 ###

TCP的状态机分为两部分，一部分是 `状态机列表` ，另一部分是 `客户端请求解码函数` 。

#### 状态机列表 ####

状态机列表是一个个状态组成的数组，形如：

    const TCP_StateMachine TCP_State_Tab[] = {
        { "start LED PWM",      START_LED_PWM,      start_LedPwm },         // 开始执行PWM控制的LED任务
        { "stop LED PWM",       STOP_LED_PWM,       stop_LedPwm },          // 停止执行PWM控制的LED任务
        { "dist LED PWM",       DIST_LED_PWM,       dist_LedPwm },          // 熄灭PWM控制的LED
        { "",                   NULL,               NULL }                  // 标记状态机列表末尾
    };

可仿照编写。

另外，状态机列表中的各个状态的state可以用枚举类型来表示。

    typedef enum{
        START_LED_PWM = 1,          // 开始执行PWM控制的LED任务
        STOP_LED_PWM,               // 停止执行PWM控制的LED任务
        DIST_LED_PWM,               // 熄灭PWM控制的LED
    }TCP_Instructions;

#### 客户端请求解码函数 ####

客户端请求解码函数用于将客户端发送的字符串数据解码成整形状态数据，一般来说不需要更改。

    uint32_t Data_Handle(String str)
    {
        TCP_StateMachine* machine;

        Serial.println("开始数据处理");
        for(machine = (TCP_StateMachine*)TCP_State_Tab; machine->ack != NULL; machine++) {
            if(str == machine->instruction) return machine->state;
        }

        return NO_STATE;
    }

#### 状态机中的act ####

每一个状态一般对应一个动作，由于本人能力有限，只能实现特定动作的支持，例如，此demo中支持 `void (*)(void)` 类型的动作，下面是动作举例。

    /* 启动模拟(PEM)LED */
    void start_LedPwm(void)
    {
        startPWM = true;
    }

## 目前发现的不足之处 ##

1. 当设置的最大连接数为1时，若有两个客户端连接TCP服务器，将会导致反复进入分配客户端位置的情况出现，可能原因如下：

> TCP服务器反复给两个客户端分配位置
