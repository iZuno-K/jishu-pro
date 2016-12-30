#include <stdio.h>

int sumation(int num){
  int sum=0;
  int i;
  for ( i = 0; i < num; i++) {
    sum+=i;
  }
  return sum;
}
