#include "NodeAddress.h"
#include <string.h>
namespace { bool allowed(char c) { return (c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_'||c=='-'||c=='.'||c=='/'||c==':'||c=='+'||c=='@'; } }
NodeAddress::NodeAddress():value_{}{}
bool NodeAddress::set(const char* value) { if(!value)return false; size_t n=0; while(n<sizeof(value_)&&value[n]!='\0'){if(!allowed(value[n]))return false;++n;} if(n==0||n>=sizeof(value_))return false; char candidate[NODE_RUNTIME_MAX_ADDRESS_LENGTH]={}; memcpy(candidate,value,n); memcpy(value_,candidate,sizeof(value_)); return true; }
bool NodeAddress::isValid()const { if(value_[0]=='\0')return false; for(size_t i=0;i<sizeof(value_);++i){if(value_[i]=='\0')return true;if(!allowed(value_[i]))return false;}return false; }
const char* NodeAddress::c_str()const{return value_;}
size_t NodeAddress::length()const{size_t n=0;while(n<sizeof(value_)&&value_[n]!='\0')++n;return n;}
bool NodeAddress::equals(const NodeAddress& other)const{return isValid()&&other.isValid()&&memcmp(value_,other.value_,sizeof(value_))==0;}
