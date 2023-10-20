#include <iostream>
#include <fstream>
#include <cstdint>
#include "Memory.h"
#include "String.h"
#include "Loader.h"

/* 
 * Loader
 * Initializes the private data members
 */
Loader::Loader(int argc, char * argv[], Memory * mem)
{
   //this method is COMPLETE
   this->lastAddress = -1;   //keep track of last mem byte written to for error checking
   this->mem = mem;          //memory instance
   this->inputFile = NULL;   
   if (argc > 1) inputFile = new String(argv[1]);  //input file name
}

/*
 * printErrMsg
 * Prints an error message and returns false (load failed)
 * If the line number is not -1, it also prints the line where error occurred
 *
 * which - indicates error number
 * lineNumber - number of line in input file on which error occurred (if applicable)
 * line - line on which error occurred (if applicable)
 */
bool Loader::printErrMsg(int32_t which, int32_t lineNumber, String * line)
{
   //this method is COMPLETE
   static char * errMsg[Loader::numerrs] = 
   {
      (char *) "Usage: yess <file.yo>\n",                       //Loader::usage
      (char *) "Input file name must end with .yo extension\n", //Loader::badfile
      (char *) "File open failed\n",                            //Loader::openerr
      (char *) "Badly formed data record\n",                    //Loader::baddata
      (char *) "Badly formed comment record\n",                 //Loader::badcomment
   };   
   if (which >= Loader::numerrs)
   {
      std::cout << "Unknown error: " << which << "\n";
   } else
   {
      std::cout << errMsg[which]; 
      if (lineNumber != -1 && line != NULL)
      {
         std::cout << "Error on line " << std::dec << lineNumber
                   << ": " << line->get_stdstr() << std::endl;
      }
   } 
   return false; //load fails
}

/*
 * openFile
 * The name of the file is in the data member openFile (could be NULL if
 * no command line argument provided)
 * Checks to see if the file name is well-formed and can be opened
 * If there is an error, it prints an error message and returns false
 * by calling printErrMsg
 * Otherwise, the file is opened and the function returns true
 *
 * modifies inf data member (file handle) if file is opened
 */
bool Loader::openFile()
{
   //TODO

   //If the user didn't supply a command line argument (inputFile is NULL)
   //then print the Loader::usage error message and return false
   //(Note: Loader::usage is a static const defined in Loader.h)
  if(this->inputFile == NULL)
  {
      mem->dump();
      return printErrMsg(Loader::usage,-1, NULL);
  }
   //If the filename is badly formed (needs to be at least 4 characters
   //long and end with .yo) then print the Loader::badfile error message 
   //and return faclse
   bool error;
   if((this->inputFile -> String::isSubString((char*)".yo", inputFile-> get_length() - 3, error)) == false)
   {
      return printErrMsg(Loader::badfile, -1, NULL);
   }
   //Open the file using an std::ifstream open
   //If the file can't be opened then print the Loader::openerr message 
   //and return false
   if(!inf)
   {
   mem->dump();
   return printErrMsg(Loader::openerr, -1, NULL);
   }
   return true;  //file name is good and file open succeeded
}

/*
 * load 
 * Opens the .yo file by calling openFile.
 * Reads the lines in the file line by line and
 * loads the data bytes in data records into the Memory.
 * If a line has an error in it, then NONE of the line will be
 * loaded into memory and the load function will return false.
 *
 * Returns true if load succeeded (no errors in the input) 
 * and false otherwise.
*/   
bool Loader::load()
{
    if (!openFile()) return false;

    std::string line;
    int lineNumber = 1;  // needed if an error is found
    while (getline(inf, line))
    {
      String inputLine(line);
      if (!empty(&inputLine)) {
         if(checkComment(&inputLine))
         {
         return printErrMsg(Loader::badcomment, lineNumber, &inputLine);
        }
        if (checkData(&inputLine)) {
            return printErrMsg(Loader::baddata, lineNumber, &inputLine);
        }
        }
        memoryLoad(&inputLine);
        lineNumber++;
    }

    return true;  // load succeeded
}


//Add helper methods definitions here and the declarations to Loader.h
//In your code, be sure to use the static const variables defined in 
//Loader.h to grab specific fields from the input line.
/*
MemoryLoad:
Loads memory in.
*/
void Loader::memoryLoad(String * s)
{
   bool err = false;
   int currentIndex = Loader::databegin;
   uint32_t address = s->convert2Hex(Loader::addrbegin, Loader::addrend - Loader::addrbegin + 1, err);
   uint8_t data;
   while (s->isHex(currentIndex, 1, err)) {
      data = s->convert2Hex(currentIndex, 2, err);
      mem->putByte(data, address, err);
      this->lastAddress = address;
      currentIndex += 2;
      address++;
   }
   
}

/*

*/
bool Loader::checkData(String * inputLine) {
   bool error = false;
   // Validate address format (0xhhh:)
   if(!inputLine->isSubString("0x", Loader::addrbegin, error))
   {
      return false;
   }
   if (!inputLine->isHex(Loader::addrbegin, 3, error) || !inputLine->isChar(':', Loader::addrend, error)) {
      return false;
   }
   // Validate data format (up to 10 bytes of hex data)
   int32_t dataLength = inputLine->get_length() - Loader::databegin;
   if (dataLength > Loader::maxbytes || !inputLine->isHex(Loader::databegin, dataLength, error)) {
      return false;
   }
   // Validate address range (address + data length <= Memory::size)
   uint32_t address = inputLine->convert2Hex(Loader::addrbegin, 3, error);
   if (address + dataLength > Memory::size) {
      return false;
   }
   // Validate if columns after data up to column 28 contain spaces
   if (!inputLine->isRepeatingChar(' ', Loader::addrend + 1, Loader::comment - Loader::addrend - 1, error)) {
      return false;
   }
   return true;
}

/*
Check Comment:
using the constant comment =28 to do bellow
A comment record is of the form:
| comment here
where columns 0 .. 27 contain space characters (' ')
 and column 28 contains a pipe ('|') character. 
 Any characters beyond column 28 are considered comment characters.
 */
bool Loader::checkComment(String * inputLine) {
   bool error = false;
   if(inputLine->isSubString("0x", Loader::addrbegin, error))
   {
      return false;
   }
   // Validate spaces in columns 0 .. 27
   if (!inputLine->isRepeatingChar(' ', 0, Loader::comment, error)) {
      return false;
   }
   // Validate column 28 contains a pipe ('|') character
   if (!inputLine->isChar('|', Loader::comment, error)) {
      return false;
   }
   return true;
}

bool Loader::empty(String * inputLine) {
    // Check if the input line is completely empty
    return inputLine->get_length() == 0;
}
