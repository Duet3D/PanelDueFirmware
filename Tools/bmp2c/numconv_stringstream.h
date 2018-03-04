/*This software is under the BSD licence:
Copyright (c) 2007, Sebastien Riou

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution. 
Neither the name of "nimp software" nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission. 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/


//The numconv_stringstream class is dedicated to conversions of number into or from text:
//numconv_stringstream s;
//unsigned int number=41;
//string number_as_text;
//s<<number;
//number_as_text=s.str();
//
//it overload only stringstream::operator << and >> for char and unsigned char because a conversion
// of those types with regular stringstream requires a cast
//for example a_stringstream<<an_unsigned_char; will write 'A' to the stream rather than "41"
//to get such conversion, we have to write a_stringstream<<(unsigned int)an_unsigned_char;
//this is not that bad...but when you want to write generic code, you don't want to know what you are converting
//therefore you can't afford such cast. 


#ifndef __NUMCONV_STRINGSTREAM_H__
#define __NUMCONV_STRINGSTREAM_H__
#include <iostream>
#include <sstream>
#include <string>
#include <time.h>

class numconv_stringstream : public std::stringstream
{
	public:
   numconv_stringstream();
   ~numconv_stringstream();

   friend numconv_stringstream & operator<<(numconv_stringstream &s,const char & c);
   friend numconv_stringstream & operator>>(numconv_stringstream &s,char & c);
   
   friend numconv_stringstream & operator<<(numconv_stringstream &s,const unsigned char & uc);
   friend numconv_stringstream & operator>>(numconv_stringstream &s,unsigned char & uc);
   
	protected:
	private:
};

#endif // __NUMCONV_STRINGSTREAM_H__
