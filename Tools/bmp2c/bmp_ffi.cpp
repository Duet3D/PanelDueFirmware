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


//see header files for an introduction to this class

#include <iostream>
using namespace std;
#include "error_msg.h"
#include "bmp_ffi.h"
#include <fstream>
#include <sstream>
#include <math.h>
#include <utility>

const unsigned short bmfh::magic_number=19778;//"BM"

archive & operator<<(archive &s, const rgba & c)
{
   s<<c.blue;
   s<<c.green;
   s<<c.red;
   s<<c.alpha;
   return s;
}

archive & operator>>(archive &s, rgba &c)
{
   s>>c.blue;
   s>>c.green;
   s>>c.red;
   s>>c.alpha;
   return s;
}

archive & operator<<(archive &s, const bmfh & h)
{
   s<<h.type;
   s<<h.size;
   s<<h.reserved1;
   s<<h.reserved2;
   s<<h.offset_bits;
   return s;
}

archive & operator>>(archive &s, bmfh & h)   
{                                                  
   s>>h.type;                                      
   s>>h.size;                                      
   s>>h.reserved1;                                 
   s>>h.reserved2;                                 
   s>>h.offset_bits;  
   return s;
}                                                  

archive & operator<<(archive &s, const bmih & h)
{
   s<<h.size;
   s<<h.width;
   s<<h.height;
   s<<h.planes;
   s<<h.bit_count;
   s<<h.compression;
   s<<h.size_image;
   s<<h.x_pixels_per_meter;
   s<<h.y_pixels_per_meter;
   s<<h.colors_used;
   s<<h.colors_important;
   return s;
}

archive & operator>>(archive &s, bmih & h)
{
   s>>h.size;
   s>>h.width;
   s>>h.height;
   s>>h.planes;
   s>>h.bit_count;
   s>>h.compression;
   s>>h.size_image;
   s>>h.x_pixels_per_meter;
   s>>h.y_pixels_per_meter;
   s>>h.colors_used;
   s>>h.colors_important;
   return s;
}


bmp_ffi::bmp_ffi()
{
}
	
bmp_ffi::bmp_ffi(const unsigned int x, const unsigned int y, const unsigned int bit_count)
{
	m_bmih.width=x;
	m_bmih.height=y;
	m_bmih.bit_count=bit_count;
	rgba black;
   m_pixels.resize(m_bmih.width*m_bmih.height, black);
}

bmp_ffi::bmp_ffi(const bmp_ffi & a)
{
	*this=a;
}

bmp_ffi::~bmp_ffi()
{
}

bmp_ffi & bmp_ffi::operator=(const bmp_ffi & a)
{
	m_bmih.width=a.m_bmih.width;
	m_bmih.height=a.m_bmih.height;
	m_bmih.bit_count=a.m_bmih.bit_count;
	m_pixels=a.m_pixels;
	return *this;
}

void bmp_ffi::clear()
{
   m_bmih.width=0;
   m_bmih.height=0;
   m_pixels.clear();
}

rgba& bmp_ffi::operator()(const unsigned int x,const unsigned int y)
try
{
   return m_pixels.at(x+y*m_bmih.width);
}
ERROR_MSG_CATCH("rgba& bmp_ffi::operator()(const unsigned int x,const unsigned int y)");

const rgba& bmp_ffi::operator()(const unsigned int x,const unsigned int y) const
try
{
   return m_pixels.at(x+y*m_bmih.width);
}
ERROR_MSG_CATCH("const rgba& bmp_ffi::operator()(const unsigned int x,const unsigned int y) const");

unsigned int bmp_ffi::get_x()
{
	return this->m_bmih.width;
}

unsigned int bmp_ffi::get_y()
{
	return this->m_bmih.height;
}

unsigned int bmp_ffi::get_bit_count()
{
	return this->m_bmih.bit_count;
}

void bmp_ffi::set_size(const unsigned int x, const unsigned int y)
{
	m_bmih.width=x;
	m_bmih.height=y;
	rgba black;
   m_pixels.resize(m_bmih.width*m_bmih.height, black);
}

void bmp_ffi::set_bit_count(const unsigned int bit_count)
{
	m_bmih.bit_count=bit_count;
}

unsigned int bmp_ffi::get_buffer_size() const
{
	return m_pixels.size()*sizeof(rgba);
}

unsigned int bmp_ffi::get_size() const
{
	return sizeof(bmp_ffi)+this->get_buffer_size();
}

archive & operator<<(archive &s,const bmp_ffi & p)
try
{
   if(p.m_bmih.bit_count!=24)
	{
	   throw error_msg("bit_count!=24 not supported for write operation");
	}

   unsigned int bytes_per_pixel=p.m_bmih.bit_count/8;
   unsigned int data_bytes_per_line=bytes_per_pixel*p.m_bmih.width;

   unsigned int padding=(4 - ( data_bytes_per_line % 4))%4;   
   unsigned int bytes_per_line=padding+data_bytes_per_line;

   unsigned int total_pixel_byte_count=p.m_bmih.height*bytes_per_line;   
   unsigned int header_size=sizeof(bmfh)-10+sizeof(bmih);
   
   bmfh file_header=p.m_bmfh;
   file_header.size = header_size+total_pixel_byte_count; 
   file_header.offset_bits = header_size;
    
   bmih info_header=p.m_bmih;
   info_header.size_image = total_pixel_byte_count;   
   info_header.colors_used = 0;
   info_header.colors_important = 0;


   //header
	s<<file_header;
	s<<info_header;
	//end of the header
		
   //24 bits bitmap, write pixels
   unsigned int j;
   int i;
   for(i=p.m_bmih.height-1;i>=0;i--)
   {
      for(j=0;j<p.m_bmih.width;j++)
      {
          s<<p(j,i).blue;
          s<<p(j,i).green;
          s<<p(j,i).red;
      }
      for (j=0; j<padding;j++) 
      {
          s<<(unsigned char)0;
      }
   }
	return s;
}
ERROR_MSG_CATCH("archive & operator<<(archive &s,const bmp_ffi & p)");

archive & operator>>(archive &s, bmp_ffi &p)
try
{
   //header
	s>>p.m_bmfh;
   
   if(p.m_bmfh.type!=bmfh::magic_number)
      throw error_msg(BMPFFI_WRONG_MAGIC, BMPFFI_WRONG_MAGIC_MSG);
   
   s>>p.m_bmih;
   rgba black;//set default value for all component (important in case of alpha)
   p.m_pixels.resize(p.m_bmih.width*p.m_bmih.height, black);
   
	switch(p.m_bmih.bit_count)
	{
	   case 8:
	   {
         char c;
         //read palette
         unsigned int n_colors = ((int) p.m_bmfh.offset_bits - 54 )/4;  
         n_colors=std::max(n_colors,(unsigned int)256);
         rgba *colors=new rgba[256];
         for(unsigned int n=0; n < n_colors ; n++ )
         {
            s>>colors[n];     
         }
         for(unsigned int n=n_colors ; n < 256 ; n++ )
         { 
            colors[n].red = 255;
            colors[n].green = 255;
            colors[n].blue = 255;
            colors[n].alpha = 0;
         }
         //read pixels
         unsigned int bf,j;
         int i;
         unsigned char index;
         bf = ((p.m_bmih.width+3)/4) * 4;
         for(i=p.m_bmih.height-1;i>=0; i--)
         {
            for(j=0; j<p.m_bmih.width;j++)
            {
               s>>index;
               p(j,i)=colors[index];
            }
            for(j=p.m_bmih.width; j<bf; j++)
            {
               s>>c;
            }
         }
      }
      break;
      case 24:     
   	{
   		//24 bits bitmap, read pixels
   		unsigned int bf,j;
   		int i;
         bf = (4-((p.m_bmih.width*3)%4)) & 0x03; // padding 32 bits.
         for(i=p.m_bmih.height-1;i>=0;i--)
         {
            for(j=0;j<p.m_bmih.width;j++)
            {
               s>>p(j,i).blue;
               s>>p(j,i).green;
               s>>p(j,i).red;
            }
            for (j=0; j<bf;j++)
            {
         		char Pad;
                s>>Pad;
            }
         }
   	}
   	break;
   	case 32:     
   	{
   		//24 bits bitmap, read pixels
   		unsigned int j;
   		int i;
         for(i=p.m_bmih.height-1;i>=0;i--)
         {
            for(j=0;j<p.m_bmih.width;j++)
            {
               s>>p(j,i);
            }
         }
   	}
   	break;
   	default:
   	{
   	   stringstream msg;
         msg<<p.m_bmih.bit_count<<" bit bmp file reading not yet implemented !";
         throw error_msg(msg.str()); 
      }
   }
   return s;
}
ERROR_MSG_CATCH("archive & operator>>(archive &s, bmp_ffi &p)");
