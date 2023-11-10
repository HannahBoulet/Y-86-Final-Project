#include "PipeRegArray.h"
#include "DecodeStage.h"
#include "ConditionCodes.h"
#include "Instruction.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "Stage.h"
#include "RegisterFile.h"
#include "M.h"
#include "W.h"
#include "D.h"
#include "E.h"

/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pipeRegs - array of the pipeline register 
                      (F, D, E, M, W instances)
 */
bool DecodeStage::doClockLow(PipeRegArray * pipeRegs)
{
   PipeReg * dreg = pipeRegs->getDecodeReg();
   PipeReg * ereg = pipeRegs->getExecuteReg();
   PipeReg * mreg = pipeRegs->getMemoryReg();
   PipeReg * wreg = pipeRegs->getWritebackReg();

   uint64_t stat = dreg->get(D_STAT);
   uint64_t icode = dreg->get(D_ICODE);
   uint64_t ifun = dreg->get(D_IFUN);

   uint64_t valC = dreg->get(D_VALC);
   //uint64_t valP = dreg->get(D_VALP);

   d_srcA = d_srcAFun(dreg);
   d_srcB = d_srcBFun(dreg);
   uint64_t rA = dreg->get(D_RA);
   uint64_t rB = dreg->get(D_RB);


   
   uint64_t dstE = d_dstEFun(dreg);
   uint64_t dstM = d_dstMFun(dreg);

   //unsure if supposed to be ra or drcA
   
   uint64_t valA = selFwdAFun(dreg, mreg, wreg, d_srcA);
   uint64_t valB = selFwdBFun(dreg, mreg, wreg, d_srcB);

   setEInput(ereg, stat, icode, ifun, valA, valC, valB, dstE, dstM, d_srcA, d_srcB);
   return false;
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 * 
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
*/
void DecodeStage::doClockHigh(PipeRegArray * pipeRegs)
{
   PipeReg * ereg = pipeRegs->getExecuteReg();  
   ereg->normal();
}

//Extra Helpers!

void DecodeStage::setEInput(PipeReg * Ereg, uint64_t stat, 
uint64_t icode, uint64_t ifun, uint64_t valA,
uint64_t valC, uint64_t valB,uint64_t dstE,
uint64_t dstM,uint64_t srcA, uint64_t srcB)
{
   Ereg->set(E_STAT, stat);
   Ereg->set(E_ICODE, icode);
   Ereg->set(E_IFUN, ifun);

   Ereg->set(E_DSTE, dstE);
   Ereg->set(E_DSTM, dstM);

   Ereg->set(E_SRCA, srcA);
   Ereg->set(E_SRCB, srcB);

   Ereg->set(E_VALC, valC);
   Ereg->set(E_VALA, valA);
   Ereg->set(E_VALB, valB);


}


uint64_t DecodeStage::d_srcAFun(PipeReg * dreg)
{
   if(dreg->get(D_ICODE)==Instruction::IRRMOVQ || dreg->get(D_ICODE)==Instruction::IRMMOVQ || dreg->get(D_ICODE)==Instruction::IOPQ || dreg->get(D_ICODE)==Instruction::IPUSHQ)
   {
      return dreg->get(D_RA);
   }
   if(dreg->get(D_ICODE)==Instruction::IPOPQ || dreg->get(D_ICODE)==Instruction::IRET)
   {
      return RegisterFile::rsp;
   }
   return RegisterFile::RNONE;
}

uint64_t DecodeStage::d_srcBFun(PipeReg * dreg)
{
   if(dreg->get(D_ICODE)==Instruction::IOPQ||dreg->get(D_ICODE)==Instruction::IRMMOVQ||dreg->get(D_ICODE)==Instruction::IMRMOVQ)
   {
      return dreg->get(D_RB);
   }
   if(dreg->get(D_ICODE)==Instruction::IPUSHQ||dreg->get(D_ICODE)==Instruction::IPOPQ||dreg->get(D_ICODE)==Instruction::ICALL||dreg->get(D_ICODE)==Instruction::IRET)
   {
      return RegisterFile::rsp;
   }
   return RegisterFile::RNONE;

}

uint64_t DecodeStage::d_dstEFun(PipeReg * dreg)
{
   if(dreg->get(D_ICODE)==Instruction::IRRMOVQ||dreg->get(D_ICODE)==Instruction::IIRMOVQ||dreg->get(D_ICODE)==Instruction::IOPQ)
   {
      return dreg->get(D_RB);
   }
   if(dreg->get(D_ICODE)==Instruction::IPUSHQ || dreg->get(D_ICODE)==Instruction::IPOPQ||dreg->get(D_ICODE)==Instruction::ICALL||dreg->get(D_ICODE)==Instruction::IRET)
   {
      return RegisterFile::rsp;
   }
   return RegisterFile::RNONE;
}


uint64_t DecodeStage::d_dstMFun(PipeReg * dreg)
{
   if(dreg->get(D_ICODE)==Instruction::IMRMOVQ||dreg->get(D_ICODE)==Instruction::IPOPQ)
   {
      return dreg->get(D_RA);
   }
   return RegisterFile::RNONE;
}

uint64_t DecodeStage::selFwdAFun(PipeReg * dreg, PipeReg * mreg, PipeReg * wreg, uint64_t srcA)
{
   bool error = false;
   if(srcA == Stage::e_dstE)
   {
      return Stage::e_valE;
   }
   else if(srcA == mreg->get(M_DSTE))
   {
      return wreg->get(M_VALE);
   }
   else if(srcA == wreg->get(W_DSTE))
   {
      return wreg->get(W_VALE);
   }
   return rf->readRegister(srcA, error);
}

uint64_t DecodeStage::selFwdBFun(PipeReg * dreg, PipeReg * mreg, PipeReg * wreg, uint64_t srcB)
{
   bool error = false;
   if(srcB == Stage::e_dstE)
   {
      return Stage::e_valE;
   }
   else if(srcB == mreg->get(M_DSTE))
   {
      return mreg->get(M_VALE);
   }
   else if(srcB == wreg->get(W_DSTE))
   {
      return wreg->get(W_VALE);
   }
   return rf->readRegister(srcB, error);
}

