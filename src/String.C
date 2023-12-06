   #include <cstdint>
   #include <string>   //for std::string functions
   #include <string.h> //for c-string functions (strlen, strcpy, ..)
   #include "String.h"
   #include <assert.h>

   /*
   * String
   *
   * Construct a String object out of a std::string 
   * This constructor would also be called on a c-str
   *
   * Modifies: str and length data members
   */
   String::String(std::string str)
   {
      assert(str.length() > 0);
      length = str.size();
      this -> str = new char[length];
      for (int i = 0; i < length ; i++) 
      {
         this->str[i] = str[i];
      }
   }

   /*
   * get_cstr
   *
   * Build and return a c-str from the array of characters.
   */
   char * String::get_cstr()
   {
      int32_t clength = length + 1;
      char * cstr = new char[clength];
      for (int  i = 0; i < length ; i++) 
      {
         cstr[i] = str[i];
      }
      cstr[length] = '\0';

      return cstr;
   }

   /*
   * get_stdstr
   *
   * Build and return a std::string from the array of characters.
   */
   std::string String::get_stdstr()
   {
      std::string stdstr;

      for (int i = 0; i < length; i++) 
      {
         stdstr += str[i];
      }
      return stdstr; 
   }

   /*
   * get_length
   *
   * return the value of the length data member
   */
   int32_t String::get_length()
   {
      return length;  
   }

   /*
   * badIndex
   *
   * Returns true if this index into the str array is
   * invalid (negative or greater than array size)
   * @param idx: the index value to be checked against the array size
   * @return: true if the index is invalid, false otherwise
   */
   bool String::badIndex(int32_t idx)
   {
      return (idx < 0 || idx >= length); 
   }

   /*
   * isRepeatingChar
   *
   * Returns true if the characters in the array from
   * startIdx to endIdx are all the character what.
   * For example, if what is ' ' and startIdx is 2 and len
   * is 3 then this function returns true if str[2], str[3],
   * and str[4] are all ' '
   *
   * Three cases are possible:
   * 1) error set to true and returns false if indices are invalid
   *    (starting index and calculated ending index are not with
   *    the string)
   * 2) error set to false and returns false if the characters in
   *    the array at the specified indices are not all the character
   *    what
   * 3) error set to false and returns true if the characters in
   *    the array at the specified indices are all the character 
   *    what
   */
   bool String::isRepeatingChar(char what, int32_t startIdx, int32_t len, bool &error) 
   {
    if (len < 0 || badIndex(startIdx) || badIndex(startIdx + len - 1)) 
    {
        error = true;
        return false;
    }
    
    for (int32_t i = startIdx; i < startIdx + len; i++) 
    {
        if (str[i] != what) 
        {
            error = false;
            return false;
        }
    }
    error = false;
    return true;
   }


   /*
   * convert2Hex
   *
   * Builds and returns a number from the values in the array from 
   * startIdx up to len characters treating those characters as 
   * if they are hex digits. Returns 0 and sets error to true 
   * if the indices are invalid or if the characters are not hex.
   *
   * Valid characters for conversion are:
   *  '0', '1' through '9'
   *  'a', 'b' through 'f'
   *  'A', 'B' through 'F'
   *
   *  Three cases are possible:
   *  1) sets error to false and returns converted number if 
   *     the characters in the specified indices are valid 
   *     hex characters
   *  2) sets error to true and returns 0 if the indices are invalid
   *  3) sets error to true and returns 0 if the indices are 
   *     valid but the characters are not hex
   */
   uint32_t String::convert2Hex(int32_t startIdx, int32_t len, bool & error)
   {
      if (len < 0 || badIndex(startIdx) || badIndex(startIdx + len - 1)) 
      {
         error = true;
         return 0;
      }
      uint32_t result = 0;
      for (int32_t i = startIdx; i < startIdx + len; i++) 
      {
         char ch = str[i];
         uint32_t value;
         if (ch >= '0' && ch <= '9') 
         {
            value = (ch - '0');
         }
         else if (ch >= 'a' && ch <= 'f') 
         {
            value = 10 + (ch - 'a');
         }
         else if (ch >= 'A' && ch <= 'F') 
         {
            value = 10 + (ch - 'A');
         }
         else 
         {
            error = true;
            return 0;
         }
         result = (result << 4) + value;
      }
      error = false;
      return result;
   }

   /* 
   * isChar
   * Returns true if str[idx] is equal to what 
   *
   * Three cases are possible:
   * 1) set error to true and return false if idx is invalid
   * 2) set error to false and return true if str[idx] is what
   * 3) set error to false and return false if str[idx] is not what
   */
   bool String::isChar(char what, int32_t idx, bool & error)
   {
   if (idx < 0 || idx >= length) {
        error = true;
        return false; 
    }
    error = false;
    return (str[idx] == what);
   } 

   /* 
   * isHex
   * Returns true if the sequence of len characters starting at index 
   * startIdx are hex 
   *
   * Three cases are possible:
   * 1) set error to true and return false if starting or 
   *    calculated ending index is invalid
   * 2) set error to false and return true if sequence of characters 
   *    starting at startIdx is hex
   *    Valid hex characters are:
   *    '0', '1' through '9'
   *    'a', 'b' through 'f'
   *    'A', 'B' through 'F'
   * 
   * 3) set error to false and return false otherwise
   */
   bool String::isHex(int32_t startIdx, int len, bool & error)
   {
      if(startIdx < 0 || len < 0 || startIdx + len > length)
      {
         error = true;
         return false;
      }
      for (int32_t i = startIdx; i < startIdx + len; ++i) 
      {
         char c = str[i];
         if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) 
         {
            error = false;
            return false;
         }
      }
      error = false; 
      return true;
   } 

   /*
   * isSubString
   *
   * Returns true if the c-str subStr is in the str array starting
   * at index startIdx 
   *
   * Three cases are possible:
   * 1) starting and/or calculated ending index are invalid: 
   *    set error to true and return false
   * 2) indices are valid and subStr is in str array: set error to 
   *    false and return true
   * 3) indices are valid and subStr is not in str array: set 
   *    error to false and return false
   */
   bool String::isSubString(const char * subStr, int32_t startIdx, bool & error)
   {

      int32_t sub = (int32_t)strlen(subStr); 
      if (badIndex(startIdx) || startIdx + sub > length)
      {
         error = true;
         return false;
      }
      for (int32_t i = startIdx; i < sub; i++)
      {
         if (subStr[i] != str[startIdx + i])
         {
            error = false;
            return false;
         }
      }
      error = false;
      return true;
   }


   /*
   * isSubString
   *
   * Returns true if the std::string subStr is in the str array 
   * starting at index startIdx 
   *
   * Three cases are possible:
   * 1) starting and/or calculated ending index are invalid: 
   *    set error to true and return false
   * 2) indices are valid and subStr is in str array: set error to 
   *    false and return true
   * 3) indices are valid and subStr is not in str array: set 
   *    error to false and return false
   */
   bool String::isSubString(std::string subStr, int32_t startIdx, 
                           bool & error)
   {  
      int32_t sub = (int32_t)subStr.length();
      if (badIndex(startIdx) || startIdx + sub > length) 
      {
         error = true;
         return false;
      }
      for (int32_t i = startIdx; i < sub; i++)
      {
         if (subStr[sub] != str[sub + i])
         {
            error = false;
            return false;
         }
      }
      error = false;
      return true;
   }

   
