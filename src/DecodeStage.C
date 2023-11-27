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
   calculateControlSignals(ereg, dreg, mreg);
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
   if (!e_bubble)
   {
      ereg->normal();
   }
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

/**
 * d_srcAFun
 * Determines the source register A for the Execute Stage based on the instruction type in the Decode Stage.
 * @param dreg Pointer to the pipeline register containing relevant information.
 * @return The source register A for the Execute Stage.
 */
uint64_t DecodeStage::d_srcAFun(PipeReg * dreg)
{
   uint64_t icode = dreg->get(D_ICODE);

   if(icode==Instruction::IRRMOVQ || icode==Instruction::IRMMOVQ || icode==Instruction::IOPQ || icode==Instruction::IPUSHQ)
   {
      return dreg->get(D_RA);
   }
   if(icode==Instruction::IPOPQ || icode==Instruction::IRET)
   {
      return RegisterFile::rsp;
   }
   return RegisterFile::RNONE;
}

/**
 * d_srcBFun
 * Determines the source register B for the Execute Stage based on the instruction type in the Decode Stage.
 * @param dreg Pointer to the pipeline register containing relevant information.
 * @return The source register B for the Execute Stage.
 */
uint64_t DecodeStage::d_srcBFun(PipeReg * dreg)
{
   uint64_t icode = dreg->get(D_ICODE);
   if(icode==Instruction::IOPQ||icode==Instruction::IRMMOVQ||icode==Instruction::IMRMOVQ)
   {
      return dreg->get(D_RB);
   }
   if(icode==Instruction::IPUSHQ||icode==Instruction::IPOPQ||icode==Instruction::ICALL||icode==Instruction::IRET)
   {
      return RegisterFile::rsp;
   }
   return RegisterFile::RNONE;
}

/**
 * d_dstEFun
 * Determines the destination register E for the Execute Stage based on the instruction type in the Decode Stage.
 * @param dreg Pointer to the pipeline register containing relevant information.
 * @return The destination register E for the Execute Stage.
 */
uint64_t DecodeStage::d_dstEFun(PipeReg * dreg)
{
   uint64_t icode = dreg->get(D_ICODE);

   if(icode==Instruction::IRRMOVQ||icode==Instruction::IIRMOVQ||icode==Instruction::IOPQ)
   {
      return dreg->get(D_RB);
   }
   if(icode==Instruction::IPUSHQ || icode==Instruction::IPOPQ||icode==Instruction::ICALL||icode==Instruction::IRET)
   {
      return RegisterFile::rsp;
   }
   return RegisterFile::RNONE;
}

/**
 * d_dstMFun
 * Determines the destination register M for the Memory Stage based on the instruction type in the Decode Stage.
 * @param dreg Pointer to the pipeline register containing relevant information.
 * @return The destination register M for the Memory Stage.
 */
uint64_t DecodeStage::d_dstMFun(PipeReg * dreg)
{
   uint64_t icode = dreg->get(D_ICODE);

   if(icode==Instruction::IMRMOVQ||icode==Instruction::IPOPQ)
   {
      return dreg->get(D_RA);
   }
   return RegisterFile::RNONE;
}

/**
 * selFwdAFun
 * Selects the forwarding source A for the current stage based on different pipeline register values.
 * @param: dreg Pointer to the Decode Stage pipeline register containing relevant information.
 * @param: mreg Pointer to the Memory Stage pipeline register containing relevant information.
 * @param: wreg Pointer to the Writeback Stage pipeline register containing relevant information.
 * @param: srcA The source register A value to determine forwarding.
 * @return The selected forwarding source A value.
 */
uint64_t DecodeStage::selFwdAFun(PipeReg * dreg, PipeReg * mreg, PipeReg * wreg, uint64_t srcA)
{
   bool error = false;
   uint64_t icode = dreg->get(D_ICODE);

   if (icode == Instruction::ICALL || icode == Instruction::IJXX)
   {
      return dreg->get(D_VALP);
   }
   if (srcA == RegisterFile::RNONE)
   { 
      return 0;
   }
   if(srcA == Stage::e_dstE)
   {
      return Stage::e_valE;
   }
   else if (srcA == mreg->get(M_DSTM))
   {
      return m_valM;
   }
   else if(srcA == mreg->get(M_DSTE))
   {
      return mreg->get(M_VALE);
   }
   else if (srcA == wreg->get(W_DSTM))
   {
      return wreg->get(W_VALM);
   }
   else if(srcA == wreg->get(W_DSTE))
   {
      return wreg->get(W_VALE);
   }
   return rf->readRegister(srcA, error);
}

/**
 * FwdBFun
 * Determines the forwarding source B for the current stage based on different pipeline register values.
 * @param: dreg Pointer to the Decode Stage pipeline register containing relevant information.
 * @param: mreg Pointer to the Memory Stage pipeline register containing relevant information.
 * @param: wreg Pointer to the Writeback Stage pipeline register containing relevant information.
 * @param: srcB The source register B value to determine forwarding.
 * @return The selected forwarding source B value.
 */
uint64_t DecodeStage::FwdBFun(PipeReg * dreg, PipeReg * mreg, PipeReg * wreg, uint64_t srcB)
{
   bool error = false;
   if (srcB == RegisterFile::RNONE) 
   {
      return 0;
   }
   if(srcB == Stage::e_dstE)
   {
      return Stage::e_valE;
   }
   else if (srcB == mreg->get(M_DSTM))
   {
      return m_valM;
   }
   else if(srcB == mreg->get(M_DSTE))
   {
      return mreg->get(M_VALE);
   }
   else if (srcB == wreg->get(W_DSTM))
   {
      return wreg->get(W_VALM);
   }
   else if(srcB == wreg->get(W_DSTE))
   {
      return wreg->get(W_VALE);
   }
   return rf->readRegister(srcB, error);
}

/**
* E_bubble
* Bubbles the Execute stage.
*
* @param: ereg a pointer to the pipeline register for the execute stage.
* @param: dreg a pointer to the pipeline register for the decode stage.
* @param: mreg a pointer to the pipeline register for the memory stage.
* @return: true if execute is bubbled, false otherwise.
*/
bool DecodeStage::E_bubble(PipeReg * ereg, PipeReg * dreg, PipeReg * mreg)
{
   if ((ereg->get(E_ICODE) == Instruction::IMRMOVQ || ereg->get(E_ICODE) == Instruction::IPOPQ) 
      && (ereg->get(E_DSTM) == Stage::d_srcA || ereg->get(E_DSTM) == Stage::d_srcB))
   {
      return true;
   }
   return false;
}

/**
* calculateControlSignals
* Calculates the control signal from E_bubble.
*
* @param: ereg a pointer to the pipeline register for the execute stage.
* @param: dreg a pointer to the pipeline register for the decode stage.
* @param: mreg a pointer to the pipeline register for the memory stage.
* @return: Calculated value for E_bubble.
*/
uint64_t DecodeStage::calculateControlSignals(PipeReg * ereg, PipeReg * dreg, PipeReg * mreg)
{
   e_bubble = E_bubble(ereg, dreg, mreg);
}
