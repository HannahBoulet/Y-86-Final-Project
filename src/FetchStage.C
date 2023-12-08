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
#include "Stage.h"
#include "E.h"
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
   PipeReg * ereg = pipeRegs->getExecuteReg();
   PipeReg * mreg = pipeRegs->getMemoryReg();
   PipeReg * wreg = pipeRegs->getWritebackReg();

   bool mem_error = false;
   uint64_t icode = Instruction::INOP, ifun = Instruction::FNONE;
   uint64_t rA = RegisterFile::RNONE, rB = RegisterFile::RNONE;
   uint64_t valC = 0, valP = 0, stat = 0, predPC = 0;
   bool needvalC = false;
   bool needregId = false;
   uint64_t f_pc = selectPC(freg, mreg, wreg);
   uint64_t inst = mem->getByte(f_pc, mem_error);

   if(mem_error)
   {
      icode = Instruction::INOP;
      ifun = Instruction::FNONE;
   }
   else
   {
      icode = Tools::getBits(inst, 4, 7);
      ifun = Tools::getBits(inst, 0, 3);
   }

   bool valid = instr_valid(icode);
   stat = f_stat(mem_error, valid, icode);

   needvalC = need_valC(icode);

   needregId = need_regids(icode);

   valP = PCincrement(f_pc, needregId, needvalC); 
   valC = buildValC(f_pc, needregId, needvalC);
   
   calculateControlSignals(ereg, dreg, mreg);

   predPC = predictPC(icode, valC, valP);

   getRegIds(f_pc, needregId, rA, rB);

   freg->set(F_PREDPC, predPC);
   
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
   
   if (!f_stall)
   {
      freg->normal();
   }
   if(D_bubble)
   {
      ((D *)dreg)->bubble();
   }
   else
   {
      if (!d_stall)
      {
         dreg->normal();
      }
   }
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

/**
* selectPC
* 
* determines the PC value to be used by the fetch stage. 
*
* @param: freg a pointer to the pipeline register for the fetch stage.
* @param: mdreg a pointer to the pipeline register for the memory stage
* @paam: wreg a pointer to the pipeline register for the writeback stage.
* @return: the selected program counter value.
*/
uint64_t FetchStage::selectPC(PipeReg * freg, PipeReg * mreg, PipeReg * wreg)
{
   uint64_t m_icode = mreg->get(M_ICODE);
   uint64_t w_icode = wreg->get(W_ICODE);

   if (m_icode == Instruction::IJXX && !mreg->get(M_CND))
   {
      return mreg->get(M_VALA);
   }
   if (w_icode == Instruction::IRET) 
   {
      return wreg->get(W_VALM);
   }
   return freg->get(F_PREDPC);
}

/**
* need_regids
*
* Checks if the icode indicates an operation that requires register identifiers.
* 
* @param: f_icode the instruction code from the fetch stage.
* @return: true if requires reg id, false otherwise.
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

/**
* need_valC
*
* Determines whether the given icode requires valC.
*
* @param: f_icode the instruction code from the fetch stage.
* @return: true if instruction code requires the valC, false otherwise.
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

/**
* predictPC
*
* Predicts the next PC value based on the icode and its associated values.
*
* @param: f_icode the instruction code from the fetch stage.
* @param: f_valC the constant value associated with the instruction.
* @param: f_valP the current PC value
* @return: the predicted PC value.
*/
uint64_t FetchStage::predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP)
{
   if (f_icode == Instruction::IJXX || f_icode == Instruction::ICALL)
   {
      return f_valC;
   }
   return f_valP;
}

/**
* PCincrement
* Calculates the address of the next sequential instruction.
*
* @param: f_pc the current PC value.
* @param: needRegIds indicates whether regids are needed.
* @param: needValC indicates whether a valC is needed.
* @return: incremented PC value to be input to the F_predPC register.
*/
uint64_t FetchStage::PCincrement(uint64_t f_pc, bool needRegIds, bool needValC)
{
   uint64_t increment = 1;
    if (needRegIds) 
    {
        increment += 1;
    }
    if (needValC) 
    {
        increment += 8;
    }
    return f_pc + increment;
}

/**
* getRegIds 
* Reads the register byte and initializes rA and rB to the rB to the 
* appropriate bits in the register byte, then used as input to the D register.
* 
* @param: f_pc the current PC value.
* @param: needRegIds indicates whether regids are needed.
* @param:  needValC indicates whether a valC is needed.
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

/**
* buildValC
* 
* Reads 8 bytes from memory and builds and returns the valC that is then used as input to the D register.
*
* @param: f_pc the current PC value.
* @param: needRegIds indicates whether regids are needed.
* @param:  needValC indicates whether a valC is needed.
*
* @return: the constructed valC. 
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

/**
* instr_valid
* Checks if the given instruction is valid.
*
* @param: icode the instruction code to be checked.
* @return: true if instruction is valid, false otherwise.
*/
bool FetchStage::instr_valid(uint64_t icode)
{
   return (icode == Instruction::INOP || icode == Instruction::IHALT 
          || icode == Instruction::IRRMOVQ || icode == Instruction::IIRMOVQ 
          || icode == Instruction::IRMMOVQ || icode == Instruction::IMRMOVQ 
          || icode == Instruction::IOPQ || icode == Instruction::IJXX
          || icode == Instruction::ICALL || icode == Instruction::IRET 
          || icode == Instruction::IPUSHQ || icode == Instruction::IPOPQ);
}

/**
* f_stat
* Determines status code for Fetch stage.
*
* @param: mem_error indicates whether a memory error has ocurred.
* @param: i_valid indicates validity of an instruction.
* @param: the fetch stage instruction code.
* @return: the status code that indicates the current state of the fetch stage.
*/
uint64_t FetchStage::f_stat(bool mem_error, bool i_valid, uint64_t f_icode)
{
   if(mem_error)
   {
      return Status::SADR;
   }
   if(!i_valid)
   {
      return Status::SINS;
   }
   if(f_icode == Instruction::IHALT)
   {
      return Status::SHLT;
   }
   return Status::SAOK;
}

/**
* F_stall
* Stalls the Fetch stage. Detects a load/use between an instruction in the ExecuteStage
* and an instruction in the DecodeStage.
*
* @param: ereg a pointer to the pipeline register for the execute stage.
* @param: dreg a pointer to the pipeline register for the fetch stage.
* @param: mreg a pointer to the pipeline register for the fetch stage.
* @return: true if stall, false otherwise
*/
bool FetchStage::F_stall(PipeReg * ereg, PipeReg * dreg, PipeReg * mreg)
{
   uint64_t e_icode = ereg->get(E_ICODE);
   uint64_t d_icode = dreg->get(D_ICODE);
   uint64_t m_icode = mreg->get(M_ICODE);

   if (((e_icode == Instruction::IMRMOVQ || e_icode == Instruction::IPOPQ)
      && (ereg->get(E_DSTM) == Stage::d_srcA || ereg->get(E_DSTM) == Stage::d_srcB)) 
      || (Instruction::IRET == d_icode || Instruction::IRET == e_icode 
      || Instruction::IRET == m_icode))
   {
      return true;
   }
   return false;
}

/**
* D_stall
* Stalls the Decode stage. Detects a load/use between an instruction in the ExecuteStage
* and an instruction in the DecodeStage.
*
* @param: ereg a pointer to the pipeline register for the execute stage.
* @return: true if stall, false otherwise
*/
bool FetchStage::D_stall(PipeReg * ereg)
{
   if ((ereg->get(E_ICODE) == Instruction::IMRMOVQ || ereg->get(E_ICODE) == Instruction::IPOPQ)
      && (ereg->get(E_DSTM) == Stage::d_srcA || ereg->get(E_DSTM) == Stage::d_srcB))
   {
      return true;
   }
   return false;
}

/**
* getD_bubble
* Calculates the value of the bubbled D register.
* @param: ereg a pointer to the pipeline register for the execute stage.
* @param: dreg a pointer to the pipeline register for the fetch stage.
* @param: mreg a pointer to the pipeline register for the fetch stage.
* @return: the calculated value of D_bubble.
*/
bool FetchStage::getD_bubble(PipeReg * ereg, PipeReg * dreg, PipeReg * mreg)
{
   return ((ereg->get(E_ICODE) == Instruction::IJXX && !Stage::e_Cnd) 
      || (!((ereg->get(E_ICODE) == Instruction::IMRMOVQ || ereg->get(E_ICODE) == Instruction::IPOPQ) 
      && (ereg->get(E_DSTM) == Stage::d_srcA || ereg->get(E_DSTM) == Stage::d_srcB))
      && (Instruction::IRET == dreg->get(D_ICODE) || Instruction::IRET == ereg->get(E_ICODE) 
      || Instruction::IRET == mreg->get(M_ICODE))));
}

/**
* calculateControlSignals 
* Calls each of the two methods that calculates F_stall and D_stall.
*
* @param: ereg a pointer to the pipeline register for the execute stage.
* @param: dreg a pointer to the pipeline register for the fetch stage.
* @param: mreg a pointer to the pipeline register for the fetch stage.
* @return: the calculated values for F_stall and D_stall
*/
void FetchStage::calculateControlSignals(PipeReg * ereg, PipeReg * dreg, PipeReg * mreg)
{
   f_stall = F_stall(ereg, dreg, mreg);
   d_stall = D_stall(ereg);
   D_bubble = getD_bubble(ereg, dreg, mreg);
}
