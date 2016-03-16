package com.vmware.identity.installer;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

/**
 * Executes a shell command
 */
class ShellRunnable implements Runnable {
   
   private String[] shellCommand;
   
   public  ShellRunnable(String[] topCmd) {
   this.shellCommand = topCmd;
   }
 
@Override
public void run() {
    try{
        Process p  = Runtime.getRuntime().exec(shellCommand);
        
        BufferedReader stdErr = new BufferedReader(new InputStreamReader(p.getErrorStream()));
        if(stdErr.readLine() != null){
            printErrors(stdErr);
        }            
    }catch (Exception e){
        e.printStackTrace();
    }
}
   
public  void printErrors(BufferedReader stdError) throws IOException{
    String s;
    // read any errors from the attempted command
    System.out.println("Here is the standard error of the command (if any):\n");
    while ((s = stdError.readLine()) != null) {
        System.out.println(s);
    }
    System.out.println("EXITTING PROGRAM DUE TO ERROR");
    System.exit(-1);
}

}