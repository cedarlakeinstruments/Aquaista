// Project sponsor: Aquaista
// Email: Aquaista@gmail.com
// Creator: Cedar Lake Instruments LLC
// Date: March, 2018
//
// Description:
// Transmit wireless relay status from 434 MHz transmitter
// Receive message and toggle relay using H-bridge
// 
//

#include <VirtualWire.h>

//*************************************** I/O Connections:
// Arduino - I/O description 
// 2 - Pushbutton input
// 3 - Relay control output
// 4 - Bridge direction output
// 5 - Transmit pin
// 6 - Receive pin
// 7 - LED color 1 output
// 8 - LED color 2 output

// Constant definitions
#define PUSHBUTTON_PIN 2
#define B2_PIN 3
#define B1_PIN 4
#define TX_PIN 5
#define RX_PIN 6
#define LED1_PIN 12
#define LED2_PIN 13
#define COMM_RATE 1000
#define ASCII0 48

//***************************** USER ADJUSTMENTS **********************************
#define RELAY_SWITCH_DELAY 250
#define ENABLE HIGH
#define DISABLE LOW

// Comment out this line to build receiver code
//#define TRANSMIT 1

//**********************************************************************************


void setup() 
{
    pinMode(B2_PIN, OUTPUT);
    pinMode(B1_PIN, OUTPUT);
    pinMode(LED1_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);
    pinMode(PUSHBUTTON_PIN, INPUT_PULLUP);
    pinMode(RX_PIN, INPUT);
    pinMode(TX_PIN, OUTPUT);

    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    // Relay coil off
    digitalWrite(B2_PIN, DISABLE);
    digitalWrite(B1_PIN, DISABLE);
    
    Serial.begin(9600);

#ifdef TRANSMIT
    Serial.println("TX Ready");
    setupTx();
#else
    Serial.println("RX Ready");
    setupRx();
#endif            

   for (int i=0; i < 3; i++)
   {
        digitalWrite(LED2_PIN, HIGH);
        delay(50);
        digitalWrite(LED2_PIN, LOW);
        delay(150);
   }
}

void loop() 
{
#ifdef TRANSMIT
    transmitFx();
#else
    receiveFx();
#endif
}

// Configure receiver
void setupRx()
{
    vw_set_ptt_inverted(true); 
    vw_set_rx_pin(RX_PIN);         
    // Bits per sec
    vw_setup(COMM_RATE);      
    vw_rx_start();               
}

// Receiver behavior
void receiveFx()
{
  //Set buffer array based on max message length
  uint8_t buf[VW_MAX_MESSAGE_LEN];

  //Set variable to indicate buffer length
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  if (vw_get_message(buf, &buflen)) 
  {
    // Message with a good checksum received, dump it.
    Serial.println("Received message");
    
    // Validate input
    if(buf[0] == 'B' && buf[2] == 'E')
    {
        Serial.println("Valid input");
        int relayState = buf[1] - ASCII0;
        Serial.println(relayState);
        if (relayState == 0)
        {
            digitalWrite(B1_PIN, DISABLE);
            digitalWrite(B2_PIN, ENABLE);
            SetLedColor(relayState);
        }
        else if (relayState == 1)
        {
            digitalWrite(B1_PIN, ENABLE);
            digitalWrite(B2_PIN, DISABLE);
            SetLedColor(relayState);
        }

        // Wait for relay state change
        delay (RELAY_SWITCH_DELAY);
        // Relay coil off
        digitalWrite(B2_PIN, DISABLE);
        digitalWrite(B1_PIN, DISABLE);
    }   
  }    
}

// Configure transmitter
void setupTx()
{
  // Required for DR3100
  vw_set_ptt_inverted(true); 
  vw_set_tx_pin(TX_PIN); 
  // Bits per sec
  vw_setup(COMM_RATE);         
}

// Transmitter behavior
void transmitFx()
{
    static byte state = 0;
    if (WaitForHighToLow(PUSHBUTTON_PIN, 30))
    {    
        state++;
        state &= 1;
        // Null-terminate the strings
        int8_t msg[] = {'B','*','E',0};
        msg[1] = state+ASCII0;
        
        // Indicate LED state
        SetLedColor(state);
        
        // send message
        vw_send((uint8_t *)msg, 4);
        vw_wait_tx();     
    }
}

//
// Sets LED color depending on the relay state
void SetLedColor(byte state)
{
    if (state == 0)
    {
        digitalWrite(LED1_PIN, HIGH);
        digitalWrite(LED2_PIN, LOW);
    }
    else
    {
        digitalWrite(LED1_PIN, LOW);
        digitalWrite(LED2_PIN, HIGH);
    }
}

///
// Hangs until a HIGH -> LOW transition is seen
// on the input parameter
///
bool WaitForHighToLow(int input, int debounce)
{
    if (digitalRead(input) == LOW || debounce <= 0)
    {
        Serial.write("Early fail on WaitForHighToLow\n");
        return false;
    }
    
    while (digitalRead(input) == HIGH || debounce > 0)
    {
        if (debounce > 0)
        {
          debounce--;
          delay(1);
        }
    }
    if (debounce == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

