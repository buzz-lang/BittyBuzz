#include "position_control.h"

/*============================================================================
Name    :   Calibrate
------------------------------------------------------------------------------
Purpose :   To calculate the motor gain between the two motors
Input   :   *float motorGain
Output  :   It will replace the motorGain with the calculated one
Return	:   True when done
Notes   :
============================================================================*/
bool calibrate(Motor *motorValues)
{
    static int CalibrationStatus = Pause;
    static int angleDesired = 0;

    static int lastCalStatus = 100;
    static int TimerTime = 0;
    float increment;
    static int alignCounter = 0;
    static bool reached = false;
    static bool aligned = false;
    static float initialangle = 0.0f;
    static float finalangle = 0.0f;
    // static long initialy = 0.0f;
    // static long finaly = 0.0f;
    static bool inLoop = false;
    static Position initialPos = {0, 0};
    static Position initialPos2 = {0, 0};
    static int8_t velocity = 0;


    switch (CalibrationStatus)
    {
    case Pause:
        if (CalibrationStatus != lastCalStatus)
        {
            motorValues->motor1 = 0;
            motorValues->motor2 = 0;

            TimerTime = HAL_GetTick();
            lastCalStatus = CalibrationStatus;
        }
        if (HAL_GetTick() - TimerTime > 1000)
            CalibrationStatus = SetMotor;
        break;

    case StartPosition: //Go to Start Position
        
        if (CalibrationStatus != lastCalStatus)
        {
            
            lastCalStatus = CalibrationStatus;
            reached = false;
        }
        if (reached == false){
          if(updateRobotPosition()){
              positionControl(500, 500, 0.0f, motorValues, &reached, false, true, true);
              minimumc(&(motorValues->motor1), motorValues->minVelocity);
              minimumc(&(motorValues->motor2), motorValues->minVelocity);
              maximumc(&(motorValues->motor1), motorValues->preferredVelocity);
              maximumc(&(motorValues->motor2), motorValues->preferredVelocity);
              setMotor1(motorValues->motor1);
              setMotor2(motorValues->motor2 * motorValues->motorGain);
          }
           
        }
        else
        {
            setMotor1(0);
            setMotor2(0);
            //positionControl(200, 500, motorValues, &reached, false);

            CalibrationStatus = Orient;//Stop
        }
        break;
    /****  FIND MIN VELOCITY *****/
    case SetMotor:
        if(updateRobotPosition()){
          if (CalibrationStatus != lastCalStatus)
          {
              lastCalStatus = CalibrationStatus;
              TimerTime = HAL_GetTick();
              motorValues->motor1 = motorValues->minVelocity;
              motorValues->motor2 = motorValues->minVelocity;
              setMotor1(motorValues->motor1);
              setMotor2(motorValues->motor2);
              initialPos2 = *getRobotPosition();
          }
          if (HAL_GetTick() - TimerTime > 100)
          {
              CalibrationStatus = IncreaseSpeed;
          }
        }
        break;

    case IncreaseSpeed:

        if (CalibrationStatus != lastCalStatus)
        {
            lastCalStatus = CalibrationStatus;
        }
        if(updateRobotPosition()){

          float distance = sqrtf((float)((getRobotPosition()->x - initialPos2.x) * (getRobotPosition()->x - initialPos2.x)) * powf((.8128) / (953.0f - 70.0f), 2) +
                               (float)((getRobotPosition()->y - initialPos2.y) * (getRobotPosition()->y - initialPos2.y)) * powf((0.508) / (790.0f - 232.0f), 2));

          if (distance > 0.01f){//> 0.01f){
              CalibrationStatus = FoundMinVel;
          }
          else
          {
              CalibrationStatus = SetMotor;
              motorValues->minVelocity = motorValues->minVelocity + 1;
          }
        }
        break;

    case FoundMinVel:
        if (CalibrationStatus != lastCalStatus)
        {
            lastCalStatus = CalibrationStatus;
            TimerTime = HAL_GetTick();
        }

        if (HAL_GetTick() - TimerTime > 250)
        {

            CalibrationStatus = Stop; //OrientPrefVel; 
            //CalibrationStatus = StartPosition;
        }
        break;
    /****  FIND MIN VELOCITY END *****/

    
    /****  FIND LEFT RIGHT MOTORS *****/
    /*
    case CheckInverted:
        if (CalibrationStatus != lastCalStatus && updateRobotPosition())
        {
            lastCalStatus = CalibrationStatus;
            TimerTime = HAL_GetTick();
            initialangle = *getRobotAngle();
            if(initialangle < 0){
              initialangle += 360.0f;
            }
            setMotor1(motorValues->minVelocity);
            setMotor2(0);
        }
        // Assuming that it cannot move more than 180 degrees.
        if (HAL_GetTick() - TimerTime > 250 && updateRobotPosition())
        {
            finalangle = *getRobotAngle();

            setMotor1(0);
            setMotor2(0);

            if(finalangle < 0){
              finalangle += 360.0f;
            }

            // Clockwise
            if(finalangle<initialangle || (initialangle < 180.0f && finalangle > 270.0f)){
              motorValues->inverted = false;
            }
            // Anticlockwise
            else if(finalangle>initialangle || (initialangle > 180.0f && finalangle < 90.0f)){
              motorValues->inverted = true;
            }

            CalibrationStatus = Stop;//Stop;
        }
        break;
    */
    /****  FIND LEFT RIGHT MOTORS END *****/
    
    /****  FIND PREFFERED VELOCITY *****/
    case OrientPrefVel:
        if (CalibrationStatus != lastCalStatus)
        {
            if(updateRobotPosition()){
              lastCalStatus = CalibrationStatus;
              aligned = false;
              angleDesired = atan2f((500 - getRobotPosition()->y), (500 - getRobotPosition()->x)) * (180.0f / PI);
            }
        }

        if (aligned == false){
            if(updateRobotPosition()){
              angleControl(angleDesired, motorValues, &aligned, true, 10.0f);
              setMotor1(motorValues->motor1);
              setMotor2(motorValues->motor2);
            }
        }
        else
            CalibrationStatus = SetMotorPrefVel;
        break;
    case SetMotorPrefVel:
        if(updateRobotPosition()){
          if (CalibrationStatus != lastCalStatus)
          {
          
              lastCalStatus = CalibrationStatus;
              TimerTime = HAL_GetTick();
              initialPos2 = *getRobotPosition();

          }
          angleControl(angleDesired, motorValues, &aligned, true, 10.0f);
          motorValues->motor1 += motorValues->preferredVelocity;
          motorValues->motor2 += motorValues->preferredVelocity;
          setMotor1(motorValues->preferredVelocity);
          setMotor2(motorValues->preferredVelocity);
          if (HAL_GetTick() - TimerTime > 300)
          {
              CalibrationStatus = IncreaseSpeedPrefVel;
          }
        }
        break;
    case IncreaseSpeedPrefVel:
        if (CalibrationStatus != lastCalStatus)
        {
            lastCalStatus = CalibrationStatus;
            motorValues->motor1 = 0;
            motorValues->motor2 = 0;
            TimerTime = HAL_GetTick();
        }
        if (HAL_GetTick() - TimerTime > 100)
        {
            if(updateRobotPosition()){
              float distance2 = sqrtf((float)((getRobotPosition()->x - initialPos2.x) * (getRobotPosition()->x - initialPos2.x)) * powf((.8128) / (953.0f - 70.0f), 2) +
                                      (float)((getRobotPosition()->y - initialPos2.y) * (getRobotPosition()->y - initialPos2.y)) * powf((0.508) / (790.0f - 232.0f), 2));

              if (distance2 > 0.11f){
                  CalibrationStatus = FoundPrefVel;

              }
              else
              {
                  CalibrationStatus = OrientPrefVel;
                  motorValues->preferredVelocity = motorValues->preferredVelocity + 1;
              }
            }
        }

        break;
    case FoundPrefVel:
        
        if (CalibrationStatus != lastCalStatus)
        {
            lastCalStatus = CalibrationStatus;
            TimerTime = HAL_GetTick();
        }

        if (HAL_GetTick() - TimerTime > 250)
        {
            CalibrationStatus = Stop;//StartPosition;
        }
        break;
    /****  FIND PREFFERED VELOCITY END *****/

    case Orient: //Orient to face left
        if (CalibrationStatus != lastCalStatus)
        {
            lastCalStatus = CalibrationStatus;
            aligned = false;
            angleDesired = 0;
        }

        if (aligned == false){
          if(updateRobotPosition()){
            angleControl(angleDesired, motorValues, &aligned, true, 10.0f);
            setMotor1(motorValues->motor1);
            setMotor2(motorValues->motor2);
          }
        }
        else{
            CalibrationStatus = GoStraight;
        }

        break;

    case GoStraight: //Let it go
        
        if (CalibrationStatus != lastCalStatus)
        {
           if(updateRobotPosition()){
              initialangle = *getRobotAngle();
              // initialy = getRobotPosition()->y;

              TimerTime = HAL_GetTick();
              lastCalStatus = CalibrationStatus;
              inLoop = true;
              //motorValues->preferredVelocity = 40;
              //motorValues->maxVelocity = 100;
           }
        }
        //increaseVelocity(&velocity,motorValues,1);
        motorValues->motor1 = motorValues->preferredVelocity;//velocity;
        motorValues->motor2 = (motorValues->preferredVelocity *(motorValues->motorGain));//velocity;
        setMotor1(motorValues->motor1);
        setMotor2(motorValues->motor2);
        //positionControl(900,500,0,motorValues, &reached, true, true);
        if (HAL_GetTick() - TimerTime > 500 && inLoop) //1000
        {
            inLoop = false;
            CalibrationStatus = StopBeforeAdjust;//Stop;
        }
        break;

    case StopBeforeAdjust:
        if (CalibrationStatus != lastCalStatus)
        {
            
            motorValues->motor1 = 0;
            motorValues->motor2 = 0;
            setMotor1(motorValues->motor1);
            setMotor2(motorValues->motor2);
            TimerTime = HAL_GetTick();
            lastCalStatus = CalibrationStatus;
        }
        if (HAL_GetTick() - TimerTime > 500){
            CalibrationStatus = Adjust; //Stop;
        }
        break;

    case Adjust:
        if (CalibrationStatus != lastCalStatus)
        {
            lastCalStatus = CalibrationStatus;
        }
        
        if(updateRobotPosition()){
          
          finalangle = *getRobotAngle();
          //finaly = getRobotPosition()->y;
          increment = 0.05f;
          if (abs(finalangle - initialangle) > 10.0f)
          //if (abs(finaly - initialy) > 20)//10)
          {
              if (finalangle > initialangle)
              //if (finaly > initialy)
              {
                  motorValues->motorGain += increment;
              }
              else
              {
                  motorValues->motorGain -= increment;
              }
              CalibrationStatus = StartPosition;
          }
          else
          {
              CalibrationStatus = Stop;//OrientPrefVel;
              setBlueLed(5);
             
          }
        }

        break;
//
//    case GoStraight: //Let it go
//        if (CalibrationStatus != lastCalStatus)
//        {
//            initialangle = *getRobotAngle();
//            initialy = getRobotPosition()->y;
//            motorValues->motor1 = motorValues->preferredVelocity;
//            motorValues->motor2 = motorValues->preferredVelocity;
//
//            TimerTime = HAL_GetTick();
//            lastCalStatus = CalibrationStatus;
//        }
//
//        if (HAL_GetTick() - TimerTime > 500)
//        {
//            CalibrationStatus = StopBeforeAdjust;
//        }
//        break;
//
//    case StopBeforeAdjust:
//        if (CalibrationStatus != lastCalStatus)
//        {
//            motorValues->motor1 = 0;
//            motorValues->motor2 = 0;
//            TimerTime = HAL_GetTick();
//            lastCalStatus = CalibrationStatus;
//        }
//        if (HAL_GetTick() - TimerTime > 500)
//            CalibrationStatus = Adjust;
//        break;
//
//    case Adjust:
//        if (CalibrationStatus != lastCalStatus)
//        {
//            lastCalStatus = CalibrationStatus;
//        }
//        finalangle = *getRobotAngle();
//        finaly = getRobotPosition()->y;
//        increment = 0.05f;
//        //if (abs(finalangle - initialangle) > 10.0f)
//        if (abs(finaly - initialy) > 10)
//        {
//            //if (finalangle > initialangle)
//            if (finaly > initialy)
//            {
//                motorValues->motorGain += increment;
//            }
//            else
//            {
//                motorValues->motorGain -= increment;
//            }
//            CalibrationStatus = StartPosition;
//        }
//        else
//        {
//            CalibrationStatus = OrientPrefVel;
//        }
//
//        break;

    //If not straight, adjust motor gain and print it
    //If it tolerable, print motor gain
    case Stop:
        if (CalibrationStatus != lastCalStatus)
        {
            motorValues->motor1 = 0;
            motorValues->motor2 = 0;
            setMotor1(0);
            setMotor2(0);
            lastCalStatus = CalibrationStatus;
        }
        return true;
        //positionControl(200, 500, 0.0f, motorValues, &reached, false, true);
        break;
    }
    return false;
}

/*============================================================================
Name    :   angleControl
------------------------------------------------------------------------------
Purpose :   To calculate the motor values needed to control the robot to face desired orientation
Input   :   angle_desired in degrees
            motovalue data
Output  :   It will replace the motorvalues with the calculated ones
Return	:   none
Notes   :
============================================================================*/
void angleControl(float angle_desired, Motor *motorValue, bool *aligned, bool forward, float angleThreshold)
{/*
    static float iTerm = 0;
    static uint32_t lastTimestamp = 0;
    float timeDelta = ((float)(HAL_GetTick() - lastTimestamp))/1000.0f;

    lastTimestamp = HAL_GetTick();

    static int counter = 0;
    static float lastAngleDiff = 0.0f;
    static int alignCounter = 0;

    int32_t motor1, motor2;
    float angleDiff = (angle_desired - *getRobotAngle()) * PI / 180.0f;

    //improve that...
    if (forward == true)
        angleDiff = atan2f(sinf(angleDiff), cosf(angleDiff));
    else
    {
        angleDiff = tanf(atan2f(sinf(angleDiff), cosf(angleDiff)));
        maximumf(&angleDiff, 5.0f);
    }
//    float PGain = 20.0f;
//    float DGain = 2.5f;
//    float IGain = 1.0f;
//
//    float pTerm = 0.0f;
//    float dTerm = 0.0f;
//    static float iTerm = 0.0f;
//    pTerm = angleDiff;
//    dTerm = (angleDiff - lastAngleDiff) / timeDelta; //(0.01367f);
//    iTerm += angleDiff / timeDelta;//2.0f * (0.01367f);

    float pTerm = angleDiff;
    float dTerm = (angleDiff - lastAngleDiff) / (0.01367f);
    iTerm = iTerm + (angleDiff + lastAngleDiff) / 2.0f * (0.01367f);

    float PGain = 20.0f;
    float DGain = 1.5f;
    float IGain = 3.0f;

    if (iTerm > 10.0f)
    {
      iTerm = 10.0f;
    }

    if (iTerm > 10.0f) iTerm = 10.0f;
    else if (iTerm < -10.0f) iTerm = -10.0f;

    if (abs(angleDiff * 180.0f / PI) > angleThreshold)
    {
        float value = PGain * pTerm + DGain * dTerm + IGain * iTerm;
        if (angleDiff < 0.0f)
        {
            motor1 = value;
            motor2 = -value;
        }
        else
        {
            motor1 = value;
            motor2 = -value;
        }
    }
    else
    {
        motor1 = 0;
        motor2 = 0;
        alignCounter++;
    }

    lastAngleDiff = angleDiff;
    motorValue->motor1 = motor1;
    motorValue->motor2 = motor2;

    if (alignCounter > 10)
    {
        *aligned = true;
        alignCounter = 0;
        iTerm = 0;
    }

//    if (counter > 100)
//    {
//      Position *pos = getRobotPosition();
//      if (DEBUG_ENABLED()) {
//        debug_printf("New Position: position= %d %d | orientation= %f\n", pos->x, pos->y, *getRobotAngle());
//        debug_printf("pTerm= %f | dTerm= %f | iTerm= %f \n", pTerm, dTerm, iTerm);
//        debug_printf("angle: motor1= %d | motor2= %d\n", motor1, motor2);
//      }
//      counter = 0;
//    }
//    else
//    {
//      counter++;
//    }*/
}

/*============================================================================
Name    :   positionControl
------------------------------------------------------------------------------
Purpose :   To calculate the motor values needed to control the robot to reach desired position
Input   :   xTarget in Pixel increments
            yTarget in Pixel increments
            motovalue data
Output  :   It will replace the motorvalues with the calculated ones
Return	:   none
Notes   :
============================================================================*/
void positionControl(int xTarget, int yTarget, float finalAngle, Motor *motorValues, bool *reached, bool forward, bool finalGoal, bool ignoreOrientation)
{/*
    float ratioOfAngle = 0.75f;
    float distanceThreshold = 0.01f;//0.0035f;//0.01f;
    bool aligned = false;
    static bool aligned1 = false;
    static bool aligned2 = false;
    static int counter = 0;
    int xNow = getRobotPosition()->x;
    int yNow = getRobotPosition()->y;
    int lastDistance = 0;
    float sign = 0.0f;
    static int TimerTime = 0;
    static bool justStarted = false;

    if (motorValues->motor1 == 0)
    {
        justStarted = true;
    }

    float distance2 = sqrtf((float)((xTarget - xNow) * (xTarget - xNow)) * powf((.8128) / (953.0f - 70.0f), 2) +
                            (float)((yTarget - yNow) * (yTarget - yNow)) * powf((0.508) / (790.0f - 232.0f), 2));
    float distance2_EU = sqrtf((float)((xTarget - xNow) * (xTarget - xNow))+//(953.0f - 70.0f), 2) +
                            (float)((yTarget - yNow) * (yTarget - yNow)));


    //long distance2 = (xTarget - xNow)*(xTarget - xNow) + (yTarget - yNow) * (yTarget - yNow);
    static int8_t velocity = 0;
    float factor = 0.75f;
    float pTerm = fabs(distance2);
    float dTerm = fabs((distance2 - lastDistance) / (0.01367f));
    float iTerm = fabs((distance2 + lastDistance) / 2 * (0.01367f));

    float PGain = 0.95f;
    float DGain = 0.0f;
    float IGain = 0.05f;

    if (distance2 > distanceThreshold)
    {
        //velocity = sign * factor * (PGain * pTerm + DGain * dTerm + IGain*iTerm);
        float angleAIM2 = atan2f((yTarget - yNow), (xTarget - xNow)) * (180.0f / PI);
        if ((distance2 < 5 * distanceThreshold) && (finalGoal == true))
        {
            //velocity = factor * (PGain * pTerm + DGain * dTerm + IGain*iTerm);
            //bool decreased = decreaseVelocity(&velocity,motorValues,5);
            velocity = motorValues->minVelocity ;
        }
        else
        {
            if (justStarted == true)
            {
                if (aligned1 == false)
                {
                    angleControl(angleAIM2, motorValues, &aligned1, forward, 20.0f);
                    velocity = 0;
                }
                else
                {
                    if (increaseVelocity(&velocity, motorValues, 50) == true)
                    {
                        justStarted = false;
                        aligned1 = false;
                    }
                    //velocity = factor * (PGain * pTerm + DGain * dTerm + IGain*iTerm);
                    velocity = motorValues->preferredVelocity;
                }
            }
        }

        //Angle Control
        
        float angleDiff = (angleAIM2 - *getRobotAngle()) * PI / 180.0f;
        angleDiff = cosf(atan2f(sinf(angleDiff), cosf(angleDiff)));
        
        if (angleDiff > 0){
            sign = 1;
        }
        else{
            sign = -1;
        }
        angleControl(angleAIM2, motorValues, &aligned, forward, 5.0f);

        int8_t tmp = motorValues->motor1;

        motorValues->motor1 = sign * velocity + motorValues->motor2 * ratioOfAngle;
        motorValues->motor2 = sign * velocity + tmp * ratioOfAngle;
        *reached = false;
        aligned2 = false;
    }
    else if (distance2 <= distanceThreshold && finalGoal == true)
    {
        //Ignore desired angle
        if(ignoreOrientation){
          *reached = true;
        }
        else{
          if (aligned2 == false)
              angleControl(finalAngle, motorValues, &aligned2, false, 5.0f);
          else
          {
            
              motorValues->motor1 = 0;
              motorValues->motor2 = 0;
              angleControl(finalAngle, motorValues, &aligned2, false, 5.0f);
              *reached = true;
          }
        }
    }
    if(fabs(distance2_EU - lastDistance) < 0.5) {
      setGreenLed(5);
    }

    lastDistance = distance2_EU;



//        if (counter > 100) {
//          Position *pos = getRobotPosition();
//          //uint32_t now = HAL_GetTick()-lasttime;
//          if (DEBUG_ENABLED()) {
//            debug_printf("New Position: position= %d %d | orientation= %f\n", pos->x, pos->y, *getRobotAngle());
//            //debug_printf("distance= %f | justStarted = %d\n",distance2, justStarted);
//            //debug_printf("velocity = %d | motor1= %d | motor2= %d\n", velocity, motorValues->motor1, motorValues->motor2);
//          }
//          counter = 0;
//          //lasttime = HAL_GetTick();
//        }
//        else
//        {
//          counter++;
//        }//*/
}

/*

void positionControl(int xTarget, int yTarget, float finalAngle, Motor *motorValues, bool *reached, bool forward, bool finalGoal, bool ignoreOrientation)
{
  float hard_turn = 0.5f;
  float L = 0.023f; // distance between wheels
  float R = 0.012f; // radius of zooid
  float v_l = 0.0;
  float v_r = 0.0;
  float v_max = 60.0;
  static float Ierr = 0.0f;
  int sign = 1;

  int xNow = getRobotPosition()->x;
  int yNow = getRobotPosition()->y;

  float angleAIM2 = atan2f((yTarget - yNow), (xTarget - xNow)) * (180.0f / PI);
  float omega =(angleAIM2 - *getRobotAngle()) * PI / 180.0f;
 if (cosf(atan2f(sinf(omega), cosf(omega))) > 0){
      sign = 1;
  }
  else{
      sign = -1;
  }
  omega = WrapValue(omega);

  float distance = sqrtf((float)(((xTarget - xNow) / (980.0f -  50.0f)) * ((xTarget - xNow) / (980.0f -  50.0f))) +//* powf((.8128) / (980.0f -  50.0f), 2) +//(953.0f - 70.0f), 2) +
                            (float)(((yTarget - yNow) / (800.0f - 220.0f)) * ((yTarget - yNow) / (800.0f - 220.0f)))) ;//* powf((0.508) / (800.0f - 220.0f), 2)); //(790.0f - 232.0f), 2));
  
  
  float gain = 12.0f;
  Ierr += distance;
  if (Ierr > 8.0f)
    Ierr = 8.0f;
  float v = gain * distance + 0.1f * Ierr;

  if(fabsf(omega) > hard_turn){
    
    //log("Hard turn...")
    //v_l = 0.5 * v_max;
    //v_r = 0.0f;
    if(omega > 0.0) {
      setGreenLed(5);
      v_l = -v_max * 0.2;//-motorValues->minVelocity;
      v_r = v_max * 0.2;//motorValues->minVelocity;
    } else {
      setGreenLed(0);
      v_l = v_max * 0.2;//motorValues->minVelocity;
      v_r = -v_max * 0.2;//-motorValues->minVelocity;//
    }
  } else {
      setGreenLed(0);
      setBlueLed(5);
      v_l = v;//(( 2.0 * v ) - (omega * L )) / (2.0 * R);
      v_r = v;//(( 2.0 * v ) + (omega * L )) / (2.0 * R);
  }

  motorValues->motor2 = sign * v_r * 10;
  motorValues->motor1 = sign * v_l * 10;
  //set_wheels(v_l, v_r)
}
*/
float WrapValue(float ang) {
  while(ang > PI)
    ang = ang - 2*PI;
  while(ang < -PI)
    ang = ang + 2*PI;
  return ang;
}

/*============================================================================
Name    :   minimum
------------------------------------------------------------------------------
Purpose :   To place a minimum on the absolute value of some variable.
Input   :   value you would like to place a minimum
            min - the value of minimum desired
Output  :   It will return the new value with the minimum
Return	:
Notes   :
============================================================================*/
void minimum(int *value, int min)
{
    if (abs(*value) < min)
    {
        if (*value > 0)
            *value = min;
        if (*value < 0)
            *value = -min;
    }
}

/*============================================================================
Name    :   maximum
------------------------------------------------------------------------------
Purpose :   To place a maximum on the absolute value of some variable.
Input   :   value you would like to place a maximum
            max - the value of maximum desired
Output  :   It will return the new value with the maximum
Return	:
Notes   :
============================================================================*/
void maximum(int *value, int max)
{
    if (abs(*value) > max)
    {
        if (*value > 0)
            *value = max;
        if (*value < 0)
            *value = -max;
    }
}
/*============================================================================
Name    :   minimum
------------------------------------------------------------------------------
Purpose :   To place a minimum on the absolute value of some variable.
Input   :   value you would like to place a minimum
            min - the value of minimum desired
Output  :   It will return the new value with the minimum
Return	:
Notes   :
============================================================================*/
void minimumf(float *value, float min)
{
    if (abs(*value) < min)
    {
        if (*value > 0)
            *value = min;
        if (*value < 0)
            *value = -min;
    }
}

/*============================================================================
Name    :   maximum
------------------------------------------------------------------------------
Purpose :   To place a maximum on the absolute value of some variable.
Input   :   value you would like to place a maximum
            max - the value of maximum desired
Output  :   It will return the new value with the maximum
Return	:
Notes   :
============================================================================*/
void maximumf(float *value, float max)
{
    if (abs(*value) > max)
    {
        if (*value > 0)
            *value = max;
        if (*value < 0)
            *value = -max;
    }
}

/*============================================================================
Name    :   minimum
------------------------------------------------------------------------------
Purpose :   To place a minimum on the absolute value of some variable.
Input   :   value you would like to place a minimum
            min - the value of minimum desired
Output  :   It will return the new value with the minimum
Return	:
Notes   :
============================================================================*/
void minimumc(int8_t *value, int8_t min)
{
    if (abs(*value) < min)
    {
        if (*value > 0)
            *value = min;
        if (*value < 0)
            *value = -min;
    }
}

/*============================================================================
Name    :   maximum
------------------------------------------------------------------------------
Purpose :   To place a maximum on the absolute value of some variable.
Input   :   value you would like to place a maximum
            max - the value of maximum desired
Output  :   It will return the new value with the maximum
Return	:
Notes   :
============================================================================*/
void maximumc(int8_t *value, int8_t max)
{
    if (abs(*value) > max)
    {
        if (*value > 0)
            *value = max;
        if (*value < 0)
            *value = -max;
    }
}

/*============================================================================
Name    :   increaseVelocity
------------------------------------------------------------------------------
Purpose :   To incrementally increase the velocity from minVelocity to preferredVelocity
Input   :   velocity - the value you want to increase
            motorValue
            timeIncrement - frequency of increment
Output  :   return true once it finishes increasing the velocity up to the preferred velocity
Return	:
Notes   :
============================================================================*/
bool increaseVelocity(int8_t *velocity, Motor *motorValue, uint32_t timeIncrement)
{
    static uint32_t lastTime = 0;
    if (HAL_GetTick() - lastTime > timeIncrement)
    {
        if (*velocity == 0)
        {
            *velocity = motorValue->minVelocity;
        }
        else if (*velocity >= motorValue->minVelocity && *velocity < motorValue->preferredVelocity)
        {
            *velocity = *velocity + 1;
        }
        else if (*velocity >= motorValue->preferredVelocity)
        {
            *velocity = motorValue->preferredVelocity;
            return true;
        }
        lastTime = HAL_GetTick();
    }
    return false;
}

bool decreaseVelocity(int8_t *velocity, Motor *motorValue, uint32_t timeIncrement)
{
    static uint32_t lastTime = 0;
    if (HAL_GetTick() - lastTime > timeIncrement)
    {
        if (*velocity <= motorValue->minVelocity)
        {
            *velocity = motorValue->minVelocity;
            return true;
        }
        else if (*velocity > motorValue->minVelocity)
        {
            *velocity = *velocity - 1;
        }
        lastTime = HAL_GetTick();
    }
    return false;
}
