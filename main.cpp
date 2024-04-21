//
//  pipeline.cpp
//  Coding
//
//  Created by vibhav kanakamedala and Bhargav Ganesh on 12/04/24.
//
#include <bits/stdc++.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
using namespace std;

void print(int8_t n,ofstream &output){
    if(n<0){
        int k = 256+n;
        output<<hex<<k+0<<endl;
        return;
    }
    output<<hex<<setw(2)<<setfill('0')<<n+0<<endl;

}

int ICache[256],PC=0;
int8_t RF[16],DCache[256];
bool reg[16];
int num_load=0,num_store=0;
int num_stall = 0, num_dat_st = 0, num_con_st = 0, num_inst = 0, num_cycles = 0;
int num_arith = 0, num_log = 0, num_con = 0, num_halt = 0,num_shift=0;
int halt = 0, can_fetch = 1;

int twoscomplement(int n)
{
    // array to store binary number
    vector<int> binaryNum(4);
    int k = n;
    // counter for binary array
    int i = 0;
    while (n > 0) {

        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }
    if(binaryNum[3]==1){
        return k-16;
    }
    return k;
}





class Instruction {
private:
    int IR[4],IPC;
    int rs1,rs2,Imm,ALUOuptut;
    int8_t Imm8;
    
    int LMD,is_stalled=0;
    
public:
    int stage=0;
    
    int is_ready(int &x,int reg_id){
        if(reg[reg_id]){
            x =  static_cast<int8_t>(RF[reg_id]);
            is_stalled=0;
        }
        else{
            is_stalled = 1;
        }
        return is_stalled;
    }
    
    void fetch(){
        IR[3] = (ICache[PC+1])&15;
        IR[2] = (ICache[PC+1]>>4)&15;
        IR[1] = (ICache[PC])&15;
        IR[0] = (ICache[PC]>>4)&15;
        PC = PC+2;
        IPC = PC;
        stage=1;
    }
    
    void decode(){
        switch(IR[0]){
            case 0: //ADD instruction
            case 1: //SUB instruction
            case 2: //MUL instruction
            case 4: //AND instruction
            case 5: //OR instruction
            case 6: //XOR instruction
                if(is_ready(rs1,IR[2])) return;
                if(is_ready(rs2,IR[3])) return;
                reg[IR[1]] = false;
                break;
            case 3: //INC instruction
                if(is_ready(rs1,IR[1])) return;
                reg[IR[1]] = false;
                break;
            case 7: //NOT instruction
                if(is_ready(rs1, IR[2])) return;
                reg[IR[1]] = false;
                break;
            case 8: //SLLI instruction
            case 9: //SRLI instruction
                if(is_ready(rs1, IR[2])) return;
                Imm = IR[3];
                reg[IR[1]] = false;
                break;
            case 11: //LD instruction
                if(is_ready(rs1, IR[2])) return;
                Imm = twoscomplement(IR[3]);
                reg[IR[1]] = false;
                break;
            case 10: //LI instruction
                Imm  = (IR[2]<<4)+(IR[3]);
                reg[IR[1]] = false;
                break;
            case 12: //ST instruction
                if(is_ready(rs1, IR[1])) return;
                if(is_ready(rs1, IR[2])) return;
                Imm = twoscomplement(IR[3]);
                break;
            case 13: //JMP instruction
                Imm = (IR[1]<<4)+(IR[2]);
                can_fetch=0;
                break;
            case 14: //BEQZ instruction
                if(is_ready(rs1, IR[1])) return;
                Imm = (IR[2]<<4) + IR[3];
                can_fetch=0;
                break;
            case 15: //HLT instruction
                halt = 1;
                break;
        }
        stage++;
    }
    
    void execute(){
        switch (IR[0]) {
            case 0://ADD
                ALUOuptut = rs1 + rs2;
                reg[IR[1]] = 0;
                break;
            case 1://SUB
                ALUOuptut = rs1 - rs2;
                reg[IR[1]] = 0;
                break;
            case 2://MUL
                ALUOuptut = rs1*rs2;
                reg[IR[1]] = 0;
                break;
            case 3://INC
                ALUOuptut = rs1 + 1;
                reg[IR[1]] = 0;
                break;
            case 4://AND
                ALUOuptut = rs1 & rs2;
                reg[IR[1]] = 0;
                break;
            case 5://OR
                ALUOuptut = rs1 | rs2;
                reg[IR[1]] = 0;
                break;
            case 6://XOR
                ALUOuptut = rs1^rs2;
                reg[IR[1]] = 0;
                break;
            case 7://NOT
                
                ALUOuptut = -1*(rs1+1);
              
                reg[IR[1]] = 0;
                break;
            case 8://SLLI
                ALUOuptut = rs1 << Imm;
                reg[IR[1]] = 0;
                break;
            case 9://SRLI
                ALUOuptut = rs1 >> Imm;
                reg[IR[1]] = 0;
                break;
            case 10://LI
                ALUOuptut = Imm;
                reg[IR[1]] = 0;
                break;
            case 11://LD
                ALUOuptut = rs1 + Imm;
                reg[IR[1]] = 0;
                break;
            case 12://ST
                ALUOuptut =rs1 + Imm;
                break;
            case 13://JMP
                Imm8 = Imm;
                ALUOuptut = IPC + Imm8*2;
                PC = ALUOuptut;
//                can_fetch = 1;
                break;
            case 14://BEQZ
                Imm8 = Imm;
                ALUOuptut = IPC + Imm8*2;
                if(!rs1) PC = ALUOuptut;
//                can_fetch = 1;
                break;
            case 15://HLT
                break;
        }
        stage++;
    }
    
    void mem_access(){
        switch (IR[0]) {
            case 10:
                LMD = Imm;
                break;
            case 11:
                LMD = DCache[ALUOuptut];
                break;
            case 12:
                DCache[ALUOuptut] = RF[IR[1]];
                break;
            case 13:
            case 14:
                can_fetch=1;
                break;
        }
        stage++;
    }
    
    void write(){
        if(IR[0]<10){
            RF[IR[1]] = ALUOuptut;
            reg[IR[1]] = 1;
        }
        else if(IR[0]<12){
            RF[IR[1]] = LMD;
            reg[IR[1]] = 1;
        }
        stage++;
        count();
    }
    
    int next_stage(){
        switch (stage) {
            case 1:
                decode();
                break;
            case 2:
                execute();
                break;
            case 3:
                mem_access();
                break;
            case 4:
                write();
                break;
        }
        return is_stalled;
    }
    
    void count(){
        if (IR[0] < 4)
            num_arith++;
        else if (IR[0] < 8)
            num_log++;
        else if (IR[0] < 10)
            num_shift++;
        else if (IR[0] < 11)
            num_load++;
        else if(IR[0]<13)
            num_store++;
        else if (IR[0]<15)
            num_con++;
        else
            num_halt++;
    }
    
};

void read_from_file(int arr[],int size,string name){
    ifstream file;
    file.open(name);
    string s;
    int i=0;
    for (int i = 0; i < size; ++i) {
        // Read hexadecimal values from the file
        std::string hexValue;
        file >> hexValue;

        // Convert hexadecimal string to integer
        arr[i] = std::stoi(hexValue, nullptr, 16);
    }

    file.close();}
void read_from_file2(int8_t arr[],int size,string name){
    ifstream file;
    file.open(name);
    string s;
    int i=0;
    for (int i = 0; i < size; ++i) {
        // Read hexadecimal values from the file
        std::string hexValue;
        file >> hexValue;

        // Convert hexadecimal string to integer
        arr[i] = std::stoi(hexValue, nullptr, 16);
    }

    file.close();}



int main(int argc,char* argv[]){
     int option_read;
 string infolder,outfolder;
  const option long_options[] =
    {
      /* These two options set a common flag to either 0 or 1. */
      
      {"in",   required_argument, 0, 'i'},
      
      {"out",  required_argument, 0, 'o'},
      
      
      {0, 0, 0, 0}
    };

  while (1)
    {
      /* getopt_long stores the option index here. */
      int option_index = 0;

      option_read = getopt_long (argc, argv, "i:o:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (option_read == -1)
        break;

      switch (option_read)
        {
        

        case 'i':
          infolder = optarg;
          break;

        case 'o':
          outfolder = optarg;
          break;
        
        

        case '?':
          /* getopt_long already printed an error message. */
          break;

        default:
          abort ();
        }
    }










    string inpICache = infolder + "/ICache.txt";
    string inpDCache = infolder + "/DCache.txt";
    string inpRF = infolder + "/RF.txt";
    string outOutput = outfolder + "/Output.txt";
    string outDcache = outfolder + "/DCache.txt";
    string outRF = outfolder + "/RF.txt";











    
    queue<Instruction> INS;
    read_from_file(ICache, 256, inpICache);
    read_from_file2(DCache, 256,inpDCache);
    read_from_file2(RF, 16, inpRF);
    for(int i = 0; i < 16; i++)
        reg[i] = true;
    
    do{
        int stalls = 0;
        num_cycles++;
        unsigned long n = INS.size();
        for(int i=0;i<n;i++){
            Instruction s = INS.front();
            stalls += s.next_stage();
            INS.pop();
            if(s.stage<5){
                INS.push(s);
            }
        }
        if(!can_fetch){
            num_stall++;
            num_con_st++;
        }
        else if (stalls){
            num_stall++;
            num_dat_st++;
        }
        else if(!halt){
            num_inst++;
            Instruction s;
            s.fetch();
            INS.push(s);
        }
        
    }while(!INS.empty());
    
    ofstream output;
    output.open(outOutput);
    output << "Total number of instructions executed        : " << num_inst << endl;
    output << "Number of instructions in each class     " << endl;
    output << "Arithmetic instructions                      : " << num_arith << endl;
    output << "Logical instructions                         : " << num_log << endl;
    output << "Shift instructions                           : " << num_shift << endl;
    output << "Memory instructions                          : " << num_store << endl;
    output << "Load immediate instructions                  : " << num_load << endl;
    output << "Control instructions                         : " << num_con << endl;
    output << "Halt instructions                            : " << num_halt << endl;
    output << "Cycles Per Instruction                       : " << (double)num_cycles / num_inst << endl;
    output << "Total number of stalls                       : " << num_stall << endl;
    output << "Data stalls (RAW)                            : " << num_dat_st << endl;
    output << "Control stalls                               : " << num_con_st << endl;
    output.close();
    
    output.open(outDcache);
    for(int i=0;i<256;i+=1){
        print(DCache[i],output);
    }
    output.close();
    output.open(outRF);
    for(int i=0;i<16;i+=1){
        print(RF[i],output);
    }
}

