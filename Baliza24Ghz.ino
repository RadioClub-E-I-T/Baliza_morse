//  ******************************************** HARDWARE IMPORTANT********************************************************
//  La primera parte del código consiste en la implementación de un codigo para establecer la frecuencia de salida
//  utilizando la frecuencia de referencia de 10Mhz del GPDS0.
//  Para ello se instalarán las conexiones como sigue:
//
//  MOSI (pin D11) al pin ADF DATA
//  SCK (pin  D13) al pin ADF CLK
//  PIN D10 al pin ADF LE
//
// Dada la salida de tension del arduino es necesario generar un divisor de tensión de modo que la tensión a cada de uno de los puertos
// Baje de 5V a 3,1V. para ello es necesario colocar una resistencia de 560[ohms] al pin del arduino, ese punto se conecta al pin del ADF 
// y posteriormente una resitencia de 1[kohm] a masa.
// 
// En nuestro caso el arduino ha sido alimentado por los puertos de 5V y GND.
// Además al usar la señal de referencia de 10Mhz esta debe de ser introducida por el puerto de RefA
//  ******************************************** CONFIGURACION IMPORTANTE ********************************************************
#include <SPI.h> //Permite controlar la frecuencia
#include <ErriezSerialTerminal.h>
#include <arduino-timer.h>

#define ADF5355_CS 10

#define BITS_MOVE 5
#define DOT_TIME 100     // In milliseconds En teoría son 100ms
#define SPACE_BTW_WORDS 2100    // 7*DOT_TIME Antes eran 1750
#define SPACE_BTW_CHARACTERS 300 // 3*DOT_TIME En teoría deben de ser 300ms
#define SPACE_BTW_PHRASES 3000


////////////////////////////DEFINICION DE LA FRECUENCIA//////////////////////////////////////////

// El datasheet establece en 50Mhz la máxima frecuencia de reloj, nosotros la dejamos en 10Mhz

SPISettings MySettings(10000000, MSBFIRST, SPI_MODE0);

//Los registros se hacen mediante una calculadora que completa los registros estipulados en el data sheets en función principalmente de la 
//frecuencia de referencia y la frecuencia estipulada.

// Prueba de 144,5 MHz con reference 10 MHz
//uint32_t registers[13] = {0x201CE0, 0x6666661 , 0x4C0322, 0x3, 0x3000A784, 0x800025, 0x15A20476, 0x120000E7, 0x102D0428, 0x302FCC9, 0xC0067A, 0x61300B, 0x1041C};

// Prueba de 432,5 MHz con reference 10 MHz Refdiv=No
//uint32_t registers[13] = {0x2015A0, 0x1 , 0x322, 0x3, 0x3000A784, 0x800025, 0x15620476, 0x120000E7, 0x102D0428, 0x302FCC9, 0xC0067A, 0x61300B, 0x1041C};

// Prueba de 432,5 MHz con reference 10 MHz Refdiv=Yes
//uint32_t registers[13] = {0x202B40, 0x1 , 0x192, 0x3, 0x3200A784, 0x800025, 0x15620476, 0x120000E7, 0x102D0428, 0x302FCC9, 0xC0033A, 0x61300B, 0x1041C};

// Prueba de 2,112 GHz con reference 10 MHz Refdiv=Yes
//uint32_t registers[13] = {0x201A60, 0x6666661 , 0x4C0322, 0x3, 0x3000A784, 0x800025, 0x15220476, 0x120000E7, 0x102D0428, 0x302FCC9, 0xC0067A, 0x61300B, 0x1041C};

// Prueba de 12 GHz con reference 10 MHz Refdiv=No
//uint32_t registers[13] = {0x202580, 0x1 , 0x322, 0x3, 0x3000A784, 0x800025, 0x15020476, 0x120000E7, 0x102D0428, 0x302FCC9, 0xC0067A, 0x61300B, 0x1041C};

// Prueba de 12 GHz con reference 10 MHz Refdiv=Yes
//uint32_t registers[13] = {0x204B00, 0x1 , 0x192, 0x3, 0x3200A784, 0x800025, 0x15020476, 0x120000E7, 0x102D0428, 0x302FCC9, 0xC0033A, 0x61300B, 0x1041C};

//Ejemplo 500,5Mhz
uint32_t registers_ON[13] = {0x201900,0x6666681 ,0x13E7FFF2 ,0x00000003,0x30008984 ,0x00800025, 0x35606476 ,0x120000E7, 0x102D0428,0x05047CC9,0xC0067A ,0x0061300B,0x0001041C };
uint32_t registers_OFF[13] = {0x201900,0x6666681 ,0x13E7FFF2 ,0x00000003,0x300089C4 ,0x00800025, 0x35606476 ,0x120000E7, 0x102D0428,0x05047CC9,0xC0067A ,0x0061300B,0x0001041C };

//Ejemplo 430Mhz
//uint32_t registers_ON[13] = {0x201580,0x11 ,0x39D55552 ,0x00000003,0x30008984 ,0x00800025, 0x35606476 ,0x120000E7, 0x102D0428,0x05047CC9,0xC0067A ,0x0061300B,0x0001041C };
//uint32_t registers_OFF[13] = {0x201580,0x11 ,0x39D55552 ,0x00000003,0x300089C4 ,0x00800025, 0x35606476 ,0x120000E7, 0x102D0428,0x05047CC9,0xC0067A ,0x0061300B,0x0001041C };

//Ejemplo 144Mhz
//uint32_t registers_ON[13] = {0x201CC0,0xCCCCCE1 ,0x2D817D2 ,0x00000003,0x30008984 ,0x00800025, 0x35A04476 ,0x120000E7, 0x102D0428,0x05047CC9,0xC0067A ,0x0061300B,0x0001041C };
//uint32_t registers_OFF[13] = {0x201CC0,0xCCCCCE1 ,0x2D817D2 ,0x00000003,0x300089C4 ,0x00800025, 0x35A04476 ,0x120000E7, 0x102D0428,0x05047CC9,0xC0067A ,0x0061300B,0x0001041C };


void WriteReg32(const uint32_t value)
{
  // send the data a byte at a time over SPI 
  for (int i = 3; i >= 0; i--)             // loop round 4 x 8 bits MSB first
  SPI.transfer((value >> 8 * i) & 0xFF); // offset, byte mask and send via SPI
  digitalWrite(ADF5355_CS, HIGH);// datasheet says 20ns minimum between HIGH and LOW below
  delayMicroseconds(1);// clock of the Load Enable line to move the data from
  digitalWrite(ADF5355_CS, LOW);// the data register into the programing register set by the bottom 4 bits
}

void Encendido()  // bung the data into the ADF5355
{ 
  // Start the SPI transaction
  SPI.beginTransaction(MySettings);
  // active low load enable
  digitalWrite(ADF5355_CS, LOW);
  // send the 13 registers
  for (int i = 12; i >= 0; i--)
    WriteReg32(registers_ON[i]);
  // de-select the SPI device
  digitalWrite(ADF5355_CS, HIGH);
  SPI.endTransaction();
}

void Apagado()  // bung the data into the ADF5355
{ 
  // Start the SPI transaction
  SPI.beginTransaction(MySettings);
  // active low load enable
  digitalWrite(ADF5355_CS, LOW);
  // send the 13 registers
  for (int i = 12; i >= 0; i--)
    WriteReg32(registers_OFF[i]);
  // de-select the SPI device
  digitalWrite(ADF5355_CS, HIGH);
  SPI.endTransaction();
}

// Structures
typedef enum {
  IDLE,
  PUNTO,
  RAYA,
  SEPARADOR_ELEMENTOS
} MORSE_STATES;

// Newline character '\r' or '\n'
char newlineChar = '\n';
// Separator character between commands and arguments
char delimiterChar = ' ';

// Create serial terminal object
SerialTerminal term(newlineChar, delimiterChar);

// Function prototypes
void unknownCommand(const char *command);
void cmdHelp();
void cmdGetText();
void cmdSetText();
void cmdOn();
void cmdOff();
void cmdStatus();
void str_to_upper(char * input, int lenght);
void text_to_morse(uint8_t morse_char);
void ptt_on();
void ptt_on();

String msg = "EA4RCT EIT";
boolean transmitiendo = false;    // Status, true if tx, false if not
boolean text_changed = false;
auto timer = timer_create_default();
MORSE_STATES estado = IDLE;
uint8_t count = 0; 
uint16_t indice_string = 0;
uint8_t mask;
uint8_t j;
int length;
unsigned long time;

uint8_t morse[] = {         // First 3 bits represents duration, then 0 dot and 1 dash.
    0b01001000, // A (2 .-   )
    0b10010000, // B (4 -... )
    0b10010100, // C (4 .. )
    0b01110000, // D (3 _..  )
    0b00100000, // E (1 .    )
    0b10000100, // F (4 ..-. )
    0b01111000, // G (3 //.  )
    0b10000000, // H (4 .... )
    0b01000000, // I (2 ..   )
    0b10001110, // J (4 .//- )
    0b01110100, // K (3 -.-  )
    0b10001000, // L (4 .-.. )
    0b01011000, // M (2 //   )
    0b01010000, // N (2 -.   )
    0b01111100, // O (3 //-  )
    0b10001100, // P (4 .//. )
    0b10011010, // Q (4 //.- )
    0b01101000, // R (3 .-.  )
    0b01100000, // S (3 ...  )
    0b00110000, // T (1 -    )
    0b01100100, // U (3 ..-  )
    0b10000010, // V (4 ...- )
    0b01101100, // W (3 .--  )
    0b10010010, // X (4 -..- )
    0b10010110, // Y (4 -.-- )
    0b10011000, // Z (4 --.. )
    0b10101111, // 1 (5 .----)
    0b10100111, // 2 (5 ..---)
    0b10100011, // 3 (5 ...--)
    0b10100001, // 4 (5 ....-)
    0b10100000, // 5 (5 .....)
    0b10110000, // 6 (5 -....)
    0b10111000, // 7 (5 --...)
    0b10111100, // 8 (5 ---..)
    0b10111110, // 9 (5 ----.)
    0b10111111  // 0 (5 -----)
};

bool morse_write(uint8_t morse_char) {
  switch(estado){
    case IDLE:
      break;
    case PUNTO:
      //Serial.println("Punto");
      Encendido();
      delay(DOT_TIME);
      estado = SEPARADOR_ELEMENTOS;
      break;
    case RAYA:
      //Serial.println("RAYA");
      Encendido();
      delay(3*DOT_TIME);
      estado = SEPARADOR_ELEMENTOS;
     /* if (count == 2) {
        estado = SEPARADOR_ELEMENTOS;
      }
      count++;*/
      break;
    case SEPARADOR_ELEMENTOS:  // Puede ser separador elemento, de caracter o de palabra, dependiendo de donde estemos tiraremos para uno y otro
      //Serial.println("--");
      Apagado();
      count = 0;
      
      if(j < BITS_MOVE - (length-1)){
        estado = IDLE;
        //Serial.println("Saliendo");
        return false;
      }

      mask = 1 << (j-1) ;
      if(morse[morse_char] & mask ) {
        estado = RAYA;
      }else{
        estado = PUNTO;
      }

      j--;
      
      break;
  }
  return true; // keep timer active? true
}


void setup() 
{
   //DEFINICION DE LA FRECUENCIA A UTILIZAR
   pinMode(ADF5355_CS, OUTPUT);          // Setup pins
   digitalWrite(ADF5355_CS, HIGH);
   SPI.begin();                          // Init SPI bus  
   //Encendido();
       

   //DEFINICION DE CW A UTILIZAR
    delay(500);

    // Initialize serial port
    Serial.begin(115200);
    Serial.println(F("\nEA4RCT E.I.T Baliza Software Serial"));
    Serial.println(F("Type 'help' to display usage."));

   /* // Initialize the built-in LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);*/

    // Set default handler for unknown commands
    term.setDefaultHandler(unknownCommand);

    // Add command callback handlers
    term.addCommand("?", cmdHelp);
    term.addCommand("help", cmdHelp);
    term.addCommand("get-text", cmdGetText);    // Get the current text
    term.addCommand("set-text", cmdSetText);    // Set a new text
    term.addCommand("on", cmdOn);            // Start trasmitting
    term.addCommand("off", cmdOff);          // Stop transmitting
    term.addCommand("status", cmdStatus);       // Get Status
 
}

void loop() 
{
 int i = 0;
    text_changed = false;

    time = millis();
  
    while(millis() < time + SPACE_BTW_PHRASES){
      timer.tick();
      term.readSerial();
    }
    
    for(; i < msg.length() && !text_changed; i++){
      
      Serial.println(msg[i]);
      switch ((int) msg[i]) {
        case 32:    // Space
          Serial.println("ESPACIO \n");

          time = millis();
  
          while(millis() < time + SPACE_BTW_WORDS){
            timer.tick();
            term.readSerial();
          }
          
          break;
        case 0:
          break;
        default:
          j = BITS_MOVE - 1;
          uint8_t morse_char = (int) msg[i] - 'A';
          length = morse[morse_char] >> BITS_MOVE;
          if(morse[morse_char] & 16 ) {   // Mask to the first bit
            //RAYA
            estado = RAYA;
          }else{
            //PUNTO
            estado = PUNTO;
          }
          
          timer.every(DOT_TIME, morse_write, morse_char);
          while(estado != IDLE){
            timer.tick();
            term.readSerial();
          }
  
          time = millis();
  
          while(millis() < time + SPACE_BTW_CHARACTERS){
            timer.tick();
            term.readSerial();
          }
          //delay(SPACE_BTW_CHARACTERS);  // Separación entre caracteres
          
      }
    }
}

void unknownCommand(const char *command)
{
    // Print unknown command
    Serial.print(F("Unknown command: "));
    Serial.println(command);
    Serial.println(F("Type help or ? for command usage"));
}

void cmdHelp()
{
    // Print usage
    Serial.println(F("Serial terminal usage:"));
    Serial.println(F("  help or ?          Print this usage"));
    Serial.println(F("  get-text           Get the current msg"));
    Serial.println(F("  set-text <string>  Set a new msg"));
    Serial.println(F("  on                 Set on status"));
    Serial.println(F("  off                Set off status"));
    Serial.println(F("  status             Get current status"));
}

void cmdGetText(){
  // Print current text
  Serial.println(msg);
}

void cmdSetText(){
  char* input = term.getRemaining();
  str_to_upper(input, strlen(input));
  msg = input;

  text_changed = true;
}

void cmdOn(){
  transmitiendo = true;
  
}

void cmdOff(){
  transmitiendo = false;
}

void cmdStatus(){
  Serial.println(transmitiendo);
}

void str_to_upper(char * input, int lenght){
  char * original = input;

  while(lenght){
    *input = toupper((unsigned char) *input);
    lenght--;
    input++;
  }

  input = original;
}
