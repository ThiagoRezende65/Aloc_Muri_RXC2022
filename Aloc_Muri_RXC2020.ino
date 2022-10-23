/*
  Atualizado em 05/10/2022
  Gravar com a placa MiniCore (baixar no GitHub), Atmega328 no modelo soldado
  Config IDE:
      Bootloader: Não
      Clock: 8Mhz Interno
      BOD: Disabled
      Compiler LTO: Disabled(default)
      Variant: (O modelo de micro gravado)

      Programador: USBasp

   Fuses do Micro:
      L: 0xE2
      H: 0xD9
      E: 0xFF
      LB: 0xFF
*/

#define CH1 4
#define CH2 3
#define LED 9
int sens = 80;
float multidir = 1;
float multiesq = 1;
int trimdir = -15;
int trimesq = -10;
float ch1max = 2030;
float ch1min = 970;
float ch2max = 2030;
float ch2min = 970;
float ch1med = (ch1max + ch1min) / 2;
float ch2med = (ch2max + ch2min)/ 2;
int tempo = 0;

int ld = 1;

#include <Servo.h>
Servo EscD, EscE;

int curva(int str, int thr){
  float pstr, pthr;
  int val = 1500;
  if (str > ch1med) pstr = (str - ch1med - sens)/(ch1max - ch1med - sens );
  else pstr = -(ch1med - sens - str)/ ( ch1med - sens - ch1min );
  if (thr > ch2med) pthr = (thr - ch2med - sens)/(ch2max - ch2med - sens );
  else pthr = (ch2med - sens - thr)/ ( ch2med - sens - ch2min );

  if(pstr > 0) val = 1500 + (500*pthr - 1000*pthr*pstr);
  else val = 1500 + (-500*pthr - 1000*pthr*pstr );  
  
  if ( val > 2000) val = 2000;
  if ( val < 1000) val = 1000;
  return val;
}

int proportional(int ch, int ms, int trm, float multi){
  int val = 1500;
  if( ch == 1 && ms > ch1med){
    val = (1500 + ((ms - ch1med - sens) * 500/ (ch1max - ch1med - sens ) )* multi);
    }
  else if( ch == 2 && ms > ch2med){
    val = (1500 + ((ms - ch2med - sens) * 500/ (ch2max - ch2med - sens ) )* multi);
    }
  else if( ch == 1 && ms < ch1med){
    val = (1500 - ((ch1med - sens - ms) * 500/ ( ch1med - sens - ch1min ) )* multi);
    }
  else if( ch == 2 && ms < ch2med){
    val = (1500 - ((ch2med - sens - ms) * 500/ ( ch2med - sens - ch2min ) )* multi);
    }
  val = val + trm;
  if ( val > 2000) val = 2000;
  if ( val < 1000) val = 1000;
  return val;  
}

int inverter(int ch, int ms, int trm, float multi){
  int val;
  if( ch == 1 && ms > ch1med){
    val = (1500 - ((ms - ch1med - sens) * 500/ (ch1max - ch1med - sens ) )* multi);
    }
  else if( ch == 2 && ms > ch2med){
    val = (1500 - ((ms - ch2med - sens) * 500/ (ch2max - ch2med - sens ) )* multi);
    }
  else if( ch == 1 && ms < ch1med){
    val = (1500 + ((ch1med - sens - ms) * 500/ ( ch1med - sens - ch1min ) )* multi);
    }
  else if( ch == 2 && ms < ch2med){
    val = (1500 + ((ch2med - sens - ms) * 500/ ( ch2med - sens - ch2min ) )* multi);
    }
  val = val + trm;
  if ( val > 2000) val = 2000;
  if ( val < 1000) val = 1000;
  return val;  
  
}
        
void setup() {
  pinMode(CH1,INPUT);
  pinMode(CH2,INPUT);
  pinMode(LED, OUTPUT);
  EscD.attach(6);
  EscE.attach(5);
  EscD.writeMicroseconds(1500);
  EscE.writeMicroseconds(1500);

 // Serial.begin(9600); 
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(100);
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(100);
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(100);
  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  delay(100);
  digitalWrite(LED, HIGH);
}

void loop() {
  int str=pulseIn(CH1,HIGH), thr=pulseIn(CH2,HIGH);
  
    if(str < 700 || str > 2300 || thr < 700 || thr> 2300){    //failsafe
      EscD.writeMicroseconds(1480);
      EscE.writeMicroseconds(1480);
     //Serial.println("FAILSAFE");
     if((millis() - tempo) > 500){
        if(ld == 1) {
          digitalWrite(LED, LOW);
          ld = 0;
        }
        else {
          digitalWrite(LED, HIGH);
          ld = 1;
        }
        tempo = millis();        
     }
     
     }
     
      
    else if((str >= (ch1med - sens) && str <= (ch1med + sens)) && (thr > (ch2med + sens) || thr < (ch2med - sens))){  //frente ou ré
      EscD.writeMicroseconds(proportional( 2, thr, trimdir, multidir));
      EscE.writeMicroseconds(inverter(2, thr, trimesq, multiesq));
      //Serial.println("frente/tras");
      digitalWrite(LED, HIGH);}
      
    else if((str>=(ch1med - sens) && str<=(ch1med + sens)) && (thr<= (ch2med + sens) && thr>=(ch2med - sens))){ //parado
      EscD.writeMicroseconds(1480);
      EscE.writeMicroseconds(1480);
      //Serial.println("parado");
      digitalWrite(LED, HIGH);}

    else if((thr >=(ch2med - 100) && thr<=(ch2med + sens)) && (str<(ch1med - sens) || str>(ch1med + sens) )){//proprio eixo
       EscD.writeMicroseconds(inverter( 1, str, trimdir, multidir));
       EscE.writeMicroseconds(inverter( 1, str, trimesq, multiesq));
       //Serial.println("giro no eixo");
       digitalWrite(LED, LOW);}
        
    else if((str > (ch1med + sens) && thr > (ch2med + sens)) ||(str<(ch1med - sens) && thr<(ch2med - sens)) ){
      EscE.writeMicroseconds(inverter(2, thr, trimesq, multiesq));
      EscD.writeMicroseconds(curva(str,thr));
      //Serial.println("curva frente dir ou tras esq");
      digitalWrite(LED, HIGH);}// frente esquerda ou tras direita;
      
    else if((str<(ch1med - sens) && thr>(ch2med + sens)) ||(str>(ch1med + sens) && thr<(ch2med - sens)) ){
      EscD.writeMicroseconds(proportional (2,thr, trimdir, multidir));
      EscE.writeMicroseconds(curva(str,thr));
      //Serial.println("curva frente esq ou tras dir");
      digitalWrite(LED, HIGH);}// frente esquerda ou tras direita;

     Serial.print(EscD.readMicroseconds());
     Serial.print(" ");
     Serial.println(EscE.readMicroseconds());
     //delay(100);
    
     
}
