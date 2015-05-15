package com.vmware.vmdir.test.stress.driver;

import java.io.IOException;
import java.util.Map;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

public class VmDirStresser {
    String configFileName = null;
    ConfigReader configReader = null;

    public VmDirStresser(String fileName) {
	configFileName = fileName;
    }

    public static void main(final String[] args) {
	VmDirStresser stresser = new VmDirStresser(args[0]);

	try {
	    stresser.loadConfig();
	    stresser.dispatch();

	    // TODO, wait till every thing is done?
	} catch (Exception e) {
	    // TODO Auto-generated catch block
	    e.printStackTrace();
	}
    }

    private void dispatch() throws Exception {
	Map<String, String> bindConfigMap = null;
	Map<String, String> searchConfigMap = null;
	Map<String, String> modifyConfigMap = null;
	Map<String, String> addConfigMap = null;
	Map<String, String> deleteConfigMap = null;
	BlockingQueue<String> dnBlockingQueue = null;

	bindConfigMap = configReader.getBindConfig();
	searchConfigMap = configReader.getSearchConfig();
	modifyConfigMap = configReader.getModifyConfig();
	addConfigMap = configReader.getAddConfig();
	deleteConfigMap = configReader.getDeleteConfig();

	String dnQueueSize = addConfigMap.get(ConfigReader.keyDNQueueSize);
	if (dnQueueSize != null) {
	    dnBlockingQueue = new ArrayBlockingQueue<String>(
		    Integer.parseInt(dnQueueSize));
	}

	// start bind runners
	Integer bindThrNum = Integer.valueOf(bindConfigMap
		.get(ConfigReader.keyThrNum));
	for (int i = 0; i < bindThrNum; i++) {
	    LdapBindRunner bindRunner = new LdapBindRunner(bindConfigMap);
	    bindRunner.start();
	}

	// start search runners
	searchConfigMap = configReader.getSearchConfig();
	Integer searchThrNum = Integer.valueOf(searchConfigMap
		.get(ConfigReader.keyThrNum));
	for (int i = 0; i < searchThrNum; i++) {
	    LdapSearchRunner searchRunner = new LdapSearchRunner(
		    searchConfigMap);
	    searchRunner.start();
	}

	// start modify runners
	Integer modifyThrNum = Integer.valueOf(modifyConfigMap
		.get(ConfigReader.keyThrNum));
	for (int i = 0; i < modifyThrNum; i++) {
	    LdapModifyRunner modifyRunner = new LdapModifyRunner(
		    modifyConfigMap);
	    modifyRunner.start();
	}

	// start add runners
	Integer addThrNum = Integer.valueOf(addConfigMap
		.get(ConfigReader.keyThrNum));
	for (int i = 0; i < addThrNum; i++) {
	    LdapAddRunner addRunner = new LdapAddRunner(addConfigMap,
		    dnBlockingQueue);
	    addRunner.start();
	}

	// start delete runners
	Integer deleteThrNum = Integer.valueOf(deleteConfigMap
		.get(ConfigReader.keyThrNum));
	for (int i = 0; i < deleteThrNum; i++) {
	    LdapDeleteRunner deleteRunner = new LdapDeleteRunner(
		    deleteConfigMap, dnBlockingQueue);
	    deleteRunner.start();
	}

	Thread.sleep(100000000); // TODO, should provide a way to stop all
				 // threads and exit. (e.g. max time, max
				 // entries...etc.)
    }

    private void loadConfig() throws IOException {
	configReader = new ConfigReader(configFileName);
    }

}
