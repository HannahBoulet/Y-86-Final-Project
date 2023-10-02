/* 
 * Driver for the yess simulator
 * Usage: yess <file>.yo 
 *
 * <file>.yo contains assembled y86-64 code.
*/

#include <iostream>
#include <fstream>
#include <string.h>
#include "Memory.h"
#include "Loader.h"

int main(int argc, char * argv[])
{
   Memory * mem = Memory::getInstance();
   Loader loadObj(argc, argv, mem);
   loadObj.load();
   if (mem != NULL) mem->dump();
   
   return 0;
}

