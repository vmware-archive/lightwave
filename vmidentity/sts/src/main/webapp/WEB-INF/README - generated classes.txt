How to generate java classes for all entities used in our wsdl(STSService.wsdl).
 
First we will need a simple tool called wsimport which will generate the classes.
It is located in toolchain/noarch/jax-ws-2.2.5/bin/wsimport.sh.
 
The following steps will generate the classes: 

0) Better first copy the entire jax-ws-2.2.5 folder on your machine and make
this wsimport.sh executable (chmod +x).

1) Create a folder in which this tool will output generated classes(java files).

2) Create a folder in which this tool will output compiled classes(class files).

3) Navigate to <path to perforce workspace>
	      /depot/vmidentity/main/casj/sts/src/main/webapp/WEB-INF/wsdl/

4) Run this command 
  <path to where you copied jax-ws-2.2.5 folder>
  /jax-ws-2.2.5/bin/wsimport.sh 
		    -Xendorsed 
		    -d <path to dir for compiled classes> 
		    -s <path to dir for java classes> 
		    -extension 
		    -Xno-addressing-databinding 
		    STSService.wsdl
 
The output as you expect will be in the folder you created in step 1).
If you want to generate the classes again, just empty the folders for
generated java and class files.
 