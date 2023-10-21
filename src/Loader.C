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
         if(!checkData(&inputLine))
         {
            if(!checkComment(&inputLine))
            {
               return printErrMsg(Loader::badcomment, lineNumber, &inputLine);
            }
         }
         else if(checkData(&inputLine))
         {
            if(checkValid(&inputLine)){
            return printErrMsg(Loader::baddata,lineNumber,&inputLine);
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
   this-> lastAddress = address;
}
/*
Check if is data aka if 0x is in front if not it is comment
*/
bool Loader::checkData(String * inputLine)
{
   bool error = false;
   if(!inputLine->isSubString("0x", Loader::addrbegin-2, error))
   {
      return false;
   }
   return true;
}

/*
using 
static const int32_t addrbegin = 2;  //begin address index
      static const int32_t addrend = 4;    //end address index
      static const int32_t databegin = 7;  //begin data index
      static const int32_t comment = 28;   //index of |
      static const int32_t maxbytes = 10;  //max bytes in data record 
      and String do so below
*/
bool Loader::checkValid(String * inputLine) {
    bool error = false;
    
    // First, check if the address fields are valid hex
    if (!inputLine->convert2Hex(Loader::addrbegin, Loader::addrend, error)) {
        return false;
    }
    // 2nd, check if there is a colon at addrend+1 and a space after the colon at addrend + 2
    if (!inputLine->isChar(':', Loader::addrend+1, error) && 
        !inputLine->isChar(' ', Loader::addrend + 2, error)) {
        return false;
    }

    // 3rd, check if there is a '|' character at the specified comment index
    if (!inputLine->isChar('|', Loader::comment, error)) {
        return false;
    }

    
   //  // Check if the data is valid hex and within the specified range
   //  if (!inputLine->convert2Hex(Loader::databegin, Loader::comment - 2, error)) {
   //      return false; 
   //  }    
   //  // Check if there is a space at index 27
   //  if (!inputLine->isChar(' ', Loader::comment - 1, error)) {
   //      return false; 
   //  }
   //  // Check if there is a '|' at the comment index
    
  //first need to check if the addrbegin to the addrend are hex using ishex and possibly to hex
   //return false if not

   //2nd need to check if there is a colon at addrend+1 and a space after the column addrend+2 
   //return false if not

   //3rd need to check if the data is also valid hex, is 10 bytes max (% 2) using maxbytes
   //need to check if its greater then the previous address
   //return false if not

   //4th check if there is a | at comment and return false if not. 

   //5th check for spaces between the data are return false if there is
   //6th check for outside mem array
   //7th make sure the data is even (% 2) and when divided by 2 its 10 or less (maxbytes)   
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


