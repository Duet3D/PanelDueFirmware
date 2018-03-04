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

/*
bmp_ffi.h
created 07/03/2005

ffi stands for File Format Interface, this is an helper class for reading and 
writing bmp files. It works with the archive class, so you can also store a bmp
in a file which contains other objects.

to write a regular bmp file:
create and fill the bmp_ffi object
    bmp_ffi my_bmp_ffi();
    ...
create an archive object:
    archive f("Name of the bmp file", "ios::out");
store the bmp_ffi object in the archive:
    f<<my_bmp_ffi;
close the archive
    f.close()
You're done !
*/

#ifndef __BMP_FFI_H__
#define __BMP_FFI_H__

#include "archive.h"
#include <math.h>
#include <vector>

class rgba
{
   public:
   rgba(){blue=0;green=0;red=0;alpha=0;};
   virtual ~rgba(){};
   friend	archive & operator<<(archive &s, const rgba & c);
	friend	archive & operator>>(archive &s, rgba &c);
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char alpha;
	unsigned char get_luminance(){return (unsigned char)floor((red*30+green*59+blue*11+0.5)/100.0);};
}; 

class bmfh
{
   public:
   static const unsigned short magic_number;
   bmfh(){type=magic_number;size=0;reserved1=0;reserved2=0;offset_bits=0;};
   virtual ~bmfh(){};
   friend	archive & operator<<(archive &s, const bmfh & h);
	friend	archive & operator>>(archive &s, bmfh &h);
   unsigned short   type;
   unsigned int     size;
   unsigned short   reserved1;
   unsigned short   reserved2;
   unsigned int     offset_bits; 
};

class bmih
{
   public:
   bmih(){  size=sizeof(bmih)-4;width=0;height=0;planes=1;bit_count=32;compression=0;size_image=0;
            x_pixels_per_meter=3780;y_pixels_per_meter=3780;colors_used=0;colors_important=0;};
   virtual ~bmih(){};
   friend	archive & operator<<(archive &s, const bmih & h);
	friend	archive & operator>>(archive &s, bmih & h); 
   unsigned int     size;
   unsigned int     width;
   unsigned int     height;
   unsigned short   planes;
   unsigned short   bit_count;
   unsigned int     compression;
   unsigned int     size_image;
   unsigned int     x_pixels_per_meter;
   unsigned int     y_pixels_per_meter;
   unsigned int     colors_used;
   unsigned int     colors_important;
};

class bmp_ffi
{
   public:
	bmp_ffi();
	bmp_ffi(const unsigned int x, const unsigned int y, const unsigned int bit_count=24);
	bmp_ffi(const bmp_ffi & a);
		
	~bmp_ffi();
	bmp_ffi & operator=(const bmp_ffi & a);
	const rgba& operator()(const unsigned int x,const unsigned int y) const;
	rgba& operator()(const unsigned int x,const unsigned int y);
	void clear();//init all pixels to 0;
	unsigned int	get_x();
	unsigned int	get_y();
	unsigned int	get_bit_count();
	void set_size(const unsigned int x, const unsigned int y);
	void set_bit_count(const unsigned int bit_count);
	
	unsigned int	get_buffer_size() const;//return pixels size in bytes
	unsigned int	get_size() const;//return complete object size in bytes

	friend	archive & operator<<(archive &s, const bmp_ffi & p);
	friend	archive & operator>>(archive &s, bmp_ffi &p);
	
	protected:
	bmfh m_bmfh;
	bmih m_bmih;
	std::vector<rgba> m_pixels;
};

#define BMPFFI_WRONG_MAGIC       		1
#define BMPFFI_WRONG_MAGIC_MSG   		"File corrupted: wrong magic number"

#endif //__BMP_FFI_H__


