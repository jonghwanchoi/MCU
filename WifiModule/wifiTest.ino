#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <WiFiEspServer.h>
#include <WiFiEspUdp.h>

#include <SoftwareSerial.h>
#include <PubSubClient.h>

SoftwareSerial mySerial(2,3); //RX, TX -> 모듈과 아두이노 사이의 시리얼 통신 

IPAddress server(146,56,159,86); //MQTT Broker 서버 IP (라즈베리파이)
char ssid[] = "Ring_2.4G"; //"iptime_soyeon";//"Ring_2.4G"; // 접속할 WIFI 아이디 -> 여기서 WIFI는 하나의 망으로 사용(같은 와이파이망에 접속해야 기기들간 통신 가능)
char pass[] = "kosta0000"; //"sy434260";//"kosta0000"; // 접속할 WIFI 비밀번호
int status = WL_IDLE_STATUS; //모듈이 실행되고 와이파이에 연결을 시도하는 일시적인 상태를 나타냄

WiFiEspClient espClient; //esp모듈 객체 생성
PubSubClient client(espClient); //Pub client 객체와 esp모듈 바인딩? 

long lastMsg = 0;
char msg[50];
int value = 0;

void setup() 
{ 
    Serial.begin(9600); 
    mySerial.begin(9600); 
    WiFi.init(&mySerial); //시리얼 통신과 와이파이 모듈을 연결

		//모듈 상태 반환 체크 -> 접속할 모듈이 존재하지 않을때 if문 실행 
    if (WiFi.status() == WL_NO_SHIELD)
    {
        Serial.println("WiFi shield not present");
        // don't continue
        while (true);
    }

    while ( status != WL_CONNECTED) //모듈이 와이파이에 접속 되기 전까지 실행
    {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
        status = WiFi.begin(ssid, pass); 
				// 와이파이 접속에 성공하면 status 값은 WL_CONNECTED으로 갱신
    }

    Serial.println("You're connected to the network"); //와이파이 접속 성공 메세지

    //connect to MQTT server
    client.setServer(server, 1883); //연결할 MQTT Broker 서버 아이피 할당
    client.setCallback(callback); //사용할 callback 함수 설정
}

//Subscribe해서 메세지를 받을때마다 사용
void callback(char* topic, byte* payload, unsigned int length) 
{
    Serial.print("Message arrived [");
    Serial.print(topic);

    Serial.print("] ");
    for (int i=0;i<length;i++) 
    {
        char receivedChar = (char)payload[i];
        Serial.print(receivedChar);
        if (receivedChar == '0')
            Serial.println("Off");
        if (receivedChar == '1')
            Serial.println("On");
    }
    Serial.println();
}

void loop() 
{ 
    if (!client.connected()) //접속이 끊어졌을때 실행
    {
        reconnect(); //재접속 및 실패시 해당하는 메시지 출력을 위한 함수
    }
    client.loop();

    long now = millis();
    if (now - lastMsg > 2000) // 2초마다 설정한 Topic으로 메세지 발행
    {
        lastMsg = now;
        ++value;
        snprintf (msg, 75, "hello world #%ld", value);//
        Serial.print("Publish message: ");
        Serial.println(msg);
        client.publish("outTopic", msg); // Topic: outTopic, msg: hello wolrd #value값
    }
}


void reconnect() // 재접속 및 실패시 해당하는 메시지 출력을 위한 함수
{
    while (!client.connected()) //접속이 끊어졌을때 동안(재접속에 성공하기 전까지) 실행
    {
        Serial.print("Attempting MQTT connection...");
        
				//ESP8266Client라는 이름으로 접속 시도 및 값 반환(true일때 실행)
        if (client.connect("ESP8266Client")) 
        {
            Serial.println("connected");
            client.publish("outTopic","hello world"); //Topic과 메세지 다시 발행
            //client.subscribe("inTopic");
        }
        else //재접속 실패시
        {
            Serial.print("failed, rc=");
            Serial.print(client.state()); //Broker 서버와의 연결 상태 출력
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}