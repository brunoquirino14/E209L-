//Princípio básico do projeto: Sistema de garagem inteligente com abertura de portão e iluminação por proximidade e acionamento.

#include <Ultrasonic.h> //Biblioteca para utilização do sensor ultrassônico 

#define LEDeMOTOR (1 << PD6) //O Led e o motor serão acionados juntos pelo pino PD6
#define BOTAO (1 << PB0) //O Botão de sinal será acionado pelo pino PD0
#define pino_trigger 4 //O Trigger do ultrassônico será acionado pelo PD4 (Não conseguimos fazer funcionar pela outra forma de atribuição)
#define pino_echo 5 //O Echo do ultrassônico será acionado pelo PD5 (Não conseguimos fazer funcionar pela outra forma de atribuição)

Ultrasonic ultrasonic(pino_trigger, pino_echo); // Criação de variável tipo Ultrasonic para o uso do sensor ultrassônico

bool leituraUltra; //Variável para checar se a leitura atende aos requisitos de ativação
int main(){
  Serial.begin(9600);// Inicia o monitor serial para teste do sensor

  //Configurações de entrada e saída
  DDRD |= LEDeMOTOR; // configura saída PD6 para o PWM, usado tanto no motor, como no LED
  PORTD &= ~LEDeMOTOR; // PWM inicia desligado
  PORTB |= BOTAO; // Ativa resistor de pull-up interno do pino PB0
  
  // Configura modo FAST PWM e modo do comparador A não-inversor
  TCCR0A |= (1 << COM0A1) | (1 << WGM01) | (1 << WGM00); //0b100000011

  // Ativa o PWM com Pré-Scaler 1024
  TCCR0B |= (1 << CS02) | (1 << CS00); //0b00000101

  // Inicia o PWM com Duty Cycle = 0%
  OCR0A = 0;

  //Configuração da interrupção externa somente no portal B, pois o botão é o único que está lá
  PCICR = (1 << PCIE0); //PCIE0 para 1, transição por mudança de estado para o grupo de pinos específico (B)
  PCMSK0 |= BOTAO; //Ativa a interrupção no pino PB0
  
  sei();//Ativa as interrupções globais
  
  for (;;){
  float cmMsec; //Variável que vai armazenar a distância do pulso de ultrassom
  long microsec = ultrasonic.timing(); //Variável que será utilizada para a conversão de valores
  cmMsec = ultrasonic.convert(microsec, Ultrasonic::CM);//Conversão da unidade do ultrassônico para centímetros
  //Exibe informacoes no serial monitor
  Serial.print("Distancia em cm: ");
  Serial.print(cmMsec);
  Serial.println(leituraUltra);
  
  //Verificação de proximidade que será usada na interrupção externa
  if(cmMsec <10){
    leituraUltra = 1;
  }else if(cmMsec >10){
    leituraUltra = 0;
  }
  delay(1000000);
  }
  
}

//Vetor da interrupção do portal B
ISR(PCINT0_vect){
  if(leituraUltra == 1){//Somente se a leitura for realizada dentro dos parâmetros, a interrupção acontecerá
    if((PINB & BOTAO) == 0){ //Quando botão for precionado e a proximidade acontecer
    OCR0A = 127; //Modifica o DutyCycle para 50% (50% de 255)
  }
  else if((PINB & BOTAO) == BOTAO){ //Quando botão for solto e a proximidade acontecer
    OCR0A = 0; //Modifica o DutyCycle para 0%
    delay(1000);
  }
  }else if(leituraUltra == 0){ //Caso a proximidade não ocorre, retorna ao estado inicial
    OCR0A = 0;//Modifica o DutyCycle para 0%
    delay(1000);
  }
}
