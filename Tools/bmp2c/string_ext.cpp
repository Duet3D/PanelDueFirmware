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


#include "string_ext.h"
#include "windows.h"
#include "wchar.h"
#include "error_msg.h"
#include <algorithm>
#include <iostream>
using namespace std;

std::string stringext::letters()
{
   return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
}

std::string stringext::numeric_digits()
{
   return "0123456789";
}

std::string stringext::hex_numeric_digits()
{
   return numeric_digits()+"abcdefABCDEFxX";
}

std::string stringext::blank()
{
   return " \t\n\r";
}

std::string stringext::valid_cpp_id_first_char()
{
   return letters()+"_";
}

std::string stringext::valid_cpp_id()
{
   return letters()+numeric_digits()+"_";
}

void stringext::replace_each_of(std::string &s, const std::string &targets, const std::string &replacement)
{
    string::size_type pos=0;
    unsigned int tSize=1;
    unsigned int rSize=replacement.length();
    do
    {
        pos=s.find_first_of(targets,pos);
        if (pos==string::npos)
            break;
        if (tSize>rSize)
        {
            s.erase(pos,tSize-rSize);
        }
        if (tSize<rSize)
        {
            s.insert(pos,rSize-tSize,'x');
        }
        s.replace(pos,rSize,replacement);
        pos+=rSize;
    }
    while (1);
}
void stringext::replace(std::string &s, const std::string &target, const std::string &replacement)
{
    string::size_type pos=0;
    unsigned int tSize=target.length();
    unsigned int rSize=replacement.length();
    do
    {
        pos=s.find(target,pos);
        if (pos==string::npos)
            break;
        if (tSize>rSize)
        {
            s.erase(pos,tSize-rSize);
        }
        if (tSize<rSize)
        {
            s.insert(pos,rSize-tSize,'x');
        }
        s.replace(pos,rSize,replacement);
        pos+=rSize;
    }
    while (1);
}

void stringext::remove_white_spaces(std::string &s, bool remove_cr_lf)
{
   s.erase(remove(s.begin(),s.end(),' '),s.end());
   s.erase(remove(s.begin(),s.end(),'\t'),s.end());
   if(remove_cr_lf)
   {
      s.erase(remove(s.begin(),s.end(),'\r'),s.end());
      s.erase(remove(s.begin(),s.end(),'\n'),s.end());
   }
}
bool stringext::is_not_blank(const std::string &s)
{
    string::size_type pos;
    pos=s.find_first_not_of(blank());
    if (pos==string::npos)
        return false;
    else
        return true;
}
bool stringext::is_blank(const std::string &s)
{
    string::size_type pos;
    pos=s.find_first_not_of(blank());
    if (pos!=string::npos)
        return false;
    else
        return true;
}
wchar_t *stringext::ansi2unicode(const std::string &s)
{
    int status=MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,s.c_str(),s.length()+1,NULL,0);
    if (status==0)
    {
        string msg;
        switch (GetLastError())
        {
        case ERROR_INSUFFICIENT_BUFFER:
            msg="ERROR_INSUFFICIENT_BUFFER";
            break;
        case ERROR_INVALID_FLAGS:
            msg="ERROR_INVALID_FLAGS";
            break;
        case ERROR_INVALID_PARAMETER:
            msg="ERROR_INVALID_PARAMETER";
            break;
        case ERROR_NO_UNICODE_TRANSLATION:
            msg="ERROR_NO_UNICODE_TRANSLATION";
            break;
        default:
            msg="unknown error code";
        }
        msg="stringext::ansi2unicode('"+s+"'): MultiByteToWideChar failed: "+msg;
        OutputDebugString(msg.c_str());
        throw error_msg(-1,msg);
    }
    wchar_t *ws = new wchar_t[status];
    status=MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,s.c_str(),s.length()+1,ws,status);
    if (status==0)
    {
        string msg;
        switch (GetLastError())
        {
        case ERROR_INSUFFICIENT_BUFFER:
            msg="ERROR_INSUFFICIENT_BUFFER";
            break;
        case ERROR_INVALID_FLAGS:
            msg="ERROR_INVALID_FLAGS";
            break;
        case ERROR_INVALID_PARAMETER:
            msg="ERROR_INVALID_PARAMETER";
            break;
        case ERROR_NO_UNICODE_TRANSLATION:
            msg="ERROR_NO_UNICODE_TRANSLATION";
            break;
        default:
            msg="unknown error code";
        }
        msg="stringext::ansi2unicode('"+s+"'): MultiByteToWideChar failed: "+msg;
        OutputDebugString(msg.c_str());
        throw error_msg(-1,msg);
    }
    return ws;
}
std::string stringext::unicode2ansi(wchar_t *s)
{
    string as;
    char *temp=0;
    int templen=WideCharToMultiByte(  CP_ACP,            // code page
                                      0,            // performance and mapping flags
                                      s,    // wide-character string
                                      -1,          // number of chars in string
                                      temp,     // buffer for new string
                                      0,          // size of buffer
                                      NULL,     // default for unmappable chars
                                      NULL  // set when default char used
                                   );
    if (templen==0)
    {
        string msg;
        switch (GetLastError())
        {
        case ERROR_INSUFFICIENT_BUFFER:
            msg="ERROR_INSUFFICIENT_BUFFER";
            break;
        case ERROR_INVALID_FLAGS:
            msg="ERROR_INVALID_FLAGS";
            break;
        case ERROR_INVALID_PARAMETER:
            msg="ERROR_INVALID_PARAMETER";
            break;
        default:
            msg="unknown error code";
        }
        msg="stringext::unicode2Ainsi(): WideCharToMultiByte(cbMultiByte=0) failed: "+msg;
        OutputDebugString(msg.c_str());
        throw error_msg(-1,msg);
    }
    temp=new char[templen];
    if (temp==0) throw error_msg(-1,"Can't allocate dynamic memomy");
    templen=WideCharToMultiByte(  CP_ACP,            // code page
                                  0,            // performance and mapping flags
                                  s,    // wide-character string
                                  -1,          // number of chars in string
                                  temp,     // buffer for new string
                                  templen*sizeof(templen),          // size of buffer
                                  NULL,     // default for unmappable chars
                                  NULL  // set when default char used
                               );
    if (templen==0)
    {
        string msg;
        switch (GetLastError())
        {
        case ERROR_INSUFFICIENT_BUFFER:
            msg="ERROR_INSUFFICIENT_BUFFER";
            break;
        case ERROR_INVALID_FLAGS:
            msg="ERROR_INVALID_FLAGS";
            break;
        case ERROR_INVALID_PARAMETER:
            msg="ERROR_INVALID_PARAMETER";
            break;
        default:
            msg="unknown error code";
        }
        msg="stringext::unicode2Ainsi(): WideCharToMultiByte failed: "+msg;
        OutputDebugString(msg.c_str());
        throw error_msg(-1,msg);
    }
    as=temp;
    if (temp) delete[] temp;
    return as;
}
std::string stringext::cpp_encode(const std::string &text)
try
{
    string cpp;
    for (string::size_type i=0;i<text.length();i++)
    {
        switch (text[i])
        {
        case '\\':
            cpp+="\\\\";
            break;
        case '\'':
            cpp+="\\'";
            break;
        case '"':
            cpp+="\\\"";
            break;
        /*case '/':
            cpp+="\\/";
            break;
        case '*':
            cpp+="\\*";
            break;*/
        default:
            cpp+=text[i];
        }
    }
    return cpp;
}
ERROR_MSG_CATCH("stringext::cpp_encode(const std::string &text)");

std::string stringext::cpp_decode(const std::string &source, unsigned int *index)
try
{
   unsigned int i=0;
   if(index!=0)//if index is used, it indicates the starting position in the input string
      i=*index;
   if(source.at(i)!='"') return "";
   i++;
   string decoded;
   bool keep_going=true;
   //cout<<endl<<"*"<<source<<"*"<<endl;
   while(keep_going)
   {
      //cout<<i<<endl;
      if(i>=source.length())
         throw error_msg("final '\"' character not found");
      switch (source[i])
      {
      case '\\':
         i++;
         decoded+=source.at(i);
         break;
      case '"':
         keep_going=false;
         break;
      default:
         decoded+=source[i];
      }
      i++;
   }   
   if(index!=0)//if index is used, update it to the first character after the decoded string
      *index=i;
   return decoded;
}
ERROR_MSG_CATCH("cpp_decode(const std::string &source, unsigned int *index)");
