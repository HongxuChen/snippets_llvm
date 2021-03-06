#include "llvm/IR/Function.h"

#include "llvm/Pass.h"
#include "llvm/Support/raw_os_ostream.h"

#include "llvm/IR/CFG.h"

#include <set>

using namespace llvm;

namespace llvm {

class DumpCFGPath : public FunctionPass {
 public:
  static char ID;
  typedef std::set<BasicBlock *> BBVisitInfo;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
  }

  DumpCFGPath() : FunctionPass(ID) {}

  bool hasPath(BasicBlock *bb1, BasicBlock *bb2, BBVisitInfo visited) {
    visited.insert(bb1);
    bool reachable = false;
    if (bb1 == bb2) {
      return true;
    }
    for (succ_iterator pi = succ_begin(bb1); pi != succ_end(bb1); pi++) {
      if (visited.find(*pi) == visited.end()) {
        reachable = hasPath(*pi, bb2, visited);
        if (reachable) {
          return true;
        }
      }
    }
    return reachable;
  }

  void traversePath(Function &F) {
    errs() << "Func: " << F.getName() << "\n";
    BBVisitInfo visited;
    for (auto &B1 : F) {
      errs() << B1.getName() << ": ";
      for (auto &B2 : F) {
        if (&B1 != &B2 && hasPath(&B1, &B2, visited)) {
          errs() << B2.getName() << " ";
        }
      }
      errs() << "\n";
      visited.clear();
    }
  }

  bool runOnFunction(Function &F) override {
    traversePath(F);
    return false;
  }

};  /// class llvm::PrintCFGPath

char DumpCFGPath::ID = 0;
static RegisterPass<DumpCFGPath> X("cfg-path", "DumpCFGPath", true, true);

}  // namespace llvm
