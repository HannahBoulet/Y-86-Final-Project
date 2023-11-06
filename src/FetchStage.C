//TODO add more #includes as you need them
#include <iostream>
#include <cstdint>
#include <stdio.h>
#include "PipeRegArray.h"
#include "PipeReg.h"
#include "Memory.h"
#include "FetchStage.h"
#include "Instruction.h"
#include "RegisterFile.h"
#include "Status.h"
#include "W.h"
#include "M.h"
#include "F.h"
#include "D.h"
#include "Tools.h"


/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pipeRegs - array of the pipeline register 
                      (F, D, E, M, W instances)
 */
bool FetchStage::doClockLow(PipeRegArray * pipeRegs)
{
   PipeReg * freg = pipeRegs->getFetchReg();
   PipeReg * dreg = pipeRegs->getDecodeReg();
   PipeReg * mreg = pipeRegs->getMemoryReg();
   PipeReg * wreg = pipeRegs->getWritebackReg();

   bool mem_error = false;
   uint64_t icode = Instruction::INOP, ifun = Instruction::FNONE;
   uint64_t rA = RegisterFile::RNONE, rB = RegisterFile::RNONE;
   uint64_t valC = 0, valP = 0, stat = 0, predPC = 0;
   bool needvalC = false;
   bool needregId = false;

   //TODO 
   //select PC value and read byte from memory
   //set icode and ifun using byte read from memory

   uint64_t f_pc = selectPC(freg, mreg, wreg);
   uint64_t inst = mem->getByte(f_pc, mem_error);
   icode = Tools::getBits(inst, 4, 7);
   ifun = Tools::getBits(inst, 0, 3);

   if (mem_error)
   {
      icode = Instruction::INOP;
      ifun = Instruction::FNONE;
   }

   //uint64_t f_pc =  .... call your select pc function
   //f_pc = selectPC(freg, mreg, wreg);
 

   //status of this instruction is SAOK (this will change later)
   if(icode == Instruction::IHALT)
   {
      stat = Status::SHLT;
   }
   else
   {
      stat = Status::SAOK;
   }
   


   //TODO
   //In order to calculate the address of the next instruction,
   //you'll need to know whether this current instruction has an
   //immediate field and a register byte. (Look at the instruction encodings.)

   needvalC = need_valC(icode);

   needregId = need_regids(icode); //.... call your need regId function

   //TODO
   //determine the address of the next sequential function
   valP = PCincrement(f_pc, needregId, needvalC); //..... call your PC increment function 

   //TODO
   //calculate the predicted PC value
   predPC = predictPC(icode, valC, valP); //.... call your function that predicts the next PC   

   valC = buildValC(f_pc, needregId, needvalC);
   getRegIds(f_pc, needregId, rA, rB);

   //set the input for the PREDPC pipe register field in the F register
   freg->set(F_PREDPC, predPC);

   //set the inputs for the D register
   setDInput(dreg, stat, icode, ifun, rA, rB, valC, valP);
   return false;
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 * 
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
*/
void FetchStage::doClockHigh(PipeRegArray * pipeRegs)
{
   PipeReg * freg = pipeRegs->getFetchReg();  
   PipeReg * dreg = pipeRegs->getDecodeReg();
   freg->normal();
   dreg->normal();
}

/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
*/
void FetchStage::setDInput(PipeReg * dreg, uint64_t stat, uint64_t icode,
                           uint64_t ifun, uint64_t rA, uint64_t rB,
                           uint64_t valC, uint64_t valP)
{
   dreg->set(D_STAT, stat);
   dreg->set(D_ICODE, icode);
   dreg->set(D_IFUN, ifun);
   dreg->set(D_RA, rA);
   dreg->set(D_RB, rB);
   dreg->set(D_VALC, valC);
   dreg->set(D_VALP, valP);
}
//TODO
//Write your selectPC, needRegIds, needValC, PC increment, and predictPC methods
//Remember to add declarations for these to FetchStage.h

// Here is the HCL describing the behavior for some of these methods. 
/*

//selectPC method: input is F, M, and W registers
word f_pc = [
    M_icode == IJXX && !M_Cnd : M_valA;
    W_icode == IRET : W_valM;
    1: F_predPC;
];
*/
uint64_t FetchStage::selectPC(PipeReg * freg, PipeReg * mdreg, PipeReg * wreg)
{
   if (mdreg->get(M_ICODE) == Instruction::IJXX && !mdreg->get(M_CND))
   {
      return mdreg->get(M_VALA);
   }
   if (wreg->get(W_ICODE) == Instruction::IRET) 
   {
      return wreg->get(W_VALM);
   }
   return freg->get(F_PREDPC);
}



/* needRegIds  method: input is f_icode
*bool need_regids = f_icode in { IRRMOVQ, IOPQ, IPUSHQ, IPOPQ, IIRMOVQ, IRMMOVQ, IMRMOVQ };
*/
bool FetchStage::need_regids(uint64_t f_icode)
{
   if (f_icode == Instruction::IRRMOVQ || f_icode == Instruction::IOPQ || f_icode == Instruction::IPUSHQ
      || f_icode ==  Instruction::IPOPQ || f_icode ==  Instruction::IIRMOVQ || f_icode ==  Instruction::IRMMOVQ
      || f_icode == Instruction::IMRMOVQ)
      {
         return true;
      }
   return false;
}

/*needValC method: input is f_icode
*bool need_valC = f_icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IJXX, ICALL };
*/
bool FetchStage::need_valC(uint64_t f_icode)
{
   if (f_icode == Instruction::IIRMOVQ || f_icode == Instruction::IRMMOVQ || f_icode == Instruction::IMRMOVQ
      || f_icode == Instruction::IJXX || f_icode == Instruction::ICALL)
   {
      return true;
   }

   return false;
}

/* predictPC method: inputs are f_icode, f_valC, f_valP
* word f_predPC = [
    f_icode in { IJXX, ICALL } : f_valC;
    1: f_valP;
];
*/
uint64_t FetchStage::predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP)
{
   if (f_icode == Instruction::IJXX || f_icode == Instruction::ICALL)
   {
      return f_valC;
   }

   return f_valP;
}

/*
* PCincrement method
*  takes as input the address of the current instruction (f_pc)
*  the result of needRegIds, and the result of needValC
*  and calculates the address of the next sequential instruction
*  The value returned by PCincrement is stored in valP
*  The value of valP is then used as input to predictPC along with
*  the icode value and the value of valC (0 for now).
*  The output of predictPC is the input to the F_predPC register
*/
uint64_t FetchStage::PCincrement(uint64_t f_pc, bool needRegIds, bool needValC)
{
   if (needRegIds && needValC)
   {
      return f_pc + 10;
   }
   else if (!needRegIds && needValC)
   {
      return f_pc + 9;
   }
   else if (needRegIds && !needValC)
   {
      return f_pc + 2;
   }
   else
   {
      return f_pc + 1;
   }

}

/*
* getRegIds - if need_regId is true, this 
* method is used to read the register byte and initialize rA
* and rB to the rB to the appropriate bits in the register byte. 
* these are then used as input to the D register.
*/
void FetchStage::getRegIds(uint64_t f_pc, bool needRegIds, uint64_t & rA, uint64_t & rB)
{
   bool error = false;

   if (needRegIds == true)
   {
      uint64_t regByte = mem->getByte(f_pc + 1, error);
      rA = Tools::getBits(regByte, 4, 7);
      rB = Tools::getBits(regByte, 0, 3);
   }
}

/*
* buildValC -  if need_valC is true, this method
* reads 8 bytes from memory and builds
* and returns the valC that is then used as
* input to the D register
*/
uint64_t FetchStage::buildValC(uint64_t f_pc, bool needRegIds, bool needvalC)
{
   
   uint8_t valArray[8];

   if (!needvalC) return 0;
   int32_t addr = f_pc + 1;
   if (needRegIds) addr++;
      bool error = false;
         for (int i = 0; i < 8; i++, addr++)
         {
            valArray[i] = mem->getByte(addr, error);
         }
      return Tools::buildLong(valArray);  
}
