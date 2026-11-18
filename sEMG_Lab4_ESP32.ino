/*
  Laboratorio N.° 4 - Instrumentación Biomédica I
  Generador demostrativo de sEMG simulada para ESP32

  Salida analógica: DAC1, GPIO25
  Frecuencia nominal de muestreo: 2048 muestras/s
  Secuencia: reposo (5 s), cierre de mano (5 s), extensión del índice (5 s)

  IMPORTANTE:
  Este programa genera una señal sEMG sintética para comprobar el circuito.
  No reproduce las muestras originales del archivo semg.h ni el dataset
  utilizado en el Laboratorio N.° 3.
*/

#include <Arduino.h>
#include <math.h>

// -------------------- Configuración general --------------------
constexpr uint8_t DAC_PIN = 25;                 // DAC1 del ESP32 clásico
constexpr uint32_t SAMPLE_RATE_HZ = 2048;
constexpr uint32_t SEGMENT_SECONDS = 5;
constexpr uint32_t SAMPLES_PER_SEGMENT = SAMPLE_RATE_HZ * SEGMENT_SECONDS;

// Distribución fraccionaria del periodo para aproximar 1/2048 s
// usando el temporizador de micros() (resolución de 1 us).
constexpr uint32_t PERIOD_BASE_US = 1000000UL / SAMPLE_RATE_HZ;
constexpr uint32_t PERIOD_REMAINDER_US = 1000000UL % SAMPLE_RATE_HZ;

// Frecuencias de filtrado para dar aspecto de señal EMG (modelo sintético).
constexpr float HP_CUTOFF_HZ = 20.0f;
constexpr float LP_CUTOFF_HZ = 450.0f;

// Nivel central del DAC: aproximadamente la mitad de 3.3 V.
constexpr int DAC_MIDPOINT = 128;

// Amplitudes expresadas en cuentas del DAC. Son parámetros de demostración,
// no valores medidos ni amplitudes recuperadas del semg.h original.
constexpr float REST_AMPLITUDE = 6.0f;
constexpr float HAND_CLOSE_AMPLITUDE = 30.0f;
constexpr float INDEX_EXTENSION_AMPLITUDE = 18.0f;

// -------------------- Estado del generador --------------------
uint32_t noiseState = 0x6D2B79F5UL;
float previousWhiteNoise = 0.0f;
float previousHighPass = 0.0f;
float lowPassState = 0.0f;
float highPassAlpha = 0.0f;
float lowPassAlpha = 0.0f;

// Generador pseudoaleatorio determinista (xorshift32).
float whiteNoiseSample() {
  noiseState ^= noiseState << 13;
  noiseState ^= noiseState >> 17;
  noiseState ^= noiseState << 5;

  const uint16_t value = static_cast<uint16_t>(noiseState & 0xFFFF);
  return (static_cast<float>(value) / 32767.5f) - 1.0f;
}

// Ruido limitado en banda mediante filtros digitales de primer orden.
float syntheticEmgSample() {
  const float white = whiteNoiseSample();

  const float highPass = highPassAlpha *
                         (previousHighPass + white - previousWhiteNoise);
  lowPassState += lowPassAlpha * (highPass - lowPassState);

  previousWhiteNoise = white;
  previousHighPass = highPass;

  return lowPassState;
}

float segmentAmplitude(uint8_t segment, float timeInSegment) {
  switch (segment) {
    case 0: // Reposo
      return REST_AMPLITUDE;

    case 1: // Cierre de mano: actividad de mayor amplitud
      return HAND_CLOSE_AMPLITUDE *
             (0.82f + 0.18f * sinf(2.0f * PI * 1.8f * timeInSegment));

    case 2: // Extensión del índice: actividad intermedia
      return INDEX_EXTENSION_AMPLITUDE *
             (0.80f + 0.20f * sinf(2.0f * PI * 1.2f * timeInSegment));

    default:
      return REST_AMPLITUDE;
  }
}

void playSequence() {
  // Reinicia el estado de los filtros al comenzar una nueva secuencia.
  previousWhiteNoise = 0.0f;
  previousHighPass = 0.0f;
  lowPassState = 0.0f;

  uint32_t nextSampleTime = micros();
  uint32_t fractionalAccumulator = 0;
  constexpr uint32_t totalSamples = 3 * SAMPLES_PER_SEGMENT;

  for (uint32_t sample = 0; sample < totalSamples; ++sample) {
    while (static_cast<int32_t>(micros() - nextSampleTime) < 0) {
      // Espera activa; no imprimir por Serial durante la reproducción.
    }

    const uint8_t segment = static_cast<uint8_t>(sample / SAMPLES_PER_SEGMENT);
    const uint32_t localSample = sample % SAMPLES_PER_SEGMENT;
    const float timeInSegment = static_cast<float>(localSample) /
                                static_cast<float>(SAMPLE_RATE_HZ);

    const float emg = syntheticEmgSample();
    const float amplitude = segmentAmplitude(segment, timeInSegment);
    const int rawDac = DAC_MIDPOINT +
                       static_cast<int>(lroundf(emg * amplitude));
    const uint8_t dacValue = static_cast<uint8_t>(constrain(rawDac, 0, 255));

    dacWrite(DAC_PIN, dacValue);

    // Periodo medio = 488.28125 us para Fs = 2048 muestras/s.
    nextSampleTime += PERIOD_BASE_US;
    fractionalAccumulator += PERIOD_REMAINDER_US;
    if (fractionalAccumulator >= SAMPLE_RATE_HZ) {
      ++nextSampleTime;
      fractionalAccumulator -= SAMPLE_RATE_HZ;
    }
  }
}

void setup() {
  Serial.begin(115200);
  dacWrite(DAC_PIN, DAC_MIDPOINT);

  highPassAlpha = expf(-2.0f * PI * HP_CUTOFF_HZ /
                       static_cast<float>(SAMPLE_RATE_HZ));
  lowPassAlpha = 1.0f - expf(-2.0f * PI * LP_CUTOFF_HZ /
                             static_cast<float>(SAMPLE_RATE_HZ));

  Serial.println();
  Serial.println("Laboratorio 4 - Generador demostrativo de sEMG para ESP32");
  Serial.println("Salida DAC: GPIO25 | Fs: 2048 muestras/s");
  Serial.println("Secuencia: reposo -> cierre de mano -> extension del indice");
  Serial.println("Nota: senal sintetica; no corresponde al semg.h original.");
}

void loop() {
  // La secuencia completa dura 15 s y se repite automáticamente.
  playSequence();
}
