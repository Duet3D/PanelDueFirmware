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

#define BMP2C_VERSION 0002
#define BMP2C_MIN_INI_VERSION 0

#define NEW_LINE_THRESHOLD_IN_OUTPUT_SOURCE_FILE 80

#define CST_BIT_BASE 0x0fffffff

#include <iostream>
#include <sstream>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <cctype>
#include <cstdint>
using namespace std;

#include "bmp_ffi.h"
#include "archive.h"
#include "error_msg.h"
#include "math.h"
#include "string_ext.h"
#include "win32ext.h" //libcomdlg32.a must be included in the project (can be comdlg32.lib on some compilers)
#include "numconv_stringstream.h"

#undef min
#undef max

////////////////////////////////////////////////////////////////////////////////////////////////
//                        Global variables (used only in this file)                           //
////////////////////////////////////////////////////////////////////////////////////////////////
//parameters from command line:
string input_full_file_name;
string src_output_file_name;
string input_file_name;

//parameters that were originally from ini file:
string x_decl = "const uint16_t xSize = ";
string x_decl_postfix = "; //width of the picture";
string y_decl = "const uint16_t ySize = ";
string y_decl_postfix = ";//height of the picture";
string array_decl = "static const uint16_t pic[] =";
unsigned int data_size = 16;
string data_map = "r7  r6  r5  r4  r3                g7  g6  g5  g4  g3  g2           b7  b6  b5  b4  b3";
unsigned int pause;
string src_endl_param = "CR_LF";

//other global variables:
vector<int> src2dest;
string src_endl;
bool compress = false;
bool binary = false;

const unsigned int MaxRepeatCount = 65536;

/////////////////////////////////////////////////////////////////////////////////////////////////
//                            Functions (used only in this file)                               //
/////////////////////////////////////////////////////////////////////////////////////////////////


void prepare_parameters()
try
{
   if(data_size>32) throw error_msg("data_size must be <=32");

   input_file_name = input_full_file_name;
   input_file_name = win32ext::remove_return_file_name(input_file_name);
   win32ext::remove_return_file_extension(input_file_name);
   
   string input_file_name_id=input_file_name;
   stringext::replace_each_of(input_file_name_id, ".\\/:?-&~^|*%+=∞≤#\"'()`@$£µß!;,<>ÈË˘‡ÙÍ˚Ó‚", "_");
 
   stringext::replace(x_decl, "#bmp2c_input_full_file_name#", input_full_file_name);
   stringext::replace(x_decl, "#bmp2c_input_file_name#", input_file_name);
   stringext::replace(x_decl, "#bmp2c_input_file_name_id#", input_file_name_id); 

   stringext::replace(x_decl_postfix, "#bmp2c_input_full_file_name#", input_full_file_name);
   stringext::replace(x_decl_postfix, "#bmp2c_input_file_name#", input_file_name);
   stringext::replace(x_decl_postfix, "#bmp2c_input_file_name_id#", input_file_name_id); 

   stringext::replace(y_decl, "#bmp2c_input_full_file_name#", input_full_file_name);
   stringext::replace(y_decl, "#bmp2c_input_file_name#", input_file_name);
   stringext::replace(y_decl, "#bmp2c_input_file_name_id#", input_file_name_id); 

   stringext::replace(y_decl_postfix, "#bmp2c_input_full_file_name#", input_full_file_name);
   stringext::replace(y_decl_postfix, "#bmp2c_input_file_name#", input_file_name);
   stringext::replace(y_decl_postfix, "#bmp2c_input_file_name_id#", input_file_name_id); 
   
   stringext::replace(array_decl, "#bmp2c_input_full_file_name#", input_full_file_name);
   stringext::replace(array_decl, "#bmp2c_input_file_name#", input_file_name);
   stringext::replace(array_decl, "#bmp2c_input_file_name_id#", input_file_name_id);
   
   stringext::remove_white_spaces(data_map);
   
   if(src_endl_param=="CR_LF") src_endl="\r\n";
   if(src_endl_param=="LF") src_endl="\n";
   if(src_endl_param=="CR") src_endl="\r";
   if(src_endl.empty()) throw error_msg("src_endl_param must be one of the following vaues:\n"
                                        "'CR_LF'\n'LF'\n'CR'");
}
ERROR_MSG_CATCH("prepare_parameters()")

void compute_bit_manipulation()
try
{
   unsigned int i=0;
   while(i<data_map.length())
   {
      switch(data_map[i])
      {
         case 'c':
            i++;
            src2dest.push_back(data_map.at(i)-'0'+CST_BIT_BASE);
            break;
         case 'a':
            i++;
            src2dest.push_back(data_map.at(i)-'0'+24);
            break;
         case 'A':
            i++;
            src2dest.push_back(data_map.at(i)-'0'-24-1);
            break;
         case 'r':
            i++;
            src2dest.push_back(data_map.at(i)-'0'+16);
            break;
         case 'R':
            i++;
            src2dest.push_back(-data_map.at(i)+'0'-16-1);
            break;
         case 'g':
            i++;
            src2dest.push_back(data_map.at(i)-'0'+8);
            break;
         case 'G':
            i++;
            src2dest.push_back(-data_map.at(i)+'0'-8-1);
            break;
         case 'b':
            i++;
            src2dest.push_back(data_map.at(i)-'0');
            break;
         case 'B':
            i++;
            src2dest.push_back(-data_map.at(i)+'0'-1);
            break;
         default:
            throw error_msg(1,"unrecognized command in data_map");
      }
      i++;
   }
   if(src2dest.size()>data_size) throw error_msg(1,"data_map defines too much destination bits");
}
ERROR_MSG_CATCH("compute_bit_manipulation()")

//////////////////////////
// do_bit_manipulation  //
//////////////////////////
//
//perform bit manipulation on unsigned int value.
//the input value is transformed according to the operations stored in the bit_assignation vector
//each element of the vector describe the value of the corresponding bit 
//(the output value is always right justified)
//if the element is CST_BIT_BASE or CST_BIT_BASE+1, constant bit 0 or 1 respectively will be copied
//if the element is below CST_BIT_BASE, a bit from the input value will be copied
//    the value of the element indicates the index of the bit to copy
//
//example: 
//input value (in binary) = 01000001
//bit_assignation    = c1 c0  0  1   |  6  7 c0 c0 c1  (c1=CST_BIT_BASE+1 and c0=CST_BIT_BASE)              
//output (in binary) =  1  0  1  0   |  1  0  0  0  1
void do_bit_manipulation(unsigned int input, unsigned int &output, vector<int> bit_assignation)
try
{
   for(unsigned int bit_cnt=0;bit_cnt<bit_assignation.size();bit_cnt++)
   {
      unsigned int dest_bit=bit_assignation.size()-bit_cnt-1;
      bool bit;
      //compute bit value
      if(bit_assignation[bit_cnt]>=CST_BIT_BASE)//this bit is stuck to a constant value
      {
         if(bit_assignation[bit_cnt]>CST_BIT_BASE+1)
            throw error_msg("internal error: bit_assignation[bit_cnt]>CST_BIT_BASE+1");
         bit=(bit_assignation[bit_cnt]-CST_BIT_BASE != 0);
      }
      else
      {
         if(bit_assignation[bit_cnt]>=0)
            bit=(input>>bit_assignation[bit_cnt])&0x01;//get the pointed bit
         else
            bit=!((input>>(-bit_assignation[bit_cnt]-1))&0x01);//get the complement of the pointed bit
      }
      //modify output
      if(bit==0)
         output&=~(((unsigned int)bit)<<dest_bit);
      else
         output|=(((unsigned int)bit)<<dest_bit);
   }
}
ERROR_MSG_CATCH("do_bit_manipulation(unsigned int input, unsigned int &output, vector<int> bit_assignation)")

int main(int argc, char **argv)
try
{
	if (argc < 2)
	{
		cout << "Usage: " << argv[0] << " input_file.bmp [output_file.cpp] [-b] [-c] [-p]" << endl;
		return 0;
	}

	while (argc > 1)
	{
		--argc;
		++argv;
		if ((*argv)[0] == '-')
		{
			switch (tolower((*argv)[1]))
			{
			case 'b':
				binary = true;
				break;
			case 'c':
				compress = true;
				break;
			case 'p':
				pause = true;
				break;
			default:
				throw error_msg(string("Unknown option: ") + *argv);
			}
		}
		else if (input_full_file_name.empty())
		{
			input_full_file_name = *argv;
		}
		else if (src_output_file_name.empty())
		{
			src_output_file_name = *argv;
		}
		else
		{
			throw error_msg(string("Too many parameters"));
		}
	}

	if (input_full_file_name.empty())
	{
		throw error_msg(string("No input filename given"));
	}
	if (src_output_file_name.empty())
	{
		src_output_file_name = input_full_file_name;
		win32ext::remove_return_file_extension(src_output_file_name);
		src_output_file_name += (binary) ? ".bin" : ".h";
	}
   
	prepare_parameters();
   
	compute_bit_manipulation();

	//open input bmp file
	archive fin(input_full_file_name.c_str(), ios::in);
	if(!fin)
	{
		stringstream msg;
		msg<<"can't open file '"<<input_full_file_name<<"'"<<endl;
		throw error_msg(msg.str());
	}

	//create an file format interface (ffi) for the bmp picture
	bmp_ffi bmp;
	cout<<"Read input file..."<<endl;
	fin>>bmp; //read from input file
	fin.close();//close it

	//create output bmp file
	ofstream fout(src_output_file_name.c_str(), ios::out|ios::binary);
	if(!fout)
	{
		stringstream msg;
		msg<<"can't create file '"<<src_output_file_name<<"'"<<endl;
		throw error_msg(msg.str());
	}
	
	cout<<"Write to output file..."<<endl;
	unsigned int x=bmp.get_x();
	unsigned int y=bmp.get_y();
	
	if (binary)
	{
		fout.write(reinterpret_cast<const char*>(&x), 2);
		fout.write(reinterpret_cast<const char*>(&y), 2);
	}
	else
	{
		// Build the variable name
		string varname;
		for (auto p = input_file_name.begin(); p != input_file_name.end(); ++p)
		{
			const char c = *p;
			if (isalpha(c) || isdigit(c) || c == '_')
			{
				varname += c;
			}
		}
		fout << "// File generated by bmp2c_escher3d" << src_endl << src_endl;
		fout << "#include <cstdint>" << src_endl << src_endl;
		fout << "extern const uint16_t " << varname << "[] =" << src_endl << "{" << src_endl;
		fout << "\t" << x << ", " << y << ",\t\t\t// width and height in pixels" << src_endl;
		fout << "\t";
	}

	unsigned int line_char_cnt = 0;	
	unsigned int repeatCount = 0;
	unsigned int lastData = 0;

	if (compress)
	{
		for (unsigned int j = y; j != 0; )
		{
			--j;
			for (unsigned int i = 0; i < x; i++)
			{
				//limit line length in output source file 
				if (!binary && line_char_cnt > NEW_LINE_THRESHOLD_IN_OUTPUT_SOURCE_FILE)
				{
					line_char_cnt=0;
					fout<<src_endl<<"\t";
				}
			
				//read original pixel and store it into a 32 bit value as follow 0xaarrggbb
				unsigned int input_dword=0;
				input_dword|=((unsigned int)(bmp(i,j).alpha))<<24;
				input_dword|=((unsigned int)(bmp(i,j).red))<<16;
				input_dword|=((unsigned int)(bmp(i,j).green))<<8;
				input_dword|=bmp(i,j).blue;

				//compute value to write in the output source file
				unsigned int output_dword=0;
				do_bit_manipulation(input_dword, output_dword, src2dest);
				//write it
				stringstream outstr;
				if (repeatCount != 0 && (lastData != output_dword || repeatCount == MaxRepeatCount))
				{
					if (binary)
					{
						const uint16_t rc = repeatCount - 1;
						fout.write(reinterpret_cast<const char *>(&rc), 2);
						fout.write(reinterpret_cast<const char *>(&lastData), 2);
					}
					else
					{
						outstr<<"0x"<<hex<<setw(data_size/4)<<setfill('0')<<(repeatCount-1);
						outstr<<", 0x"<<hex<<setw(data_size/4)<<setfill('0')<<lastData;
						outstr<<", ";
					}
					repeatCount = 0;
				}
				lastData = output_dword;
				++repeatCount;
				if (i+1 == x && j == 0)
				{
					if (binary)
					{
						const uint16_t rc = repeatCount - 1;
						fout.write(reinterpret_cast<const char *>(&rc), 2);
						fout.write(reinterpret_cast<const char *>(&lastData), 2);
					}
					else
					{
						outstr<<"0x"<<hex<<setw(data_size/4)<<setfill('0')<<(repeatCount-1);
						outstr<<", 0x"<<hex<<setw(data_size/4)<<setfill('0')<<lastData;
					}
				}
				if (!binary)
				{
					fout<<outstr.str();
					line_char_cnt+=outstr.str().length();
				}
			}
		}
	}
	else
	{
		for (unsigned int i=0;i<x;i++)
		{
			for (unsigned int j=0;j<y;j++)
			{
				//limit line length in output source file 
				if (!binary && line_char_cnt > NEW_LINE_THRESHOLD_IN_OUTPUT_SOURCE_FILE)
				{
					line_char_cnt=0;
					fout<<src_endl;
				}
			
				//read original pixel and store it into a 32 bit value as follow 0xaarrggbb
				unsigned int input_dword=0;
				input_dword|=((unsigned int)(bmp(i,j).alpha))<<24;
				input_dword|=((unsigned int)(bmp(i,j).red))<<16;
				input_dword|=((unsigned int)(bmp(i,j).green))<<8;
				input_dword|=bmp(i,j).blue;

				//compute value to write in the output source file
				unsigned int output_dword=0;
				do_bit_manipulation(input_dword, output_dword, src2dest);
				//write it
				if (binary)
				{
					fout.write(reinterpret_cast<const char*>(&output_dword), 2);
				}
				else
				{
					stringstream outstr;
					outstr<<"0x"<<hex<<setw(data_size/4)<<setfill('0')<<output_dword;
					if(i+1<x || j+1<y)
					{
						outstr<<", ";
					}
					fout<<outstr.str();
					line_char_cnt+=outstr.str().length();
				}
			}
		}
	}

	//close output source file
	if (!binary)
	{
		fout<<src_endl<<"};"<<src_endl<<src_endl;
	}
	fout.close();

	string preview_bmp_file_name=src_output_file_name+"_preview.bmp";
	if(pause) {system("pause");}
	return 0;
}
catch(error_msg e)
{
	cout <<e.get_err_text()<<endl<<"Error code "<<e.get_err_code()<<endl;
	system("pause");
	return e.get_err_code();
}
catch(...)
{
	cout <<"unknown exception caught !"<<endl;
	system("pause");
	return -1;
}

