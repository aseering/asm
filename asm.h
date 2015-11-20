

namespace Asm
{
enum Register32 {
  EAX = 00,
  ECX = 01,
  EDX = 02,
  EBX = 03,
  ESP = 04,
  EBP = 05,
  ESI = 06,
  EDI = 07
};

enum Register64 {
// Implementation note:  4 bits; high bit is encoded separately, in REX byte
  RAX = 000,
  RCX = 001,
  RDX = 002,
  RBX = 003,
  RSP = 004,
  RBP = 005,
  RSI = 006,
  RDI = 007,
  R8  = 010,
  R9  = 011,
  R10 = 012,
  R11 = 013,
  R12 = 014,
  R13 = 015,
  R14 = 016,
  R15 = 017
};

static int Register64_High(Register64 r) {
  return (r & 0x8) >> 2;
}

static int Register64_Low(Register64 r) {
  return (r & 0x7);
}


enum Mod {
  INDIRECT = 00,
  DISPLACEMENT_8BIT = 01,
  DISPLACEMENT_32BIT = 02,
  DIRECT = 03
};

namespace Op
{
struct REX {
  unsigned B : 1;
  unsigned X : 1;
  unsigned R : 1;
  unsigned W : 1;
  unsigned header : 4;
  static const unsigned HEADER = 0x4;
} __attribute__((packed));

struct ModRM {
  unsigned rm : 3;
  unsigned reg : 3;
  unsigned mod : 2;
} __attribute__((packed));
}

}
