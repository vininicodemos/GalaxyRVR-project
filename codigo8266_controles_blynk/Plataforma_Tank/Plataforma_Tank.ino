
// CREDENCIAIS DO BLYNK 
#define BLYNK_TEMPLATE_ID "TMPL2uKLTOf-x"
#define BLYNK_TEMPLATE_NAME "CONTROLE RVR"
#define BLYNK_AUTH_TOKEN    "933FrcfnUsdBzbPgOLO0xUZs1WkmByqn"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WebSocketsClient.h>

// CONFIGURAÇÃO DA REDE 
char ssid[] = "carrinho";    
char pass[] = "carrinho123";       

// ROBÔ
const char* robotIP = "172.21.223.46"; 
const uint16_t robotPort = 8765;

WebSocketsClient webSocket;
BlynkTimer timer; // Temporizador para manter o robô andando fluidamente

// VARIÁVEIS GLOBAIS 
int speeds = 80;       // Velocidade máxima em linha reta (0 a 100)
int CurveSpeed = 60;   // Velocidade máxima nas curvas (0 a 100)
int anguloCamera = 90; // Posição inicial da câmera (90 = centro)

int velocidadeEsq = 0;
int velocidadeDir = 0;

int eixoX = 0; // Leitura horizontal do Joystick
int eixoY = 0; // Leitura vertical do Joystick

// Função que envia o pacote JSON para o GalaxyRVR
void enviarTankDrive() {
    String json = "{\"K\":" + String(velocidadeEsq) + ",\"Q\":" + String(velocidadeDir) + ",\"D\":" + String(anguloCamera) + "}";
    webSocket.sendTXT(json);
}

// LÓGICA DO JOYSTICK (Cálculo do Movimento)
void calcularMovimento() {
    // Zona morta: se o joystick estiver no centro (com margem de erro), para os motores
    if(abs(eixoX) < 20 && abs(eixoY) < 20) {
        velocidadeEsq = 0;
        velocidadeDir = 0;
    } 
    // Movimentos principais
    else if(eixoY > 30) {        // Empurrou para Frente
        velocidadeEsq = speeds;
        velocidadeDir = speeds;
    } 
    else if(eixoY < -30) {       // Puxou para Trás
        velocidadeEsq = -speeds;
        velocidadeDir = -speeds;
    } 
    else if(eixoX > 30) {        // Puxou para a Direita
        velocidadeEsq = CurveSpeed;
        velocidadeDir = -CurveSpeed;
    } 
    else if(eixoX < -30) {       // Puxou para a Esquerda
        velocidadeEsq = -CurveSpeed;
        velocidadeDir = CurveSpeed;
    }
}

// MAPEAMENTO DOS PINOS VIRTUAIS DO BLYNK 

// Eixo X do Joystick (Pino V1)
BLYNK_WRITE(V1) {
    eixoX = param.asInt();
    calcularMovimento();
}

// Eixo Y do Joystick (Pino V2)
BLYNK_WRITE(V2) {
    eixoY = param.asInt();
    calcularMovimento();
}

// Slider de Velocidade Reta (Pino V3)
BLYNK_WRITE(V3) { 
    speeds = param.asInt(); 
    calcularMovimento(); // Atualiza a velocidade na hora se o robô estiver andando
}

// Slider de Velocidade de Curva (Pino V4)
BLYNK_WRITE(V4) { 
    CurveSpeed = param.asInt(); 
    calcularMovimento();
}

// Slider da Câmera (Pino V8)
BLYNK_WRITE(V8) { 
    anguloCamera = param.asInt(); 
}

// Monitora a conexão do WebSocket
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED: Serial.println("[WS] Desconectado!"); break;
        case WStype_CONNECTED: Serial.println("[WS] Conectado!"); break;
    }
}

void setup() {
    Serial.begin(115200);

    // Conecta ao Wi-Fi e ao Blynk
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Conecta ao Robô
    webSocket.begin(robotIP, robotPort, "/");
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(1000); 

    // Envia a velocidade atual a cada 150ms 
    // Isso impede o sistema de segurança do carrinho de travar as rodas
    timer.setInterval(150L, enviarTankDrive);
}

void loop() {
    Blynk.run();      
    webSocket.loop(); 
    timer.run(); 
}