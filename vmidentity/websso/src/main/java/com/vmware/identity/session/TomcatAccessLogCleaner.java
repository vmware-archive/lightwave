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

package com.vmware.identity.session;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FilenameFilter;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Timer;
import java.util.TimerTask;
import java.util.regex.Pattern;

import org.apache.commons.lang.SystemUtils;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

public class TomcatAccessLogCleaner {

  private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(TomcatAccessLogCleaner.class);
  private static final long TIMER_TASK_DELAY_IN_MILLIS = 3600000; // TODO : tune this param
  private static final String ACCESS_LOG_PATTERN = "localhost_access_log\\.yyyy-MM-dd\\.txt";
  private static final String DATE_FORMAT = "yyyy-MM-dd";
  private static final long MAX_PERMISSIBLE_SIZE_IN_BYTES = 524288000; // 500 MB
  private static final boolean IS_DAEMON = true;

  public TomcatAccessLogCleaner() {

    // Verify the appropriate operating system and assign logDirectory
    File cleanupDirectory = getLogDirectory();

    // Run the Timer which triggers log cleanup tasks at specific interval schedule.
    startLogCleaningProcess(cleanupDirectory);
  }

  /**
   * Perform clean up of the tomcat access log files
   */
  public void startLogCleaningProcess(File accessLogDirectory) {
    logger.info("Cleaning up tomcat access log files under : {}", accessLogDirectory.getAbsolutePath());
    Timer timer = new Timer(IS_DAEMON);
    AccessLogCleanerTask tomcatAccessLogCleaner = new AccessLogCleanerTask(accessLogDirectory);
    logger.info("Starting to schedule the log clean up tasks..");
    timer.scheduleAtFixedRate(tomcatAccessLogCleaner, 0, TIMER_TASK_DELAY_IN_MILLIS);
  }

  /**
   * Get the tomcat access log directory based on operating system.
   */
  private File getLogDirectory(){
    File logDirectory = null;
    if (SystemUtils.IS_OS_LINUX) {
      logDirectory = new File(File.separator + "var" + File.separator + "log" + File.separator + "vmware" + File.separator  + "sso");
    } else if (SystemUtils.IS_OS_WINDOWS) {
      String WIN_VMWARE_CIS_VMIDENTITY_PATH =
          "C:" + File.separator + "ProgramData" + File.separator + "VMware" + File.separator
          + "vCenterServer" + File.separator + "runtime" + File.separator + "VMWareSTSService"
          + File.separator + "logs";
      logDirectory = new File(WIN_VMWARE_CIS_VMIDENTITY_PATH);
    } else {
      logger.error("Failed to start tomcat access log cleaner for operatingSystem : {}",
          System.getProperty("os.name"));
    }
    return logDirectory;
  }

  /**
   * <p>
   * Utility that extends {@link java.util.TimerTask} which cleans up the tomcat access log files on
   * disk which exceeds max threshold size {@value TomcatAccessLogCleaner.MAX_PERMISSIBLE_SIZE_IN_BYTES}. 
   * The file deletion order is based on FIFO structure. (i.e the most out dated file is chosen as primary candidate.)
   * </p>
   * 
   * @author bboggaramrama
   */
  class AccessLogCleanerTask extends TimerTask {

    private Pattern accessLogFilePattern;
    private File accessLogDir;

    public AccessLogCleanerTask(File accessLogDir) {
      this(accessLogDir, ACCESS_LOG_PATTERN);
    }

    public AccessLogCleanerTask(File accessLogDir, String accessLogPattern) {
      this.accessLogDir = accessLogDir;
      this.accessLogFilePattern = Pattern.compile(accessLogPattern.replace(DATE_FORMAT, "(.+?)"));
    }

    @Override
    public void run() {

      // Get all the files matching the given acccess log pattern
      File[] accessLogFiles = retrieveLogFilesMatchingPattern(accessLogDir, accessLogFilePattern);

      if(accessLogFiles != null && accessLogFiles.length > 0) {
         logger.info("Total number of tomcat access log files retrieved : {}" , accessLogFiles.length);
          // Compute total size of these files
          long currentTotalSize = 0;
          for (File file : accessLogFiles) {
            logger.info("Retrieved log file : {}" , file.getAbsolutePath());
            currentTotalSize += file.length();
          }

          // Compare currentsize of these file with total permissible size
          if (currentTotalSize > MAX_PERMISSIBLE_SIZE_IN_BYTES) {
            logger.info("Trying to delete the files ");
            // The deletion principle used here is FIFO. (i.e the oldest file is a candidate for deletion )
            Arrays.sort(accessLogFiles, new Comparator<File>() {
              public int compare(File file1, File file2) {
                return Long.valueOf(file1.lastModified()).compareTo(file2.lastModified());
              }
            });

            /**
             * Delete the files until the totalSize on disk is less than {@value #MAX_PERMISSIBLE_SIZE_IN_BYTES}
             */
            for (File file : accessLogFiles) {
              try {
                long fileSize = file.length();
                file.delete();
                currentTotalSize -= fileSize;
                if (currentTotalSize < MAX_PERMISSIBLE_SIZE_IN_BYTES)
                  break;
              } catch (Exception ex) {
                logger.error("Unable to delete file :{}", file.getAbsolutePath());
              }
            }
          } else {
            // The size of files haven't reached its max permissible size. Wait till it consumes..
            logger.info(
                "Access logs have not yet reached its threshold ({} mb) to clean. Nothing to clean",
                MAX_PERMISSIBLE_SIZE_IN_BYTES / (1024 * 1024));
          }
      } else {
          String warnMessage = String.format("Failed to find tomcat access log file under : '%s'. Nothing to clean", accessLogDir.getAbsolutePath());
          logger.warn(warnMessage);
      }
    }

    /**
     * Retrieve all the files matching the given file pattern
     *
     * @param directory Absolute path of directory
     * @param filePattern Pattern to search for files in given directory
     * @return All files matching the given pattern in a directory
     */
    private File[] retrieveLogFilesMatchingPattern(File directory, final Pattern filePattern) {
      File files[] = null;
      try{
        files = directory.listFiles(new FilenameFilter() {
          public boolean accept(File dir, String file) {
            return filePattern.matcher(file).matches();
          }
        });
      }catch (Exception ex){
        String errorMessage = String.format("Failed to retrieve files from directory : %s matching pattern : %s",directory.getAbsolutePath(),filePattern.toString());
        throw new RuntimeException(errorMessage,ex);
      }
      return files;
    }
  }

}
