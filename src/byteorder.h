/*
  * Copyright (C) 2014 Rene Kijewski
  *
  * This file is subject to the terms and conditions of the GNU Lesser
  * General Public License v2.1. See the file LICENSE in the top level
  * directory for more details.
  */
 
 #ifndef BYTEORDER_H
 #define BYTEORDER_H
 
 #include <stdint.h>
 
 #if defined(__MACH__)
 #   include "clang_compat.h"
 #endif
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /* ******************************* INTERFACE ******************************* */
 
 
 typedef union __attribute__((packed)) {
     uint16_t    u16;    
     uint8_t      u8[2]; 
 } le_uint16_t;
 
 typedef union __attribute__((packed)) {
     uint32_t    u32;    
     uint8_t      u8[4]; 
     uint16_t    u16[2]; 
     le_uint16_t l16[2]; 
 } le_uint32_t;
 
 typedef union __attribute__((packed)) {
     uint64_t    u64;    
     uint8_t      u8[8]; 
     uint16_t    u16[4]; 
     uint32_t    u32[2]; 
     le_uint16_t l16[4]; 
     le_uint32_t l32[2]; 
 } le_uint64_t;
 
 typedef union __attribute__((packed)) {
     uint16_t    u16;    
     uint8_t      u8[2]; 
 } be_uint16_t;
 
 typedef union __attribute__((packed)) {
     uint32_t    u32;    
     uint8_t      u8[4]; 
     uint16_t    u16[2]; 
     be_uint16_t b16[2]; 
 } be_uint32_t;
 
 typedef union __attribute__((packed)) {
     uint64_t    u64;    
     uint8_t      u8[8]; 
     uint16_t    u16[4]; 
     uint32_t    u32[2]; 
     be_uint16_t b16[4]; 
     be_uint32_t b32[2]; 
 } be_uint64_t;
 
 typedef be_uint16_t network_uint16_t;
 
 typedef be_uint32_t network_uint32_t;
 
 typedef be_uint64_t network_uint64_t;
 
 static inline be_uint16_t byteorder_ltobs(le_uint16_t v);
 
 static inline be_uint32_t byteorder_ltobl(le_uint32_t v);
 
 static inline be_uint64_t byteorder_ltobll(le_uint64_t v);
 
 static inline le_uint16_t byteorder_btols(be_uint16_t v);
 
 static inline le_uint32_t byteorder_btoll(be_uint32_t v);
 
 static inline le_uint64_t byteorder_btolll(be_uint64_t v);
 
 static inline network_uint16_t byteorder_htons(uint16_t v);
 
 static inline network_uint32_t byteorder_htonl(uint32_t v);
 
 static inline network_uint64_t byteorder_htonll(uint64_t v);
 
 static inline uint16_t byteorder_ntohs(network_uint16_t v);
 
 static inline uint32_t byteorder_ntohl(network_uint32_t v);
 
 static inline uint64_t byteorder_ntohll(network_uint64_t v);
 
 static inline uint16_t byteorder_swaps(uint16_t v);
 
 static inline uint32_t byteorder_swapl(uint32_t v);
 
 static inline uint64_t byteorder_swapll(uint64_t v);
 
 static inline uint16_t byteorder_bebuftohs(const uint8_t *buf);
 
 static inline void byteorder_htobebufs(uint8_t *buf, uint16_t val);
 
 static inline uint16_t htons(uint16_t v);
 
 static inline uint32_t htonl(uint32_t v);
 
 static inline uint64_t htonll(uint64_t v);
 
 static inline uint16_t ntohs(uint16_t v);
 
 static inline uint32_t ntohl(uint32_t v);
 
 static inline uint64_t ntohll(uint64_t v);
 
 
 /* **************************** IMPLEMENTATION ***************************** */
 
 #ifdef HAVE_NO_BUILTIN_BSWAP16
 static inline unsigned short __builtin_bswap16(unsigned short a)
 {
     return (a<<8)|(a>>8);
 }
 #endif
 
 static inline uint16_t byteorder_swaps(uint16_t v)
 {
 #ifndef MODULE_MSP430_COMMON
     return __builtin_bswap16(v);
 #else
     network_uint16_t result = { v };
     uint8_t tmp = result.u8[0];
     result.u8[0] = result.u8[1];
     result.u8[1] = tmp;
     return result.u16;
 #endif
 }
 
 static inline uint32_t byteorder_swapl(uint32_t v)
 {
     return __builtin_bswap32(v);
 }
 
 static inline uint64_t byteorder_swapll(uint64_t v)
 {
     return __builtin_bswap64(v);
 }
 
 static inline be_uint16_t byteorder_ltobs(le_uint16_t v)
 {
     be_uint16_t result = { byteorder_swaps(v.u16) };
     return result;
 }
 
 static inline be_uint32_t byteorder_ltobl(le_uint32_t v)
 {
     be_uint32_t result = { byteorder_swapl(v.u32) };
     return result;
 }
 
 static inline be_uint64_t byteorder_ltobll(le_uint64_t v)
 {
     be_uint64_t result = { byteorder_swapll(v.u64) };
     return result;
 }
 
 static inline le_uint16_t byteorder_btols(be_uint16_t v)
 {
     le_uint16_t result = { byteorder_swaps(v.u16) };
     return result;
 }
 
 static inline le_uint32_t byteorder_btoll(be_uint32_t v)
 {
     le_uint32_t result = { byteorder_swapl(v.u32) };
     return result;
 }
 
 static inline le_uint64_t byteorder_btolll(be_uint64_t v)
 {
     le_uint64_t result = { byteorder_swapll(v.u64) };
     return result;
 }
 
 #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
 #   define _byteorder_swap(V, T) (byteorder_swap##T((V)))
 #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
 #   define _byteorder_swap(V, T) (V)
 #else
 #   error "Byte order is neither little nor big!"
 #endif
 
 static inline network_uint16_t byteorder_htons(uint16_t v)
 {
     network_uint16_t result = { _byteorder_swap(v, s) };
     return result;
 }
 
 static inline network_uint32_t byteorder_htonl(uint32_t v)
 {
     network_uint32_t result = { _byteorder_swap(v, l) };
     return result;
 }
 
 static inline network_uint64_t byteorder_htonll(uint64_t v)
 {
     network_uint64_t result = { _byteorder_swap(v, ll) };
     return result;
 }
 
 static inline uint16_t byteorder_ntohs(network_uint16_t v)
 {
     return _byteorder_swap(v.u16, s);
 }
 
 static inline uint32_t byteorder_ntohl(network_uint32_t v)
 {
     return _byteorder_swap(v.u32, l);
 }
 
 static inline uint64_t byteorder_ntohll(network_uint64_t v)
 {
     return _byteorder_swap(v.u64, ll);
 }
 
 static inline uint16_t htons(uint16_t v)
 {
     return byteorder_htons(v).u16;
 }
 
 static inline uint32_t htonl(uint32_t v)
 {
     return byteorder_htonl(v).u32;
 }
 
 static inline uint64_t htonll(uint64_t v)
 {
     return byteorder_htonll(v).u64;
 }
 
 static inline uint16_t ntohs(uint16_t v)
 {
     network_uint16_t input = { v };
     return byteorder_ntohs(input);
 }
 
 static inline uint32_t ntohl(uint32_t v)
 {
     network_uint32_t input = { v };
     return byteorder_ntohl(input);
 }
 
 static inline uint64_t ntohll(uint64_t v)
 {
     network_uint64_t input = { v };
     return byteorder_ntohll(input);
 }
 
 static inline uint16_t byteorder_bebuftohs(const uint8_t *buf)
 {
     return (uint16_t)((buf[0] << 8) | (buf[1] << 0));
 }
 
 static inline void byteorder_htobebufs(uint8_t *buf, uint16_t val)
 {
     buf[0] = (uint8_t)(val >> 8);
     buf[1] = (uint8_t)(val >> 0);
 }
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif /* BYTEORDER_H */