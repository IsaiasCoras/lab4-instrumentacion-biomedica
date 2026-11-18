# Laboratorio N.° 4 - Generador demostrativo de sEMG con ESP32

Programa para Arduino IDE que genera una señal sEMG sintética y la entrega por la salida DAC1 del ESP32 (GPIO25).

## Secuencia generada

El programa repite tres tramos consecutivos de cinco segundos:

1. Reposo.
2. Cierre de mano (mayor amplitud).
3. Extensión del índice (amplitud intermedia).

La frecuencia nominal de muestreo es de 2048 muestras/s. La salida del DAC se centra alrededor del código 128 para mantener la señal dentro del rango unipolar del DAC.

## Requisitos

- Placa ESP32 clásica con DAC en GPIO25.
- Arduino IDE.
- Paquete de placas ESP32 instalado en Arduino IDE.
- Cable USB para programar y alimentar la placa.

## Uso

1. Abrir \`sEMG_Lab4_ESP32.ino\` en Arduino IDE.
2. Seleccionar la placa ESP32 correspondiente (por ejemplo, **ESP32 Dev Module**).
3. Seleccionar el puerto COM de la placa.
4. Compilar y cargar el programa.
5. Conectar la salida GPIO25 al nodo de entrada VIN del circuito, según la etapa experimental.
6. Unir el GND del ESP32 con el GND común del circuito.

## Conexión según la etapa

- **Etapa B:** GPIO25 se conecta directamente a VIN.
- **Etapas C y D:** insertar el condensador de acoplamiento de 100 nF entre GPIO25 y VIN y conectar la resistencia de 1 MΩ entre VIN y GND.

La alimentación del TL084 y el resto del circuito se realiza según el esquemático de la guía de laboratorio.

## Alcance y limitación

Este sketch genera una **señal sintética de prueba** con ruido limitado en banda y tres niveles de actividad. No contiene ni reconstruye las muestras originales del archivo \`semg.h\` del Laboratorio N.° 3 y no debe describirse como el firmware original o como una reproducción exacta de registros experimentales.

Los niveles de amplitud son parámetros de demostración ajustables en el código. Para reproducir los resultados originales del Laboratorio N.° 3 se necesitaría el archivo de muestras correspondiente.

## Salida

- Pin de señal: GPIO25 (DAC1).
- Referencia: GND del ESP32.
- Velocidad serie de diagnóstico: 115200 baudios.
