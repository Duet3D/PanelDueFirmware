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
#include <fstream>
using namespace std;
#include "archive.h"
#include "error_msg.h"

archive::archive(const char *filename, std::ios::openmode mode):
   fstream(filename, mode | ios::binary)
{ 
}

archive::~archive()
{
}

archive & operator<<(archive &s,const bool & b)
{
   char c=b;
   s<<c;
   return s;
}

archive & operator>>(archive &s,bool & b)
{
   char c;
   s>>c;
   b=(c != 0);
   return s;
}

////////////////////////////////////////////////////////
archive & operator<<(archive &s,const time_t & t)
{
   s.write((char*)&t,sizeof(time_t));
   return s;
}

archive & operator>>(archive &s,time_t & t)
{
   s.read((char*)&t,sizeof(time_t));
   return s;
}

////////////////////////////////////////////////////////
archive & operator<<(archive &s,const std::string & str)
{
   s.write(str.c_str(),str.length()+1);
   return s;
}

archive & operator>>(archive &s,std::string & str)
{
   char c;
   s.get(c);
   while(c!=0)
   {
      str+=c;
      s.get(c);
   }
   return s;
}
////////////////////////////////////////////////////////
archive & operator<<(archive &s,const char * str)
{
   s.write(str,strlen(str)+1);
   return s;
}

archive & operator>>(archive &s,char * str)
{
   char c;
   s.get(c);
   while(c!=0)
   {
      str+=c;
      s.get(c);
   }
   return s;
}

////////////////////////////////////////////////////////
archive & operator<<(archive &s,const char & c)
{
   s.put(c);
	return s;
}

archive & operator>>(archive &s,char & c)
{
   s.get(c);
	return s;
}

////////////////////////////////////////////////////////
archive & operator<<(archive &s,const unsigned char & uc)
{
   s.put(uc);
	return s;
}

archive & operator>>(archive &s,unsigned char & uc)
{
   s.get((char &)uc);
	return s;
}

////////////////////////////////////////////////////////
archive & operator<<(archive &s,const short & sh)
{
   s.write((char*)&sh,sizeof(short));
	return s;
}

archive & operator>>(archive &s,short & sh)
{
   s.read((char*)&sh,sizeof(short));
   return s;
}

////////////////////////////////////////////////////////  
archive & operator<<(archive &s,const unsigned short & sh)         
{                                                         
   s.write((char*)&sh,sizeof(unsigned short));                     
	return s;                                              
}                                                         
                                                          
archive & operator>>(archive &s,unsigned short & sh)               
{                                                         
   s.read((char*)&sh,sizeof(unsigned short));                      
   return s;                                              
}                                                         

////////////////////////////////////////////////////////
archive & operator<<(archive &s,const int & i)
{
   s.write((char*)&i,sizeof(int));
   return s;
}

archive & operator>>(archive &s,int & i)
{
   s.read((char*)&i,sizeof(int));
   return s;
}

////////////////////////////////////////////////////////
archive & operator<<(archive &s,const unsigned int & ui)
{
   s.write((char*)&ui,sizeof(unsigned int));
   return s;
}

archive & operator>>(archive &s,unsigned int & ui)
{
   s.read((char*)&ui,sizeof(unsigned int));
   return s;
}



