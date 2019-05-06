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

#ifndef __WIN32EXT_H__
#define __WIN32EXT_H__

#include <string>
#include <list>
#include "wchar.h"
#include "windows.h"

namespace win32ext
{
   void center_child_window(HWND child);
   std::string remove_return_file_extension(std::string &file_name);
   std::string remove_return_file_name(std::string &full_file_name);
   std::string get_module_path();   
   std::string get_module_file_name();

   enum compare_last_write_time_result {  unknown_error=0, file_A_not_found,
                                          file_B_not_found, newer_is_A, same_write_time, newer_is_B};
   int compare_last_write_time(std::string file_nameA, std::string file_nameB);
   
   bool is_file_exist(std::string file_name);
   
   enum file_existence {any_file_name=0,file_must_exist}; 
   std::string get_file_name( std::string window_title,
                                        file_existence exist_spec=file_must_exist,
                                        const char *filter=0,
                                        std::string def_ext="");
   std::string get_current_directory();
   void relative_path_to_absolute(std::string &path);//absolute path can be passed as well
   bool create_long_directory(const std::string &path);
   bool is_directory(const std::string &path);
   void find_files(const std::string &file_name_filter, std::list<std::string> *results);
}
#endif //__WIN32EXT_H__
