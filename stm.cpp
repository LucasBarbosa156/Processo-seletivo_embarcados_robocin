#include "MPU6050.h"

#define PI 3.14159265359

MPU6050 mpu(PB_9, PB_8); 

float Velocidade_ang = 0.0;
float movi_ang = 0.0;
float gZ_offset = 0.0;

const int filtro = 10;
float janela[filtro] = {0};
int indice = 0;

long ultimo_tempo = 0;
const int intervalo = 10;

// Função de calibração
template<typename T>
void calibrarG(T& sensor, int amostras = 1000) {
    printf("Calibrando giroscópio...\n");
    float soma = 0.0;
    int16_t gyroReadings[3];
    
    for (int i = 0; i < amostras; i++) {
        sensor.readGyroRaw(gyroReadings); 
        soma += gyroReadings[2]; 
        ThisThread::sleep_for(3ms);
    }
    gZ_offset = soma / amostras;
    printf("Offset: %.5f\n", gZ_offset);
}

// Filtro de média móvel
float filtragem(float valor) {
    janela[indice] = valor;
    indice = (indice + 1) % filtro;

    float soma = 0.0;
    for (int i = 0; i < filtro; i++) {
        soma += janela[i];
    }
    return soma / filtro;
}

int main() {
    printf("Iniciando...\n");
    if (!mpu.initialize()) { 
        printf("Falha na conexão com o sensor!\n");
        while (true) {}
    }
    printf("Conexão bem-sucedida!\n");
    calibrarG(mpu);
    
    ultimo_tempo = std::chrono::duration_cast<std::chrono::milliseconds>(
                      Kernel::Clock::now().time_since_epoch()).count();
    
    while (true) {
        long agora = std::chrono::duration_cast<std::chrono::milliseconds>(
                        Kernel::Clock::now().time_since_epoch()).count();
        if (agora - ultimo_tempo >= intervalo) {
            ultimo_tempo = agora;

            int16_t gyroReadings[3];
            mpu.readGyroRaw(gyroReadings); // Corrigindo a leitura do giroscópio
            float gZ_raw = gyroReadings[2] - gZ_offset; // Apenas eixo Z
            float gZ_filtrado = filtragem(gZ_raw);
            Velocidade_ang = gZ_filtrado * (PI / 180.0);
            movi_ang += Velocidade_ang * (intervalo / 1000.0);

            printf("Giroscópio Z (Filtrado): %.5f | Velocidade Angular (rad/s): %.5f | Movimento Angular (rad): %.5f\n",
                   gZ_filtrado, Velocidade_ang, movi_ang);
            ThisThread::sleep_for(10ms);
        }
    }
}
