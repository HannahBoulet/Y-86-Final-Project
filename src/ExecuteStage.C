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
#include "W.h"
#include "Status.h"

/**
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
   PipeReg * wreg = pipeRegs->getWritebackReg();
  
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

   if(set_cc(ereg, wreg))
   {
      CC(valE, V_aluA, V_aluB, fun);
   }
   Stage::e_valE = valE;
   Stage::e_dstE = dste;
   Stage::e_Cnd = cnd;

   setMInput(mreg, stat, icode, cnd, valE, valA, dste, dstm);
   M_bubble = calculateControlSignals(wreg);

   return false;
}

/**
 * doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 * 
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
*/
void ExecuteStage::doClockHigh(PipeRegArray * pipeRegs)
{
   PipeReg * mreg  = pipeRegs->getMemoryReg();
   if(!M_bubble)
   {
      mreg->normal();
   }
   else
   {
      ((M *)mreg)->bubble();
   }
}

/**
 * setMInput 
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

/**
 * aluA
 * Computes the value of operand A for the ALU based on the instruction code.
 * @param: ereg Pointer to the pipeline register containing relevant information.
 * @return The computed value of operand A.
 */
uint64_t ExecuteStage::aluA(PipeReg * ereg)
{
   uint64_t icode = ereg->get(E_ICODE);
   if (icode == Instruction::IRRMOVQ || icode == Instruction::IOPQ)
   {
      return ereg->get(E_VALA);
   }
   if (icode == Instruction::IIRMOVQ || icode == Instruction::IRMMOVQ 
      || icode == Instruction::IMRMOVQ)
   {
      return ereg->get(E_VALC);
   }
   if (icode == Instruction::ICALL || icode == Instruction::IPUSHQ)
   {
      return -8;
   }
   if (icode == Instruction::IRET || icode == Instruction::IPOPQ)
   {
      return 8;
   }
   return 0;
}

/**
 * aluB
 * Computes the value of operand B for the ALU based on the instruction code.
 * @param: ereg Pointer to the pipeline register containing relevant information.
 * @return The computed value of operand B.
 */

uint64_t ExecuteStage::aluB(PipeReg * ereg)
{
   uint64_t icode = ereg->get(E_ICODE);
   if (icode == Instruction::IRMMOVQ || icode == Instruction::IMRMOVQ || icode == Instruction::IOPQ
      || icode == Instruction::ICALL || icode == Instruction::IPUSHQ || icode == Instruction::IRET
      || icode == Instruction::IPOPQ)
   {
         return ereg->get(E_VALB);
   }
   if (icode == Instruction::IRRMOVQ || icode == Instruction::IIRMOVQ)
   {
      return 0;
   }
   return 0;
}
/**
 * aluFun
 * Determines the ALU operation code based on the instruction code.
 * @param: ereg Pointer to the pipeline register containing relevant information.
 * @return The ALU operation code.
 */

uint64_t ExecuteStage::aluFun(PipeReg * ereg)
{
   uint64_t icode = ereg->get(E_ICODE);
   if (icode == Instruction::IOPQ)
   {
      return ereg->get(E_IFUN);
   }
   return Instruction::ADDQ;
}

/**
 * set_cc
 * Checks if condition codes need to be set based on the instruction code.
 * @param: ereg Pointer to the pipeline register containing relevant information.
 * @return True if condition codes need to be set, otherwise false.
 */

bool ExecuteStage::set_cc(PipeReg * ereg, PipeReg * wreg)
{
   uint64_t icode = ereg->get(E_ICODE);
   uint64_t w_stat = wreg->get(W_STAT);
   if ((icode == Instruction::IOPQ) && (Stage::m_stat != Status::SADR && Stage::m_stat != Status::SINS 
      && Stage::m_stat != Status::SHLT) && (w_stat != Status::SADR && w_stat != Status::SINS 
      && w_stat != Status::SHLT))
   {
      return true;
   }
   return false;
}

/**
 * set_dstE
 * Sets the destination register for the result of the ALU operation based on the instruction code and condition.
 * @param: ereg Pointer to the pipeline register containing relevant information.
 * @param: cnd Condition indicating whether to set the destination register.
 * @return The destination register for the result of the ALU operation.
 */

uint64_t ExecuteStage::set_dstE(PipeReg * ereg, uint64_t cnd)
{
   uint64_t icode = ereg->get(E_ICODE);
   if((icode == Instruction::IRRMOVQ) && !cnd)
   {
      return RegisterFile::RNONE;
   } 
   return ereg->get(E_DSTE);
}

/**
 * getALU
 * Performs the Arithmetic Logic Unit (ALU) operation based on the specified function code.
 * @param: aluA The first operand for the ALU operation.
 * @param: aluB The second operand for the ALU operation.
 * @param: aluFun The ALU function code indicating the operation type.
 * @return The result of the ALU operation on the given operands.
 */
uint64_t ExecuteStage::getALU(uint64_t aluA, uint64_t aluB, uint64_t alufun)
{
   switch (alufun) {
        case Instruction::ADDQ:
            return aluA + aluB;
        case Instruction::SUBQ:
            return aluB - aluA;
        case Instruction::ANDQ:
            return aluA & aluB;
        case Instruction::XORQ:
            return aluA ^ aluB;
        default:
            return 0;
   }
}

/**
 * CC
 * Updates the condition codes based on the executed ALU operation and values.
 * @param: valE The value computed by the ALU.
 * @param: aluA The first ALU operand.
 * @param: aluB The second ALU operand.
 * @param: aluFun The ALU function code indicating the operation type.
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

/**
 * CondFun
 * Determines the condition for executing a specific stage based on instruction and function codes.
 * @param: icode The instruction code.
 * @param: ifun The function code.
 * @return Boolean indicating whether the stage should be executed based on the condition.
 */
bool ExecuteStage::CondFun(uint64_t icode, uint64_t ifun)
{
   bool error = false;
   bool sf = cc->getConditionCode(ConditionCodes::SF, error);
   bool of = cc->getConditionCode(ConditionCodes::OF, error);
   bool zf = cc->getConditionCode(ConditionCodes::ZF, error);
   
   bool result = false;

    switch (icode) {
        case Instruction::IJXX:
        case Instruction::ICMOVXX:
            switch (ifun) {
                case Instruction::UNCOND:
                    result = true;
                    break;
                case Instruction::LESSEQ:
                    result = (sf ^ of) | zf;
                    break;
                case Instruction::LESS:
                    result = (sf ^ of);
                    break;
                case Instruction::EQUAL:
                    result = zf;
                    break;
                case Instruction::NOTEQUAL:
                    result = !zf;
                    break;
                case Instruction::GREATER:
                    result = !(sf ^ of) & !zf;
                    break;
                case Instruction::GREATEREQ:
                    result = !(sf ^ of);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return result;
}

/**
* calculateControlSignals
* Calculates the control signals for the Execute stage.
*
 * @param: wreg Pointer to the Writeback Stage pipeline register.
 * @return: true if a bubble should be used in the Execute stage, false otherwise.
*/
bool ExecuteStage::calculateControlSignals(PipeReg * wreg)
{
   uint64_t w_stat = wreg->get(W_STAT);
   if ((Stage::m_stat == Status::SADR || Stage::m_stat == Status::SINS || Stage::m_stat == Status::SHLT) 
      || (w_stat == Status::SADR ||w_stat == Status::SINS || w_stat == Status::SHLT))
   {
            return true;
   }
   return false;
}