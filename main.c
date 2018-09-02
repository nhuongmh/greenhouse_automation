#include <main.h>
//version1.0  , just make it works, not optimized (pointer, struct, ...)




#define     TURN_ON             1
#define     TURN_OFF            0
#define     RELAY_P        PIN_B6   //PUMP
#define     RELAY_L        PIN_A2   //LIGHT
//#define     LIGHT_SDA      PIN_A4   // need to rename
//#define     LIGHT_SCL      PIN_A5   // need to rename
#define     DHT11_PIN       PIN_A3
#define     LM35_OUT       PIN_A0
#define     LED_1          PIN_B7

#define     BUTTON1        PIN_E2
#define     BUTTON2        PIN_A5
#define     BUTTON3        PIN_E0
#define     BUTTON4        PIN_D1
#define     BUTTON_EXT     PIN_B0

#define     LCD_ENABLE_PIN PIN_D5
#define     LCD_RS_PIN     PIN_D7
#define     LCD_RW_PIN     PIN_D6
#define     LCD_DATA4      PIN_D4
#define     LCD_DATA5      PIN_C7
#define     LCD_DATA6      PIN_C6
#define     LCD_DATA7      PIN_C5


#include <lcd.c>
#include <BH1750.c>

//LCD
//LM35
//BH1750
//Button
//timer, relay

//


unsigned int8 u4sec = 0;
unsigned int8 u1min = 0;
//enum pump_mode {MODE1, MODE2, MODE3};
typedef  struct   {
      unsigned int8  time_on;
      unsigned int8  time_off;
      int1  state;
} pumpMode;

void system_init(void);
void main_display(void);
void update_main(void);
void display_int1(unsigned int8 value);
void display_int2(unsigned int8 value);
void display_int3(unsigned int16 value);
void display_mode3(int1 oo, unsigned int8 min);
void display_int4(unsigned int16 value);
void set_mode3();
unsigned int8 get_temperature(void);
unsigned int16 get_lux(void);
void get_humi(void);
void set_mode(void);
void start_signal();
int1 check_response();
int1 Read_Data(int8 *dht_data);
unsigned int8 get_hdata();



pumpMode      mode, mode1, mode2, mode3;
int1        mode4 = 0;
int8        x= 8;
int1        flag_update = 0;
int1        flag_setmode = 0;
int1        flag_humi = 0;


#INT_EXT
void  EXT_isr(void) 
{
   flag_setmode =1;
   while(!input(BUTTON_EXT));
}

////////////////////////////////////////////////////////////////////////////////
//

#INT_TIMER1
void  TIMER1_isr(void) 
{


u4sec++;
flag_update = ~flag_update;
flag_humi = ~flag_humi;
if(u4sec==30)
{
   u1min++;
   if(!mode4)
   {
      if(u1min==mode.time_on && mode.state == TURN_ON)
      {
      mode.state = TURN_OFF;
      u1min = 0;
      output_low(RELAY_P);
      }
      else if(u1min==mode.time_off && mode.state ==TURN_OFF)
      {
      mode.state = TURN_ON;
      u1min = 0;
      output_high(RELAY_P);
      }
    }
   
   u4sec = 0;
   
}
}

////////////////////////////////////////////////////////////////////////////////
//

void system_init()
{
   setup_adc(ADC_CLOCK_INTERNAL);
   setup_adc_ports(AN0);
   set_adc_channel(0);
   delay_ms(100);
   BH1750_init();
   setup_timer_1(T1_EXTERNAL|T1_DIV_BY_1|T1_CLK_OUT);      //2.0 ms overflow
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_4| RTCC_8_bit);      //204 us overflow
   //enable_interrupts(INT_EXT);
   enable_interrupts(INT_TIMER1);
   lcd_init();
   mode1.time_on  =  5;
   mode1.time_off =  5;
   mode1.state    =  TURN_OFF;
   
   mode2.time_on  = 2;
   mode2.time_off = 8;
   mode2.state = TURN_OFF;
   
   mode3.time_on  = 1;
   mode3.time_off = 1;
   mode3.time_off = TURN_OFF;
   
   mode  = mode1;
}

////////////////////////////////////////////////////////////////////////////////
//

void main_display(void)
{
  
   lcd_putc('\f');
   lcd_gotoxy(1,1);
   display_int2(0);
   lcd_putc(223);
   lcd_putc("C");

   lcd_gotoxy(9,1);
   display_int4(0);
   lcd_putc(" lux");
  
   lcd_gotoxy(1,2);
   display_int2(0);
   lcd_putc("%");
   
   if(!mode4)
   {
   if((mode.time_on>9) || (mode.time_off>9))
      {
      x = 8;
      lcd_gotoxy(x,2);
      display_int2(u1min);
      lcd_putc("(");
      display_int2(mode.time_on);
      lcd_putc("/");
      display_int2(mode.time_off);
      lcd_putc(")");
      }
   else
     {
     x = 11;
     lcd_gotoxy(x,2);
      display_int1(u1min);
      lcd_putc("(");
      display_int1(mode.time_on);
      lcd_putc("/");
      display_int1(mode.time_off);
      lcd_putc(")");
     }
   }
   else
      {
      x = 10;
      lcd_gotoxy(x,2);
      display_int2(u1min);
      lcd_putc("(-/-)");
      }
   
}
////////////////////////////////////////////////////////////////////////////////
//
void update_display(void)
{
   unsigned int8  temperature  = 30;
   unsigned int16 lux         = 1000;
   //unsigned int8  humi        = 80;
   
   temperature = get_temperature();
   lux = get_lux_value(cont_H_res_mode1,180);
   //humi = get_hdata();
   //temperature++;
   //lux++;
   lcd_gotoxy(1,1);
   display_int2(temperature);
   
   lcd_gotoxy(9,1);
   display_int4(lux);
   //display_int4(temperature);


   
   lcd_gotoxy(x,2);
   if(mode4 || mode.time_on>9 || mode.time_off >9)
   display_int2(u1min);
   else
   display_int1(u1min);
      
   
   
}
////////////////////////////////////////////////////////////////////////////////
//
void display_mode(void)
{
   lcd_putc('\f');
   lcd_putc("1.(5/5)");
   lcd_gotoxy(10,1);
   lcd_putc("2.(2/8)");
   lcd_gotoxy(1,2);
   lcd_putc("3.(x/y)");
   lcd_gotoxy(13,2);
   lcd_putc("4.SW");
}

////////////////////////////////////////////////////////////////////////////////
//

void get_mode(void)
{

  // unsigned int8 input_buttons;
   int1  done_flag = FALSE;
   //input_buttons = input_buttons & 0x1F;
   while(!input(BUTTON_EXT));
   display_mode();
   while(!done_flag)          
   {
   if(!input(BUTTON_EXT))
         {
         done_flag = TRUE;
         while(!input(BUTTON_EXT));
         }
   else if(!input(BUTTON1))
         {
         mode1.state = TURN_ON;
         output_high(RELAY_P);
         mode = mode1;
         done_flag = TRUE;
         mode4 = 0;
         u1min= 0;
         u4sec =0;
         }
    else if(!input(BUTTON2))
         {
         mode2.state = TURN_ON;
         output_high(RELAY_P);
          mode = mode2;
          done_flag = TRUE;
          mode4=0;
          u1min= 0;
          u4sec =0;
         }
     else if(!input(BUTTON3))
         { 
         set_mode3();
         mode3.state = TURN_ON;
         output_high(RELAY_P);
         mode = mode3;
         //output_high(LED_1);
         done_flag = TRUE;
         mode4=0;
         u1min= 0;
         u4sec =0;
         }
      else if(!input(BUTTON4))
         {
         mode4    = 1;
         done_flag = TRUE;
         }
       else done_flag = FALSE;
   }
   main_display();
   
}
////////////////////////////////////////////////////////////////////////////////
//
void set_mode3(void)
{
   lcd_putc('\f');
   
   //set time on
   lcd_putc("time on: ");
   display_mode3(1,mode3.time_on);
   while (input(BUTTON_EXT))
   {
      if(!input(BUTTON1))
         {
         if(mode3.time_on<99)
            {(mode3.time_on)++;
            display_mode3(1,mode3.time_on);
            }
          while(!input(BUTTON1));
         }
      if(!input(BUTTON3))
         {
         if(mode3.time_on>1)
            {
            (mode3.time_on) --;
            display_mode3(1,mode3.time_on);
            }
            while(!input(BUTTON3));
         }
   }
   while(!input(BUTTON_EXT));
      //set time off
   lcd_gotoxy(1,2);
   lcd_putc("time off:");
   display_mode3(0,mode3.time_off);
   while (input(BUTTON_EXT))
   {
      if(!input(BUTTON1))
         {
         if(mode3.time_off<99)
            {mode3.time_off++;
            display_mode3(0,mode3.time_off);
            }
            while(!input(BUTTON1));
         }
      if(!input(BUTTON3))
         {
         if(mode3.time_off>1)
            {
            mode3.time_off--;
            display_mode3(0,mode3.time_off);
            }
            while(!input(BUTTON3));
         }
   }
      while(!input(BUTTON_EXT));
}

////////////////////////////////////////////////////////////////////////////////
//
void display_mode3(int1 oo, unsigned int8 min)
{

   if(oo)   //oo=1 -> set_time_on, vice versa
   {
      lcd_gotoxy(11,1);    //is it need to be cleared?
      display_int2(min);
   }
   else 
   {
      lcd_gotoxy(11,2);
      display_int2(min);
   }
}
////////////////////////////////////////////////////////////////////////////////
//
void display_int1(unsigned int8 value)
{
   unsigned int8  local_value= 0;
   if(value > 9)
      {
         if(value>99)
            local_value = (value %100)%10;
         else
            local_value = value % 10;
      }
   else
      local_value = value;
   
   lcd_putc(local_value+48);
}
void display_int2(unsigned int8 value)
{  
   unsigned int8  local_value_unit = 0;
   unsigned int8  local_value_tens = 0;
   unsigned int8  modulo100 = 0;
   if(value >99)
   {
   modulo100       = value %100;
   local_value_tens = modulo100/10;
   local_value_unit = modulo100 % 10;
   }
   else
   {
      local_value_tens = value / 10;
      local_value_unit = value % 10;
   }
   lcd_putc(local_value_tens+48);
   lcd_putc(local_value_unit+48);
   
}
void display_int3(unsigned int16 value)
{  
   unsigned int8  local_value_unit= 0;
   unsigned int8  local_value_tens= 0;
   unsigned int8  local_value_hund= 0;
   unsigned int16  modulo1= 0;
   unsigned int8  modulo2 =0;
   if(value >999)
   {
   modulo1       = value % 1000;
   local_value_hund = modulo1/100;
   modulo2        = modulo1 % 100;
   local_value_tens = modulo2/10;
   local_value_unit = modulo2 % 10;
   }
   else
   {
      local_value_hund =value/100;
      modulo2          = value % 100;
      local_value_tens = modulo2 / 10;
      local_value_unit = modulo2 % 10;
   }
   if(local_value_hund==0)
      local_value_hund = ' ';
   else
      local_value_hund += 48;
   lcd_putc(local_value_hund);
   lcd_putc(local_value_tens+48);
   lcd_putc(local_value_unit+48);
   
}
void display_int4(unsigned int16 value)
{  

   unsigned int8  local_value_unit = 0;
   unsigned int8  local_value_tens= 0;
   unsigned int8  local_value_hund = 0;
   unsigned int8  local_value_thou = 0;
   unsigned int16 modulo0 = 0;
   unsigned int16  modulo1 = 0;
   unsigned int8  modulo2 = 0;
   if(value >9999)
      modulo0       = value % 10000;
   else
      modulo0       =  value;
   
   local_value_thou = modulo0/1000;
   modulo1          = modulo0 % 1000; 
   local_value_hund = modulo1/100;
   modulo2        = modulo1 % 100;
   local_value_tens = modulo2/10;
   local_value_unit = modulo2 % 10;
   
   if(local_value_thou ==0)
      local_value_thou = ' ';
   else 
      local_value_thou+=48;
      
    local_value_hund+=48;
    local_value_tens+=48;
    local_value_unit+=48;
   
   lcd_putc(local_value_thou);
   lcd_putc(local_value_hund);
   lcd_putc(local_value_tens);
   lcd_putc(local_value_unit);
   
}

////////////////////////////////////////////////////////////////////////////////
//
unsigned int8 get_temperature(void)
{
  unsigned int16 adc_value;
  adc_value = read_adc()*0.489;
  if(adc_value>99)
   adc_value = 99;
  return adc_value;
}

void get_humi (void)
{
   unsigned int8 huminity;
   huminity = get_hdata();
   if(huminity>99)
      huminity = 99;
   if(huminity>1)
   {
   lcd_gotoxy(1,2);
   display_int2(huminity);
   }
    
}
////////////////////////////////////////////////////////////////////////////////
// DHT11
void start_signal(){
  output_drive(DHT11_PIN);                            // Configure connection pin as output
  output_low(DHT11_PIN);                              // Connection pin output low
  delay_ms(25);                                       // Wait 25 ms
  output_high(DHT11_PIN);                             // Connection pin output high
  delay_us(30);                                       // Wait 30 us
  output_float(DHT11_PIN);  
}
int1 check_response(){
  while(!input(DHT11_PIN) && get_timer0() < 125);     // Wait until DHT11_PIN becomes high (cheking of 80µs low time response)
  if(get_timer0() >= 125)                             // If response time >= 100µS  ==> Response error
    return 0;                                         // Return 0 (Device has a problem with response)
  else {
    set_timer0(0);                                    // Set Timer1 value to 0
    while(input(DHT11_PIN) && get_timer0() < 125);    // Wait until DHT11_PIN becomes low (cheking of 80µs high time response)
    if(get_timer0() >= 125)                           // If response time >= 100µS  ==> Response error
      return 0;                                       // Return 0 (Device has a problem with response)
    else
      return 1; 
}
}
int1 Read_Data(int8 *dht_data) {
  int8 j;
  *dht_data = 0;
  for(j = 0; j < 8; j++){
    set_timer0(0);                                    // Reset Timer1
    while(!input(DHT11_PIN))                          // Wait until DHT11_PIN becomes high
      if(get_timer0() >= 125) {                       // If low time >= 100µs  ==>  Time out error (Normally it takes 50µs)
        return 1;
      }
    set_timer0(0);                                    // Reset Timer1
    while(input(DHT11_PIN))                           // Wait until DHT11_PIN becomes low
      if(get_timer0() > 125) {                        // If high time > 100µs  ==>  Time out error (Normally it takes 26-28µs for 0 and 70µs for 1)
        return 1;                                     // Return 1 (timeout error)
      }
     if(get_timer0() > 62)                            // If high time > 50µS  ==>  Sensor sent 1
       bit_set(*dht_data, (7 - j));                   // Set bit (7 - j)
  }
 
  return 0;                                           // Return 0 (data read OK)
}
unsigned int8 get_hdata()
   {
   unsigned int8 T_byte1, T_byte2, RH_byte1, RH_byte2, CheckSum ;
    //delay_ms(1000);
    start_signal();
    
    if(check_response()) {         // Check if there is a response from sensor (If OK start reding humidity and temperature data)
        // Response OK ==> read (and save) data from the DHT11 sensor and check time out errors
        Read_Data(&RH_Byte1);      // Read humidity 1st byte and store its value in the variable RH_Byte1
        Read_Data(&RH_Byte2);      // Read humidity 2nd byte and store its value in the variable RH_Byte2
        Read_Data(&T_Byte1);       // Read temperature 1st byte and store its value in the variable T_Byte1
        Read_Data(&T_Byte2);       // Read temperature 2nd byte and store its value in the variable T_Byte2
        Read_Data(&CheckSum);      // Read checksum and store its value in the variable CheckSum
 
        // Test if all data were sent correctly
        if(CheckSum == ((RH_Byte1 + RH_Byte2 + T_Byte1 + T_Byte2) & 0xFF)) {
          return RH_Byte1;
        }
 
        // Checksum error
        else {
          return 0;
        }
 
    }
 
    // Sensor response error (connection error)
    else {
      return 1;
    }
  //  } else {return 2; }
  }



////////////////////////////////////////////////////////////////////////////////
//

void main()
{

   system_init();
   delay_ms(100);
   enable_interrupts(GLOBAL);
   main_display();
   output_high(LED_1);
   output_low(RELAY_P);
   output_low(RELAY_L);

   while(TRUE)
   {
      if(flag_humi)
      {
       get_humi();
       flag_humi = 0;
      }
      if(flag_update)
         {update_display();
         flag_update = 0;
         output_toggle(LED_1);
         delay_ms(50);
         }
      if(!input(BUTTON_EXT))
         {
         //disable_interrupts(INT_EXT);
         get_mode();
         //output_toggle(LED_1);
         main_display();
         //flag_setmode = 0;
         //enable_interrupts(INT_EXT);
         while(!input(BUTTON_EXT));
         }
         
       if(mode4)
       {
       if(!input(BUTTON4))
       {
         output_toggle(RELAY_P);
        while(!input(BUTTON4));
       }
       }
      
      
        // sleep();
   }

}

