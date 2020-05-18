#include "Bintrans.h"
#include <elf.h>

int *DecToHex (int64_t num) {
	int *ans = (int *) calloc (16, sizeof (int));
	for (int i = 0 ; i < 16; ++i)
		ans[i] = 0;
	int i = 0;
	while (num) {
		if (i % 2 == 0)
			ans [i + 1] = num % 16;
		else
			ans [i - 1] = num % 16;
		num /= 16;
		++i;
	}
	return ans;
}

size_t GetProgramSize () {
	return 12;
}

void ELF_Header (FILE *writefile) {
	Insert (4, h (7, F), h (4, 5), h (4, C), h (4, 6)) //EI_MAG = ELF
	Insert (1, h (0, 2)) //EI_CLASS = 64 BIT
	Insert (1, h (0, 1)) //EI_DATA = DATA2LSB (Little-Endian)
	Insert (1, h (0, 1)) //EI_VERSION = EV_CURRENT
	Insert (1, h (0, 0)) //EI_OS/ABI = UNIX SYSTEM V ABI
	Insert (1, h (0, 0)) //EI_OS/ABI VER = 0
	MultiInsert (7, h (0,0)) //EmptyStuff
	Insert (2, h (0, 2), h (0, 0)) //E_TYPE = EXEC
	Insert (2, h (3, E), h (0, 0)) //E_MACHINE = EM_X86_64 (AMD x86- 64 architecture)
	Insert (4, h (0, 1), h (0, 0), h (0, 0), h (0, 0)) //E_VERSION = EV_CURRENT
	//Entry offset
	Insert (4, h (8, 0), h (0, 0), h (4, 0), h (0, 0))
	MultiInsert (4, h (0,0)) //Empty Stuff
	//Program header offset
	Insert (4, h (4, 0), h (0, 0), h (0, 0), h (0, 0))
	MultiInsert (4, h (0, 0))
	//Section header offset
	MultiInsert (8, h (0, 0))
	//Flags
	MultiInsert (4, h (0, 0))
	//ELF header size
	Insert (2, h (4, 0), h (0, 0))
	//Program header size
	Insert (2, h (3, 8), h (0, 0))
	//Program headers quantity (number)
	Insert (2, h (0, 1), h (0, 0))
	//Section header size
	Insert (2, h (4, 0), h (0, 0))
	//Section headers quantity (number)
	MultiInsert (2, h (0, 0))
	//Section headers index table
	MultiInsert (2, h (0, 0))
}

void Program_Header (FILE *writefile) {
	Insert (4, h (0, 1), h (0, 0), h (0, 0), h (0, 0)) //P_TYPE = Loadable program segment
	Insert (4, h (0, 5), h (0, 0), h (0, 0), h (0, 0)) //P_FLAGS = Segment is readable and executable
	MultiInsert (8, h (0, 0)) //P_OFFSET
	//Virtual address
	Insert (4, h (0, 0), h (0, 0), h (4, 0), h (0, 0))
	MultiInsert (4, h (0, 0))
	//Physical address
	Insert (4, h (0, 0), h (0, 0), h (4, 0), h (0, 0))
	MultiInsert (4, h (0, 0))

	int64_t sz = GetProgramSize () + HEADERS_SIZE;
	int *hex_sz = DecToHex (sz);

	//File size
	for (int j = 0; j < 16; j += 2) // i variable is already used
		Insert (1, h (hex_sz [j], hex_sz [j + 1]))
	//Memory size
	for (int j = 0; j < 16; j += 2)
		Insert (1, h (hex_sz [j], hex_sz [j + 1]))
	//Align
	Insert (4, h (0, 0), h (0, 0), h (2, 0), h (0, 0))
	MultiInsert (4, h (0, 0))
	MultiInsert (8, h (0, 0)) //Empty Stuff
}

void CreateELF () {
	FILE *writefile = fopen ("../my_bintrans/test", "w");
	ELF_Header (writefile);
	Program_Header (writefile);
	Insert (12, h (B, 8), h (3, C), h (0, 0), h (0, 0), h (0, 0), h (B, F), h (0, 0), h (0, 0), h (0, 0), h (0, 0), h (0, F), h (0, 5))
	fclose (writefile);
}
