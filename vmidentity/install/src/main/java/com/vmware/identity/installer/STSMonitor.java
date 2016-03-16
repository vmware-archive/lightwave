package com.vmware.identity.installer;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Arrays;

public class STSMonitor {
    
    public static void main(String args[]) 
    {
        System.out.println("Running the monitoring process to monitor Lotus components");
        
        try {
            /**
             * MONITOR STS PROCESS - TOP COMMAND
             */
            System.out.println("Starting monitoring : STS process");
            String[] cmd = {
                    "/bin/sh",
                    "-c",
                    "ps -ef | grep stsd | grep -v grep | awk '{print $2}'"
                    //"ps aux | grep idmd | awk '{print $2}'"
                    };
            System.out.println("Executing command :" + Arrays.toString(cmd));
            Process idmProcess = Runtime.getRuntime().exec(cmd);
            BufferedReader stdInput = new BufferedReader(new InputStreamReader(idmProcess.getInputStream()));
            BufferedReader stdError = new BufferedReader(new InputStreamReader(idmProcess.getErrorStream()));
            String s;
            if(stdError.readLine() != null)
            printErrors(stdError);

            
            // GET THE CHILD PROCESS
            int biggestProcessId = 0;
            while ((s = stdInput.readLine()) != null) {
                int processId = Integer.parseInt(s);
                if (biggestProcessId < processId) {
                    biggestProcessId = processId;
                }
            }
            System.out.println("STS PROCESS : " + biggestProcessId);
            String processFile = createFile(String.valueOf(biggestProcessId));
            String[] topCmd = {
                    "/usr/bin/sh",
                    "-c",
                    "nohup /usr/bin/top -p " + biggestProcessId + " -b > " + processFile + " &" };

            System.out.println("TOP command : " + Arrays.toString(topCmd));
            Process p  = Runtime.getRuntime().exec(topCmd);
            //ShellRunnable s1 = new ShellRunnable(topCmd);
            //s1.run();
            System.exit(1);
        } catch (Exception e) {
            e.printStackTrace();
        }
        
        /**
         * MONITOR STS PROCESS - TOP COMMAND
         */
        
    }
    
    public static void printErrors(BufferedReader stdError) throws IOException{
        String s;
        // read any errors from the attempted command
        System.out.println("Here is the standard error of the command (if any):\n");
        while ((s = stdError.readLine()) != null) {
            System.out.println(s);
        }
        System.out.println("EXITTING PROGRAM DUE TO ERROR");
        System.exit(-1);
    }
    
    public static String createFile(String s) throws IOException{
        //TOUCH AND EXECUTE TOP COMMAND
        String processFile = String.format("%s-dump.log" , s);
        File file = new File(processFile);
        
        if (file.createNewFile()){
          System.out.println("File is created!");
        }else{
          System.out.println("File already exists.");
        }
        return processFile;
    }



}
