/*
The license is based on the zlib/libpng license. For more details see
http://www.opensource.org/licenses/zlib-license.html. The intent of the
license is to: 

- keep the license as simple as possible
- encourage the use of CuTest in both free and commercial applications
  and libraries
- keep the source code together 
- give credit to the CuTest contributors for their work

If you ship CuTest in source form with your source distribution, the
following license document must be included with it in unaltered form.
If you find CuTest useful we would like to hear about it. 

LICENSE

Copyright (c) 2003 Asim Jalis

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software in
a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not
be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/
#include <stdio.h>

#include "CuTest.h"

int  vmdns_syslog_level = 0;
int  vmdns_syslog = 0;
int  vmdns_debug = 0;

//CuSuite* CuGetZoneSuite();
CuSuite* CuGetUtilSuite();
CuSuite* CuGetLruSuite();
CuSuite* CuGetRecordListSuite();

void RunAllTests(void)
{
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();

    //CuSuite* zoneSuite = CuGetZoneSuite();
    //CuSuiteAddSuite(suite, zoneSuite);
    //CuSuiteDelete(zoneSuite);

    CuSuite* zoneSuite = CuGetRecordListSuite();
    CuSuiteAddSuite(suite, zoneSuite);
    CuSuiteDelete(zoneSuite);

    CuSuite* utilSuite = CuGetUtilSuite();
    CuSuiteAddSuite(suite, utilSuite);
    CuSuiteDelete(utilSuite);

    CuSuite* lruSuite = CuGetLruSuite();
    CuSuiteAddSuite(suite, lruSuite);
    CuSuiteDelete(lruSuite);

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
