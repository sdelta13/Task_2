#pragma once
#include "head.h"
template<typename type>
byte* encode(type length,byte*countOut=0){
	byte*res;
	if(length<128){
		res=(byte*)malloc(1);
		*res=length;
		*res<<=1;
		*res&=0b11111110;
		if(countOut){
			*countOut=1;
		}
	}else{
		type t=1;
		size i=0;
		while((t<length)&&(t)){
			t<<=8;
			i++;
		}
		if(i>127){
			return 0;
		}
		res=(byte*)malloc(i+1);
		*res=i;
		*res<<=1;
		*res|=0b00000001;
		res+=i;
		byte*h=((byte*)&length);
		if(countOut){
			*countOut=i+1;
		}
		while(i){
			*res=*h;
			res--;
			h++;
			i--;
		}
	}
	return res;
}
template<typename type>
type decode(byte*data){
	type t=0;
	byte i=*data>>1;
	if(*data&0b00000001){
		data++;
		while(i){
			t<<=8;
			t|=*data;
			data++;
			i--;
		}
		return t;
	}else{
		return i;
	}
}
