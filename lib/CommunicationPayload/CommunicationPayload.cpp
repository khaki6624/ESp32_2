#include "CommunicationPayload.h"
CommunicationReadPayload::CommunicationReadPayload():data_(nullptr),length_(0U){}
CommunicationReadPayload::CommunicationReadPayload(const uint8_t* data,size_t length):data_(data),length_(length){}
bool CommunicationReadPayload::isValid()const{return data_!=nullptr&&length_>0U&&length_<=COMMUNICATION_MAX_PAYLOAD_LENGTH;}
const uint8_t* CommunicationReadPayload::data()const{return data_;} size_t CommunicationReadPayload::length()const{return length_;}
CommunicationWritePayload::CommunicationWritePayload():data_(nullptr),capacity_(0U){}
CommunicationWritePayload::CommunicationWritePayload(uint8_t* data,size_t capacity):data_(data),capacity_(capacity){}
bool CommunicationWritePayload::isValid()const{return data_!=nullptr&&capacity_>0U&&capacity_<=COMMUNICATION_MAX_PAYLOAD_LENGTH;}
uint8_t* CommunicationWritePayload::data()const{return data_;} size_t CommunicationWritePayload::capacity()const{return capacity_;}
