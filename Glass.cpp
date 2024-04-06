#include "Glass.h"

Vector<Glass*> Glass::GlassArray;

int Glass::Get(int i, Glass* glass) {
  if (i < 0 || i >= GlassArray.size()) 
    return 0;

  glass = GlassArray[i];
  return 1;
}

int Glass::DetectGlasses(Servo& servo, int (*checkObstacle)()){
  servo.write(0);
  delay(500);
  
  for (int i = 0; i < GlassArray.size(); i++)
    delete GlassArray[i];
  GlassArray.clear();
  Serial.println("Cleared glass array");

  int startDeg = -1;
  int endDeg = 0;
  Serial.print("Checking ");
  for(int i = 0; i <= 180; i += 5)
  {
    Serial.print("[");
    Serial.print(i);
    Serial.print("] ");

    servo.write(i);
    delay(1000);
    if (checkObstacle() > 0)
    {
      if (startDeg > -1)
        continue;
      
      startDeg = i;
    }
    else
    {
      if (startDeg == -1)
        continue;

      endDeg = i;
      int center = startDeg + (endDeg / 2);
      servo.write(center);
      delay(1000);

      GlassArray.push_back(new Glass(startDeg, endDeg));

      startDeg = -1;
      servo.write(i);
      delay(1000);
    }
  }

  Serial.println();
  Serial.print("Found ");
  Serial.print(GlassArray.size());
  Serial.println(" glasses");
  return GlassArray.size();
}