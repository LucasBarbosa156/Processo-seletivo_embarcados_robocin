#include "I2Cdev.h"
#include "MPU6050.h"

MPU6050 sensor;
#define SDA  21 
#define SCL  22
#define frq 100 
#define interv (1000 / frq) // Intervalo em ms para leitura

int16_t gx, gy, gz;
float Velocidade_ang = 0.0;
float movi_ang = 0.0;
float gZ_offset = 0.0;

// Variáveis para filtro de média móvel

const int mostras = 10;
float janela[mostras] = {0};
int indice = 0;
long ultimo_tempo = 0;

// Função de calibração

void calibrarG(int Amostras = 500) {
    Serial.println("Calibrando giroscópio...");
    float soma = 0.0;
    for (int i = 0; i < Amostras; i++) {
        sensor.getRotation(nullptr, nullptr, &gz); 
        soma += gz;
        delay(3);
    }

    gZ_offset = soma / Amostras;
    Serial.print("Offset: ");
    Serial.println(gZ_offset);
}
// função para filtro de média móvel

float filtragem(float valor) {

    janela[indice] = valor;
    indice = (indice + 1) % mostras;
    float soma = 0.0;
    for (int i = 0; i < mostras; i++) {
        soma += janela[i];
    }
    return soma / mostras;
}



void setup() {

    Serial.begin(115200);
    Wire.begin(SDA, SCL);
    sensor.initialize();

    //teste de conexão

    if (sensor.testConnection() == false) {
        Serial.println("Falha na conexão com o sensor!");
        while (true);
    } else {
        Serial.println("Conexão bem-sucedida!");
    }

    calibrarG();
    ultimo_tempo = millis();

}

void loop() {
    long agora = millis();
    if (agora - ultimo_tempo >= interv) {
        ultimo_tempo = agora;

        // Leitura do giroscópio
        sensor.getRotation(nullptr, nullptr, &gz);
        float gZ_raw = gz - gZ_offset;

        float gZ_filtrado = filtragem(gZ_raw);
        Velocidade_ang = gZ_filtrado * (PI / 180.0) / 131.0;
        movi_ang += Velocidade_ang * (interv / 1000.0);

        Serial.print("Giroscópio Z (Filtrado): ");
        Serial.print(gZ_filtrado);
        Serial.print(" | Velocidade Angular (rad/s): ");
        Serial.print(Velocidade_ang, 5);
        Serial.print(" | movimento Angular (rad): ");
        Serial.println(movi_ang, 5);

    }
}
