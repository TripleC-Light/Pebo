#pragma once
#include <stddef.h>
namespace Text {
// Bounded storage with snprintf-style required-length accounting.
struct Buffer {
  char *data;size_t capacity,count=0;
  constexpr void put(char c){if(capacity&&count<capacity-1)data[count]=c;++count;}
  constexpr void finish(){if(capacity)data[count<capacity?count:capacity-1]=0;}
};
constexpr void number(Buffer &out,unsigned long value,unsigned base,unsigned width,bool negative,bool zero){
  char digits[sizeof(unsigned long)*8]{};unsigned n=0;
  do{digits[n++]="0123456789ABCDEF"[value%base];value/=base;}while(value);
  unsigned length=n+unsigned(negative);unsigned padding=width>length?width-length:0;
  if(negative&&zero)out.put('-');
  for(unsigned i=0;i<padding;++i)out.put(zero?'0':' ');
  if(negative&&!zero)out.put('-');
  while(n)out.put(digits[--n]);
}
// Supported project subset only: %s,%d,%u,%X, optional l, width0..16, zero-pad.
// No floats, precision, *, %n, locale, heap or global mutable buffer.
int format(char *data,size_t capacity,const char *pattern,...);
}
