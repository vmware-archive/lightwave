using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using VmDirInterop.SuperLogging;
using VmDirInterop.SuperLogging.Constants;
using VmDirInterop.SuperLogging.Interfaces;

namespace ConsoleApplication1
{
    class SuperLoggingConnectionTest
    {
        static void Main(string[] args)
        {
            ISuperLoggingConnection conn = new SuperLoggingConnection();
            conn.openA(
                "localhost",
                "vsphere.local",
                "administrator",
                "Admin!23");

            testEnableDisable(conn);
            testGetEntries(conn, false);
            testGetTable(conn);
            testClear(conn);
            testSetGetCapacity(conn);

            conn.close();
            return;
        }

        private static void testEnableDisable(ISuperLoggingConnection conn)
        {
            conn.disable();
            Console.WriteLine("Disabled: " + (!conn.isEnabled() ? "succeeded" : "failed"));
            conn.enable();
            Console.WriteLine("Enabled: " + (conn.isEnabled() ? "succeeded" : "failed"));
            return;
        }

        private static void testGetEntries(ISuperLoggingConnection conn, bool printEntries)
        {
            int counter = 0;
            ISuperLoggingCookie cookie = new SuperLoggingCookie();
            uint pageSize = 100;
            ISuperLogEntryList list;

            do
            {
                list = conn.getPagedEntries(cookie, pageSize);
                counter += list.getCount();
            } while (list.getCount() == pageSize);

            list = conn.getAllEntries();

            if (printEntries)
            {
                foreach (ISuperLogEntry e in list)
                {
                    Console.WriteLine(e);
                }
            }

            Console.WriteLine("GetPagedEntries: " + counter + ", GetAllEntries: " + list.getCount());
            return;
        }

        private static void testGetTable(ISuperLoggingConnection conn)
        {
            conn.enable();
            ISuperLogEntryList list = conn.getAllEntries();
            SuperLogTableColumnSet colSet = new SuperLogTableColumnSet();
            colSet[SuperLogTableColumn.LOGIN_DN] = true;
            colSet[SuperLogTableColumn.OPERATION] = true;
            colSet[SuperLogTableColumn.STRING] = true;
            colSet[SuperLogTableColumn.ERROR_CODE] = true;
            colSet[SuperLogTableColumn.AVG_TIME] = true;
            ISuperLogTable table = conn.aggregate(list, colSet);

            foreach (ISuperLogTableRow r in table)
            {
                Console.WriteLine(r.ToString());
            }
            return;
        }

        private static void testClear(ISuperLoggingConnection conn)
        {
            conn.clear();
            ISuperLogEntryList list = conn.getAllEntries();
            Console.WriteLine("GetAllEntries after clear: " + list.getCount());

            conn.clear();
            ISuperLoggingCookie cookie = new SuperLoggingCookie();
            uint pageSize = 100;
            list = conn.getPagedEntries(cookie, pageSize);
            Console.WriteLine("GetPagedEntries after clear: " + list.getCount());

            return;
        }

        private static void testSetGetCapacity(ISuperLoggingConnection conn)
        {
            uint expectedCapacity = 20000;
            conn.setCapacity(expectedCapacity);
            uint actualCapacity = conn.getCapacity();
            Console.WriteLine("Expected capacity: " + expectedCapacity + ", Actual capacity: " + actualCapacity);
            return;
        }
    }
}
