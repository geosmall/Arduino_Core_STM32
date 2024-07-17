#include <cmath>
#include <cstdio>
#include <cstring>
#include "daisy_seed.h"
#include "common.h"
#include "src/example-selftest/example-selftest.h"

using namespace daisy;

static DaisySeed hw;

int main(void)
{
    /* try printing out something before the hardware init */
    hw.PrintLine("This should too appear in the log");

    hw.Configure();
    hw.Init();
    hw.StartLog(true); /* true == wait for PC: will block until a terminal is connected */

    /** check that floating point printf is supported
     * linker flags modified in the Makefile to enable this
     */
    hw.PrintLine("Verify CRT floating point format: %.3f", 123.0f);
}

extern "C" void hw_PrintLine(const char* format, ...)
{
    va_list va;                 // variable argument list
    va_start(va, format);       // initialize va with the variable argument list
	hw.PrintLine(format, va);   // call the PrintLine function with the variable argument list
    va_end(va);                 // clean up the variable argument list
}