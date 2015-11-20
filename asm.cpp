#include <iostream>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <cstdint>
#include <cassert>

#include "asm.h"

using namespace std;

const char hex_ch[16] = {'0','1','2','3','4','5','6','7',
		         '8','9','A','B','C','D','E','F'};

string to_hex(uint8_t* buf, size_t sz) {
  string ret(sz*3 - 1, ' ');
  for (size_t i = 0; i < sz; i++) {
    ret[i*3    ] = hex_ch[buf[i]>>4];
    ret[i*3 + 1] = hex_ch[buf[i] & 0x0f];
  }
  return ret;
}

namespace Asm {

class Asm {

  size_t capacity;

  uint8_t *bytes;
  size_t bytes_written;


  void ensure(size_t sz) {
    if (capacity - bytes_written <= sz) {
      // Common case
      return;
    }
    size_t new_capacity = capacity;
    while ((new_capacity <<= 1) - bytes_written < sz);
    bytes = (uint8_t*)mremap(bytes, capacity, new_capacity,
			     MREMAP_MAYMOVE);
    capacity = new_capacity;
  }
  
  template <typename T>
  void put(T x) {
    ensure(sizeof(T));
    *(T*)(bytes+bytes_written) = x;
    bytes_written += sizeof(T);
  }
  
public:
  typedef int (*func)();
  
  Asm()
    : capacity(4096),
      bytes((uint8_t*)mmap(
       NULL,
       capacity,
       PROT_READ | PROT_WRITE | PROT_EXEC,
       MAP_ANONYMOUS | MAP_PRIVATE,
       0,
       0)),
      bytes_written(0) {}
  ~Asm() { munmap(bytes, capacity); }
    
  void rex(Register64 dst, Register64 src) {
    Op::REX rex;
    rex.header = Op::REX::HEADER;
    rex.W = 1;
    rex.R = Register64_High(src);
    rex.X = 0;
    rex.B = Register64_High(dst);
    put(rex);
  }

  void rex(Register64 dst, uint64_t val) {
    Op::REX rex;
    rex.header = Op::REX::HEADER;
    rex.W = 1;
    rex.R = 0;
    rex.X = 0;
    rex.B = Register64_High(dst);
    put(rex);
  }

  void mov(Register64 reg, int64_t val) {
    rex(reg, val);
    put<uint8_t>(0xb8 + Register64_Low(reg));
    put(val);
  }

  void mov(Register32 reg, int32_t val) {
    put<uint8_t>(0xb8 + reg);
    put(val);
  }

  void mov(Register64 dst, Register64 src) {
    rex(dst, src);
    put<uint8_t>(0x89);
    Op::ModRM m;
    m.mod = DIRECT;
    m.rm = Register64_Low(dst);
    m.reg = Register64_Low(src);
    put(m);
  }

  void push(Register64 r) {
    put<uint8_t>(0x50 + r);
  }

  void pop(Register64 r) {
    put<uint8_t>(0x58 + r);
  }

  void retq() {
    put<uint8_t>(0xc3);
  }
  
  func get_fn() { return (func)bytes; }

  string as_hex() {
    return to_hex(bytes, bytes_written);
  }
  
};

}

/*
  400f7a:	55                   	push   %rbp
  400f7b:	48 89 e5             	mov    %rsp,%rbp
  400f7e:	b8 62 00 00 00       	mov    $0x62,%eax
  400f83:	5d                   	pop    %rbp
  400f84:	c3                   	retq   
 */


int fn() {
  return 98;
}

int main() {
  Asm::Asm a;
  a.push(Asm::RBP);
  a.push(Asm::RBX);
  a.mov(Asm::RBP, Asm::RSP);
  
  //a.mov(Asm::EAX, 99);
  a.mov(Asm::RBX, 99);
  a.mov(Asm::RAX, Asm::RBX);
  
  a.pop(Asm::RBX);
  a.pop(Asm::RBP);
  a.retq();

  cout << a.as_hex() <<endl;
  
  int x = a.get_fn()();
  cout << x << endl;
  return 0;
}
