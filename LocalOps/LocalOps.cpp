#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
using namespace llvm;

namespace {
  struct LocalOpsPass : public FunctionPass {
    static char ID;
    LocalOpsPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) { 
      for (auto &B : F) {
        std::vector<Instruction*> insRemoval;
        for (auto &I : B) {
          if (auto *op = dyn_cast<BinaryOperator>(&I)) {
            // Insert at the point where the instruction `op` appears.
            IRBuilder<> builder(op);

            Value *lhs = op->getOperand(0);
            Value *rhs = op->getOperand(1);
            Value *newVal = nullptr;
            Constant* lcv = dyn_cast<Constant>(lhs);
            Constant* rcv = dyn_cast<Constant>(rhs);
            ConstantFP* lcfp = dyn_cast<ConstantFP>(lhs);
            ConstantFP* rcfp = dyn_cast<ConstantFP>(rhs);
            ConstantInt* lcint = dyn_cast<ConstantInt>(lhs);
            ConstantInt* rcint = dyn_cast<ConstantInt>(rhs);
            switch (op->getOpcode()) {
              case Instruction::Add:
              case Instruction::FAdd:
                if (lcv && lcv->isZeroValue()) {
                  newVal = rhs;
                  outs() << "Algebraic identity" << " 0 + x" << '\n';
                } else if (rcv && rcv->isZeroValue()) {
                  newVal = lhs;
                  outs() << "Algebraic identity" << " x + 0" << '\n';
                } else if (lcfp && rcfp) {
                    // Both are Float constants
                    newVal = llvm::ConstantFP::get(lcfp->getType(), lcfp->getValueAPF().convertToDouble() + rcfp->getValueAPF().convertToDouble());
                    outs() << "Constant Folding" << lcfp->getValueAPF().convertToDouble() << " + " << rcfp->getValueAPF().convertToDouble() << '\n';
                } else if (lcint && rcint) {
                    // Both are int constants
                    newVal = llvm::ConstantInt::get(lcint->getType(), lcint->getValue().getLimitedValue() + rcint->getValue().getLimitedValue());
                    outs() << "Constant Folding" << lcint->getValue().getLimitedValue() << " + " << rcint->getValue().getLimitedValue() << '\n';
                }
                break;
              case Instruction::Mul:
              case Instruction::FMul:
                if (lcfp && rcfp) {
                    // Both are Float constants
                    newVal = llvm::ConstantFP::get(lcfp->getType(), lcfp->getValueAPF().convertToDouble() * rcfp->getValueAPF().convertToDouble());
                    outs() << "Constant Folding" << lcfp->getValueAPF().convertToDouble() << " * " << rcfp->getValueAPF().convertToDouble() << '\n';
                } else if (lcint && rcint) {
                    // Both are int constants
                    newVal = llvm::ConstantInt::get(lcint->getType(), lcint->getValue().getLimitedValue() * rcint->getValue().getLimitedValue());
                    outs() << "Constant Folding" << lcint->getValue().getLimitedValue() << " * " << rcint->getValue().getLimitedValue() << '\n';
                } else if ((lcv && lcv->isZeroValue()) || (rcv && rcv->isZeroValue())) {
                  // One is zero
                  newVal = builder.getInt32(0);
                  outs() << "Constant Folding" << " mul 0" << '\n';
                } else if (lcv && lcv->isOneValue()) {
                  // lhs is one
                  newVal = rhs;
                  outs() << "Algebraic identity" << " 1 * x" << '\n';
                } else if (rcv && rcv->isOneValue()) {
                  // rhs is one
                  newVal = lhs;
                  outs() << "Algebraic identity" << " x * 1" << '\n';
                } else if (lcint) {
                  auto intVal = lcint->getValue().getLimitedValue();
                  if ((intVal & (intVal - 1) ) == 0 && (intVal != 0)) {
                    newVal = builder.CreateShl(rhs, log2(intVal));
                    outs() << "Strength reduction" << " x << " << log2(intVal) << '\n';
                  }
                } else if (rcint) {
                  auto intVal = rcint->getValue().getLimitedValue();
                  if ((intVal & (intVal - 1) ) == 0 && (intVal != 0)) {
                    newVal = builder.CreateShl(lhs, log2(intVal));
                    outs() << "Strength reduction" << " x << " << log2(intVal) << '\n';
                  }
                }
                break;
              case Instruction::Sub:
              case Instruction::FSub:
                if (lhs == rhs) {
                  if (lhs->getType()->isFloatingPointTy()) {
                    newVal = ConstantFP::get(lhs->getType(), 0);
                    outs() << "Subtract same number \n";
                  } else if (lhs->getType()->isIntegerTy()) {
                    newVal = ConstantInt::get(lhs->getType(), 0);
                    outs() << "Subtract same number \n";
                  }
                } else if (lcfp && rcfp) {
                    // Both are Float constants
                    newVal = llvm::ConstantFP::get(lcfp->getType(), lcfp->getValueAPF().convertToDouble() - rcfp->getValueAPF().convertToDouble());
                    outs() << "Constant Folding" << lcfp->getValueAPF().convertToDouble() << " - " << rcfp->getValueAPF().convertToDouble() << '\n';
                } else if (lcint && rcint) {
                    // Both are int constants
                    newVal = llvm::ConstantInt::get(lcint->getType(), lcint->getValue().getLimitedValue() - rcint->getValue().getLimitedValue());
                    outs() << "Constant Folding" << lcint->getValue().getLimitedValue() << " - " << rcint->getValue().getLimitedValue() << '\n';
                } else if (rcv && rcv->isZeroValue()) {
                  newVal = lhs;
                  outs() << "Constant Folding" << " x - 0" << '\n';
                }
                break;
              case Instruction::UDiv:
              case Instruction::SDiv:
              case Instruction::FDiv:
                if (lhs == rhs) {
                  if (lhs->getType()->isFloatingPointTy()) {
                    newVal = ConstantFP::get(lhs->getType(), 1);
                    outs() << "Divide same number \n";
                  } else if (lhs->getType()->isIntegerTy()) {
                    newVal = ConstantInt::get(lhs->getType(), 1);
                    outs() << "Divide same number \n";
                  }
                } else if (lcfp && rcfp) {
                  // Both are Float constants
                  newVal = llvm::ConstantFP::get(lcfp->getType(), lcfp->getValueAPF().convertToDouble() / rcfp->getValueAPF().convertToDouble());
                  outs() << "Constant Folding" << lcfp->getValueAPF().convertToDouble() << " / " << rcfp->getValueAPF().convertToDouble() << '\n';
                } else if (lcint && rcint) {
                  // Both are int constants
                  newVal = llvm::ConstantInt::get(lcint->getType(), lcint->getValue().getLimitedValue() / rcint->getValue().getLimitedValue());
                  outs() << "Constant Folding" << lcint->getValue().getLimitedValue() << " / " << rcint->getValue().getLimitedValue() << '\n';
                } else if (rcint) {
                  auto intVal = rcint->getValue().getLimitedValue();
                  if ((intVal & (intVal - 1) ) == 0 && (intVal != 0)) {
                    newVal = builder.CreateLShr(lhs, log2(intVal));
                    outs() << "Strength reduction" << " x >> " << log2(intVal) << '\n';
                  }
                }
                break;
            }
            if (newVal) {
              outs() << "replacing " << *op << " with " << *newVal << '\n';
              outs() << B << '\n';
              I.replaceAllUsesWith(newVal);
              outs() << "after replace\n";
              outs() << B << '\n';
              insRemoval.push_back(&I);
            }
          }
        }
        for (auto &I: insRemoval) {
          outs() << "remove " << *I << "\n";
          I->eraseFromParent();
        }
      }

      // We modified the code.
      return true;
    }
  };
}

char LocalOpsPass::ID = 0;
static RegisterPass<LocalOpsPass> X("local-ops", "Optimize local", false, false);
