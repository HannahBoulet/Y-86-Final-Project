#include "PipeRegArray.h"
#include "ExecuteStage.h"
#include "RegisterFile.h"
#include "ConditionCodes.h"
#include "Tools.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "Stage.h"
#include "Instruction.h"
#include "E.h"
#include "M.h"
/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pipeRegs - array of the pipeline register 
                      (F, D, E, M, W instances)
 */
bool ExecuteStage::doClockLow(PipeRegArray * pipeRegs)
{
   PipeReg * ereg = pipeRegs->getExecuteReg();
   PipeReg * mreg = pipeRegs->getMemoryReg();

  
   uint64_t stat = ereg->get(E_STAT);
   uint64_t icode = ereg->get(E_ICODE);
   uint64_t ifun = ereg->get(E_IFUN);

   uint64_t cnd = CondFun(icode, ifun);
   uint64_t valA = ereg->get(E_VALA);
   //uint64_t valB = ereg->get(E_VALB);

   uint64_t dstm = ereg->get(E_DSTM);

   uint64_t V_aluA = aluA(ereg);
   uint64_t V_aluB = aluB(ereg);
   uint64_t fun = aluFun(ereg);


   uint64_t dste = set_dstE(ereg, cnd);

   uint64_t valE = getALU(V_aluA, V_aluB, fun);

   if(set_cc(ereg))
   {
      CC(valE, V_aluA, V_aluB, fun);
   }

   Stage::e_valE = valE;
   Stage::e_dstE = dste;
   
   setMInput(mreg, stat, icode, cnd, valE, valA, dste, dstm);
 
   return false;
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 * 
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
*/
void ExecuteStage::doClockHigh(PipeRegArray * pipeRegs)
{
   PipeReg * mreg  = pipeRegs->getMemoryReg();
   mreg->normal();
}

/* setMInput 
 * provides the input to potentially be stored in the E register
 * during doClockHigh.
 * 
 *   @param: mreg - Pointer to the memory pipeline register.
 *   @param: stat - Status code for the instruction.
 *   @param: icode - Instruction code.
 *   @param:  cnd - Condition code.
 *   @param: valE - Value computed by the ALU using operands from the execute stage.
 *   @param: valA - Value read from memory or register file.
 *   @param: dstE - Destination register for the result of the ALU operation.
 *   @param: dstM - Destination register for memory operations.
 */
void ExecuteStage::setMInput(PipeReg * mreg, uint64_t stat, uint64_t icode, uint64_t cnd, 
                     uint64_t valE, uint64_t valA,
                     uint64_t dstE, uint64_t dstM)
{
    mreg->set(M_STAT, stat);
    mreg->set(M_ICODE, icode);
    mreg->set(M_CND, cnd);
    mreg->set(M_VALE, valE);
    mreg->set(M_VALA, valA);
    mreg->set(M_DSTE, dstE);
    mreg->set(M_DSTM, dstM);
}

/*
 * aluA - Computes the value of operand A for the ALU.
*/
uint64_t ExecuteStage::aluA(PipeReg * ereg)
{
   if (ereg->get(E_ICODE) == Instruction::IRRMOVQ || ereg->get(E_ICODE) == Instruction::IOPQ)
   {
      return ereg->get(E_VALA);
   }
   if (ereg->get(E_ICODE) == Instruction::IIRMOVQ || ereg->get(E_ICODE) == Instruction::IRMMOVQ 
      || ereg->get(E_ICODE) == Instruction::IMRMOVQ)
   {
      return ereg->get(E_VALC);
   }
   if (ereg->get(E_ICODE) == Instruction::ICALL || ereg->get(E_ICODE) == Instruction::IPUSHQ)
   {
      return -8;
   }
   if (ereg->get(E_ICODE) == Instruction::IRET || ereg->get(E_ICODE) == Instruction::IPOPQ)
   {
      return 8;
   }
   return 0;
}

/*
 * aluB - Computes the value of operand B for the ALU.
*/
uint64_t ExecuteStage::aluB(PipeReg * ereg)
{
   if (ereg->get(E_ICODE) == Instruction::IRMMOVQ || ereg->get(E_ICODE) == Instruction::IMRMOVQ || ereg->get(E_ICODE) == Instruction::IOPQ
      || ereg->get(E_ICODE) == Instruction::ICALL || ereg->get(E_ICODE) == Instruction::IPUSHQ || ereg->get(E_ICODE) == Instruction::IRET
      || ereg->get(E_ICODE) == Instruction::IPOPQ)
      {
         return ereg->get(E_VALB);
      }
   if (ereg->get(E_ICODE) == Instruction::IRRMOVQ || ereg->get(E_ICODE) == Instruction::IIRMOVQ)
   {
      return 0;
   }

   return 0;
}
/*
 * aluFun - Determines the ALU operation code.
*/
uint64_t ExecuteStage::aluFun(PipeReg * ereg)
{
   if (ereg->get(E_ICODE) == Instruction::IOPQ)
   {
      return ereg->get(E_IFUN);
   }

   return Instruction::ADDQ;
}

/*
 * set_cc - Checks if condition codes need to be set.
*/
bool ExecuteStage::set_cc(PipeReg * ereg)
{
   if (ereg->get(E_ICODE) == Instruction::IOPQ)
   {
      return true;
   }
   return false;
}
/*
 * set_dstE - Sets the destination register for the result of the ALU operation.
*/
uint64_t ExecuteStage::set_dstE(PipeReg * ereg, uint64_t cnd)
{
   if((ereg->get(E_ICODE) == Instruction::IRRMOVQ) && !cnd)
   {
      return RegisterFile::RNONE;
   } 
   return ereg->get(E_DSTE);
}
/*
 * getALU - Performs the ALU operation based on the ALU function code.
*/
uint64_t ExecuteStage::getALU(uint64_t aluA, uint64_t aluB, uint64_t alufun)
{
   if(alufun == Instruction::ADDQ)
   {
      return aluA + aluB;
   }
   if(alufun == Instruction::SUBQ)
   {
      return aluB - aluA;
   }

   if(alufun == Instruction::ANDQ)
   {
      return aluA & aluB;
   }
   else
   {
      return aluA ^ aluB;
   }

}
/*
 * CC - Sets the condition codes based on the result of the ALU operation.
*/
void ExecuteStage::CC(uint64_t valE, uint64_t aluA, uint64_t aluB, uint64_t aluFun)
{
   bool error;

   cc->setConditionCode(Tools::sign(valE), ConditionCodes::SF, error);

   if(aluFun == Instruction::ADDQ)
   {
      cc->setConditionCode(Tools::addOverflow(aluA,aluB), ConditionCodes::OF, error);
   }
   else if(aluFun == Instruction::SUBQ)
   {
      cc->setConditionCode(Tools::subOverflow(aluA,aluB), ConditionCodes::OF, error);
   }
   cc->setConditionCode(!valE, ConditionCodes::ZF, error); 
}

bool ExecuteStage::CondFun(uint64_t icode, uint64_t ifun)
{
   bool error = false;
   bool sf = cc->getConditionCode(ConditionCodes::SF, error);
   bool of = cc->getConditionCode(ConditionCodes::OF, error);
   bool zf = cc->getConditionCode(ConditionCodes::ZF, error);
   
   if (icode ==  Instruction::IJXX || icode == Instruction::ICMOVXX) 
   {
      if (ifun == Instruction::UNCOND) 
      {
         return 1;
      }
      if (ifun == Instruction::LESSEQ) 
      {
         return (sf ^ of) | zf;
      }
      if (ifun == Instruction::LESS) 
      {
         return (sf ^ of);
      }
      if (ifun == Instruction::EQUAL) 
      {
         return zf;
      }
      if (ifun == Instruction::NOTEQUAL) 
      {
         return !zf;
      }
      if (ifun == Instruction::GREATER) 
      {
         return !(sf ^ of) & !zf;
      }
      if (ifun == Instruction::GREATEREQ) 
      {
         return !(sf ^ of);
      }
   }
   return 0;
}

