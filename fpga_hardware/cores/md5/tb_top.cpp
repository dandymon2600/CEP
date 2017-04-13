#include "verilated.h"
#include "Vmd5_top.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include "MD5.h"
#include "input.h"

// Current simulation time
vluint64_t main_time = 0;

// Called by $time in Verilog
// converts to double, to match
// what SystemC does
double sc_time_stamp () {
  return main_time;
}

const unsigned int CLOCK_PERIOD = 10;
Vmd5_top* top;

void runForClockCycles(const unsigned int pCycles) {
  int doubleCycles = 0;
  while(doubleCycles < (pCycles << 1)) {
    top->eval();
    if((main_time % (CLOCK_PERIOD >> 1)) == 0) {
      ++doubleCycles;
      top->wb_clk_i = ~top->wb_clk_i;
    }
    main_time++;
  }
}

void waitForACK() {
  while(top->wb_ack_o == 0) {
    runForClockCycles(1);
  }
}

uint32_t readFromAddress(uint32_t pAddress) {
  top->wb_adr_i = pAddress;
  top->wb_dat_i = 0x00000000;
  top->wb_we_i = 0;
  top->wb_stb_i = 1;
  runForClockCycles(10);
  waitForACK();
  uint32_t data = top->wb_dat_o;
  top->wb_stb_i = 0;
  runForClockCycles(10);

  return data;
}

void writeToAddress(uint32_t pAddress, uint32_t pData) {
  top->wb_adr_i = pAddress;
  top->wb_dat_i = pData;
  top->wb_we_i = 1;
  top->wb_stb_i = 1;
  runForClockCycles(10);
  waitForACK();
  top->wb_stb_i = 0;
  top->wb_we_i = 0;
  runForClockCycles(10);
}

void reportHash() {
  cout << "Hash: 0x";
  for(int i = (128 / 32) - 1; i >= 0; --i) {
    uint32_t hashPart = readFromAddress(HASH_BASE + i);
    printWordAsBytes(hashPart);
  }
  cout << endl;
}

void updateHash(char *pHash) {
    uint32_t* hPtr = (uint32_t *)pHash;
    
    for(int i = (128 / 32) - 1; i >= 0; --i) {
        *hPtr++ = readFromAddress(HASH_BASE + i);
    }
}

void reportAppended() {
    cout << "Padded input:" << endl;
    for(int i = (512 / 32) - 1; i >= 0; --i) {
        cout << "0x";
	uint32_t dataPart = readFromAddress(MSG_BASE + i);
        printWordAsBytes(dataPart);
        cout << endl;
    }
}

void waitForReady() {
  while(readFromAddress(MD5_READY) == 0) {
    runForClockCycles(15);
  }
}

void waitForValidOut() {
  waitForReady();
}

void resetAndReady() {
  writeToAddress(MD5_RST, 0x1);
  runForClockCycles(30);
  writeToAddress(MD5_RST, 0x0);
  waitForReady();
}

void strobeMsgValid() {
  writeToAddress(MD5_READY, 0x1);
  writeToAddress(MD5_READY, 0x0);
}

void loadPaddedMessage(const char* msg_ptr) {
  for(int i = 0; i < ((512 / 8) / 4); ++i) {
    uint32_t temp = 0;
    for(int j = 0; j < 4; ++j) {
      ((char *)(&temp))[j] = *msg_ptr++;
    }
    writeToAddress(MSG_BASE + i, temp);
  }
}

int main(int argc, char **argv, char **env) {
  Verilated::commandArgs(argc, argv);

  top = new Vmd5_top;
  char hash[128 /8];
    
  cout << "Resetting the wishbone interface..." << endl;

  // Unused/constant signals
  top->wb_rst_i = 0;
  top->wb_bte_i = 0;
  top->wb_cti_i = 0;
  top->wb_cyc_i = 0;
  top->wb_sel_i = 15;
    
  // Reset bus interface
  top->wb_rst_i = 1;
  runForClockCycles(10);
  top->wb_rst_i = 0;

  cout << "Reset complete" << endl;
  cout << "Resetting the MD5 module..." << endl;
    
  // Test reset behavior
  resetAndReady();

  cout << "Reset complete" << endl;

  resetAndReady();
  hashString("a", hash);
  compareHash(hash, "0cc175b9c0f1b6a831c399e269772661", "single character");
    
  resetAndReady();
  hashString("abc", hash);
  compareHash(hash, "900150983cd24fb0d6963f7d28e17f72", "three character");
    
  // Test a message that requires an extra chunk to be added to hold padding
  resetAndReady();
  hashString("Lincoln LaboratoLincoln LaboratoLincoln LaboratoLincoln Laborato", hash);
  compareHash(hash, "93f3763d2c20d33e5031e20480eaca58", "512-bit sized message");

  // Hash a test file
  resetAndReady();
  ifstream inFile("input.txt", ios::in|ios::binary|ios::ate);
  if(inFile.is_open()) {
    streampos size = inFile.tellg();

    inFile.seekg(0, ios::beg);

    int messageBits = (size - inFile.tellg()) * 8;
    printf("Message length: %d bits\n", messageBits);
      
    char * block = new char[messageBits / 8];
    memset(block, 0, messageBits/8);
      
    inFile.read(block, size);
    inFile.close();
      
    hashString(block, hash);
    compareHash(hash, "a3af880db6067a6c7592fb01e877e767", "text file");
  } else {
    cout << "ERROR: unable to open input.txt" << endl;
  }

  top->final();
  delete top;
  exit(0);
}