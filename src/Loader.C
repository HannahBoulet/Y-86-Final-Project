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
   inf.open(inputFile->String::get_cstr(), std::ifstream::in);
   if(!inf.is_open())
   {
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
   int lineNumber = 1;  //needed if an error is found
   while (getline(inf, line))
   {
      String inputLine(line);
         if(checkData(&inputLine))
         {
            if(!checkValid(&inputLine)){
            return printErrMsg(Loader::baddata,lineNumber,&inputLine);
            }
         }
         else
         {
            if(!checkComment(&inputLine))
            {
               return printErrMsg(Loader::badcomment, lineNumber, &inputLine);
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
void Loader::memoryLoad(String * inputLine)
{
   bool err = false;
   int currentIndex = Loader::databegin;
   uint32_t address = inputLine->convert2Hex(Loader::addrbegin, Loader::addrend - Loader::addrbegin + 1, err);
   uint8_t data;
   while (inputLine->isHex(currentIndex, 1, err)) {
      data = inputLine->convert2Hex(currentIndex, 2, err);
      mem->putByte(data, address, err);
      currentIndex += 2;
      address++;
   }
   this->lastAddress = address;
}

/*
Check if is data aka if 0x is in front if not it is comment
*/
bool Loader::checkData(String * inputLine)
{
   bool error = false;
   if(inputLine->isSubString("0x", 0, error) == false)
   {
      return false;
   }
   return true;
}

/*
Checks if data and address given are valid
*/
bool Loader::checkValid(String * inputLine) {
   bool error = false;
   // 1st, check if there is a '|' character at the specified comment index
   if (!inputLine->isChar('|', Loader::comment, error)) {
           return false;
   }
   // 2nd, check xif it has an address field then is valid hex 000: 
   if (inputLine->isHex(Loader::addrbegin, 3, error) == false) {
      return false;
   }
   //3rd, check if there is a colon at addrend+1 and a space after the colon at addrend + 2
   if(inputLine->isChar(':', Loader::addrend + 1, error) == false) {
      return false;
   }
   //a space after the colon at addrend + 2
   if(inputLine->isChar(' ', Loader::addrend + 2, error) == false)
   {
      return false;
   }
   // Check if there is a space at index 27
   if(inputLine->isChar(' ', Loader::comment - 1, error) == false) {
      return false; 
   }
   int count = 0;
   int dataLength = 0;
   for (int i = Loader::databegin; i < 27; i++) {
      if (!inputLine->isChar(' ', i, error)) {
         count++;
      }
   }
   dataLength += count;
   if(!inputLine->isHex(Loader::databegin, dataLength, error))
   {
      return false;
   }
   //testing if length is even
   if(dataLength % 2 != 0 )
   {
      return false;
   }
   //testing if less that 10
   if(dataLength / 2 > Loader::maxbytes)
   {
      return false;
   }

   //check to see if last address is not greater then current address
   int32_t currentAddress = inputLine->convert2Hex(Loader::addrbegin, 3, error);
   if(currentAddress < lastAddress)
   {
      return false;
   }

   //check if data and address is in mememory array
   if((currentAddress + dataLength / 2)> Memory::size)
   {
      return false;
   }
   return true;
}


/*
 Any characters beyond column 28 are considered comment characters.
*/
bool Loader::checkComment(String * inputLine) {
   bool error = false;
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


