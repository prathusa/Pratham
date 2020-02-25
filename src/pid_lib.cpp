#include "vex.h"

//Settings
double kP = 0.0;
double kI = 0.0;
double kD = 0.0;
double turnkP = 0.0;
double turnkI = 0.0;
double turnkD = 0.0;
int maxTurnIntegral = 300; // These cap the integrals
int maxIntegral = 300;
int integralBound = 3; //If error is outside the bounds, then apply the integral. This is a buffer with +-integralBound degrees

//Autonomous Settings
int desiredValue = 200;
int desiredTurnValue = 0;

int error; //SensorValue - DesiredValue : Position
int prevError = 0; //Position 20 miliseconds ago
int derivative; // error - prevError : Speed
int totalError = 0; //totalError = totalError + error

int turnError; //SensorValue - DesiredValue : Position
int turnPrevError = 0; //Position 20 miliseconds ago
int turnDerivative; // error - prevError : Speed
int turnTotalError = 0; //totalError = totalError + error

bool resetDriveSensors = false;

//Variables modified for use
bool enableDrivePID = true;

//Pasted from a C++ resource
double signnum_c(double x) 
{
  if (x > 0.0) return 1.0;
  if (x < 0.0) return -1.0;
  return x;
}

int drivePID()
{
  
  while(enableDrivePID){

    if (resetDriveSensors) {
      resetDriveSensors = false;
      l.setPosition(0,degrees);
      r.setPosition(0,degrees);
    }


    //Get the position of both motors
    int leftMotorPosition = l.position(degrees);
    int rightMotorPosition = r.position(degrees);

    ///////////////////////////////////////////
    //Lateral movement PID
    /////////////////////////////////////////////////////////////////////
    //Get average of the two motors
    int averagePosition = (leftMotorPosition + rightMotorPosition)/2;

    //Potential
    error = averagePosition - desiredValue;

    //Derivative
    derivative = error - prevError;

    //Integral
    if(abs(error) < integralBound){
    totalError+=error; 
    }  else {
    totalError = 0; 
    }
    //totalError += error;

    //This would cap the integral
    totalError = abs(totalError) > maxIntegral ? signnum_c(totalError) * maxIntegral : totalError;

    double lateralMotorPower = error * kP + derivative * kD + totalError * kI;
    /////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////
    //Turning movement PID
    /////////////////////////////////////////////////////////////////////
    //Get average of the two motors
    int turnDifference = leftMotorPosition - rightMotorPosition;

    //Potential
    turnError = turnDifference - desiredTurnValue;

    //Derivative
    turnDerivative = turnError - turnPrevError;

    //Integral
    if(abs(error) < integralBound){
    turnTotalError+=turnError; 
    }  else {
    turnTotalError = 0; 
    }
    //turnTotalError += turnError;

    //This would cap the integral
    turnTotalError = abs(turnTotalError) > maxIntegral ? signnum_c(turnTotalError) * maxIntegral : turnTotalError;

    double turnMotorPower = turnError * turnkP + turnDerivative * turnkD + turnTotalError * turnkI;
    /////////////////////////////////////////////////////////////////////

    l.spin(forward, lateralMotorPower + turnMotorPower, voltageUnits::volt);
    r.spin(forward, lateralMotorPower - turnMotorPower, voltageUnits::volt);


    

    prevError = error;
    turnPrevError = turnError;
    vex::task::sleep(20);

  }

  return 1;
}

void demoAutonomous(void) {

  vex::task billWiTheScienceFi(drivePID);

  resetDriveSensors = true;
  desiredValue = 300;
  desiredTurnValue = 600;

  vex::task::sleep(1000);

  resetDriveSensors = true;
  desiredValue = 300;
  desiredTurnValue = 300;


}

void controlPID(void) {

  enableDrivePID = false;

  ///////////////////////////
  //Settings
  ///////////////////////////////////////////////////////////////////////////

  //Drivetrain
  double turnImportance = 0.5;

  while (1) {

    ///////////////////////////
    //Driver Control
    ///////////////////////////////////////////////////////////////////////////
    double turnVal = Controller1.Axis1.position(percent);
    double forwardVal = Controller1.Axis3.position(percent);

    double turnVolts = turnVal * 0.12;
    double forwardVolts = forwardVal * 0.12 * (1 - (std::abs(turnVolts)/12.0) * turnImportance);

    //0 - 12 = -12
    //0 + 12 = 12(due to cap)


    l.spin(forward, forwardVolts + turnVolts, voltageUnits::volt);
    r.spin(forward, forwardVolts - turnVolts, voltageUnits::volt);
    ///////////////////////////////////////////////////////////////////////////


    ///////////////////////////
    //Arm Control
    ///////////////////////////////////////////////////////////////////////////
    bool topRightButton = Controller1.ButtonR1.pressing();
    bool bottomRightButton = Controller1.ButtonR2.pressing();

    if (topRightButton){
      Lift.spin(forward, 12.0, voltageUnits::volt);
    }
    else if (bottomRightButton){
      Lift.spin(forward, -12.0, voltageUnits::volt);
    }
    else{
      Lift.spin(forward, 0, voltageUnits::volt);
    }
    ///////////////////////////////////////////////////////////////////////////

    ///////////////////////////
    //Claw Control
    ///////////////////////////////////////////////////////////////////////////
    bool topLeftButton = Controller1.ButtonL1.pressing();
    bool bottomLeftButton = Controller1.ButtonL2.pressing();

    if (topLeftButton){
      intake.spin(forward, 12.0, voltageUnits::volt);
    }
    else if (bottomLeftButton){
      intake.spin(forward, -12.0, voltageUnits::volt);
    }
    else{
      intake.spin(forward, 0, voltageUnits::volt);
    }
    ///////////////////////////////////////////////////////////////////////////



    wait(20, msec); // Sleep the task for a short amount of time to
                    // prevent wasted resources.
  }
}