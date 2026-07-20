#include "NodeDescriptor.h"
NodeDescriptor::NodeDescriptor():nodeId_(INVALID_NODE_ID),nodeType_(NodeType::COUNT),address_{},offlineTimeoutMs_(0),enabled_(false),valid_(false){}
NodeDescriptor NodeDescriptor::create(NodeId id,NodeType type,const NodeAddress& address,NodeDuration timeout,bool enabled){NodeDescriptor r;if(id==INVALID_NODE_ID||!isValidNodeType(type)||!address.isValid()||timeout==0)return r;r.nodeId_=id;r.nodeType_=type;r.address_=address;r.offlineTimeoutMs_=timeout;r.enabled_=enabled;r.valid_=true;return r;}
bool NodeDescriptor::isValid()const{return valid_&&nodeId_!=INVALID_NODE_ID&&isValidNodeType(nodeType_)&&address_.isValid()&&offlineTimeoutMs_!=0;}
NodeId NodeDescriptor::nodeId()const{return nodeId_;} NodeType NodeDescriptor::nodeType()const{return nodeType_;} const NodeAddress& NodeDescriptor::address()const{return address_;} NodeDuration NodeDescriptor::offlineTimeoutMs()const{return offlineTimeoutMs_;} bool NodeDescriptor::enabled()const{return enabled_;}
