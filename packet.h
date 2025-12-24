#ifndef PACKET_H
#define PACKET_H

#include <cstdint>

struct Packet {
    uint32_t id;
    uint32_t source_ip;
    uint32_t dest_ip;
    uint16_t source_port;
    uint16_t dest_port;
    uint64_t arrival_time;
    uint64_t processing_start_time;
    uint32_t size;
    uint8_t priority;
    
    Packet() : id(0), source_ip(0), dest_ip(0), source_port(0),
               dest_port(0), arrival_time(0), processing_start_time(0),
               size(1500), priority(1) {}
    
    // FIXED: Changed initialization order to match declaration order
    Packet(uint32_t id, uint32_t src, uint32_t dst, uint64_t time)
        : id(id), source_ip(src), dest_ip(dst),
          source_port(0), dest_port(0),
          arrival_time(time), processing_start_time(0),
          size(1500), priority(1) {}

    Packet(uint32_t id, uint32_t src, uint32_t dst, uint64_t time,
           uint8_t prio, uint32_t sz)
        : id(id), source_ip(src), dest_ip(dst),
          source_port(0), dest_port(0),
          arrival_time(time), processing_start_time(0),
          size(sz), priority(prio) {}
};

#endif