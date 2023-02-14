## Shell Implementation
Implemention of Shell ( UART ) to control the resources present on the ```TivaC``` Hardware.
Interupts and other best coding practices are implemented so that the hardware is responsive and
reliable at all times. ```PuTTY``` is used for the CLI Interface.  
  
  Follwing command are implemented.
  ```commands
    blink   <blink_rate..[0-20]>                  
    switch  <switch_id..[1,2]>                
    color   <Color_val..[1-7]>                
    motor   <motor_id..[L,R]> <motor_speed..[20-100]>    
  ```
  Motor Connection to board

```connection
L_PWM     15  
L_DIR     13
M_ENABLE  19 
R_PWM     14
R_DIR     18
```
