#define setup confirmTokenRuntimeTestSetup
#define loop confirmTokenRuntimeTestLoop
#include "../test/test_confirm_token_runtime/test_main.cpp"

void (*volatile confirmTokenRuntimeTestLinkProbe)() = confirmTokenRuntimeTestSetup;
