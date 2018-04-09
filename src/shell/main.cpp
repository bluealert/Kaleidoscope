#include <gflags/gflags.h>
#include "shell/shell.h"

DEFINE_bool(verbose, true, "dump LLVM IR");

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  kaso::shell::Shell myShell;
  myShell.repl(FLAGS_verbose);

  gflags::ShutDownCommandLineFlags();
  return 0;
}
