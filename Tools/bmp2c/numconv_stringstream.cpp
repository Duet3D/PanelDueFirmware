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


#include <iostream>
#include <sstream>
using namespace std;
#include "numconv_stringstream.h"
#include "error_msg.h"

numconv_stringstream::numconv_stringstream()
{ 
}

numconv_stringstream::~numconv_stringstream()
{
}

////////////////////////////////////////////////////////
numconv_stringstream & operator<<(numconv_stringstream &s,const char & c)
{
   stringstream * ss=&s;
   (*ss)<<(const int)c;
	return s;
}

numconv_stringstream & operator>>(numconv_stringstream &s,char & c)
{
   s>>(int &)c;
	return s;
}

////////////////////////////////////////////////////////
numconv_stringstream & operator<<(numconv_stringstream &s,const unsigned char & uc)
{
   stringstream * ss=&s;
   (*ss)<<(const unsigned int)uc;
	return s;
}

numconv_stringstream & operator>>(numconv_stringstream &s,unsigned char & uc)
{
   s>>(unsigned int &)uc;
	return s;
}
