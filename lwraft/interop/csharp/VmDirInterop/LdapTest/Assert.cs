using System;
using System.Diagnostics;

namespace LdapTest
{
    public class Assert
    {
        public static void Fail()
        {
            System.Console.WriteLine("Test {0} failed!", new StackTrace().GetFrame(2).GetMethod().Name);
            System.Console.WriteLine("Stack: ");
            System.Console.WriteLine("{0}", Environment.StackTrace);

            Environment.Exit(1);
        }

        public static void IsNull(object o)
        {
            if (o != null)
            {
                Fail();
            }
        }

        public static void IsNotNull(object o)
        {
            if (o == null)
            {
                Fail();
            }
        }

        public static void IsTrue(bool condition)
        {
            if (!condition)
            {
                Fail();
            }
        }

        public static void IsFalse(bool condition)
        {
            if (condition)
            {
                Fail();
            }
        }
    }
}
