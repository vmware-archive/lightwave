/*
 *  Copyright (c) 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *  use this file except in compliance with the License.  You may obtain a copy
 *  of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, without
 *  warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */

//--------------------------------------------------------------------------------------
// Logging Api

/**
 * Logging Target Api class defintion.
 */
function LoggingTargetApi(conn) {
   ApiBase.call(this);
   this.__init__(conn, "loggingTarget");
}

LoggingTargetApi.prototype = new ApiBase();
LoggingTargetApi.prototype.constructor = LoggingTargetApi;

ClientSupportApiFactory.registerApi("loggingTarget", function(conn) {
   return new LoggingTargetApi(conn);
});

/**
 * Set the config arguments for the log target.
 * @param args.targetName - Target filename prefix.
 * @param args.logFileSize - Max log file size.
 * @param args.maxLogFiles - Max number of log files.
 * @param args.logTime - "true" or "false"
 * @param args.flushDelay - seconds (no-op - currently ignored)
 */
LoggingTargetApi.prototype.addSimpleApiCall("setConfig");

/**
 * Write a log line to the log target file.
 * @param args.line Line to log.
 * @return None - do not pass a callback, it will not be called,
 *                and will cause a leak!
 */
LoggingTargetApi.prototype.addSimpleApiCall("log");

/**
 * Roll the log to the next file.
 */
LoggingTargetApi.prototype.addSimpleApiCall("roll");

/**
 * Roll the log to the next file.
 * @return The callback(response) will be called with response.result will have the log path.
 */
LoggingTargetApi.prototype.addSimpleApiCall("getLogFilePath");
