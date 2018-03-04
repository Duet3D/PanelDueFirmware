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

#include "wchar.h"
#include "windows.h"
#include <sstream>
#include <iostream>
using namespace std;
#include "win32ext.h"
#include "error_msg.h"

#include "string_ext.h"

#define WIN32EXT_MAX_PATH_SIZE 0x7fff


void win32ext::center_child_window(HWND child)
try
{
   HWND hwndOwner; 
   RECT rc, rcDlg, rcOwner;
   if ((hwndOwner = GetParent(child)) == NULL) 
   {
      hwndOwner = GetDesktopWindow(); 
   }
   
   GetWindowRect(hwndOwner, &rcOwner); 
   GetWindowRect(child, &rcDlg); 
   CopyRect(&rc, &rcOwner); 
   
   // Offset the owner and dialog box rectangles so that 
   // right and bottom values represent the width and 
   // height, and then offset the owner again to discard 
   // space taken up by the dialog box. 
   
   OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top); 
   OffsetRect(&rc, -rc.left, -rc.top); 
   OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom); 
   
   // The new position is the sum of half the remaining 
   // space and the owner's original position. 
   
   SetWindowPos(child, 
      HWND_TOP, 
      rcOwner.left + (rc.right / 2), 
      rcOwner.top + (rc.bottom / 2), 
      0, 0,          // ignores size arguments 
      SWP_NOSIZE); 

}
ERROR_MSG_CATCH("win32ext::center_child_window(HWND child)");

std::string win32ext::remove_return_file_extension(std::string &file_name)
try
{
   string::size_type pos;
   pos=file_name.find_last_of(".");
   if((pos==0)||(pos==string::npos)) return "";
   string ext=file_name.substr(pos+1);
   file_name.erase(pos);
   return ext;
}
ERROR_MSG_CATCH("win32ext::remove_return_file_extension(std::string &file_name)");

std::string win32ext::remove_return_file_name(std::string &full_file_name)
try
{//return the filename, full_file_name contain the path to the file
   string::size_type pos;
   pos=full_file_name.find_last_of("\\/");
   if((pos==0)||(pos==string::npos)) return full_file_name;
   string file_name=full_file_name.substr(pos+1);
   full_file_name.erase(pos);
   return file_name;
}
ERROR_MSG_CATCH("win32ext::remove_return_file_name(std::string &full_file_name)");

std::string win32ext::get_module_path()   
try
{
   string s=get_module_file_name();
   string::size_type pos;
   pos=s.find_last_of("\\/");
   if(pos==string::npos)
   {
      throw error_msg(-1,"Internal Error: win32ext::get_module_file_name() returned an absolute file name without any slash");
   }
   if(pos==s.length())
   {
      throw error_msg(-1,"Internal Error: win32ext::get_module_file_name() returned a file name ended by a slash");      
   }
   //s.erase(pos+1);
   s.erase(pos);
   return s;
}
ERROR_MSG_CATCH("win32ext::get_module_path()");

std::string win32ext::get_module_file_name()
try
{
   string s;
   char *module_file_name=new char[WIN32EXT_MAX_PATH_SIZE];
   if(module_file_name==0) throw error_msg(-1,"Can't allocate dynamic memomy");
   if(GetModuleFileName(NULL,module_file_name,WIN32EXT_MAX_PATH_SIZE)==0)
   {
      throw error_msg(-1,"Win32 function GetModuleFileName() failed");
   }
   s=module_file_name;
   if(module_file_name!=0) delete[] module_file_name;
   return s;
}
ERROR_MSG_CATCH("win32ext::get_module_file_name()");

int win32ext::compare_last_write_time(std::string file_nameA, std::string file_nameB)
try
{
   OutputDebugString("int win32ext::compare_last_write_time(std::string file_nameA, std::string file_nameB)");
      
   HANDLE fileA=CreateFile( file_nameA.c_str(),
               0,//DWORD dwDesiredAccess=0 -> just test the existence of the file
               0,//DWORD dwShareMode,
               0,//LPSECURITY_ATTRIBUTES lpSecurityAttributes,
               OPEN_EXISTING,//DWORD dwCreationDisposition,
               0,//DWORD dwFlagsAndAttributes,
               NULL//HANDLE hTemplateFile
               );
   if(fileA==INVALID_HANDLE_VALUE)
   {//the file doesn't exist
      OutputDebugString("CreateFile fileA error");
      return file_A_not_found;
   } 
   FILETIME writeTimeA;
   if(!GetFileTime(fileA,NULL,NULL,&writeTimeA))
   {
      OutputDebugString("GetFileTime fileA error");
      CloseHandle(fileA);
      return unknown_error;
   }
   CloseHandle(fileA);
   HANDLE fileB=CreateFile( file_nameB.c_str(),
               0,//DWORD dwDesiredAccess=0 -> just test the existence of the file
               0,//DWORD dwShareMode,
               0,//LPSECURITY_ATTRIBUTES lpSecurityAttributes,
               OPEN_EXISTING,//DWORD dwCreationDisposition,
               0,//DWORD dwFlagsAndAttributes,
               NULL//HANDLE hTemplateFile
               );
   if(fileB==INVALID_HANDLE_VALUE)
   {//the file doesn't exist
      OutputDebugString("CreateFile fileB error");
      return file_B_not_found;
   }  

   FILETIME writeTimeB;
   if(!GetFileTime(fileB,NULL,NULL,&writeTimeB))
   {
      OutputDebugString("GetFileTime fileB error");
      CloseHandle(fileB);
      return unknown_error;
   }
   CloseHandle(fileB);
   int status=CompareFileTime(&writeTimeA,&writeTimeB);
   switch(status)
   {
      case 0:
         OutputDebugString("same_write_time");
         return same_write_time;
      case -1:
         OutputDebugString("newer_is_B");
         return newer_is_B;
      case 1:
         OutputDebugString("newer_is_A");
         return newer_is_A;
      default:
         OutputDebugString("unknown_error");
         return unknown_error;
   }
}
ERROR_MSG_CATCH("win32ext::compare_last_write_time(std::string file_nameA, std::string file_nameB)");

bool win32ext::is_file_exist(std::string file_name)
try
{
   OutputDebugString("win32ext::is_file_exist(std::string file_name)");
   OutputDebugString(file_name.c_str());
   relative_path_to_absolute(file_name);
   OutputDebugString(file_name.c_str());
   HANDLE file=CreateFile( file_name.c_str(),
               0,//DWORD dwDesiredAccess=0 -> just test the existence of the file
               0,//DWORD dwShareMode,
               0,//LPSECURITY_ATTRIBUTES lpSecurityAttributes,
               OPEN_EXISTING,//DWORD dwCreationDisposition,
               0,//DWORD dwFlagsAndAttributes,
               NULL//HANDLE hTemplateFile
               );
   if(file!=INVALID_HANDLE_VALUE)
   {//the file is there
      CloseHandle(file);
      return true;
   }
   return false;
}
ERROR_MSG_CATCH("win32ext::is_file_exist(std::string file_name)");

std::string win32ext::get_file_name(std::string window_title, file_existence exist_spec, const char *filter, std::string def_ext)
try
{
    char file_name[WIN32EXT_MAX_PATH_SIZE];
    file_name[0]=0;
    OPENFILENAME ofn;       // common dialog box structure
    ofn.lStructSize=sizeof(OPENFILENAME);
    ofn.hwndOwner=NULL;
    ofn.hInstance=0; //ignored in our case
    
    if(filter==0)
      ofn.lpstrFilter = "All files\0*.*\0";
    else
      ofn.lpstrFilter = filter;
      
    ofn.lpstrCustomFilter=NULL; //no custom filter
    ofn.nMaxCustFilter=0; //ignored in our case
    ofn.nFilterIndex = 1;
    ofn.lpstrFile=file_name;
    ofn.nMaxFile=WIN32EXT_MAX_PATH_SIZE;
    ofn.lpstrFileTitle=NULL;
    ofn.nMaxFileTitle=0; //ignored in our case
    ofn.lpstrInitialDir=NULL;
    ofn.lpstrTitle=window_title.c_str();
    ofn.Flags=  //OFN_ALLOWMULTISELECT|
                //OFN_EXPLORER|
                (OFN_FILEMUSTEXIST * exist_spec)|
                OFN_PATHMUSTEXIST;
                //OFN_SHOWHELP;
                //OFN_FORCESHOWHIDDEN
                
    ofn.nFileOffset=0;
    //ofn.nFileExtension=NULL;
    ofn.lpstrDefExt=def_ext.c_str();
    ofn.lCustData=0;
    ofn.lpfnHook=NULL;
    ofn.lpTemplateName=NULL;
    //ofn.pvReserved=NULL;
    //ofn.dwReserved=0;
    //ofn.FlagsEx=0;
                
    if(GetOpenFileName(&ofn)==0)
        return "";
    
    char *cur_file_name=ofn.lpstrFile+ofn.nFileOffset;
    string s;
    //no support for multi file selection
    //while(cur_file_name[0])
    if(cur_file_name[0])
    {
        string full_name=ofn.lpstrFile;
        if(full_name.length()<ofn.nFileOffset)
        {//if the user has selected more than one file
            full_name+="\\";
            full_name+=cur_file_name;  
            cur_file_name+=strlen(cur_file_name)+1;  
        }
        else
        {
            cur_file_name+=strlen(cur_file_name);
        }
        s=full_name.c_str();
    }
   return s;
}
ERROR_MSG_CATCH("win32ext::get_file_name(std::string window_title, file_existence file_must_exist, const char *filter, std::string def_ext)");

std::string win32ext::get_current_directory()
try
{
   char *cur_path=new char[WIN32EXT_MAX_PATH_SIZE];
   if(cur_path==0) throw error_msg(-1,"Can't allocate dynamic memomy");
   if(GetCurrentDirectory(WIN32EXT_MAX_PATH_SIZE,cur_path)==0)
   {
      throw error_msg(-1,"Win32 function GetCurrentDirectory() failed, maybe you have a path too long");
   }
   string s=cur_path;
   if(cur_path) delete cur_path;
   OutputDebugString("Current directory:");
   OutputDebugString(s.c_str());
   return s;
}
ERROR_MSG_CATCH("win32ext::get_current_directory()");

void win32ext::relative_path_to_absolute(std::string &path)
try
{
   OutputDebugString("relative_path_to_absolute_path:");
   if(path.empty()) throw error_msg("Path is empty");
   if(path[0]=='.')
   {//convert the relative path into an absolute one
      OutputDebugString("win32ext::relative_path_to_absolute(): relative path detected");
      OutputDebugString(path.c_str());
      string::size_type pos=path.find_first_of("/\\");
      string slash, point_slash;
      if(pos!=string::npos)
      {
         slash=path[pos];
         point_slash="."+slash;
      }
      else
      {
         slash="\\";
         point_slash=".\\";
      }
      //OutputDebugString(path.c_str());
      string slash_point_slash=slash+point_slash;
      stringext::replace(path, slash_point_slash,slash);//replace all '\.\' by '\'
      if(path.substr(0,2)==point_slash)
      {
         path.erase(0,2);//erase heading point and slash
      }
      //OutputDebugString(Path.c_str());
      string absolute_path;
      absolute_path=get_current_directory();
      
      string point_point_slash="."+point_slash;
      //OutputDebugString("point_point_slash:");
      //OutputDebugString(point_point_slash.c_str());
      string::size_type rel_point_point_slash_pos=0, abs_slash_pos=absolute_path.length()-1;//-1 to skip any trailing slash
      rel_point_point_slash_pos=path.find(point_point_slash);
      OutputDebugString("pointpointslash loop:");
      while(rel_point_point_slash_pos!=string::npos)
      {
         OutputDebugString(path.c_str());
         if(rel_point_point_slash_pos!=0)
         {
            absolute_path+=slash+path.substr(0,rel_point_point_slash_pos);  
         }
         path.erase(rel_point_point_slash_pos,point_point_slash.length());
         abs_slash_pos=absolute_path.rfind(slash);
         if(abs_slash_pos==string::npos) throw error_msg(-1,"Incorrect relative path encountred");
         absolute_path.erase(abs_slash_pos);
         OutputDebugString(absolute_path.c_str());
         rel_point_point_slash_pos=path.find(point_point_slash,rel_point_point_slash_pos);      
      }
      path=absolute_path+slash+path;
   }
   else
   {
      if(path.length()<2 || path[1]!=':')//the input was just a file name without any path
      {
         path=get_current_directory()+"\\"+path;
      }
   }
   if((path.at(path.length()-1)=='\\')||(path.at(path.length()-1)=='/'))
   {
      path.erase(path.length()-1);
   }
   OutputDebugString(path.c_str());
}
ERROR_MSG_CATCH("win32ext::relative_path_to_absolute(std::string &path)");

bool win32ext::create_long_directory(const std::string &path)
try
{
   //make sure that we pass an absolute path to CreateDirectoryW
   //we must do that to make CreateDirectoryW accept path longer than 255 characters...
   string p=path;
   relative_path_to_absolute(p);
   //convert the string to unicode
   p="\\\\?\\"+p;
   int status=MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,p.c_str(),p.length()+1,NULL,0);
   if(status==0)
   {
      string msg;
      switch(GetLastError())
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
      msg="win32ext::create_long_directory('"+path+"'): MultiByteToWideChar failed: "+msg;
      OutputDebugString(msg.c_str());
      throw error_msg(-1,msg);
   }
   wchar_t *wp = new wchar_t[status];
   status=MultiByteToWideChar(CP_ACP,MB_ERR_INVALID_CHARS,p.c_str(),p.length()+1,wp,status);
   if(status==0)
   {
      string msg;
      switch(GetLastError())
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
      msg="win32ext::create_long_directory('"+path+"'): MultiByteToWideChar failed: "+msg;
      OutputDebugString(msg.c_str());
      throw error_msg(-1,msg);    
   }
   //create the directory
   status=CreateDirectoryW(wp,NULL);
   if(status==0)
   {
      OutputDebugStringW(wp);
      string msg="CreateDirectoryW() failed, see previous OutputDebugString";
      OutputDebugString(msg.c_str());
      //throw error_msg(-1,msg);
   }
   if(wp) delete[] wp;
   return status != 0;
   
}
ERROR_MSG_CATCH("win32ext::create_long_directory(const std::string &Path)");

bool win32ext::is_directory(const std::string &path)
try
{
   //make sure that we pass an absolute path to GetFileAttributesW
   //we must do that to make GetFileAttributesW accept path longer than 255 characters...
   string p=path;
   relative_path_to_absolute(p);
   //prepend "\\?\" to the path (GetFileAttributesW requirement)
   p="\\\\?\\"+p;
   //convert the string to unicode
   wchar_t *wp=stringext::ansi2unicode(p);
   //get info
   unsigned int status=GetFileAttributesW(wp);
   if(status==INVALID_FILE_ATTRIBUTES)
   {
      OutputDebugStringW(wp);
      stringstream msg;
      msg<<"GetFileAttributesW() failed to retrieve information about the following file or directory:"<<endl;
      msg<<path;
      throw error_msg(msg.str().c_str());
   }
   if(wp) delete[] wp;
   if(status&FILE_ATTRIBUTE_DIRECTORY)
      return true;
   return false;
   
}
ERROR_MSG_CATCH("win32ext::is_directory(const std::string &path)");

void win32ext::find_files(const std::string &file_name_filter, std::list<std::string> *results)
try
{
   WIN32_FIND_DATAW FindFileData;
   HANDLE hFind;
   
   string abs_filter=file_name_filter;
   cout<<"file filter: \""<<abs_filter<<"\""<<endl;
   //delete trailing slash of any
   if(abs_filter.length()>2 && abs_filter[abs_filter.length()-1]=='\\')
      abs_filter.substr(0,abs_filter.length()-1);
   
   relative_path_to_absolute(abs_filter);
   cout<<"file filter: \""<<abs_filter<<"\""<<endl;
   string path=abs_filter;
   remove_return_file_name(path);
   cout<<"path=\""<<path<<"\""<<endl;
   //abs_filter="\\\\?\\"+abs_filter;
   wchar_t *w_file_name_filter=stringext::ansi2unicode("\\\\?\\"+abs_filter);
   
   hFind = FindFirstFileW(w_file_name_filter, &FindFileData);
   if(w_file_name_filter) delete[] w_file_name_filter;
   
   do
   {
      if (hFind == INVALID_HANDLE_VALUE) 
      {
         if(GetLastError()!=ERROR_FILE_NOT_FOUND)
         {
            stringstream msg;
            msg<<"System error while searching a file, please check the following path:"<<endl;
            msg<<abs_filter<<endl;
            msg<<"Returned error:"<<endl;
            msg<<"Invalid File Handle. GetLastError reports "<<GetLastError();
            throw error_msg(msg.str());
         }
         break;
      }
      if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
      {
         string file=stringext::unicode2ansi(FindFileData.cFileName);
         cout<<"  file found: *"<<file<<"*"<<endl;
         file=path+"\\"+file;
         cout<<"  absolute path: \n*"<<file<<"*"<<endl;
         
         results->push_back(file);
      }
   }
   while(FindNextFileW(hFind,&FindFileData));
   cout<<"  "<<results->size()<<" file(s) found"<<endl;
   FindClose(hFind);
}
ERROR_MSG_CATCH("win32ext::find_files(const std::string &file_name_filter, std::vector<std::string> *results)");
