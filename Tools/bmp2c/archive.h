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

//Purpose:
//The archive class is equivalent to iofstream except that it stores data in the binary
//for example, with iofstream, the_stream<<an_unsigned_int; will write "4294967295" -> 10 bytes
//with archive, the_archive<<an_unsigned_int; will write 0xffffffff -> 4 bytes

#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>

class archive : public std::fstream
{
	public:
   archive(const char *filename, std::ios::openmode mode);
   ~archive();

   friend archive & operator<<(archive &s,const bool & b);
   friend archive & operator>>(archive &s,bool & b);

   friend archive & operator<<(archive &s,const time_t & t);
   friend archive & operator>>(archive &s,time_t & t);
   
   friend archive & operator<<(archive &s,const std::string & str);
   friend archive & operator>>(archive &s,std::string & str);

   friend archive & operator<<(archive &s,const char * str);
   friend archive & operator>>(archive &s,char * str);
   
   friend archive & operator<<(archive &s,const char & c);
   friend archive & operator>>(archive &s,char & c);
   
   friend archive & operator<<(archive &s,const unsigned char & uc);
   friend archive & operator>>(archive &s,unsigned char & uc);
   
   friend archive & operator<<(archive &s,const short & sh);
   friend archive & operator>>(archive &s,short & sh);
   
   friend archive & operator<<(archive &s,const unsigned short & sh);
   friend archive & operator>>(archive &s,unsigned short & sh);
      
   friend archive & operator<<(archive &s,const int & i);
   friend archive & operator>>(archive &s,int & i);
   
   friend archive & operator<<(archive &s,const unsigned int & ui);
   friend archive & operator>>(archive &s,unsigned int & ui);

	protected:
	private:
};

#endif // __ARCHIVE_H__
