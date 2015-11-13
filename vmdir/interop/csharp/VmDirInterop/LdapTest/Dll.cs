using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace LdapTest
{
    public class Dll
    {
        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool SetDllDirectory(string lpPathName);
    }
}
