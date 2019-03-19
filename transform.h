#pragma once

typedef struct
{
  float xmin;
  float xmax;
  float avg;
  int   trigger; // -1 = no trigger; else offset
}CaptureStats;

int transform(int32_t *bfer, float *out,int count, VoltageSettings *set,int expand,CaptureStats &stats, float triggerValue, DSOADC::TriggerMode mode);
