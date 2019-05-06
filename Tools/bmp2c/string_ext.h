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


#ifndef __STRINGEXT_H__
#define __STRINGEXT_H__
#include <string>
#include "wchar.h"
namespace stringext
{
   std::string letters();
   std::string numeric_digits();
   std::string hex_numeric_digits();
   std::string blank();
   std::string valid_cpp_id_first_char();
   std::string valid_cpp_id();

//replace each occurence of any character in targets by replacement in the string s
void replace_each_of(std::string &s,
                     const std::string &targets,
                     const std::string &replacement);
//replace each occurence of target by replacement in the string s
void replace(std::string &s,
             const std::string &target,
             const std::string &replacement);

void remove_white_spaces(std::string &s, bool remove_cr_lf=false);

bool is_not_blank(const std::string &s);
bool is_blank(const std::string &s);
wchar_t *ansi2unicode(const std::string &s); //the returned buffer must be deleted using the "delete" keyword
std::string unicode2ansi(wchar_t *s);
std::string cpp_encode(const std::string &text);
std::string cpp_decode(const std::string &source, unsigned int *index=0);
}
#endif //__STRINGEXT_H__
