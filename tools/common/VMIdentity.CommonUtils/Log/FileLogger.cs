/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

using System;
using System.Collections.Generic;
using System.IO;

namespace VMIdentity.CommonUtils.Log
{
    public class FileLogger : ILogger
    {
        private string _filePath;
        private bool _canLog;
        private static object _mutex;
        public FileLogger(string filePath)
        {
            _filePath = filePath;
            _canLog = true;
            _mutex = new object();
            Init();
        }
        public void Init()
        {
            var fileInfo = new FileInfo(_filePath);

            if (!Directory.Exists(fileInfo.DirectoryName))
            {
                Directory.CreateDirectory(fileInfo.DirectoryName);
            }

            if (!File.Exists(_filePath))
            {
                lock (_mutex)
                {
                    File.WriteAllText(_filePath,string.Empty);
                }
            }
            Start();
        }

        public void Start()
        {
            _canLog = true;
            var message = "============= STARTED LOGGING =============";
            LogMessage(message, LogLevel.Info);
        }

        private void LogMessage(string message, LogLevel level)
        {
            var log = GetFormattedLog(message, level, null);
            WriteLog(log);
        }

        public void Stop()
        {
            _canLog = false;
            var message = "============= STOPPED LOGGING =============";
            LogMessage(message, LogLevel.Info);
        }

        public void Log(string message, LogLevel level)
        {
            if (_canLog)
            {
                var log = GetFormattedLog(message, level, null);
                WriteLog(log);
            }
        }

        private void WriteLog(string log)
        {
            var fileInfo = new FileInfo(_filePath);
            if (fileInfo.Length > 1024 * 1024 * 10)
            {
                var oldFileName = fileInfo.Name.Replace(fileInfo.Extension, string.Empty);
                var newFilePath = string.Format(@"{0}\{1}_{2}.log", fileInfo.DirectoryName, oldFileName, DateTime.Now.ToString("ddMMyyyhhmmss"));
                File.Copy(_filePath, newFilePath);
                lock (_mutex)
                {
                    File.WriteAllText(_filePath, string.Empty);
                }
            }
            lock (_mutex)
            {
                File.AppendAllLines(_filePath, new List<string> { log });
            }
        }

        private string GetFormattedLog(string message, LogLevel level, string stackTrace)
        {
            return string.Format("TIMESTAMP: {0}\t\tLEVEL: {1}\t\tDETAILS: {2}\t\t STACKTRACE: {3}\n"
                , DateTime.Now.ToString("dd-MMM-yyyy hh:mm:ss")
                , level
                , message
                , stackTrace == null ? "N/A" : stackTrace);
        }

        private string GetFormattedLog(string message, LogLevel level, string stackTrace, string custom)
        {
            return string.Format("TIMESTAMP: {0}\t\tLEVEL: {1}\t\tDETAILS: {2}\t\t STACKTRACE: {3} {4}\n"
                , DateTime.Now.ToString("dd-MMM-yyyy hh:mm:ss")
                , level
                , message
                , stackTrace == null ? "N/A" : stackTrace,
                custom);
        }

        public void LogException(Exception exception)
        {
            if (_canLog)
            {
                var log = GetFormattedLog(exception.Message, LogLevel.Error, exception.StackTrace);
                File.AppendAllLines(_filePath, new List<string> { log });
            }
        }

        public void LogException(Exception exception, string custom)
        {
            if (_canLog)
            {
                var log = GetFormattedLog(exception.Message, LogLevel.Error, exception.StackTrace, custom);
                File.AppendAllLines(_filePath, new List<string> { log });
            }
        }
    }
}
