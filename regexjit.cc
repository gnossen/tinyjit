#include <iostream>
#include <cstdint>

#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstring>
#include <cstdlib>

#include "fsm.h"


static constexpr uint8_t square_procedure_code[] = {
  0x55,             /* push %rbp */
  0x48, 0x89, 0xe5, /* mov %rsp, %rbp */
  0x89, 0x7d, 0xfc, /* mov %edi, -0x4(%rbp) */
  0x8b, 0x45, 0xfc, /* mov -0x4(%rbp), %eax */
  0x0f, 0xaf, 0xc0, /* imul, %eax, %eax*/
  0x5d,             /* pop %rbp */
  0xc3              /* retq */
};

typedef unsigned int (*procedure)(unsigned int);

static procedure square_procedure = nullptr;

void init_procedures() {
  square_procedure = reinterpret_cast<procedure>(
    mmap(nullptr, sizeof(square_procedure_code),
         PROT_EXEC | PROT_READ | PROT_WRITE,
         MAP_PRIVATE | MAP_ANONYMOUS,
         -1, 0));
  memcpy((void*)square_procedure, (void*)square_procedure_code,
      sizeof(square_procedure_code));
}


int main(int argc, char ** argv) {
    if (argc != 2) {
        std::cerr << "USAGE: " << argv[0] << " to_square" << std::endl;
        exit(1);
    }

    init_procedures();

    int to_square = std::atoi(argv[1]);
    std::cout << square_procedure(to_square) << std::endl;
}
