/*
 *      Arduino Mega Server for Non-Parallel Hexarotor Pure Force/Torque Control
 *      
 *      written by: Jameson Lee, UNLV Mech. Eng. Intelligent Structures and Control Lab
 *      last edit: 4/12/2017
 * 
 * This code serves as server side controller for an Arduino Mega for control of a non-parallel
 * Hexarotor. The Motor mapping is for a 30 deg cant (rot 1 is negative x cant rot. CCW omega) 
 * replace with  custom preinverted quadrants as needed (mot_map_quadi[9] is the Left/Right Up/Down
 * order of quadrants for the 6x6 inverse motor mapping. The code recieves force/torque messages out
 * of 0-100, 50 being neutral for desired force torque. ex. {50,50,50,50,50,50} is a full message.
 * The controller automatically arms when a message is sent, of this form, however,
 * the platform disarms when a # is read. 
 */

#include <Servo.h>

//Servo pin defines
#define s1 2
#define s2 3
#define s3 4
#define s4 5
#define s5 6
#define s6 7

#define armLED 13       // Indicator for Arm state
#define om_scale 16.5   // scale the square omega valued for mapping to PWM (us) 

// Initialize servo objects
Servo S1;  
Servo S2;
Servo S3;  
Servo S4;
Servo S5;  
Servo S6;

String buff;    // global string buffer for messages

// preinverted motor mapping for non-parallel hexarotor 
float mot_map_quad1[9] = { 0.1667, 0.2500, 0.1667,-0.3333,0.0, 0.1667, 0.1667,-0.2500, 0.1667};
float mot_map_quad2[9] = { 0.1667, 0.2500, 0.1667, 0.3333,0.0,-0.1667, 0.1667,-0.2500, 0.1667};
float mot_map_quad3[9] = { 0.1667, 0.2500, 0.1667,-0.3333,0.0, 0.1667, 0.1667,-0.2500, 0.1667};
float mot_map_quad4[9] = {-0.1667,-0.2500,-0.1667,-0.3333,0.0, 0.1667,-0.1667, 0.2500,-0.1667};

float om[6] = {0,0,0,0,0,0};    // rotor effort to ESC in (us) PWM signal
float F[3] = {0,0,0};           // Desired Force XYZ
float T[3] = {0,0,0};           // Desired Torque XYZ
bool arm = false;               // arm state

void setup(){
  S1.attach(s1);                // attach servo 1 to pin
  S2.attach(s2);                // attach servo 2 to pin
  S3.attach(s3);                // attach servo 3 to pin
  S4.attach(s4);                // attach servo 4 to pin
  S5.attach(s5);                // attach servo 5 to pin  
  S6.attach(s6);                // attach servo 6 to pin
  pinMode(armLED, OUTPUT);      // arm state indicator pin
  Serial.begin(9600);           // begin Serial commuinication
  Serial.println('a');          // flag serial to indicate server up
}
void getSerialData(){
  char a;                       // check char on serial
  if(Serial.available() > 0){   // look for available data on serial
    a = Serial.read();          // set char check from available data
    if(a == '{'){               // if { then message begin for armed, begin F/T update and servo write
      arm = true;                                     // because { message recieved arm state set to true
      buff = Serial.readStringUntil('}');             // we expect a message until } read into buffer
      int v[7] = {-1,0,0,0,0,0,buff.length()};        // vector with index of each comma deliminator of message
      int k = 0;                                      // counter for comma check
      for(int i = 0;i < buff.length(); i++){          // check every index of the message for commas
        if(buff[i] == ','){                           // if you find one
          v[k + 1] = i;                               // set the k + 1 element of v to the comma index
          k++;                                        // increment k
        }
      }                
      for(int i = 0;i < 6;i++){                               // begin filling in F/T data as int from String message 
        int buff_ = 0;                                        // place holder for each individual number 
        for(int k = v[i] + 1; k < v[i + 1]; k++){             // iterate the length from just after a comma to just before the next
          int a = (buff[k] - '0')*pow(10,v[i + 1] - k - 1);   // interpret char as DEC from ASCII and add the numbers
          if(buff[k] != '0') buff_ = buff_ + a;               // if you arent adding 0 go ahead and add to buff_
        }
        if(i < 3){
          F[i] = float(buff_) - 50.0;                         // for the first 3 data points fill Force vector, offset for negatives
        }
        else{
          T[i - 3] = float(buff_) - 50.0;                     // for the last 3 data points fill Torque vector, offset for negatives
        }
        F[2] = F[2];
        if(F[2] < 0) F[2] = 0;                                // Force we can set as positive only
      }
    }                                 // At the end of this conditional a == '{' we have set all desired force torques (-50) <-> (+50)
                                      // except thrust which is now (0) <-> (+50). Equivalent to PX4 and ArduPilot (-1) <-> (1) and (0) <-> (1)
                                      // controller effort
    else if(a == '#') arm = false;    // This is the alternative Conditional in which a '#' was read, then simply set state to disarmed.
  }
}               // end message recieve function

void loop() {                       // begin main loop

  getSerialData();                  // at this point F/T is set along with arm state
  digitalWrite(armLED,arm);         // set indicator LED for arm state

  for(int i = 0; i < 6; i++){
    om[i] = 0;                      // clear PWM vector
  }
  if(arm == true){                  // if armed then F/T is set..
    for(int i = 0; i < 3; i++){     // set all 6 rotors based on desired F/T and motor mapping
      om[0] = om[0] + (mot_map_quad1[i]*F[i] + mot_map_quad2[i]*T[i]);
      om[1] = om[1] + (mot_map_quad1[i + 3]*F[i] + mot_map_quad2[i + 3]*T[i]);
      om[2] = om[2] + (mot_map_quad1[i + 6]*F[i] + mot_map_quad2[i + 6]*T[i]);
      om[3] = om[3] + (mot_map_quad3[i]*F[i] + mot_map_quad4[i]*T[i]);
      om[4] = om[4] + (mot_map_quad3[i + 3]*F[i] + mot_map_quad4[i + 3]*T[i]);
      om[5] = om[5] + (mot_map_quad3[i + 6]*F[i] + mot_map_quad4[i + 6]*T[i]);
    }
    for(int i = 0; i < 6; i++){     // constrain and map from rotor effort to (us) PWM value
      om[i] = map(constrain(om[i],0,om_scale),0,om_scale,1100,2000);
    }
  }
  else{                             // if not armed/ F/T is irrelevant, and write out disarmed PWM value in (us)
    for(int i = 0; i < 6; i++){
      om[i] = 700;
    }
  }
  S1.writeMicroseconds(int(om[0]));                  // sets the servo position 
  S2.writeMicroseconds(int(om[1]));                  // sets the servo position 
  S3.writeMicroseconds(int(om[2]));                  // sets the servo position 
  S4.writeMicroseconds(int(om[3]));                  // sets the servo position 
  S5.writeMicroseconds(int(om[4]));                  // sets the servo position 
  S6.writeMicroseconds(int(om[5]));                  // sets the servo position 
}



