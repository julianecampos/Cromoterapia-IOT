// Juliane Campos e Nicolly Correa

// ===================
// Incluir bibliotecas
// ===================
#include <ESP8266WiFi.h> //inclui suporte ao NodeMCU
#include <PubSubClient.h> //inclui suporte ao MQTT no HiveMQ Cloud

// =======================
// Definicoes e constantes
// =======================

// Configuracoes da rede WiFi
const char* ssid = "2.4GNetvirtua164"; //SSID da rede WiFi
const char* password = "3397268240"; //senha da rede WiFi

const char* mqtt_server = "broker.mqtt-dashboard.com"; //URL do broker MQTT
const int mqtt_server_port = 1883; //porta do broker MQTT

// ============================
// Variaveis globais e objetos
// ============================

#define MSG_BUFFER_SIZE (500) //define MSG_BUFFER_SIZE como 500
WiFiClient client; //cria o objeto client
PubSubClient mqtt_client(client); //cria o objeto mqtt_client

String clientID = "ESP8266Client-"; //identificacao do cliente

String topicoPrefixo = "MACK32080204"; //para o prefixo do topico, utilizar MACK seguido do TIA
String topicoTodos = topicoPrefixo + "/#"; //para retornar todos os topicos
String topico_0 = topicoPrefixo + "/hello"; //topico para teste
String mensagem_0 = "NodeMCU Connected"; //mensagem para o topico 0
String sensor_1 = topicoPrefixo + "/sensor1"; //topico para o sensor 1
String mensagem_sensor = ""; //mensagem para o topico 1
String led_1 = topicoPrefixo + "/ledAzul"; //topico para o atuador 1
String mensagem_1 = ""; //mensagem para o led 1
String mensagemTemp = ""; //armazena temporariamente as mensagens recebidas via MQTT

long lastMsg = 0;

int Status = 12;  // Digital pin D6
int sensor = 13;  // Digital pin D7

//int estadoBotao = 0; //variavel que le o estado atual do botao conectado em D7
//int ultimoEstadoBotao = 0; //variavel que guarda o ultimo estado do botao conectado em D7
//int controlaEstadoBotao = 0; //variavel que controla o estado do botao conectado em D7

// ==============================
// Funcoes definidas pelo usuario
// ==============================

// Funcao para configurar a conexao WiFi
void setup_wifi() {
  //WiFi.mode(WIFI_STA); //ativa o modo de estacao
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

// Funcao para receber mensagens
void callback(char* topic, byte* payload, unsigned int length) {
  mensagemTemp = "";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    mensagemTemp += (char)payload[i];
  }

  if (String(topic) == led_1) {
    mensagem_1 = mensagemTemp;
  }

  Serial.println();

    // Switch on the LED if the first character is present
  if ((char)payload[0] != NULL) {
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on
    // Note that LOW is the voltage level but actually the LED is on;
    // this is because it is active low on the ESP-01
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  } else {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  }
}

// ==============================
// Funcao para conectar no broker MQTT
// ==============================

void reconnect() {
  // Loop until we’re reconnected
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection…");

  // Create a random client ID
    randomSeed(micros()); //inicializa a semente do gerador de numeros aleatorios
    clientID += String(random(0xffff), HEX);

  // Attempt to connect
    if (mqtt_client.connect(clientID.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqtt_client.publish(topico_0.c_str(), mensagem_0.c_str());
      // ... and resubscribe
      mqtt_client.subscribe(topicoTodos.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
} 

// =======
// setup()
// =======

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(sensor, INPUT);   // declare sensor as input
  pinMode(Status, OUTPUT);  // declare LED as output
  setup_wifi();
  mqtt_client.setServer(mqtt_server, mqtt_server_port); //configura a conexao com o broker MQTT
  mqtt_client.setCallback(callback); //configura o recebimento de mensagens
}

// ======
// loop()
// ======

void loop() {
  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop(); //processa as mensagens e mantem a conexao com o broker MQTT

  //Verifica e controla o estado do botao conectado em D7
  long estadoSensor = digitalRead(sensor);
  if(estadoSensor == HIGH) {
    digitalWrite (Status, HIGH);
    //Serial.println("Motion detected!");
    mensagem_1 = "A luz de cor azul foi acesa! Uma ótima escolha para quando você precisa de um estímulo cerebral ou que só quer relaxar e ter um conforto visual =)!";
    delay(5000);
    } else {
      digitalWrite (Status, LOW);
      //Serial.println("Motion absent!");
      //delay(200);
    }
  delay(50); //espera para evitar efeito bouncing
  //Publica mensagem
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    mqtt_client.publish(led_1.c_str(), mensagem_1.c_str());
  }
  
  //Subscreve mensagem
  //A subscricao eh realizada na funcao reconnect(),
  //e o recebimento das mensagens na funcao callback().
  //Para filtrar as mensagens dos topicos, verificar a funcao callback().

  //Controla o estado do LED conectado em D6
  if (mensagem_1.toInt() == 0) {
    digitalWrite(Status, 0);
  } else {
    digitalWrite(Status, 1);
  }
}
