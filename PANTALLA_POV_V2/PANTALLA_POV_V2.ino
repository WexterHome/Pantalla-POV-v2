#include <FastLED.h>
#include "imgs.h"
#include <pgmspace.h>

//Sensor de efecto Hall
const int HALL_SENSOR = 12;     //CAMBIAR ESTE NÚMERO
volatile unsigned long lastTime = 0;
unsigned long threshold = 50;

//Pantalla POV (efecto visual)
int resolution = 6;  //Cambiamos color de los leds cada 10 grados
byte leds_samples = 360/resolution;
volatile int leds_pos = 0;
volatile bool leds_pos_changed = false;

//LEDS
#define NUM_LEDS 50   //LED_TOTALES/4 (4 pines en paralelo)
#define LEDS_TYPE WS2812B
#define BRIGHTNESS 10
#define COLOR_ORDER GRB

#define DATA_PIN_1 19   
#define DATA_PIN_2 21
#define DATA_PIN_3 22
#define DATA_PIN_4 23

CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];
CRGB leds3[NUM_LEDS];
CRGB leds4[NUM_LEDS];

//Timer
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
int preescaler = 80;
int frequency = 80000000/preescaler;
volatile float lap_time = 1000;
volatile float pulses_number = 0;


//Prototipos de las funciones
void draw_leds(int leds_pos);
void IRAM_ATTR onTimer();
uint32_t expandColor(uint16_t color);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(2000);

  //Pin del sensor de efecto Hall
  pinMode(HALL_SENSOR, INPUT);

  //Inicilizamos leds
  FastLED.addLeds<LEDS_TYPE, DATA_PIN_1, COLOR_ORDER>(leds1, NUM_LEDS);
  FastLED.addLeds<LEDS_TYPE, DATA_PIN_2, COLOR_ORDER>(leds2, NUM_LEDS);
  FastLED.addLeds<LEDS_TYPE, DATA_PIN_3, COLOR_ORDER>(leds3, NUM_LEDS);
  FastLED.addLeds<LEDS_TYPE, DATA_PIN_4, COLOR_ORDER>(leds4, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5,1000);
  FastLED.setBrightness(BRIGHTNESS);

  //Apagamos leds
  for(int i=0; i<NUM_LEDS; i++){
    leds1[i].setRGB(255,0,0);
    leds2[i].setRGB(0,0,0);
    leds3[i].setRGB(0,0,0);
    leds4[i].setRGB(0,0,0);
  }
  FastLED.show();

  //Encendemos el leds 1, 51, 101 y 151 en rojo
  delay(1000);
  leds1[0].setRGB(0, 0, 255);
  leds2[0].setRGB(0, 0, 255);
  leds3[0].setRGB(0, 0, 255);
  leds4[0].setRGB(0, 0, 255);
  FastLED.show();

  //El programa se detiene hasta que la barra led pase
  //por el sensor Hall
  //while(!digitalRead(HALL_SENSOR)){ };
  delay(3000);

  //Configuración del timer
  timer = timerBegin(0, 80, true); //Frecuencia 1.000.000 con preescaler 80
  timerAttachInterrupt(timer, &onTimer, true); //Creamos interrupción
  //El 1000 es para pasar la frecuencia de segundos a ms.
  pulses_number = (lap_time*frequency)/(1000*leds_samples);
  timerAlarmWrite(timer, pulses_number, true);


  //Cuando la barra ya está girando se muestran los leds
  //1, 51, 101 y 151 en verde durante 5 segundos
  Serial.println("Holaaa");
  
  leds1[0].setRGB(0, 255, 0);
  leds2[0].setRGB(0, 255, 0);
  leds3[0].setRGB(0, 255, 0);
  leds4[0].setRGB(0, 255, 0);
  Serial.println("Adioos");
  delay(5000);
  timerAlarmEnable(timer);
}

void loop() {
  // put your main code here, to run repeatedly:
  //if(!digitalRead(HALL_SENSOR) && (millis()-lastTime) > threshold && lap_time > 250){
  if(!digitalRead(HALL_SENSOR) && (millis()-lastTime) > threshold){
    //leds_pos = 0;
    //leds_pos_changed = true;
    
    lap_time = millis() - lastTime;
    pulses_number = (lap_time*frequency)/(1000*leds_samples);
    timerAlarmWrite(timer, pulses_number, true);
    timerAlarmEnable(timer);  //¿¿ES NECESARIO??
    
    lastTime = millis();
  }

  
  if(leds_pos_changed){
    leds_pos_changed = false;
    if(leds_pos!=0)
      draw_leds(leds_pos);
    FastLED.show();
  }
}


void draw_leds(int leds_pos){
  int row = leds_pos/resolution;
  int i_aux = 0;
  uint16_t color16 = 0;
  uint32_t color24 = 0;

  for(int i=0; i<NUM_LEDS; i++){
    color16 = pgm_read_dword(&(frame5[row][i]));
    color24 = expandColor(color16);
    leds1[i] = color24;
    
    color16 = pgm_read_dword(&(frame5[row][NUM_LEDS + i]));
    color24 = expandColor(color16);
    leds2[i] = color24;
  }

  if(leds_pos < 180)
    row += leds_samples/2;
  else
    row -= leds_samples/2;
  

  for(int i=0; i<NUM_LEDS; i++){
    color16 = pgm_read_dword(&(frame5[row][2*NUM_LEDS-i-1]));
    color24 = expandColor(color16);
    leds3[i] = color24;

    color16 = pgm_read_dword(&(frame5[row][NUM_LEDS-i-1]));
    color24 = expandColor(color16);
    leds4[i] = color24;
  }    
}

//Convierte colores RGB 5:6:5 a RGB 8:8:8
uint32_t expandColor(uint16_t color){
  uint16_t blue5 = 0x001F & color;
  uint16_t green6 = (0x07E0 & color) >> 5;
  uint16_t red5 = (0xF800 & color) >> 11;

  uint32_t blue8 = (blue5*527 + 23) >> 6;
  uint32_t green8 = (green6*259 + 33) >> 6;
  uint32_t red8 = (red5*527 + 23) >> 6;

  //Serial.println(red8, HEX);

  uint32_t converted_color = (red8<<16) + (green8<<8) + blue8;
  return converted_color;
}

//Interrupcion del timer
void IRAM_ATTR onTimer(){
  leds_pos_changed = true;
  if(leds_pos<350){
    leds_pos += resolution;
  }
  else{
    leds_pos = 0;
  }
}
