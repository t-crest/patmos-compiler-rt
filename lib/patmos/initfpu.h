#pragma once
/* 
 * To control this variable from a regular patmos application simply 
 * declare and define the variable outside of any function with the value you want
 * i.e: 
 *      unsigned __USE_HWFPU__ = 1;
 *      int main(){...}
*/
extern unsigned __USE_HWFPU__;