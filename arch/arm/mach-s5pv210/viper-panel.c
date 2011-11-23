/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/tl2796.h>
#include <linux/nt35580.h>

#define SLEEPMSEC		0x1000
#define ENDDEF			0x2000

static const u16 s6e63m0_SEQ_DISPLAY_ON[] = {
	0x029,
	ENDDEF, 0x0000
};

static const u16 s6e63m0_SEQ_DISPLAY_OFF[] = {
	0x028,
	ENDDEF, 0x0000
};

static const u16 s6e63m0_SEQ_STANDBY_ON[] = {
	0x010,	/* Stand-by On Command */
	SLEEPMSEC, 120,
	ENDDEF, 0x0000
};

static const u16 s6e63m0_SEQ_STANDBY_OFF[] = {
	0x011,	/* Stand-by Off Command */
	SLEEPMSEC, 120,
	ENDDEF, 0x0000
};

static const u16 s6e63m0_SEQ_DISPLAY_SETTING[] = {
	0x0F2,
	0x13B,
	0x13F,
	0x103,
	0x108,
	0x104,
	0x108,
	0x108,
	0x100,
	0x108,
	0x108,
	0x100,
	0x100,
	0x100,
	0x100,
	0x13F,
	0x108,
	0x104,
	0x108,
	0x104,

	0x0F4,
	0x108,
	0x100,
	0x100,
	0x100,
	0x100,
	0x100,
	0x100,
	0x100,
	0x104,
	0x170,
	0x103,
	0x104,
	0x170,
	0x103,

	0x0F5,
	0x100,
	0x168,
	0x170,
	0x100,
	0x100,
	0x102,
	0x100,
	0x100,
	0x100,
	0x100,
	0x168,
	0x170,
 
	ENDDEF,0x10000
};

static const u16 s6e63m0_SEQ_ETC_SETTING[] = {
	0x0F9,
	0x114,

	0x0FA,
	0x101,
	0x110,
	0x109,
	0x124,
	0x127,
	0x133,
	0x137,
	0x10E,
	0x11C,
	0x122,
	0x129,
	0x11A,
	0x102,
	0x100,
	0x100,
	0x100,

	0x0FB,
	0x10E,
	0x101,
	0x102,
	0x11A,
	0x129,
	0x122,
	0x11C,
	0x10E,
	0x137,
	0x133,
	0x127,
	0x124,
	0x109,
	0x100,
	0x100,
	0x100,

	0x0F9,
	0x112,

	0x0FA,
	0x111,
	0x119,
	0x109,
	0x12A,
	0x12E,
	0x134,
	0x138,
	0x10D,
	0x11A,
	0x11E,
	0x125,
	0x11E,
	0x102,
	0x100,
	0x100,
	0x100,

	0x0FB,
	0x117,
	0x111,
	0x102,
	0x11E,
	0x125,
	0x11E,
	0x11A,
	0x10D,
	0x138,
	0x134,
	0x12E,
	0x12A,
	0x109,
	0x100,
	0x100,
	0x100,

	0x0F9,
	0x111,

	0x0FA,
	0x138,
	0x116,
	0x101,
	0x101,
	0x108,
	0x10B,
	0x10F,
	0x135,
	0x13F,
	0x13E,
	0x136,
	0x110,
	0x102,
	0x100,
	0x100,
	0x100,

	0x0FB,
	0x114,
	0x138,
	0x102,
	0x110,
	0x136,
	0x13E,
	0x13F,
	0x135,
	0x10F,
	0x10B,
	0x108,
	0x101,
	0x101,
	0x100,
	0x100,
	0x100,

 	ENDDEF,0x10000
};

static const u16 s6e63m0_22gamma_300cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x170,
	0x16E,
	0x14E,
	0x1BC,
	0x1C0,
	0x1AF,
	0x1B3,
	0x1B8,
	0x1A5,
	0x1C5,
	0x1C7,
	0x1BB,
	0x100,
	0x1B9,
	0x100,
	0x1B8,
	0x100,
	0x1FC,
                                                          
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_290cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x171,
	0x170,
	0x150,
	0x1BD,
	0x1C1,
	0x1B0,
	0x1B2,
	0x1B8,
	0x1A4,
	0x1C6,
	0x1C7,
	0x1BB,
	0x100,
	0x1B6,
	0x100,
	0x1B6,
	0x100,
	0x1FA,
                                                                                        
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_280cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x16E,
	0x16C,
	0x14D,
	0x1BE,
	0x1C3,
	0x1B1,
	0x1B3,
	0x1B8,
	0x1A5,
	0x1C6,
	0x1C8,
	0x1BB,
	0x100,
	0x1B4,
	0x100,
	0x1B3,
	0x100,
	0x1F7,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_270cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x171,
	0x16C,
	0x150,
	0x1BD,
	0x1C3,
	0x1B0,
	0x1B4,
	0x1B8,
	0x1A6,
	0x1C6,
	0x1C9,
	0x1BB,
	0x100,
	0x1B2,
	0x100,
	0x1B1,
	0x100,
	0x1F4,                                       
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_260cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x174,
	0x16E,
	0x154,
	0x1BD,
	0x1C2,
	0x1B0,
	0x1B5,
	0x1BA,
	0x1A7,
	0x1C5,
	0x1C9,
	0x1BA,
	0x100,
	0x1B0,
	0x100,
	0x1AE,
	0x100,
	0x1F1,                                       
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_250cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x174,
	0x16D,
	0x154,
	0x1BF,
	0x1C3,
	0x1B2,
	0x1B4,
	0x1BA,
	0x1A7,
	0x1C6,
	0x1CA,
	0x1BA,
	0x100,
	0x1AD,
	0x100,
	0x1AB,
	0x100,
	0x1ED,                                                                                      
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_240cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x176,
	0x16F,
	0x156,
	0x1C0,
	0x1C3,
	0x1B2,
	0x1B5,
	0x1BA,
	0x1A8,
	0x1C6,
	0x1CB,
	0x1BB,
	0x100,
	0x1AA,
	0x100,
	0x1A8,
	0x100,
	0x1E9,
                                               
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_230cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x175,
	0x16F,
	0x156,
	0x1BF,
	0x1C3,
	0x1B2,
	0x1B6,
	0x1BB,
	0x1A8,
	0x1C7,
	0x1CB,
	0x1BC,
	0x100,
	0x1A8,
	0x100,
	0x1A6,
	0x100,
	0x1E6,
                                             
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_220cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x178,
	0x16F,
	0x158,
	0x1BF,
	0x1C4,
	0x1B3,
	0x1B5,
	0x1BB,
	0x1A9,
	0x1C8,
	0x1CC,
	0x1BC,
	0x100,
	0x1A6,
	0x100,
	0x1A3,
	0x100,
	0x1E2,
                                                                                       
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_210cd[] = {
	0x0FA,
	0x102,                                        

	0x118,
	0x108,
	0x124,
	0x179,
	0x16D,
	0x157,
	0x1C0,
	0x1C4,
	0x1B4,
	0x1B7,
	0x1BD,
	0x1AA,
	0x1C8,
	0x1CC,
	0x1BD,
	0x100,
	0x1A2,
	0x100,
	0x1A0,
	0x100,
	0x1DD,
                                                                                     
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_200cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x179,
	0x16D,
	0x158,
	0x1C1,
	0x1C4,
	0x1B4,
	0x1B6,
	0x1BD,
	0x1AA,
	0x1CA,
	0x1CD,
	0x1BE,
	0x100,
	0x19F,
	0x100,
	0x19D,
	0x100,
	0x1D9,
                                               
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_190cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x17A,
	0x16D,
	0x159,
	0x1C1,
	0x1C5,
	0x1B4,
	0x1B8,
	0x1BD,
	0x1AC,
	0x1C9,
	0x1CE,
	0x1BE,
	0x100,
	0x19D,
	0x100,
	0x19A,
	0x100,
	0x1D5,
                                               
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_180cd[] = {
	0x0FA,
	0x102,

	0x118,	
	0x108,
	0x124,
	0x17B,
	0x16D,
	0x15B,
	0x1C0,
	0x1C5,
	0x1B3,
	0x1BA,
	0x1BE,
	0x1AD,
	0x1CA,
	0x1CE,
	0x1BF,
	0x100,
	0x199,
	0x100,
	0x197,
	0x100,
	0x1D0,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_170cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x17C,
	0x16D,
	0x15C,
	0x1C0,
	0x1C6,
	0x1B4,
	0x1BB,
	0x1BE,
	0x1AD,
	0x1CA,
	0x1CF,
	0x1C0,
	0x100,
	0x196,
	0x100,
	0x194,
	0x100,
	0x1CC,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_160cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x17F,
	0x16E,
	0x15F,
	0x1C0,
	0x1C6,
	0x1B5,
	0x1BA,
	0x1BF,
	0x1AD,
	0x1CB,
	0x1CF,
	0x1C0,
	0x100,
	0x194,
	0x100,
	0x191,
	0x100,
	0x1C8,                                        
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_150cd[] = {
	0x0FA,
	0x102,


	0x118,
	0x108,
	0x124,
	0x180,
	0x16E,
	0x15F,
	0x1C1,
	0x1C6,
	0x1B6,
	0x1BC,
	0x1C0,
	0x1AE,
	0x1CC,
	0x1D0,
	0x1C2,
	0x100,
	0x18F,
	0x100,
	0x18D,
	0x100,
	0x1C2,                                     
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_140cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x180,
	0x16C,
	0x15F,
	0x1C1,
	0x1C6,
	0x1B7,
	0x1BC,
	0x1C1,
	0x1AE,
	0x1CD,
	0x1D0,
	0x1C2,
	0x100,
	0x18C,
	0x100,
	0x18A,
	0x100,
	0x1BE,                                     
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_130cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x18C,
	0x16C,
	0x160,
	0x1C3,
	0x1C7,
	0x1B9,
	0x1BC,
	0x1C1,
	0x1AF,
	0x1CE,
	0x1D2,
	0x1C3,
	0x100,
	0x188,
	0x100,
	0x186,
	0x100,
	0x1B8,
                                             
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_120cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x182,
	0x16B,
	0x15E,
	0x1C4,
	0x1C8,
	0x1B9,
	0x1BD,
	0x1C2,
	0x1B1,
	0x1CE,
	0x1D2,
	0x1C4,
	0x100,
	0x185,
	0x100,
	0x182,
	0x100,
	0x1B3,                                    
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_110cd[] = {
	0x0FA,                                                                                      
	0x102,

	0x118,
	0x108,
	0x124,
	0x186,
	0x16A,
	0x160,
	0x1C5,
	0x1C7,
	0x1BA,
	0x1BD,
	0x1C3,
	0x1B2,
	0x1D0,
	0x1D4,
	0x1C5,
	0x100,
	0x180,
	0x100,
	0x17E,
	0x100,
	0x1AD,                                       
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
                                                
static const u16 s6e63m0_22gamma_100cd[] = {                                 
	0x0FA,                                        	                                              
	0x102,
	
	0x118,
	0x108,
	0x124,
	0x186,
	0x169,
	0x160,
	0x1C6,
	0x1C8,
	0x1BA,
	0x1BF,
	0x1C4,
	0x1B4,
	0x1D0,
	0x1D4,
	0x1C6,
	0x100,
	0x17C,
	0x100,
	0x17A,
	0x100,
	0x1A7,
                                           
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_90cd[] = {                                 
	0x0FA,                                                                                        
	0x102,  
	
	0x118,
	0x108,
	0x124,
	0x1B9,
	0x169,
	0x164,
	0x1C7,
	0x1C8,
	0x1BB,
	0x1C0,
	0x1C5,
	0x1B4,
	0x1D2,
	0x1D5,
	0x1C9,
	0x100,
	0x177,
	0x100,
	0x176,
	0x100,
	0x1A0,
                                             
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_80cd[] = {                                   
	0x0FA,                                                                                       
	0x102,                                        
	0x118,
	0x108,
	0x124,
	0x189,
	0x168,
	0x165,
	0x1C9,
	0x1C9,
	0x1BC,
	0x1C1,
	0x1C5,
	0x1B6,
	0x1D2,
	0x1D5,
	0x1C9,
	0x100,
	0x173,
	0x100,
	0x172,
	0x100,
	0x19A,
                                                                                     
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_70cd[] = {                                 
	0x0FA,                                                                                     
	0x102,                                        
	0x118,
	0x108,
	0x124,
	0x18E,
	0x162,
	0x16B,
	0x1C7,
	0x1C9,
	0x1BB,
	0x1C3,
	0x1C7,
	0x1B7,
	0x1D3,
	0x1D7,
	0x1CA,
	0x100,
	0x16E,
	0x100,
	0x16C,
	0x100,
	0x194,
                                     
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_60cd[] = {                                  
	0x0FA,                                                                                        
	0x102,                                        
	0x118,
	0x108,
	0x124,
	0x191,
	0x15E,
	0x16E,
	0x1C9,
	0x1C9,
	0x1BD,
	0x1C4,
	0x1C9,
	0x1B8,
	0x1D3,
	0x1D7,
	0x1CA,
	0x100,
	0x169,
	0x100,
	0x167,
	0x100,
	0x18D,
                                              
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_22gamma_50cd[] = {                                  
	0x0FA,                                        
	0x102,
	
	0x118,                                        
	0x108,
	0x124,
	0x196,
	0x158,
	0x172,
	0x1CB,
	0x1CA,
	0x1BF,
	0x1C6,
	0x1C9,
	0x1BA,
	0x1D6,
	0x1D9,
	0x1CD,
	0x100,
	0x161,
	0x100,
	0x161,
	0x100,
	0x183,
                                              
	ENDDEF, 0x0000                                
}; 

static const u16 s6e63m0_22gamma_40cd[] = { 
	0x0FA,	
	0x102,
	
	0x118,
	0x108,
	0x124,
	0x197,
	0x158,
	0x171,
	0x1CC,
	0x1CB,
	0x1C0,
	0x1C5,
	0x1C9,
	0x1BA,
	0x1D9,
	0x1DC,
	0x1D1,
	0x100,
	0x15B,
	0x100,
	0x15A,
	0x100,
	0x17A,

	ENDDEF, 0x0000                               
}; 

static const u16 s6e63m0_22gamma_30cd[] = { 
	0x0FA,	
	0x102,
	
	0x118,
	0x108,
	0x124,
	0x1A1,
	0x151,
	0x17B,
	0x1CE,
	0x1CB,
	0x1C2,
	0x1C7,
	0x1CB,
	0x1BC,
	0x1DA,
	0x1DD,
	0x1D3,
	0x100,
	0x153,
	0x100,
	0x152,
	0x100,
	0x16F, 	                                      

	ENDDEF, 0x0000                              
}; 


#if !defined(CONFIG_ARIES_NTT)
static const u16 *p22Gamma_set[] = {
	s6e63m0_22gamma_30cd,
	s6e63m0_22gamma_40cd,                         
	s6e63m0_22gamma_70cd,                         
	s6e63m0_22gamma_90cd,                         
	s6e63m0_22gamma_100cd,                     
	s6e63m0_22gamma_110cd,                    
	s6e63m0_22gamma_120cd,                        
	s6e63m0_22gamma_130cd,                        
	s6e63m0_22gamma_140cd,	                      
	s6e63m0_22gamma_150cd,                    
	s6e63m0_22gamma_160cd,                
	s6e63m0_22gamma_170cd,                        
	s6e63m0_22gamma_180cd,                        
	s6e63m0_22gamma_190cd,	                      
	s6e63m0_22gamma_200cd,                    
	s6e63m0_22gamma_210cd,                
	s6e63m0_22gamma_220cd,                        
	s6e63m0_22gamma_230cd,                        
	s6e63m0_22gamma_240cd,                        
	s6e63m0_22gamma_250cd,                   
	s6e63m0_22gamma_260cd,                       
	s6e63m0_22gamma_270cd,                        
	s6e63m0_22gamma_280cd,                        
	s6e63m0_22gamma_290cd,                        
	s6e63m0_22gamma_300cd,         
};                                             
#else                                
static const u16 *p22Gamma_set[] = {        
	s6e63m0_22gamma_30cd,
	s6e63m0_22gamma_40cd,  
	s6e63m0_22gamma_50cd,
	s6e63m0_22gamma_60cd,
	s6e63m0_22gamma_70cd,	
	s6e63m0_22gamma_80cd,
	s6e63m0_22gamma_90cd,
	s6e63m0_22gamma_100cd,
	s6e63m0_22gamma_110cd,	
	s6e63m0_22gamma_120cd,	
	s6e63m0_22gamma_130cd,
	s6e63m0_22gamma_140cd,
	s6e63m0_22gamma_150cd,
	s6e63m0_22gamma_160cd,	
	s6e63m0_22gamma_170cd,	
	s6e63m0_22gamma_180cd,
	s6e63m0_22gamma_190cd,
	s6e63m0_22gamma_200cd,
	s6e63m0_22gamma_210cd,	
	s6e63m0_22gamma_220cd,	
	s6e63m0_22gamma_230cd,
	s6e63m0_22gamma_240cd,
	s6e63m0_22gamma_260cd,
	s6e63m0_22gamma_280cd,
	s6e63m0_22gamma_300cd,     
};        
#endif
                                                
static const u16 s6e63m0_19gamma_300cd[] = {                                 
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x179,
	0x17A,
	0x15B,
	0x1C1,
	0x1C5,
	0x1B5,
	0x1B8,
	0x1BD,
	0x1AB,
	0x1CB,
	0x1CE,
	0x1C1,
	0x100,
	0x1B8,
	0x100,
	0x1B7,
	0x100,
	0x1FC,
                                               
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_290cd[] = {                              
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x178,
	0x17A,
	0x15B,
	0x1C2,
	0x1C7,
	0x1B6,
	0x1BA,
	0x1BE,
	0x1AC,
	0x1CB,
	0x1CE,
	0x1C2,
	0x100,
	0x1B8,
	0x100,
	0x1B6,
	0x100,
	0x1FB,
                                               
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_280cd[] = {                                 
	0x0FA,                                        	                                              
	0x102,  
	
	0x118,
	0x108,
	0x124,
	0x17B,
	0x17D,
	0x15F,
	0x1C1,
	0x1C7,
	0x1B5,
	0x1BA,
	0x1BE,
	0x1AD,
	0x1CC,
	0x1CE,
	0x1C2,
	0x100,
	0x1B5,
	0x100,
	0x1B4,
	0x100,
	0x1F8,
                                               
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_270cd[] = {                               
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x17C,
	0x17E,
	0x161,
	0x1C1,
	0x1C6,
	0x1B5,
	0x1BA,
	0x1BF,
	0x1AD,
	0x1CC,
	0x1CF,
	0x1C2,
	0x100,
	0x1B3,
	0x100,
	0x1B1,
	0x100,
	0x1F5,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_260cd[] = {
	0x0FA,
	0x102,

	0x118,
	0x108,
	0x124,
	0x179,
	0x17C,
	0x15E,
	0x1C3,
	0x1C7,
	0x1B6,
	0x1BA,
	0x1BF,
	0x1AE,
	0x1CC,
	0x1D0,
	0x1C2,
	0x100,
	0x1B1,
	0x100,
	0x1AE,
	0x100,
	0x1F2,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_250cd[] = {                                   
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x178,
	0x17C,
	0x15E,
	0x1C3,
	0x1C8,
	0x1B6,
	0x1BC,
	0x1C0,
	0x1AF,
	0x1CC,
	0x1CF,
	0x1C2,
	0x100,
	0x1AE,
	0x100,
	0x1AC,
	0x100,
	0x1EE,
                                                 
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_240cd[] = {                             
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x17A,
	0x17F,
	0x161,
	0x1C2,
	0x1C7,
	0x1B6,
	0x1BC,
	0x1C1,
	0x1AF,
	0x1CE,
	0x1D0,
	0x1C3,
	0x100,
	0x1AB,
	0x100,
	0x1A9,
	0x100,
	0x1EA,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_230cd[] = {                                 
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x17A,
	0x17E,
	0x161,
	0x1C4,
	0x1C8,
	0x1B8,
	0x1BB,
	0x1C1,
	0x1AF,
	0x1CE,
	0x1D1,
	0x1C3,
	0x100,
	0x1A8,
	0x100,
	0x1A6,
	0x100,
	0x1E6,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_220cd[] = {                                  
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x17C,
	0x17F,
	0x162,
	0x1C5,
	0x1C8,
	0x1B9,
	0x1BC,
	0x1C1,
	0x1B0,
	0x1CE,
	0x1D2,
	0x1C3,
	0x100,
	0x1A5,
	0x100,
	0x1A3,
	0x100,
	0x1E2,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_210cd[] = {                                
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x17B,
	0x17F,
	0x161,
	0x1C6,
	0x1C8,
	0x1B9,
	0x1BE,
	0x1C2,
	0x1B2,
	0x1CE,
	0x1D3,
	0x1C4,
	0x100,
	0x1A2,
	0x100,
	0x1A0,
	0x100,
	0x1DD,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_200cd[] = {                                   
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x17B,
	0x17E,
	0x161,
	0x1C5,
	0x1C9,
	0x1B9,
	0x1BF,
	0x1C3,
	0x1B2,
	0x1CF,
	0x1D3,
	0x1C5,
	0x100,
	0x19F,
	0x100,
	0x19D,
	0x100,
	0x1D9,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_190cd[] = {                                  
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x17D,
	0x17F,
	0x163,
	0x1C5,
	0x1C9,
	0x1BA,
	0x1BF,
	0x1C4,
	0x1B2,
	0x1D0,
	0x1D3,
	0x1C6,
	0x100,
	0x19C,
	0x100,
	0x19A,
	0x100,
	0x1D5,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_180cd[] = {                                
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x180,
	0x182,
	0x165,
	0x1C6,
	0x1C9,
	0x1BB,
	0x1BF,
	0x1C4,
	0x1B3,
	0x1D0,
	0x1D4,
	0x1C6,
	0x100,
	0x199,
	0x100,
	0x197,
	0x100,
	0x1D0,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_170cd[] = {                                 
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x17D,
	0x17F,
	0x161,
	0x1C7,
	0x1CA,
	0x1BB,
	0x1C0,
	0x1C5,
	0x1B5,
	0x1D1,
	0x1D4,
	0x1C8,
	0x100,
	0x196,
	0x100,
	0x194,
	0x100,
	0x1CB,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_160cd[] = {                                 
	0x0FA,                                        
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x17C,
	0x17E,
	0x161,
	0x1C7,
	0x1CB,
	0x1BB,
	0x1C1,
	0x1C5,
	0x1B5,
	0x1D1,
	0x1D5,
	0x1C8,
	0x100,
	0x193,
	0x100,
	0x190,
	0x100,
	0x1C7,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_150cd[] = {                                
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x17E,
	0x17F,
	0x163,
	0x1C8,
	0x1CB,
	0x1BC,
	0x1C1,
	0x1C6,
	0x1B6,
	0x1D2,
	0x1D6,
	0x1C8,
	0x100,
	0x18F,
	0x100,
	0x18D,
	0x100,
	0x1C2,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_140cd[] = {                                   
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x17E,
	0x17F,
	0x165,
	0x1C8,
	0x1CC,
	0x1BC,
	0x1C2,
	0x1C6,
	0x1B6,
	0x1D3,
	0x1D6,
	0x1C9,
	0x100,
	0x18C,
	0x100,
	0x18A,
	0x100,
	0x1BE,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_130cd[] = {                               
	0x0FA,                                                                                      
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x182,
	0x17F,
	0x169,
	0x1C8,
	0x1CC,
	0x1BD,
	0x1C2,
	0x1C7,
	0x1B6,
	0x1D4,
	0x1D7,
	0x1CB,
	0x100,
	0x188,
	0x100,
	0x186,
	0x100,
	0x1B8,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_120cd[] = {                             
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x183,
	0x17F,
	0x169,
	0x1C9,
	0x1CC,
	0x1BE,
	0x1C3,
	0x1C8,
	0x1B8,
	0x1D4,
	0x1D7,
	0x1CB,
	0x100,
	0x185,
	0x100,
	0x183,
	0x100,
	0x1B3,
                                               
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_110cd[] = {                         
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x185,
	0x181,
	0x16A,
	0x1CB,
	0x1CD,
	0x1BF,
	0x1C4,
	0x1C8,
	0x1B9,
	0x1D5,
	0x1D9,
	0x1CC,
	0x100,
	0x180,
	0x100,
	0x17E,
	0x100,
	0x1AD,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_100cd[] = {                               
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x186,
	0x181,
	0x16B,
	0x1CA,
	0x1CD,
	0x1BE,
	0x1C6,
	0x1C9,
	0x1BB,
	0x1D5,
	0x1DA,
	0x1CD,
	0x100,
	0x17C,
	0x100,
	0x17A,
	0x100,
	0x1A7,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_90cd[] = {                                  
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x188,
	0x181,
	0x16D,
	0x1CB,
	0x1CE,
	0x1C0,
	0x1C6,
	0x1CA,
	0x1BB,
	0x1D6,
	0x1DA,
	0x1CE,
	0x100,
	0x178,
	0x100,
	0x176,
	0x100,
	0x1A1,
                                                 
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_80cd[] = {                         
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x18E,
	0x17F,
	0x172,
	0x1CB,
	0x1CF,
	0x1C1,
	0x1C6,
	0x1CB,
	0x1BB,
	0x1D8,
	0x1DB,
	0x1CF,
	0x100,
	0x173,
	0x100,
	0x171,
	0x100,
	0x19B,
                                               
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_70cd[] = {                              
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x192,
	0x17F,
	0x175,
	0x1CC,
	0x1CF,
	0x1C2,
	0x1C7,
	0x1CC,
	0x1BD,
	0x1D8,
	0x1DC,
	0x1CF,
	0x100,
	0x16E,
	0x100,
	0x16C,
	0x100,
	0x194,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_60cd[] = {                                  
	0x0FA,                                        	                                              
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x193,
	0x17F,
	0x174,
	0x1CD,
	0x1CF,
	0x1C3,
	0x1CA,
	0x1CD,
	0x1C0,
	0x1D8,
	0x1DD,
	0x1D1,
	0x100,
	0x169,
	0x100,
	0x167,
	0x100,
	0x18C,
                                                
	ENDDEF, 0x0000                                
};                                              
                                                
static const u16 s6e63m0_19gamma_50cd[] = {                                
	0x0FA,                                                                                      
	0x102,                                        
                                                
	0x118,
	0x108,
	0x124,
	0x194,
	0x17C,
	0x173,
	0x1CF,
	0x1D0,
	0x1C5,
	0x1CA,
	0x1CE,
	0x1C0,
	0x1DB,
	0x1DF,
	0x1D4,
	0x100,
	0x162,
	0x100,
	0x160,
	0x100,
	0x183,
                                                
	ENDDEF, 0x0000                                
};                                              

static const u16 s6e63m0_19gamma_40cd[] = { 
	0x0FA,
	0x102,	
                                                
	0x118,
	0x108,
	0x124,
	0x193,
	0x177,
	0x172,
	0x1CF,
	0x1D0,
	0x1C5,
	0x1CB,
	0x1CF,
	0x1C1,
	0x1DD,
	0x1E0,
	0x1D6,
	0x100,
	0x15B,
	0x100,
	0x15A,
	0x100,
	0x17A,

	ENDDEF, 0x0000                              
}; 

static const u16 s6e63m0_19gamma_30cd[] = { 
	0x0FA,
	0x102,	
                                                
	0x118,
	0x108,
	0x124,
	0x19D,
	0x175,
	0x17C,
	0x1D0,
	0x1D0,
	0x1C6,
	0x1CD,
	0x1D1,
	0x1C3,
	0x1DE,
	0x1E1,
	0x1D8,
	0x100,
	0x153,
	0x100,
	0x152,
	0x100,
	0x16F,

	ENDDEF, 0x0000                               
}; 

#if !defined(CONFIG_ARIES_NTT)  
static const u16 *p19Gamma_set[] = {        
	s6e63m0_19gamma_30cd,                                   
	s6e63m0_19gamma_40cd,                         
	s6e63m0_19gamma_70cd,                         
	s6e63m0_19gamma_90cd,                         
	s6e63m0_19gamma_100cd,                    
	s6e63m0_19gamma_110cd,                        
	s6e63m0_19gamma_120cd,                        
	s6e63m0_19gamma_130cd,                        
	s6e63m0_19gamma_140cd,	                      
	s6e63m0_19gamma_150cd,            
	s6e63m0_19gamma_160cd,                        
	s6e63m0_19gamma_170cd,                        
	s6e63m0_19gamma_180cd,                        
	s6e63m0_19gamma_190cd,	                      
	s6e63m0_19gamma_200cd,               
	s6e63m0_19gamma_210cd,                        
	s6e63m0_19gamma_220cd,                        
	s6e63m0_19gamma_230cd,                        
	s6e63m0_19gamma_240cd,                        
	s6e63m0_19gamma_250cd,              
	s6e63m0_19gamma_260cd,                        
	s6e63m0_19gamma_270cd,                        
	s6e63m0_19gamma_280cd,                        
	s6e63m0_19gamma_290cd,                        
	s6e63m0_19gamma_300cd,                  
}; 
#else
static const u16 *p19Gamma_set[] = {        
	s6e63m0_19gamma_30cd,             
	s6e63m0_19gamma_40cd,  
	s6e63m0_19gamma_50cd,
	s6e63m0_19gamma_60cd,
	s6e63m0_19gamma_70cd,	
	s6e63m0_19gamma_80cd,
	s6e63m0_19gamma_90cd,
	s6e63m0_19gamma_100cd,
	s6e63m0_19gamma_110cd,	
	s6e63m0_19gamma_120cd,	
	s6e63m0_19gamma_130cd,
	s6e63m0_19gamma_140cd,
	s6e63m0_19gamma_150cd,
	s6e63m0_19gamma_160cd,	
	s6e63m0_19gamma_170cd,	
	s6e63m0_19gamma_180cd,
	s6e63m0_19gamma_190cd,
	s6e63m0_19gamma_200cd,
	s6e63m0_19gamma_210cd,	
	s6e63m0_19gamma_220cd,	
	s6e63m0_19gamma_230cd,
	s6e63m0_19gamma_240cd,
	s6e63m0_19gamma_260cd,
	s6e63m0_19gamma_280cd,
	s6e63m0_19gamma_300cd,
}; 
#endif

static const u16 gamma_updates[] = {
	0x053,
	0x12C,

	0x0C3,
	0x100,
	0x12F,
	0x114,

	0x051,
	0x180,
	
	ENDDEF,0x10000
}; 

static const u16 acl_cutoff_off[] = {
	/* ACL Off */
	0x0C0,
	0x100,

	ENDDEF, 0x0000 
};

/* ACL INIT : delta Y is all zero */
static const u16 acl_cutoff_init[] = {
	0x0F0,
	0x15A,
	0x15A,

	/* ACL parameter set */
	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x100,  0x100,  0x100,	
	
	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_8p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x102,  0x103,       	
	0x104,  0x105,  0x106,  0x107,       	
	0x108,  0x109,  0x10A,  0x10B,

	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_14p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x103,  0x105,       	
	0x106,  0x108,  0x10A,  0x10C,       	
	0x10E,  0x10F,  0x111,  0x113,

	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_20p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x103,  0x108,       	
	0x10C,  0x10F,  0x112,  0x114,       	
	0x115,  0x117,  0x118,  0x119,

	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_24p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x104,  0x107,       	
	0x10A,  0x10D,  0x111,  0x114,       	
	0x117,  0x11A,  0x11D,  0x120,

	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_28p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x105,  0x108,       	
	0x10C,  0x10F,  0x113,  0x117,       	
	0x11A,  0x11E,  0x121,  0x125,

	0x0C0,
	0x101,
	ENDDEF, 0x0000
};


static const u16 acl_cutoff_32p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x105,  0x109,       	
	0x10D,  0x111,  0x116,  0x11A,       	
	0x11E,  0x122,  0x126,  0x12A,

	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_35p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x106,  0x10A,       	
	0x10F,  0x113,  0x118,  0x11C,       	
	0x121,  0x125,  0x12A,  0x12E,

	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_37p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x106,  0x10B,       	
	0x110,  0x115,  0x11A,  0x11E,       	
	0x123,  0x128,  0x12D,  0x132,

	0x0C0,
	0x101,
	ENDDEF, 0x0000
};


static const u16 acl_cutoff_40p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x106,  0x111,       	
	0x11A,  0x120,  0x125,  0x129,       	
	0x12D,  0x130,  0x133,  0x135,
	
	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_43p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x107,  0x112,       	
	0x11C,  0x123,  0x129,  0x12D,       	
	0x131,  0x134,  0x137,  0x13A,
	
	0x0C0,
	0x101,
	ENDDEF, 0x0000
};


static const u16 acl_cutoff_45p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x107,  0x113,       	
	0x11E,  0x125,  0x12B,  0x130,       	
	0x134,  0x137,  0x13A,  0x13D,
	
	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_47p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x107,  0x114,       	
	0x120,  0x128,  0x12E,  0x133,       	
	0x137,  0x13B,  0x13E,  0x141,
	
	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_48p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x108,  0x115,
	0x120,  0x129,  0x12F,  0x134,       	
	0x139,  0x13D,  0x140,  0x143,
	
	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_50p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x108,  0x116,       	
	0x122,  0x12B,  0x131,  0x137,       	
	0x13B,  0x13F,  0x143,  0x146,	
	
	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_55p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x109,  0x118,       	
	0x126,  0x12F,  0x137,  0x13D,       	
	0x142,  0x147,  0x14A,  0x14E,	
	
	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_60p[] = {	
	0x0F0,	
	0x15A,	
	0x15A,	

	0x0C1,	
	0x14D,	0x196,	0x11D,	
	0x100,	0x100,	0x101,	0x1DF,	
	0x100,	0x100,	0x103,	0x11F,	
	0x100,  0x100,  0x100,  0x100,
	0x100,  0x101,  0x10A,  0x11B,
	0x12A,  0x135,  0x13D,  0x144,
	0x14A,  0x14F,  0x153,  0x157,
	
	0x0C0,
	0x101,	
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_65p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x10B,  0x11E,       	
	0x12F,  0x13B,  0x144,  0x14C,       	
	0x152,  0x158,  0x15D,  0x161,	

	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_70p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x10C,  0x121,       	
	0x134,  0x141,  0x14B,  0x153,       	
	0x15B,  0x161,  0x166,  0x16B,
	
	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_75p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x10D,  0x124,       	
	0x139,  0x147,  0x153,  0x15C,       	
	0x164,  0x16B,  0x171,  0x176,	

	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_80p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x10E,  0x128,       	
	0x13F,  0x14F,  0x15C,  0x166,       	
	0x16F,  0x176,  0x17D,  0x183,	

	0x0C0,
	0x101,
	ENDDEF, 0x0000
};

static const u16 acl_cutoff_85p[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0C1,
	0x14D,	0x196,	0x11D,
	0x100,	0x100,	0x101,	0x1DF,
	0x100,	0x100,	0x103,	0x11F,
	0x100,  0x100,  0x100,  0x100,       	
	0x100,  0x101,  0x110,  0x12D,       	
	0x147,  0x159,  0x167,  0x173,       	
	0x17C,  0x185,  0x18C,  0x193,	

	0x0C0,
	0x101,
	ENDDEF, 0x0000
};



static const u16 *ACL_cutoff_set[] = {
	acl_cutoff_off,
	acl_cutoff_8p,
	acl_cutoff_14p,
	acl_cutoff_20p,
	acl_cutoff_24p,
	acl_cutoff_28p,
	acl_cutoff_32p,
	acl_cutoff_35p,
	acl_cutoff_37p,
	acl_cutoff_40p,
	acl_cutoff_45p,
	acl_cutoff_47p,
	acl_cutoff_48p,
	acl_cutoff_50p, 
	acl_cutoff_60p,
	acl_cutoff_75p,
	acl_cutoff_43p,
};

static const u16 s6e63m0_SEQ_PANEL_SETTING[] = {
	0x0F0,
	0x15A,
	0x15A,

	0x0F1,
	0x15A,
	0x15A,
 
	ENDDEF,0x10000
};
	
static const u16 SEQ_GAMMA_180_SETTING[] = {
	0x03A,
	0x177,

	0x036,
	0x1D0,		

	0x02A,
	0x100,
	0x100,
	0x101,
	0x13F,

	0x02B,
	0x100,
	0x100,
	0x101,
	0x1DF,

	0x0F6,
	0x103,
	0x100,
	0x108,
	0x103,
	0x103,
	0x100,
	0x103,
	0x100,

	0x0F7,
	0x148,
	0x101,
	0x1F0,
	0x112,
	0x100,
 	
	0x0F8,
	0x111,
	0x100,
	
	ENDDEF,0x10000
};

struct s5p_panel_data aries_panel_data = {
	.seq_display_set = s6e63m0_SEQ_DISPLAY_SETTING,
	.seq_panel_set = s6e63m0_SEQ_PANEL_SETTING,
	.seq_etc_set = s6e63m0_SEQ_ETC_SETTING,
	.gamma180 = SEQ_GAMMA_180_SETTING,
	.display_on = s6e63m0_SEQ_DISPLAY_ON,
	.display_off = s6e63m0_SEQ_DISPLAY_OFF,
	.standby_on = s6e63m0_SEQ_STANDBY_ON,
	.standby_off = s6e63m0_SEQ_STANDBY_OFF,
	.gamma19_table = p19Gamma_set,
	.gamma22_table = p22Gamma_set,
	.gamma_update = gamma_updates,
	.acl_table = ACL_cutoff_set,
	.acl_init = acl_cutoff_init,
	.gamma_table_size = ARRAY_SIZE(p22Gamma_set),
}; 

static unsigned short brightness_setting_table[] = {
	0x051, 0x17f,
	ENDDEF, 0x0000
};

const unsigned short nt35580_SEQ_DISPLAY_ON[] = {
	0x029,
	ENDDEF, 0x0000
};

const unsigned short nt35580_SEQ_DISPLAY_OFF[] = {
	0x028,
	SLEEPMSEC,	27, /* more than 25ms */
	ENDDEF, 0x0000
};

const unsigned short nt35580_SEQ_SETTING[] = {
	/* SET_PIXEL_FORMAT */
	0x3A,
	0x177,	/* 24 bpp */
	/* RGBCTRL */
	0x3B,
	/* RGB Mode1, DE is sampled at the rising edge of PCLK,
	* P-rising edge, EP- low active, HSP-low active, VSP-low active */
	0x107,
	0x10A,
	0x10E,
	0x10A,
	0x10A,
	/* SET_HORIZONTAL_ADDRESS (Frame Memory Area define) */
	0x2A,
	0x100,
	0x100,
	0x101,	/* 480x800 */
	0x1DF,	/* 480x800 */
	/* SET_VERTICAL_ADDRESS  (Frame Memory Area define) */
	0x2B,
	0x100,
	0x100,
	0x103,	/* 480x800 */
	0x11F,	/* 480x800 */
	/* SET_ADDRESS_MODE */
	0x36,
	0x1D4,
	SLEEPMSEC, 30,	/* recommend by Sony-LCD, */
	/* SLPOUT */
	0x11,
	SLEEPMSEC, 155, /* recommend by Sony */
	/* WRCTRLD-1 */
	0x55,
	0x100,	/* CABC Off   1: UI-Mode, 2:Still-Mode, 3:Moving-Mode */
	/* WRCABCMB */
	0x5E,
	/* Minimum Brightness Value Setting 0:the lowest, 0xFF:the highest */
	0x100,
	/* WRCTRLD-2 */
	0x53,
	/* BCTRL(1)-PWM Output Enable, A(0)-LABC Off,
	* DD(1)-Enable Dimming Function Only for CABC,
	* BL(1)-turn on Backlight Control without dimming effect */
	0x12C,
	ENDDEF, 0x0000
};

const unsigned short nt35580_SEQ_SLEEP_IN[] = {
	0x010,
	SLEEPMSEC, 155,  /* more than 150ms */
	ENDDEF, 0x0000
};

struct s5p_tft_panel_data aries_tft_panel_data = {
	.seq_set = nt35580_SEQ_SETTING,
	.sleep_in = nt35580_SEQ_SLEEP_IN,
	.display_on = nt35580_SEQ_DISPLAY_ON,
	.display_off = nt35580_SEQ_DISPLAY_OFF,
	.brightness_set = brightness_setting_table,
};

