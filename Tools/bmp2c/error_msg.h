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

#ifndef __ERROR_MSG_H__
#define __ERROR_MSG_H__

#include <string>

#define ERROR_MSG_CATCH(func_name) catch(error_msg &e)\
                                 {\
                                    e.add_to_err_text(func_name);\
                                    throw(e);\
                                 }\
                                 catch(std::exception &std_e)\
                                 {\
                                    error_msg e(std_e.what());\
                                    e.add_to_err_text("std::exception");\
                                    e.add_to_err_text(func_name);\
                                    throw e;\
                                 }\
                                 catch(...)\
                                 {\
                                    error_msg e("unknown exception caught");\
                                    e.add_to_err_text(func_name);\
                                    throw e;\
                                 };
class error_msg
{
public:
error_msg::error_msg(int err_code, std::string err_text);
error_msg::error_msg(std::string err_text);
error_msg::~error_msg();
int error_msg::get_err_code();
std::string error_msg::get_err_text();
void error_msg::set_err_code(int err_code);
int error_msg::add_to_err_text(std::string err_text);
private:
	std::string 	m_err_text;
	int		m_err_code;
};
enum error_msgStdExceptionCodes {  UNKNOWN_EXCEPTION=0x10000000,
                                   UNEXPECTED_RESULT_FORMAT,
                                   UNEXPECTED_RESULT,
                                   OBJECT_CORRUPTED,
                                   SYS_FAILURE,
                                   BAD_ARGUMENT};
#endif //__ERROR_MSG_H__
