#define DEBUG_TYPE "TestUser"
#include "Version.hh"

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/TypeBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IRReader/IRReader.h"


#if LLVM_VERSION_CODE >= LLVM_VERSION(3, 5)
#include "llvm/IR/Verifier.h"
#else
#include "llvm/Analysis/Verifier.h"
#endif

#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"

#include "FnUtils.hh"
#include "Logging.hh"

using namespace llvm;

struct TestUser final : public ModulePass {
  static char ID;
  TestUser() : ModulePass(ID) {}

  void dumpUsesInfo(Value &V) {
    if (V.getNumUses() > 0) {
      WITH_COLOR(raw_ostream::YELLOW, errs() << "uses of: ";);
      logging::prettyPrint(&V);
      errs() << "\n";
      for (auto b = V.use_begin(), e = V.use_end(); b != e; ++b) {
        logging::prettyPrint(*b, 0, 0);
      }
    }
  }

  void dumpAllUsesInFunc(Function &F) {
    if (F.getNumUses() > 0) {
      dumpUsesInfo(F);
    }
    for (auto &B : F) {
      dumpUsesInfo(B);
      for (auto &I : B) {
        dumpUsesInfo(I);
      }
    }
  }

  bool runOnFunction(Function &F) { return false; }

  bool testUse(Module &M) {
    for (auto &F : M) {
      if (F.isDeclaration()) continue;
      runOnFunction(F);
    }
    return false;
  }

  void specialUse(Module &M) {
    GlobalVariable *GV = nullptr;
    for (auto gvb = M.global_begin(), gve = M.global_end(); gvb != gve; ++gvb) {
      if ((*gvb).getName() == "my_global_v") {
        GV = &*(gvb);
      }
    }
    Function &mainFn = *M.getFunction("main");
    for (auto &B : mainFn) {
      errs() << B.getName() << "\tuses " << GV->getName() << "? "
             << GV->isUsedInBasicBlock(&B) << "\n";
    }
  }

  bool runOnModule(Module &M) override {
    specialUse(M);
    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }
};

char TestUser::ID = 0;
static RegisterPass<TestUser> X("TestUser", "TestUser pass", true, true);
