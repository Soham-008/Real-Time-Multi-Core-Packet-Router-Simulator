#ifndef EVENT_H
#define EVENT_H

#include "packet.h"

enum EventType {
    PACKET_ARRIVAL,
    PACKET_FORWARD,
    PACKET_DROP
};

struct Event {
    uint64_t timestamp;
    EventType type;
    Packet packet;
    int core_id;
    
    Event() : timestamp(0), type(PACKET_ARRIVAL), packet(), core_id(-1) {}
    
    Event(uint64_t time, EventType t, Packet p, int core = -1)
        : timestamp(time), type(t), packet(p), core_id(core) {}
    
    bool operator>(const Event& other) const {
        return timestamp > other.timestamp;
    }
};

#endif