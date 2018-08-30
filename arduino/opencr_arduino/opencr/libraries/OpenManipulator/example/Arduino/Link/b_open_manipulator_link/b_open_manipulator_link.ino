/*******************************************************************************
* Copyright 2016 ROBOTIS CO., LTD.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/* Authors: Hye-Jong KIM */

#include "OMLink.h"
#include "Processing.h"
#include "Motion.h"
#include "RemoteController.h"

#define BDPIN_PUSH_SW_1         34
#define BDPIN_PUSH_SW_2         35

void setup()
{
  Serial.begin(57600);
  DEBUG.begin(57600);
  // while (!Serial)
  //   ;

  connectProcessing();
  //connectRC100();

  switchInit();
  
  initOMLink();
  suctionInit();              //suction pin set 

  std::vector<float> init_joint_angle;
  init_joint_angle.push_back(0.0*DEG2RAD);
  init_joint_angle.push_back(-90.0*DEG2RAD);
  init_joint_angle.push_back(-160.0*DEG2RAD);
  omlink.jointMove(init_joint_angle, MOVETIME);
  init_joint_angle.clear();

  initThread();
  startThread();

#ifdef DEBUGFLUG
  DEBUG.print("start");
  DEBUG.println();
#endif
}

void loop()
{
  switchRead();
  getData(10);
  setMotion();
  
  osDelay(LOOP_TIME * 1000);
}

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////THREAD///////////////////////////////////////////
////////////////////////////DON'T TOUCH BELOW CODE///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

namespace THREAD
{
osThreadId loop;
osThreadId robot_state;
osThreadId actuator_control;
} // namespace THREAD

void initThread()
{
  MUTEX::create();

  // define thread
  osThreadDef(THREAD_NAME_LOOP, Loop, osPriorityNormal, 0, 1024 * 10);
  osThreadDef(THREAD_NAME_ROBOT_STATE, THREAD::Robot_State, osPriorityNormal, 0, 1024 * 10);
  osThreadDef(THREAD_NAME_ACTUATOR_CONTROL, THREAD::Actuator_Control, osPriorityNormal, 0, 1024 * 10);

  // create thread
  THREAD::loop = osThreadCreate(osThread(THREAD_NAME_LOOP), NULL);
  THREAD::robot_state = osThreadCreate(osThread(THREAD_NAME_ROBOT_STATE), NULL);
  THREAD::actuator_control = osThreadCreate(osThread(THREAD_NAME_ACTUATOR_CONTROL), NULL);
}

void startThread()
{
  // start kernel
  //Serial.println("Thread Start");
  osKernelStart();
}

static void Loop(void const *argument)
{
  (void)argument;

  for (;;)
  {
    loop();
    showLedStatus();
  }
}

/////////////////////////////////////////////////////////////////////////////////////

void getData(uint32_t wait_time)
{
  static uint8_t state = 0;
  static uint32_t tick = 0;

  bool rc100_flag = false;
  bool processing_flag = false;

  uint16_t get_rc100_data = 0;
  String get_processing_data = "";

  if (availableRC100())
  {
    get_rc100_data = readRC100Data();
    rc100_flag = true;
  }

  if (availableProcessing())
  {
    get_processing_data = readProcessingData();
    processing_flag = true;
  }

  switch (state)
  {
    case 0:
      if (rc100_flag)
      {
        MUTEX::wait();
        fromRC100(get_rc100_data);
        MUTEX::release();

        tick = millis();
        state = 1;
      }
      else if (processing_flag)
      {
        MUTEX::wait();
        fromProcessing(get_processing_data);
        MUTEX::release();

        tick = millis();
        state = 1;
      }
     break;

    case 1:
      if ((millis() - tick) >= wait_time)
      {
        state = 0;
      }
     break;

    default:
      state = 0;
     break;
  }
}

void switchInit()
{
  pinMode(BDPIN_PUSH_SW_1, INPUT);
  pinMode(BDPIN_PUSH_SW_2, INPUT);
}

void switchRead()
{
  if(digitalRead(BDPIN_PUSH_SW_1))
  {
    motionStart();
  }
  if(digitalRead(BDPIN_PUSH_SW_2))
  {
    motionStop();
  }
}