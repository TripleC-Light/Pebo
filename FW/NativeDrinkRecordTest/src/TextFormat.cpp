#include "TextFormat.h"
#include <stdarg.h>
#include <limits.h>
int Text::format(char *data,size_t capacity,const char *pattern,...){
  if((capacity&&!data)||!pattern)return -1;
  Buffer out{data,capacity};va_list args;va_start(args,pattern);bool good=true;
  while(*pattern&&good){
    char c=*pattern++;if(c!='%'){out.put(c);continue;}
    if(*pattern=='%'){++pattern;out.put('%');continue;}
    bool zero=*pattern=='0';if(zero)++pattern;unsigned width=0;
    while(*pattern>='0'&&*pattern<='9'){
      width=width*10+unsigned(*pattern++-'0');if(width>16){good=false;break;}
    }
    if(!good)break;
    bool wide=*pattern=='l';if(wide)++pattern;
    char kind=*pattern;if(!kind){good=false;break;}++pattern;
    if(kind=='s'&&!wide&&!width){
      const char *s=va_arg(args,const char*);if(!s)s="(null)";while(*s)out.put(*s++);
    }else if(kind=='d'){
      long v=wide?va_arg(args,long):va_arg(args,int);bool negative=v<0;
      unsigned long u=static_cast<unsigned long>(v);if(negative)u=0ul-u;
      number(out,u,10,width,negative,zero);
    }else if(kind=='u'||kind=='X'){
      unsigned long u=wide?va_arg(args,unsigned long):va_arg(args,unsigned int);
      number(out,u,kind=='X'?16:10,width,false,zero);
    }else good=false;
  }
  va_end(args);out.finish();return good&&out.count<=INT_MAX?int(out.count):-1;
}
