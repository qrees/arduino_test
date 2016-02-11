
//Pin connected to ST_CP of 74HC595
#define STCP 30
//Pin connected to SH_CP of 74HC595
#define SHCP 31
////Pin connected to DS of 74HC595
#define DS 29
//#define OE 27
//#define MR 30
#define LAYER0 28
#define LAYER1 27
#define LAYER2 26
#define LAYER3 25
#define LAYER_LOW 25
#define LAYER_HIGH 29

typedef unsigned char uchar;
typedef unsigned int uint;

uint mask[4];
uchar current_layer;
uchar frame = 0;
uchar brightness = 0;

void setup() {
  pinMode(STCP, OUTPUT);
  pinMode(SHCP, OUTPUT);
  pinMode(DS, OUTPUT);
  pinMode(LAYER0, OUTPUT);
  pinMode(LAYER1, OUTPUT);
  pinMode(LAYER2, OUTPUT);
  pinMode(LAYER3, OUTPUT);
  digitalWrite(LAYER0, HIGH);
  digitalWrite(LAYER1, HIGH);
  digitalWrite(LAYER2, HIGH);
  digitalWrite(LAYER3, HIGH);

  current_layer = 0;
  for(uchar y = 0; y < 4; y++){
    mask[y] = (1 << 16) - 1;
  };
  
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 5;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12)  | (1 << CS10);
  TIMSK |= (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect)
{
  update();
}
int counter = 0;

void couting(){
  digitalWrite(STCP, LOW);
  shiftOut(DS, SHCP, MSBFIRST, counter);
  shiftOut(DS, SHCP, MSBFIRST, counter >> 8);
  enable_layer(counter % 4);
  digitalWrite(STCP, HIGH);
  
  counter++;
}

void disable_layers(){
  for(int i = LAYER_LOW; i < LAYER_HIGH; i++){
    digitalWrite(i, HIGH);
  }
}

void enable_layer(int layer){
  layer += LAYER_LOW;
  for(int i = LAYER_LOW; i < LAYER_HIGH; i++){
    digitalWrite(i, i != layer);
  }
}

void update(){
  uint layer = mask[current_layer];
  if(brightness > frame){
    digitalWrite(STCP, LOW);
    shiftOut(DS, SHCP, MSBFIRST, layer);
    shiftOut(DS, SHCP, MSBFIRST, layer >> 8);
    enable_layer(current_layer);
    digitalWrite(STCP, HIGH);
  }else{
    disable_layers();
    digitalWrite(STCP, LOW);
    shiftOut(DS, SHCP, MSBFIRST, 0);
    shiftOut(DS, SHCP, MSBFIRST, 0);
    digitalWrite(STCP, HIGH);
  }
  current_layer += 1;
  current_layer %= 4;
  frame++;
  frame %= 15;
}

void loop() {
  counter++;
  brightness = abs(counter % 32 - 16);
  delay(25);
}
