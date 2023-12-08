   #include <iostream>
   #include <fstream>
   #include <cstdint>
   #include "Memory.h"
   #include "String.h"
   #include "Loader.h"

   /**
   * Loader
   * Initializes the private data members.
   *
   * @param: argc The number of command-line arguments.
   * @param: argv[] Array of command-line arguments.
   * @param: mem Pointer to Memory instance
   */
   Loader::Loader(int argc, char * argv[], Memory * mem)
   {
      this->lastAddress = -1;   //keep track of last mem byte written to for error checking
      this->mem = mem;          //memory instance
      this->inputFile = NULL;   
      if (argc > 1) inputFile = new String(argv[1]);  //input file name
   }

   /**
   * printErrMsg
   * Prints an error message and returns false (load failed)
   * If the line number is not -1, it also prints the line where error occurred
   *
   * @param: which Indicates error number
   * @param: lineNumber Number of line in input file on which error occurred (if applicable)
   * @param: line Line on which error occurred (if applicable)
   * @return: True when loaded, false if load fails.
   */
   bool Loader::printErrMsg(int32_t which, int32_t lineNumber, String * line)
   {
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
      }
      else
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

   /**
   * openFile
   * 
   * Checks to see if the file name is well-formed and can be opened
   * If there is an error, it prints an error message and returns false
   * by calling printErrMsg
   * Otherwise, the file is opened and the function returns true
   *
   * modifies inf data member (file handle) if file is opened
   * @return: True if file name is good and file is opened, error message otherwise
   */
   bool Loader::openFile()
   {
      //TODO

      //If the user didn't supply a command line argument (inputFile is NULL)
      //then print the Loader::usage error message and return false
      //(Note: Loader::usage is a static const defined in Loader.h)
      if(this->inputFile == NULL) return printErrMsg(Loader::usage,-1, NULL);

      //If the filename is badly formed (needs to be at least 4 characters
      //long and end with .yo) then print the Loader::badfile error message 
      //and return faclse
      bool error;
      if((this->inputFile -> String::isSubString((char*)".yo", inputFile-> get_length()
          - 3, error)) == false) return printErrMsg(Loader::badfile, -1, NULL);
      //Open the file using an std::ifstream open
      //If the file can't be opened then print the Loader::openerr message 
      //and return false
      inf.open(inputFile->String::get_cstr(), std::ifstream::in);
      if(!inf.is_open()) return printErrMsg(Loader::openerr, -1, NULL);
   
      return true;  //file name is good and file open succeeded
   }

   /**
   * load 
   * Opens the .yo file by calling openFile.
   * Reads the lines in the file line by line and
   * loads the data bytes in data records into the Memory.
   * If a line has an error in it, then NONE of the line will be
   * loaded into memory and the load function will return false.
   *
   * @return: True if load succeeded (no errors in the input) and false otherwise.
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
            if(!checkValid(&inputLine))
            {
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

   /**
   * MemoryLoad:
   * Helper method that loads memory in.
   * 
   * @param: inputLine Input line to be read 
   */
   void Loader::memoryLoad(String * inputLine)
   {
      bool err = false;
      int currentIndex = Loader::databegin;
      uint32_t address = inputLine->convert2Hex(Loader::addrbegin,
          Loader::addrend - Loader::addrbegin + 1, err);

      uint8_t data;
      while (inputLine->isHex(currentIndex, 1, err)) 
      {
         data = inputLine->convert2Hex(currentIndex, 2, err);
         mem->putByte(data, address, err);
         currentIndex += 2;
         address++;
      }
      this->lastAddress = address;
   }

   /**
   * checkData
   *
   * Helper method that checks if the input line is data. 
   *
   * @param: inputLine Line to be checked.
   * @return: True if line is data (starts with 0x), false if not (comment)
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

   /**
   * checkValid
   * Helper method that checks if data and address given are valid.
   * 
   * @param: inputLine Line to be checked.
   * @return: True if a valid data and address are given, false otherwise.
   */
   bool Loader::checkValid(String * inputLine) 
   {
      bool error = false;
      // check if there is a '|' character, address field valid hex
      // if colon and spaces are in right positions
      
      if (!inputLine->isChar('|', Loader::comment, error)) return false;
      if (inputLine->isHex(Loader::addrbegin, 3, error) == false) return false;
      if (inputLine->isChar(':', Loader::addrend + 1, error) == false) return false;
      if (inputLine->isChar(' ', Loader::addrend + 2, error) == false) return false;
      if (inputLine->isChar(' ', Loader::comment - 1, error) == false) return false; 
      
      // count non-spaces in data
      int count = 0;
      int dataLength = 0;
      for (int i = Loader::databegin; i < Loader::comment - 1; i++) 
      {
         if (!inputLine->isChar(' ', i, error)) 
         {
            count++;
         }
      }
      dataLength += count;
      
      //testing valid hex for data
      if(!inputLine->isHex(Loader::databegin, dataLength, error)) return false;
      
      //testing if length is even
      if(dataLength % 2 != 0 ) return false;
      
      //testing if less that 10
      if(dataLength / 2 > Loader::maxbytes) return false;
      
      //check to see if last address is not greater then current address
      int32_t currentAddress = inputLine->convert2Hex(Loader::addrbegin, 3, error);
      if(currentAddress < lastAddress) return false;
      
      //check if data and address is in mememory array
      if((currentAddress + dataLength / 2) > Memory::size) return false;
   
      return true;
   }

   /**
   * checkComment
   * Checks if line is a comment. Any characters beyond column 28 are considered comment characters.
   *
   * @param: inputLine Line to be checked.
   * @return: True if a valid comment, false otherwise.
   */
   bool Loader::checkComment(String * inputLine) 
   {
      bool error = false;
      // Validate spaces in columns 0 .. 27
      if (!inputLine->isRepeatingChar(' ', 0, Loader::comment, error)) return false;
      
      // Validate column 28 contains a pipe ('|') character
      if (!inputLine->isChar('|', Loader::comment, error)) return false;
   
      return true;
   }


