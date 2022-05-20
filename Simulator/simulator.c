/*
 * Author: Daniel Kopta
 * Updated by: Erin Parker
 * CS 4400, University of Utah
 *
 * Simulator handout
 * A simple x86-like processor simulator.
 * Read in a binary file that encodes instructions to execute.
 * Simulate a processor by executing instructions one at a time and appropriately 
 * updating register and memory contents.
 *
 * Some code and pseudo code has been provided as a starting point.
 *
 * Completed by: Scott Crowley (u1178178)
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "instruction.h"

// Forward declarations for helper functions
unsigned int get_file_size(int file_descriptor);
unsigned int* load_file(int file_descriptor, unsigned int size);
instruction_t* decode_instructions(unsigned int* bytes, unsigned int num_instructions);
unsigned int execute_instruction(unsigned int program_counter, instruction_t* instructions, 
				 int* registers, unsigned char* memory);
void print_instructions(instruction_t* instructions, unsigned int num_instructions);
void error_exit(const char* message);

// 17 registers
#define NUM_REGS 17
// 1024-byte stack
#define STACK_SIZE 1024

int main(int argc, char** argv)
{
  // Make sure we have enough arguments
  if(argc < 2)
    error_exit("must provide an argument specifying a binary file to execute");

  // Open the binary file
  int file_descriptor = open(argv[1], O_RDONLY);
  if (file_descriptor == -1) 
    error_exit("unable to open input file");

  // Get the size of the file
  unsigned int file_size = get_file_size(file_descriptor);
  // Make sure the file size is a multiple of 4 bytes
  // since machine code instructions are 4 bytes each
  if(file_size % 4 != 0)
    error_exit("invalid input file");

  // Load the file into memory
  // We use an unsigned int array to represent the raw bytes
  // We could use any 4-byte integer type
  unsigned int* instruction_bytes = load_file(file_descriptor, file_size);
  close(file_descriptor);

  unsigned int num_instructions = file_size / 4;


  /****************************************/
  /**** Begin code to modify/implement ****/
  /****************************************/

  // Allocate and decode instructions (left for you to fill in)
  instruction_t* instructions = decode_instructions(instruction_bytes, num_instructions);

  // Optionally print the decoded instructions for debugging
  // Will not work until you implement decode_instructions
  // Do not call this function in your submitted final version
  //print_instructions(instructions, num_instructions);

  // Allocate and initialize registers
  int* registers = (int*)malloc(sizeof(int) * NUM_REGS);
  int idx;
  for(idx = 0; idx < NUM_REGS; idx = idx + 1) {
  	registers[idx] = 0;
  }
  registers[6] = STACK_SIZE;  // registers[6] is %esp (stack pointer)

  // Stack memory is byte-addressed, so it must be a 1-byte type
  unsigned char* memory = (char*)malloc(STACK_SIZE);
  // Clear memory / initialize to 0.
  for(idx = 0; idx < STACK_SIZE; idx = idx + 1) {
    memory[idx] = 0;
  }

  // Run the simulation
  unsigned int program_counter = 0;

  // program_counter is a byte address, so we must multiply num_instructions by 4 
  // to get the address past the last instruction
  while(program_counter != num_instructions * 4)
  {
    program_counter = execute_instruction(program_counter, instructions, registers, memory);
  }
  
  return 0;
}

/*
 * Decodes the array of raw instruction bytes into an array of instruction_t
 * Each raw instruction is encoded as a 4-byte unsigned int
*/
instruction_t* decode_instructions(unsigned int* bytes, unsigned int num_instructions)
{
  // Allocate memory for the instruction_t pointer that will be returned.
  instruction_t* retval = (instruction_t*) malloc(num_instructions * sizeof(instruction_t));
  
  int idx;
  for(idx = 0; idx < num_instructions; idx = idx + 1) {
  	// Copy the next binary instruction from bytes array.
  	unsigned int inst = bytes[idx];
  	// The opcode is the 5 left-most bits (31-27).
  	unsigned char opcode = (unsigned char) (inst >> 27);
  	// The 1st register is the next 5 bits (26-22).
  	unsigned int temp = inst >> 22;
  	unsigned char reg1 = (unsigned char) (temp & 0x1F);  // 0x1F = 0001 1111
  	// The 2nd register is the next 5 bits (21-17).
  	temp = inst >> 17;
  	unsigned char reg2 = (unsigned char) (temp & 0x1F);
  	// The immediate is the right-most 16 bits (15-0);
  	int16_t imm = (int16_t) (inst & 0xFFFF);  // 0xFFFF = 1111 1111 1111 1111
  	// Initialize an instruction_t with the above values.
  	instruction_t instStruct = {opcode, reg1, reg2, imm};
  	// Place the instruction_t in the array.
    retval[idx] = instStruct;
  }
  return retval;
}


/*
 * Executes a single instruction and returns the next program counter
*/
unsigned int execute_instruction(unsigned int program_counter, instruction_t* instructions, int* registers, unsigned char* memory)
{
  // program_counter is a byte address, but instructions are 4 bytes each
  // divide by 4 to get the index into the instructions array
  instruction_t instr = instructions[program_counter / 4];
  
  int compInst = 0;

  switch(instr.opcode)
  {
  case subl:
    registers[instr.first_register] = registers[instr.first_register] - instr.immediate;
    break;
  case addl_reg_reg:
    registers[instr.second_register] = registers[instr.first_register] + registers[instr.second_register];
    break;
  case printr:
    printf("%d (0x%x)\n", registers[instr.first_register], registers[instr.first_register]);
    break;
  case readr:
    scanf("%d", &(registers[instr.first_register]));
    break;
  case addl_imm_reg:
    registers[instr.first_register] = registers[instr.first_register] + instr.immediate;
    break;
  case imull:
    registers[instr.second_register] = registers[instr.first_register] * registers[instr.second_register];
    break;
  case shrl:
    registers[instr.first_register] = (unsigned int)registers[instr.first_register] >> 1;
    break;
  case movl_reg_reg:
    registers[instr.second_register] = registers[instr.first_register];
    break;
  case movl_deref_reg:
    registers[instr.second_register] = *(int*)(memory + registers[instr.first_register] + instr.immediate);
    break;
  case movl_reg_deref:
    *(int*)(memory + registers[instr.second_register] + instr.immediate) = registers[instr.first_register];
    break;
  case movl_imm_reg:
    registers[instr.first_register] = instr.immediate;
    break;
  case cmpl:
    registers[16] = 0;  // Reset all condition codes to false; registers[16] is %eflags.
  	unsigned long a = registers[instr.second_register];
  	unsigned long b = registers[instr.first_register];
    unsigned long c = a + ((~b) + 1);         // Subtraction by two's complement
    int result = (int)c;
    if(result == 0) {
      registers[16] = registers[16] | 0x40;   // 0x40 = 0100 0000 (the ZF bit)
      break;
    }
    if(result < 0) {
      registers[16] = registers[16] | 0x80;   // 0x80 = 1000 0000 (the SF bit)
    }
    if((int)a > 0 && (int)b < 0 && result < 0 || (int)a < 0 && (int)b > 0 && result > 0) {
      registers[16] = registers[16] | 0x800;  // 0x800 = 1000 0000 0000 (the OF bit)
    }
    if((unsigned int)a < (unsigned int)b)
      registers[16] = registers[16] | 0x1;    // 0x1 = 0001 (the CF bit)
    break;
  case je:
    if((registers[16] >> 6) & 0x1) {          // Extract the ZF bit and check if on.
      program_counter = program_counter + instr.immediate;
    }
    break;
  case jl:                                    // If SF xor OF, then jump.
    if((registers[16] >> 7) == 0x1 || (registers[16] >> 7) == 0x10) {
      program_counter = program_counter + instr.immediate;
    }
    break;
  case jle:                                   // If ZF or (SF xor OF), then jump.
    if((registers[16] >> 6) & 0x1 || (registers[16] >> 7) == 0x1 || (registers[16] >> 7) == 0x10) {
      program_counter = program_counter + instr.immediate;
    }
    break;
  case jge:                                   // If SF xnor OF, then jump.
    if(!((registers[16] >> 7) == 0x1 || (registers[16] >> 7) == 0x10)) {
      program_counter = program_counter + instr.immediate;
    }
    break;
  case jbe:                                   // If CF or ZF, then jump.
    if(registers[16] & 0x1 || (registers[16] >> 6) & 0x1) {
      program_counter = program_counter + instr.immediate;
    }
    break;
  case jmp:
    program_counter = program_counter + instr.immediate;
    break;
  case call:
    registers[6] = registers[6] - 4;                     // Subtract 4 from stack pointer.
    *(int*)(memory + registers[6]) = program_counter;    // Save return to memory.
    program_counter = program_counter + instr.immediate;
    break;
  case ret:
    if(registers[6] == 1024) {
      exit(0);                                           // Exit if stack pointer reached 1024.
    }
    program_counter = *(int*)(memory + registers[6]);    // Get return from memory.
    registers[6] = registers[6] + 4;                     // Add 4 to stack pointer.
    break;
  case pushl:
    registers[6] = registers[6] - 4;                     // Subtract 4 from stack pointer.
    *(int*)(memory + registers[6]) = registers[instr.first_register];
    break;                                               // Save val in register to memory.
  case popl:                                             // Reverse pushl.
    registers[instr.first_register] = *(int*)(memory + registers[6]);
    registers[6] = registers[6] + 4;
    break;
  }
  // program_counter + 4 represents the subsequent instruction
  return program_counter + 4;
}


/*********************************************/
/****  DO NOT MODIFY THE FUNCTIONS BELOW  ****/
/*********************************************/

/*
 * Returns the file size in bytes of the file referred to by the given descriptor
*/
unsigned int get_file_size(int file_descriptor)
{
  struct stat file_stat;
  fstat(file_descriptor, &file_stat);
  return file_stat.st_size;
}

/*
 * Loads the raw bytes of a file into an array of 4-byte units
*/
unsigned int* load_file(int file_descriptor, unsigned int size)
{
  unsigned int* raw_instruction_bytes = (unsigned int*)malloc(size);
  if(raw_instruction_bytes == NULL)
    error_exit("unable to allocate memory for instruction bytes (something went really wrong)");

  int num_read = read(file_descriptor, raw_instruction_bytes, size);

  if(num_read != size)
    error_exit("unable to read file (something went really wrong)");

  return raw_instruction_bytes;
}

/*
 * Prints the opcode, register IDs, and immediate of every instruction, 
 * assuming they have been decoded into the instructions array
*/
void print_instructions(instruction_t* instructions, unsigned int num_instructions)
{
  printf("instructions: \n");
  unsigned int i;
  for(i = 0; i < num_instructions; i++)
  {
    printf("op: %d, reg1: %d, reg2: %d, imm: %d\n", 
	   instructions[i].opcode,
	   instructions[i].first_register,
	   instructions[i].second_register,
	   instructions[i].immediate);
  }
  printf("--------------\n");
}

/*
 * Prints an error and then exits the program with status 1
*/
void error_exit(const char* message)
{
  printf("Error: %s\n", message);
  exit(1);
}
