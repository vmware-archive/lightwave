#include <stdio.h>

#include "CuTest.h"

int  vmdns_syslog_level = 0;
int  vmdns_syslog = 0;
int  vmdns_debug = 0;

CuSuite* CuGetZoneSuite();

void RunAllTests(void)
{
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();

    CuSuite* zoneSuite = CuGetZoneSuite();
    CuSuiteAddSuite(suite, zoneSuite);
    CuSuiteDelete(zoneSuite);

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);

    CuSuiteDelete(suite);
    CuStringDelete(output);
}

int main(void)
{
    RunAllTests();
        return 0;
}
