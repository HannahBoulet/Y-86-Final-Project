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
   uint64_t d_srcA = d_srcAFun(dreg);
   uint64_t d_srcB = d_srcBFun(dreg);
   
   uint64_t dstE = d_dstEFun(dreg);
   uint64_t dstM = d_dstMFun(dreg);
   
   uint64_t valA = selFwdAFun(dreg, mreg, wreg, d_srcA);
   uint64_t valB = FwdBFun(dreg, mreg, wreg, d_srcB);

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

/* setEInput
 *
 * provides the input to potentially be stored in the E register
 * during doClockHigh.
 *
 * @param Ereg - Pointer to the Execute stage pipeline register instance.
 * @param stat - Status code to be set in the E register.
 * @param icode - Instruction code to be set in the E register.
 * @param ifun - Instruction function to be set in the E register.
 * @param valA - Value of operand A to be set in the E register.
 * @param valC - Constant value to be set in the E register.
 * @param valB - Value of operand B to be set in the E register.
 * @param dstE - Destination register for the Execute stage to be set in the E register.
 * @param dstM - Destination register for the Memory stage to be set in the E register.
 * @param srcA - Source register A to be set in the E register.
 * @param srcB - Source register B to be set in the E register.
 */
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

/*
 * d_srcAfun - Determines the source register
 * A for the Execute Stage based on the instruction 
 * type in the Decode Stage
*/
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
/*
 * d_srcBFun - Determines the source register B 
 * for the Execute Stage based on the instruction
 * type in the Decode Stage.
 */
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
/*
 * d_dstEFun - Determines the destination register 
 * E for the Execute Stage based on the 
 * instruction type in the Decode Stage.
*/
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

/*
 * d_dstMFun - Determines the destination 
 * register M for the Memory Stage based on the 
 * instruction type in the Decode Stage.
*/
uint64_t DecodeStage::d_dstMFun(PipeReg * dreg)
{
   if(dreg->get(D_ICODE)==Instruction::IMRMOVQ||dreg->get(D_ICODE)==Instruction::IPOPQ)
   {
      return dreg->get(D_RA);
   }
   return RegisterFile::RNONE;
}

/*
 * selFwdAFun - Selects the source value 
 * for operand A in the Execute Stage, 
 * considering forwarding from previous stages.
*/
uint64_t DecodeStage::selFwdAFun(PipeReg * dreg, PipeReg * mreg, PipeReg * wreg, uint64_t srcA)
{
   bool error = false;
   if(srcA == Stage::e_dstE)
   {
      return Stage::e_valE;
   }
   else if(srcA == mreg->get(M_DSTE))
   {
      return mreg->get(M_VALE);
   }
   else if(srcA == wreg->get(W_DSTE))
   {
      return wreg->get(W_VALE);
   }
   return rf->readRegister(srcA, error);
}
/*
 * FwdBFun - Selects the source value for 
 * operand B in the Execute Stage, considering 
 * forwarding from previous stages.
*/
uint64_t DecodeStage::FwdBFun(PipeReg * dreg, PipeReg * mreg, PipeReg * wreg, uint64_t srcB)
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

