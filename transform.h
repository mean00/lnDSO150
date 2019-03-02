#pragma once

typedef struct
{
  float xmin;
  float xmax;
  float avg;
}CaptureStats;

int transform(int32_t *bfer, float *out,int count, VoltageSettings *set,int expand,CaptureStats &stats);