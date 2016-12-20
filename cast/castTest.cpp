#include <stdio.h>
#include <iostream>
#include <bitset>
using namespace std;
int main(void){
  // char a = '1';
  // int b ;
  // b = (int)a;
  // printf("%c\n", a);
  // printf("%d\n", b);
  // printf("0x%x\n",a);
  // printf("0x%x\n",b);
  // int c = 300;
  // cout << static_cast <bitset<16> > ((c<<16)>>16) <<endl;
  // cout << static_cast <bitset<8> > ((b<<16)>>16) <<endl;

  int ah = 0b10000001;
  int al = 0b00001010;
  int a,b;
  cout << static_cast <bitset<16> > (ah<<8) << endl;
  a = (ah<<8|al);
  /////////////////////
  //右シフトしないと符号桁（一番左）が維持されない！！！！！！
  //8子だけずらすのじゃダメ！余分にずらして右シフトで左側を全部符号桁で埋める！！！
  /////////////////////
  b = (ah<<24|al<<16)>>16;
  cout << ah << endl;
  cout << al << endl;
  cout << a << endl;
  cout << b << endl;
  cout << static_cast <bitset<32> > (a) << endl;
  cout << static_cast <bitset<32> > (b) << endl;


  return 0;
}
