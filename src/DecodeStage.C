#include "PipeRegArray.h"
#include "DecodeStage.h"
#include "ConditionCodes.h"
#include "Instruction.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "Stage.h"
#include "RegisterFile.h"
#include "D.h"
#include "E.h"

/*
will use the values of the pipeline register fields in 
the D pipeline register and determine the values to 
be stored in the E pipeline register. The doClockHigh 
method stores those values in the E pipeline register.
*/

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

   uint64_t stat = dreg->get(D_STAT);
   uint64_t icode = dreg->get(D_ICODE);
   uint64_t ifun = dreg->get(D_IFUN);

   uint64_t valC = dreg->get(D_VALC);
   uint64_t valP = dreg->get(D_VALP);

   d_srcA = d_srcAFun(dreg);
   d_srcB = d_srcBFun(dreg);

   uint64_t dstE = d_dstEFun(dreg);
   uint64_t dstM = d_dstMFun(dreg);

   uint64_t valA= selFwdBFun(d_srcA);
   uint64_t valB = selFwdBFun(d_srcB);

   // Call setEinput with the obtained values
   /*
   PipeReg * Ereg, uint64_t stat, 
uint64_t icode, uint64_t ifun, uint64_t valA,
uint64_t valC, uint64_t valB,uint64_t dstE,
uint64_t dstM,uint64_t srcA, uint64_t srcB)
   */
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

// HCL for srcA component
// word d_srcA = [
//    D_icode in { IRRMOVQ, IRMMOVQ, IOPQ, IPUSHQ } : D_rA;
//    D_icode in { IPOPQ, IRET } : RSP;
//    1: RNONE;  //no register needed
//
// ];
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


// HCL for srcB component
// word d_srcB = [
//     D_icode in { IOPQ, IRMMOVQ, IMRMOVQ } : D_rB;
//     D_icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RSP;
//     1: RNONE;   //no register needed
// ];
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
// HCL for dstE component
// word d_dstE = [
//     D_icode in { IRRMOVQ, IIRMOVQ, IOPQ} : D_rB;
//     D_icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RSP;
//     1: RNONE;
// ];
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

// HCL for dstM component
// word d_dstM = [
//     D_icode in { IMRMOVQ, IPOPQ } : D_rA;
//     1: RNONE;
// ];
uint64_t DecodeStage::d_dstMFun(PipeReg * dreg)
{
   if(dreg->get(D_ICODE)==Instruction::IMRMOVQ||dreg->get(D_ICODE)==Instruction::IPOPQ)
   {
      return dreg->get(D_RA);
   }
   return RegisterFile::RNONE;
}

//HCL for Sel+FwdA
// word d_valA = [
//     1: d_rvalA;   //value from register file
// ];

uint64_t DecodeStage::selFwdAFun(uint64_t rA)
{
   bool error = false;
   return rf->readRegister(rA, error);
}

// HCL for FwdB
// word d_valB = [
//    1: d_rvalB;  //value from register file
// ];

uint64_t DecodeStage::selFwdBFun(uint64_t rB)
{
   bool error = false;
   return rf->readRegister(rB, error);
}

