
//Pin connected to ST_CP of 74HC595
#define STCP 30
//Pin connected to SH_CP of 74HC595
#define SHCP 31
////Pin connected to DS of 74HC595
#define DS 29
//#define OE 27
//#define MR 30
#define LAYER_LOW 25
#define LAYER_HIGH 29

typedef unsigned char uchar;
typedef unsigned int uint;

uint mask[4][4];
uchar current_layer;
uchar frame = 0;
uchar brightness = 0;

#define SHIFT_REGISTER DDRB
#define SHIFT_PORT PORTB
#define DATA (1<<PB5)           //MOSI (SI)
#define LATCH (1<<PB4)          //SS   (RCK)
#define CLOCK (1<<PB7)          //SCK  (SCK)

void enable_spi(){
  SHIFT_REGISTER |= (DATA | LATCH | CLOCK);     //Set control pins as outputs
  SHIFT_PORT &= ~(DATA | LATCH | CLOCK);                //Set control pins low
  SPCR = (1<<SPE) | (1<<MSTR);
}

void enable_soft(){
  pinMode(STCP, OUTPUT);
  pinMode(SHCP, OUTPUT);
  pinMode(DS, OUTPUT);
  
}

void setup() {
  delay(3000);
//  enable_soft();
  enable_spi();
  for (int layer = LAYER_LOW; layer < LAYER_HIGH; layer++){
    pinMode(layer, OUTPUT);
    digitalWrite(layer, HIGH);
  }

  current_layer = 0;
  for(uchar y = 0; y < 4; y++){
  for(uchar b = 0; b < 4; b++){
    mask[y][b] = (1 << 16) - 1;
  }};
  
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 1;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12)  | (1 << CS10);
  TIMSK |= (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect)
{
  update();
}
int counter = 0;

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


uchar frame_to_bmask[15] = {
  0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3
};
void update(){
  
  uint layer = mask[current_layer][frame_to_bmask[frame]];

  SHIFT_PORT &= ~LATCH;
  SPDR = layer & 0b11111111;
  while(!(SPSR & (1<<SPIF)));
  SPDR = layer >> 8;
  while(!(SPSR & (1<<SPIF)));
  enable_layer(current_layer);
  SHIFT_PORT |= LATCH;
  
  current_layer += 1;
  current_layer %= 4;
  frame++;
  frame %= 15;
}

inline void set(uchar x, uchar y, uchar z, int b){
  uchar bit_to_set = y + (z << 2);
  uint bit_mask = 1 << bit_to_set;
  if(b > 15){
    b = 15;
  }
  if(b < 0) b = 0;
  for (uchar bit_from_b = 0; bit_from_b < 4; bit_from_b++){
    if(b & (1 << bit_from_b)){
      mask[x][bit_from_b] |= bit_mask;
    } else {
      mask[x][bit_from_b] &= ~bit_mask;
    }
  }
}

void loop(){
  
  ball();
//  blink_();
}

void ball(){
  counter++;
  counter %= 1024;
  brightness = abs(counter % 32 - 16);
  
  float bx = (sin(float(counter) / 9) + 1) * 1.5;
  float by = (sin(float(counter) / 5) + 1) * 1.5;
  float bz = (cos(float(counter) / 7) + 1) * 1.5;
//  float bx = 1;
//  float by = 1;
//  float bz = 1;
  
  for(char x = 0; x < 4; x++){
  for(char y = 0; y < 4; y++){
  for(char z = 0; z < 4; z++){
    float b = abs(bx - x) + abs(by - y) + abs(bz - z);
    set(x, y, z, int(20 - b * 8));
  }}};
  
  delay(10);
}

void blink_() {
  counter++;
  counter %= 1024;
  brightness = abs(counter % 32 - 16);
  
  for(uchar x = 0; x < 4; x++){
  for(uchar y = 0; y < 4; y++){
  for(uchar z = 0; z < 4; z++){
    set(x, y, z, brightness);
  }}};
  delay(30);
}
