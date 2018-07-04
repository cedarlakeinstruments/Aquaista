// Project sponsor: 
// Email: aquaista@gmail.com
// Creator: Cedar Lake Instruments LLC
// Date: January, 2016
//
// Description:
// Control gear motor with PIR sensor or water sensor
// 1. PiR triggers the gear motor to move forward 0-60 seconds - stop
// 2. PiR triggers the gear motor to move in reverse 0-60 seconds - stop
// 3. 0-60 second pause between each trigger.
// 4. If the water sensor detect water it trigger gear motor to      reverse 0-60 second - stop, 
// 5. If water detected, triggers pezo alarm - 0-60 second -stop.
//


// I/O Connections:
// Arduino pin   - I/O description 
// 2 PiR sensor
// 3,4. liquid sensor
// 5. Piezo alarm - buzzer
// 6. 12vdc mini gear motor phase A
// 7. 12vdc mini gear motor phase B

// Global variables

// Constant definitions
#define IDLE 0
#define FORWARD 1
#define REVERSE 2
#define PAUSE 3


#define PIR 2
#define LIQUID1 3
#define LIQUID2 4
#define ALARM 5
#define MOTOR_A 6
#define MOTOR_B 7


//***************************** USER ADJUSTMENTS **********************************

// Time motor runs
#define MOTOR_TIME 5 * 1000

// Time alarm is on
#define ALARM_TIME 12 * 1000 

// Time between motions
#define PAUSE_TIME 3 * 1000

//**********************************************************************************

// Runs once
void setup() 
{
    pinMode(PIR, INPUT);
    pinMode(LIQUID1, INPUT);
    pinMode(LIQUID2, INPUT);
    pinMode(ALARM, OUTPUT);
    pinMode(MOTOR_A, OUTPUT);
    pinMode(MOTOR_B, OUTPUT);
    
    digitalWrite(MOTOR_A, LOW);
    digitalWrite(MOTOR_B, LOW);
    digitalWrite(ALARM, LOW);
    
    Serial.begin(115200);
    Serial.println("Ready");
}

// Runs continuously
void loop() 
{
    static int state = IDLE;
    static int lastState = FORWARD;
    static unsigned long alarmEndTime = 0;
    static unsigned long motorEndTime = 0;
    
    // Pace loop
    delay(50);
    
    bool pir = digitalRead(PIR);
    bool liquid = liquidDetected();
    Serial.println(state);
    
    switch (state)
    {
        case IDLE:
            digitalWrite(MOTOR_A, LOW);
            digitalWrite(MOTOR_B, LOW);
            if (liquid && lastState != REVERSE)
            {
                state = REVERSE;
                digitalWrite(ALARM, HIGH);
                alarmEndTime = ALARM_TIME + millis();
            }
            else if (pir)
            {
                if (lastState == FORWARD)
                {
                    state = REVERSE;
                }
                else if (lastState == REVERSE)
                {
                    state = FORWARD;
                }
            }
            break;
        case FORWARD:
            digitalWrite(MOTOR_A, HIGH);
            digitalWrite(MOTOR_B, LOW);
            if (motorEndTime == 0)
            {
                motorEndTime = MOTOR_TIME + millis();
            }
            lastState = FORWARD;
            state = PAUSE;
            break;
        case REVERSE:
            digitalWrite(MOTOR_A, LOW);
            digitalWrite(MOTOR_B, HIGH);
            if (motorEndTime == 0)
            {
                motorEndTime = MOTOR_TIME + millis();
            }
            lastState = REVERSE;
            state = PAUSE;
            break;      
        case PAUSE:
            delay(PAUSE_TIME);
            state = IDLE;
            break;      
        default:
            break;
    }
    
    // Update alarm
    if (alarmEndTime > 0)
    {
        if (millis() > alarmEndTime)
        {
            digitalWrite(ALARM, LOW);
            alarmEndTime = 0;
        }
    }
    
    // Update motor
    if (motorEndTime > 0)
    {
        if (millis() > motorEndTime)
        {
            digitalWrite(MOTOR_A, LOW);
            digitalWrite(MOTOR_B, LOW);
            state = PAUSE;
            motorEndTime = 0;
        }
    }
    
}

// Liquid detection by conductivity
// Applies A/C signal to pins to avoid plating
bool liquidDetected()
{        
    static int pinToggle = 0;
    pinToggle++;
    pinToggle &= 1;
    
    if (pinToggle == 0)
    {
        pinMode(LIQUID1, INPUT_PULLUP);
        pinMode(LIQUID2, OUTPUT);
        digitalWrite(LIQUID2, LOW);
        delay(150);
        return !digitalRead(LIQUID1);
    }
    else
    {
        pinMode(LIQUID1, OUTPUT);
        pinMode(LIQUID2, INPUT_PULLUP);
        digitalWrite(LIQUID1, LOW);
        delay(150);
        return !digitalRead(LIQUID2);
    }
}

