#include "functions.h"
#include "colors.h"
#include <stdlib.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Timer Output Compare Configuration Structure declaration */

/* Peripherals initialized states */

/* Interrupt flags */
volatile bool touchChanged = false;
volatile bool radioEvent = false;
volatile bool reached = false; // used by position control
bool atDestination = false; // true if stopped moving
uint8_t atDestCounter = 0;

bool positionSent = true;
uint8_t currentTouch = 0;

Target currentGoal = {500, 500, 0, true, true};

Motor motorValues = {0, 0, 1.0f, 22, MAX_SPEED, 60};
//Motor motorValues = {0, 0, 1.0f, 15, 25,40};

bool isPositionControl = true;
float targetAngle = 0.0f;
volatile CHARGING_STATE_t chargingStatus = DISCONNECTED;

volatile message_tx_t         message_tx;
volatile message_tx_success_t message_tx_success;
volatile message_rx_t         message_rx;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*============================================================================
Name    :   upateRobot
------------------------------------------------------------------------------
Purpose :   updates the position and orientation of the robot and prepare the
            message to be sent
Input   :
Output  :
Return	:
Notes   :
============================================================================*/
void initRobot() {
  HAL_Init();
  SystemClock_Config();

  initRGBLed();

  initMotors();

  if (!initSensors())
    Error_Handler();

  if (!initRadio())
    Error_Handler();

  initPhotoDiodes();

  //while(calibrate(&motorValues) == false);
  chargingStatus = (HAL_GPIO_ReadPin(CHARGING_STATUS_GPIO_PORT, CHARGING_STATUS_PIN) == LOW) ? CHARGING : CHARGED;
}

/*============================================================================
Name    :   upateRobot
------------------------------------------------------------------------------
Purpose :   updates the position and orientation of the robot and prepare the
            message to be sent
Input   :
Output  :
Return	:
Notes   :
============================================================================*/
void updateRobot() {
  if(atDestCounter>=3){
    atDestination = true;
    setMotor1(0);
    setMotor2(0);
    //setRedLed(5); //UNCOMMENT TO GET A VISUAL FEEDBACK WHEN THE ROBOT REACHS ITS DESTINATION
    setGreenLed(0);
  }
  if (updateRobotPosition()) {
    setBlueLed(0);
    if (isPositionControl)
    {
      positionControl(currentGoal.x, currentGoal.y, currentGoal.angle, &motorValues, (bool*)&reached, false, currentGoal.finalGoal, currentGoal.ignoreOrientation);
      if(reached){
        setMotor1(0);
        setMotor2(0);
        atDestCounter++;
      }
      else{
        setRedLed(0);
        atDestination = false;
        atDestCounter = 0;
        minimumc(&motorValues.motor1, motorValues.minVelocity);
        minimumc(&motorValues.motor2, motorValues.minVelocity);
        maximumc(&motorValues.motor1, motorValues.maxVelocity);
        maximumc(&motorValues.motor2, motorValues.maxVelocity);

        setMotor1(motorValues.motor1);
        setMotor2(motorValues.motor2);
      }
    }

    prepareMessageToSend(getRobotPosition(), getRobotAngle(), &currentTouch, &atDestination, getBatteryLevel());
  }
  if(isBlinded())
  {
        atDestination = true;
        setBlueLed(5);
        setMotor1(0);
        setMotor2(0);
  }
}

/*============================================================================
Name    :   sleep
------------------------------------------------------------------------------
Purpose :   Sleeping procedure (TODO)
Input   :
Output  :
Return	:
Notes   :
============================================================================*/
void sleep() {
  SleepMode_Measure();
}

void delay(uint16_t ms) {
  for(volatile uint16_t j = 0; j < ms; ++j) {
    for(volatile uint16_t i = 0; i < 1000; ++i);
    //delayMicroseconds(ms * 1000);
  }
}

/*============================================================================
Name    :   checkRadio
------------------------------------------------------------------------------
Purpose :   acquires new incoming data from the radio
Input   :
Output  :
Return	:
Notes   :
============================================================================*/
void checkRadio() {
  if (radioEvent) {
    bool tx, fail, rx;
    whatHappened(&tx, &fail, &rx);
    clearInterruptFlag(rx, tx, fail);
    radioEvent = false;
    if (rx)
      handleIncomingRadioMessage();
    if (fail) {
      flush_tx();
      handleOutgoingRadioMessage();
    }
    if (tx) {
      message_tx_success();
      handleOutgoingRadioMessage();
    }
  }
}

/*============================================================================
Name    :   handleIncomingMessage
------------------------------------------------------------------------------
Purpose :   acquires new incoming data from the radio
Input   :
Output  :
Return	:
Notes   :
============================================================================*/
void handleIncomingRadioMessage() {
  // Position tmpPosition = {0, 0};
  // uint64_t tmpPipeAddress = 0;
  // PositionControlMessage* positionMessage;
  uint8_t payloadSize = getDynamicPayloadSize();
  if (payloadSize > PAYLOAD_MAX_SIZE)
    flush_rx();
  else {
    Message msg;
    memset(&msg, 0, sizeof(msg));
    readRadio((uint8_t *)&msg, payloadSize);
    if (msg.header.id == RECEIVER_ID) {
      switch (msg.header.type) {
      case TYPE_UPDATE:
        break;
      case TYPE_MOTORS_VELOCITY:
        // isPositionControl = false;
        // setMotor1((int8_t)msg.payload[0]);
        // setMotor2((int8_t)msg.payload[1]);
        // setRGBLed(msg.payload[2] / 8, msg.payload[3] / 8, msg.payload[4] / 8);
        // prepareMessageToSend(getRobotPosition(), getRobotAngle(), &currentTouch, &atDestination, getBatteryLevel());
        break;
      case TYPE_ROBOT_POSITION:
        // isPositionControl = true;
        // positionMessage = (PositionControlMessage*)msg.payload;
        // if(currentGoal.x != positionMessage->positionX || currentGoal.y != positionMessage->positionY){
        //   currentGoal.x = positionMessage->positionX;
        //   currentGoal.y = positionMessage->positionY;

        //   reached = false;
        //   atDestination = false;
        //   setRedLed(0);
        //   atDestCounter = 0;
        // }
        // //setRGBLed(positionMessage->colorRed/8, positionMessage->colorGreen/8, positionMessage->colorBlue/8);

        // motorValues.preferredVelocity = positionMessage->preferredSpeed;

        // if(positionMessage->preferredSpeed == 0){
        //   setMotor1(0);
        //   setMotor2(0);
        //   isPositionControl = false;
        // }

        // currentGoal.angle = ((float)positionMessage->orientation)/100.0f;
        // currentGoal.ignoreOrientation = positionMessage->ignoreOrientation;

        // if(motorValues.preferredVelocity > MAX_SPEED)
        //   motorValues.preferredVelocity = MAX_SPEED;
        // if(motorValues.preferredVelocity < -MAX_SPEED)
        //   motorValues.preferredVelocity = -MAX_SPEED;
        //  // Required ??
        // if(motorValues.preferredVelocity < motorValues.minVelocity)
        //   motorValues.preferredVelocity = motorValues.minVelocity;

        // currentGoal.finalGoal = (bool)positionMessage->isFinalGoal;
        // prepareMessageToSend(getRobotPosition(), getRobotAngle(), &currentTouch, &atDestination, getBatteryLevel());
        break;

      case TYPE_REBOOT_ROBOT:
        Reboot();
        break;
      case TYPE_BBZ_MESSAGE:
        // TODO Calculate the distance
        message_rx(&msg, 0);
        break;
      default:
        break;
      }
    }
  }
}

void handleOutgoingRadioMessage(void) {
  Message* msg = message_tx();
  if (msg != NULL) {
    writeAckPayload(0, (uint8_t *)&msg, sizeof(Header) + sizeof(Position) + 9);
  }
}

/*============================================================================
Name    :   prepareMessageToSend
------------------------------------------------------------------------------
Purpose :   prepares the new position message and stacks it as
            a new ack packet
Input   :   *position : pointer to the robot's current position
            *orientation : pointer to the robot's current orientation
            which pin has triggered the interrupt
Output  :
Return	:
Notes   :
============================================================================*/
void prepareMessageToSend(Position *position, float *orientation, uint8_t *touch, bool *destination, uint16_t batteryLevel) {
  //setRedLed(0);
  int16_t tmpOrientation = 0;
  Message msg;
  msg.header.id = getRobotId();
  msg.header.type = TYPE_STATUS;

  if (position && orientation && touch) {
    tmpOrientation = (int16_t)(*orientation * 100.0f);

    memcpy_fast(msg.payload, (uint8_t *)position, sizeof(*position));
    memcpy_fast(msg.payload + sizeof(*position), (uint8_t *)&tmpOrientation, sizeof(tmpOrientation));
    memcpy_fast(msg.payload + sizeof(*position) + sizeof(tmpOrientation), touch, sizeof(*touch));
    memcpy_fast(msg.payload + sizeof(*position) + sizeof(tmpOrientation) + sizeof(*touch), destination, sizeof(*destination));
    //            memcpy_fast(msg.payload+sizeof(*position)+sizeof(*tmpOrientation)+sizeof(*touch), &photoDiodesPositions[0], 2*sizeof(Position));
    memcpy_fast(msg.payload + sizeof(*position) + sizeof(tmpOrientation) + sizeof(*touch) + sizeof(*destination), &batteryLevel, sizeof(batteryLevel));
    writeAckPayload(0, (uint8_t *)&msg, sizeof(Header) + sizeof(*position) + sizeof(tmpOrientation) + sizeof(*touch) + sizeof(*destination) + sizeof(batteryLevel)); // + 2*sizeof(Position));

  }
}

/*============================================================================
Name    :   checkTouch
------------------------------------------------------------------------------
Purpose :   acquires touch information when a new event happened and then
            stores it
Input   :
Output  :
Return	:
Notes   :
============================================================================*/
void checkTouch() {
  // if a new touch happened
  if (touchChanged) {
    currentTouch = readQTKeyStatus();
    if (HAL_GPIO_ReadPin(TOUCH_CHANGE_GPIO_PORT, TOUCH_CHANGE_PIN) == HIGH) {
      touchChanged = false;
    }
  }
}

/*============================================================================
Name    :   switchToChargingMode
------------------------------------------------------------------------------
Purpose :   put the robot into sleep mode while charging
Input   :
Output  :
Return	:
Notes   :
============================================================================*/
void switchToChargingMode() {
  setGreenLed(0);
  setBlueLed(0);
  powerDown();
  setMotor1(0);
  setMotor2(0);

  while (chargingStatus == CHARGING) {
    setRedLed(5);
    //glowRedLed();
  }

  powerUp();
}

void __errno() {}

/*============================================================================
Name    :   HAL_GPIO_EXTI_Callback
------------------------------------------------------------------------------
Purpose :   Handles the GPIO interrupt routines
Input   :   GPIO_Pin
            which pin has triggered the interrupt
Output  :
Return	:
Notes   :
============================================================================*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  //Touch change IRQ
  if (GPIO_Pin & TOUCH_CHANGE_PIN)
    touchChanged = true;

  if (GPIO_Pin & PHOTODIODE_1_PIN)
    readPhotoDiode(0);
  //
  if (GPIO_Pin & PHOTODIODE_2_PIN)
    readPhotoDiode(1);

  //charger IRQ
  if ((GPIO_Pin & CHARGING_STATUS_PIN) > 0) {
    chargingStatus = (HAL_GPIO_ReadPin(CHARGING_STATUS_GPIO_PORT, CHARGING_STATUS_PIN) == LOW) ? CHARGING : CHARGED;
    if (chargingStatus == CHARGING)
      switchToChargingMode();
  }

  //nRF IRQ
  if ((GPIO_Pin & RADIO_IRQ_PIN) > 0)
    radioEvent = true;

  //IMU IRQ
  //    if(GPIO_Pin && IMU_INT_PIN);
}