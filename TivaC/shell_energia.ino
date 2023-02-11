
// headers
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/pwm.h"


// Global Variables
int is_LedOn = 0;
int ledColorSet = 7;
int blinkRate = 1;
int sw_counter = 0;
bool ledSetForBlink = 0;
int motor_L_speed = 20;
int motor_R_speed = 20;
int motor_enable = 0;


// defining pin numbers
#define RED_LED 30
#define BLUE_LED 40
#define GREEN_LED 39
#define SW1 31
#define SW2 17
#define L_PWM 15
#define L_DIR 13
#define M_ENABLE 19 // One enable signal is suuficient. if needed separate we need to break the bypass
#define R_PWM 14
#define R_DIR 18
#define ARR(x) (x + 32) // pin 0-7, 8-Led
 
void setup() {

  pinMode(L_PWM,OUTPUT);
  pinMode(L_DIR,OUTPUT);
  pinMode(M_ENABLE,OUTPUT);
  pinMode(R_PWM,OUTPUT);
  pinMode(R_DIR,OUTPUT); 
  for (int  i = 0; i<=7 ; ++i)
  {
    pinMode(ARR(i), INPUT_PULLUP);
  }
  /*
   * Configuring the Timer
   * Interupt generated at every 1ms 
   * Interupt Handler := IntHandler_TIMER0
   */
 
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);     // Enabling the Timer Peripheral     
  TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);  // Configuring the Timer_0 as continuous down counter
  TimerLoadSet(TIMER0_BASE,TIMER_A,0xFA0 *1000);    // Setting the load to 0.1s
  TimerEnable(TIMER0_BASE, TIMER_A);                // Enabling the timer
  TimerIntEnable(TIMER0_BASE,TIMER_TIMA_TIMEOUT);   // Enabling interupt for the timer at TIMEOUT
  TimerIntRegister(TIMER0_BASE,TIMER_A,&IntHandler_TIMER0);// Registering the Interupt Handler
  
  /*
   * Configuring the UART
   * Interupt is enabled so UART is read only when available
   * Interupt handler := IntHandler_UART
   */
   
  // Enable the UART and corresponding GPIO peripherals
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  SysCtlDelay(1);
  // Configure GPIO registers controlling the port pins
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  // Configure port control register tp specify UART type
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
  // Configure the UART for 115200, 8-N-1 operation.
  UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
  // Enable Interupts
  UARTIntEnable(UART0_BASE, UART_INT_RX);
  UARTIntRegister(UART0_BASE,&IntHandler_UART);
  // Welcome message
  char version[] = "version 1.0";
  char greetings[] = "hello, world!";
  tty_puts(greetings);
  tty_puts(version);

   /*
   * Configuring PWM module
   */


    // Enabling the peripheral
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
     SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);  

    //Configure PB6,PB7 Pins as PWM
    GPIOPinConfigure(GPIO_PB6_M0PWM0);
    GPIOPinConfigure(GPIO_PB7_M0PWM1);
    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6 | GPIO_PIN_7 );


    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);  

    //Set the Period (expressed in clock ticks)
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 65000);

    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,65000*0.2);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1,650*20);

    // Enable the PWM generator
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);

    // Turn on the Output pins
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT | PWM_OUT_1_BIT , true);



   /*
   * Configuring the pins
   */
   
  pinMode(RED_LED,OUTPUT);
  pinMode(BLUE_LED,OUTPUT);
  pinMode(GREEN_LED,OUTPUT);
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2,INPUT_PULLUP);


    // Disabling the motors initially
  digitalWrite(M_ENABLE,0);
}

 
void loop() { 
  
}


/*********************************************************************
Function definitions
 */

String get_arg(String cmd, int n){
   /*
   * returns nth argument from the command passed,   
   * n = 1 : is the command itself
   */
  String ret_arg;
  cmd.trim(); // removes trailing whitespaces
  int from = 0;
  for (int i = 0; i < n ; ++i){
    ret_arg = cmd.substring(from, cmd.indexOf(' '));
    cmd = cmd.substring( cmd.indexOf(' '), cmd.length());
    cmd.trim();
  }
  return ret_arg;
}

void IntHandler_TIMER0(){
  /*
   * Interupt Handler for TIMER0
   */
  if (!ledSetForBlink){
    digitalWrite(RED_LED,LOW);
    digitalWrite(GREEN_LED,LOW);
    digitalWrite(BLUE_LED,LOW);
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    return;
  }
  // Implementing software counter with 0.1sec/count 
  sw_counter > 20 ? sw_counter = 0: sw_counter++;
  if (sw_counter == blinkRate){
    if (is_LedOn){
      digitalWrite(RED_LED,LOW);
      digitalWrite(GREEN_LED,LOW);
      digitalWrite(BLUE_LED,LOW);
    }
     
    else{
      ledColorSet & 0x01 ? digitalWrite(RED_LED,HIGH) : digitalWrite(RED_LED,LOW) ;
      ledColorSet & 0x02 ? digitalWrite(GREEN_LED,HIGH): digitalWrite(GREEN_LED,LOW);
      ledColorSet & 0x04 ? digitalWrite(BLUE_LED,HIGH): digitalWrite(BLUE_LED,LOW);
    }
    is_LedOn = !is_LedOn;
    sw_counter = 0;
  }
  
  // After every ISR clear the Interupt flag
  TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

String blinkLed(int br){
  /*
   * This function sets the blinking rate of Led in multiples of 0.1 sec
   */
  String ret_str; 
  if( br == 0 ){
    ledSetForBlink = false;
    ret_str =  "LED turned off"; 
  }
  else if (br <= 20){
    ledSetForBlink = 1;
    blinkRate = br;
    sw_counter = 0;
    ret_str =  "Blink Rate set to " + String(br);
  }
  else
    ret_str =  "Set Blink rate between 0-20";
  return ret_str;
}

String colorLed(int led_color){
  /*
   * This function sets the color of LED from 1-7
   */
  String ret_str; 
  if (led_color > 0 && led_color <= 7){
    ledColorSet = led_color;
    ret_str =  "Led Color set to " + String(led_color);
  }
  else
    ret_str = "Set color between 1-7";
return ret_str;
}


String switchState(int sw){
  /*
   * This function returns the status of the switch
   */
  String ret_str;
  bool sw_1 = digitalRead(SW1);
  bool sw_2 = digitalRead(SW2);
  if (sw == 1 && !sw_1 )
     ret_str = "switch 1 active";
  else if (sw == 2 && !sw_2 )
    ret_str = "switch 2 active";
  else 
    ret_str = "switch " + String(sw) + " is Inactive";
  return ret_str;
}

void tty_puts(char *s)
{ 
   /*
   * Prints the string s to the terminal
   */   
  while (*s != '\0')  // loop until we find the null character
    {
        UARTCharPut(UART0_BASE, *s);
        s++;
    }
  UARTCharPut(UART0_BASE, '\n');

  return;
}

void tty_gets(char *s)
{
   /*
   * gets the input from the shell
   */  
    char ch;

    do
    {
        ch = UARTCharGet(UART0_BASE);
        *s = ch;
        s++;
    }
    while((ch != '\n') && (ch != '\r'));

    *s ='\0';   // null-terminate
}

void IntHandler_UART(){
  // reading commands
  char s[80], ret_str[80];
  tty_gets(s);
  String cmd = s;
  String Command1 = get_arg(cmd, 1);
  //delay(10);
  if (Command1 == "blink") 
    blinkLed((get_arg(cmd,2)).toInt()).toCharArray(ret_str, 80);
  if (Command1 == "switch") 
    switchState((get_arg(cmd,2)).toInt()).toCharArray(ret_str, 80);
  if (Command1 == "color") 
    colorLed((get_arg(cmd,2)).toInt()).toCharArray(ret_str, 80);
  if (Command1 == "motor") 
    run_motor(get_arg(cmd,2),get_arg(cmd,3).toInt()).toCharArray(ret_str, 80);
  if (Command1 == "refArray") 
    Ref_array().toCharArray(ret_str, 80);
 
  tty_puts( ret_str );
  UARTIntClear(UART0_BASE,  UART_INT_RX);
}


String run_motor(String id, int spd){
  /*
   * This function runs the motor according to set speed
   */
  String ret_str;
  if (spd == 0)
  {
    if(id == "L")
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,spd*650);
    if(id == "R")
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1,spd*650);
    digitalWrite(M_ENABLE,0);
    ret_str = "Motor shutdown";    
  }
  else if (spd >=20 && spd <=100)
  {
    if(id == "L")
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,spd*650);
    if(id == "R")
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1,spd*650);
    digitalWrite(M_ENABLE,1);
    ret_str = "Motor " + String(id) + " is set to speed " + String(spd);   
  }
  else
  ret_str = "Set speed between 0 and 100";
return ret_str;
}

String Ref_array(){
  /*
   * This function returns the status reading of the reflectance array
   */
  String ret_str, ref_read = "";
  for (int i = 0; i<=7 ; ++i)
  {
    ref_read = ref_read + String(digitalRead(ARR(i)));
  }
  ret_str = "The reflectance array reading is " + ref_read;
  
  return ret_str;
}
