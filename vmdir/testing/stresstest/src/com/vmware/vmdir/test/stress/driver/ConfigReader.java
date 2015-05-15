package com.vmware.vmdir.test.stress.driver;

import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.ini4j.*;

public class ConfigReader {

    static public final String keyHost = "host";
    static public final String keyPort = "port";
    static public final String keyProtocol = "protocol";
    static public final String keyThrNum = "threadNum";
    static public final String keyBaseDN = "baseDN";
    static public final String keyBindDN = "bindDN";
    static public final String keyBindPassword = "bindPassword";
    static public final String keyDNQueueSize = "DNQueueSize";
    static public final String keyDomainDN = "domainDN";
    static public final String keyThreadIDPrefix = "threadIDPrefix";

    static public final String keyBindContainerDN = "bindContainerDN";
    static public final String keyBindPattern = "bindPattern";
    static public final String keyBindRDNPrefix = "bindRDNPrefix";
    static public final String keyBindPasswordPrefix = "bindPasswordPrefix";

    static public final String keyScope = "searchScope";
    static public final String keySearchBase = "searchBaseDN";
    static public final String keyFilter = "filter";
    static public final String keySearchDefinitionNum = "searchDefinitionNum";

    static public final String keyModifyAttribute = "modifyAttribute";
    static public final String keyModifyValuePrefix = "modifyValuePrefix";

    static public final String keyRDNAttribute = "rdnAttribute";
    static public final String keyObjectclass = "objectclass";
    static public final String keyObjectclassNum = "objectclassNum";
    static public final String keyAttribute = "attribute";
    static public final String keyAttributeNum = "attributeNum";

    public ConfigReader(String configFileName) throws IOException {
        super();
        loadConfigFile(configFileName);
    }

    public Map<String, String> getBindConfig() {
        return getConfig(bindThrConfigSection);
    }

    public Map<String, String> getSearchConfig() {
        return getConfig(searchThrConfigSection);
    }

    public Map<String, String> getModifyConfig() {
        return getConfig(modifyThrConfigSection);
    }

    public Map<String, String> getAddConfig() {
        return getConfig(addThrConfigSection);
    }

    public Map<String, String> getDeleteConfig() {
        return getConfig(deleteThrConfigSection);
    }

    private Map<String, String> getConfig(Ini.Section sectionConfig) {
        Map<String, String> configMap = new HashMap<String, String>();

        configBase(configMap);

        if (sectionConfig.fetch(keyBindDN) != null) {
            configMap.put(keyBindDN, sectionConfig.fetch(keyBindDN));
        }

        if (sectionConfig.fetch(keyBindPassword) != null) {
            configMap.put(keyBindPassword, sectionConfig.fetch(keyBindPassword));
        }

        if (sectionConfig.fetch(keyThrNum) != null) {
            configMap.put(keyThrNum, sectionConfig.fetch(keyThrNum));
        }

        if (sectionConfig.fetch(keyBindContainerDN) != null) {
            configMap.put(keyBindContainerDN, sectionConfig.fetch(keyBindContainerDN));
        }

        if (sectionConfig.fetch(keyBindPattern) != null) {
            configMap.put(keyBindPattern, sectionConfig.fetch(keyBindPattern));
        }

        if (sectionConfig.fetch(keyBindRDNPrefix) != null) {
            configMap.put(keyBindRDNPrefix, sectionConfig.fetch(keyBindRDNPrefix));
        }

        if (sectionConfig.fetch(keyBindPasswordPrefix) != null) {
            configMap.put(keyBindPasswordPrefix, sectionConfig.fetch(keyBindPasswordPrefix));
        }

        if (sectionConfig.fetch(keyScope) != null) {
            configMap.put(keyScope, sectionConfig.fetch(keyScope));
        }

        if (sectionConfig.fetch(keyFilter) != null) {
            configMap.put(keyFilter, sectionConfig.fetch(keyFilter));
        }

        if (sectionConfig.fetch(keySearchDefinitionNum) != null) {
            String tmpFilterNum = sectionConfig.fetch(keySearchDefinitionNum);
            configMap.put(keySearchDefinitionNum, tmpFilterNum);

            for (Integer i = 1; i <= Integer.valueOf(tmpFilterNum); i++) {
                configMap.put(keyFilter + i, sectionConfig.fetch(keyFilter + i));
                configMap.put(keySearchBase + i, sectionConfig.fetch(keySearchBase + i));
                configMap.put(keyScope + i, sectionConfig.fetch(keyScope + i));
            }
        }

        if (sectionConfig.fetch(keyModifyAttribute) != null) {
            configMap.put(keyModifyAttribute, sectionConfig.fetch(keyModifyAttribute));
        }

        if (sectionConfig.fetch(keyModifyValuePrefix) != null) {
            configMap.put(keyModifyValuePrefix, sectionConfig.fetch(keyModifyValuePrefix));
        }

        if (sectionConfig.fetch(keyAttributeNum) != null) {
            String tmpAttrNum = sectionConfig.fetch(keyAttributeNum);
            configMap.put(keyAttributeNum, tmpAttrNum);

            for (Integer i = 1; i <= Integer.valueOf(tmpAttrNum); i++) {
                configMap.put(keyAttribute + i, sectionConfig.fetch(keyAttribute + i));
            }
        }

        if (sectionConfig.fetch(keyObjectclassNum) != null) {
            String tmpObjectclassNum = sectionConfig.fetch(keyObjectclassNum);
            configMap.put(keyObjectclassNum, tmpObjectclassNum);

            for (Integer i = 1; i <= Integer.valueOf(tmpObjectclassNum); i++) {
                configMap.put(keyObjectclass + i, sectionConfig.fetch(keyObjectclass + i));
            }
        }

        if (sectionConfig.fetch(keyRDNAttribute) != null) {
            configMap.put(keyRDNAttribute, sectionConfig.fetch(keyRDNAttribute));
        }

        if (sectionConfig.fetch(keyThreadIDPrefix) != null) {
            configMap.put(keyThreadIDPrefix, sectionConfig.fetch(keyThreadIDPrefix));
        }

        return configMap;
    }

    private void configBase(Map<String, String> config) {
        config.put(keyHost, baseConfigSection.fetch(keyHost));
        config.put(keyPort, baseConfigSection.fetch(keyPort));
        config.put(keyProtocol, baseConfigSection.fetch(keyProtocol));
        config.put(keyBaseDN, baseConfigSection.fetch(keyBaseDN));
        config.put(keyBindDN, baseConfigSection.fetch(keyBindDN));
        config.put(keyBindPassword, baseConfigSection.fetch(keyBindPassword));
        config.put(keyThrNum, baseConfigSection.fetch(keyThrNum));
        config.put(keyDNQueueSize, baseConfigSection.fetch(keyDNQueueSize));
        config.put(keyDomainDN, baseConfigSection.fetch(keyDomainDN));
    }

    private Ini ini;
    private Ini.Section baseConfigSection;
    private Ini.Section bindThrConfigSection;
    private Ini.Section searchThrConfigSection;
    private Ini.Section modifyThrConfigSection;
    private Ini.Section addThrConfigSection;
    private Ini.Section deleteThrConfigSection;

    private void loadConfigFile(final String fileName) throws IOException {

        ini = new Ini();
        try {
            ini.load(new FileReader(fileName));
        } catch (IOException e) {
            throw e;
        }

        baseConfigSection = ini.get("base");
        bindThrConfigSection = ini.get("bind thread");
        searchThrConfigSection = ini.get("search thread");
        modifyThrConfigSection = ini.get("modify thread");
        addThrConfigSection = ini.get("add thread");
        deleteThrConfigSection = ini.get("delete thread");
    }

    public static void main(final String[] args) {
        ConfigReader cfgReader = null;

        try {
            cfgReader = new ConfigReader(args[0]);
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        System.out.println(cfgReader.getBindConfig());
        System.out.println(cfgReader.getSearchConfig());
        System.out.println(cfgReader.getModifyConfig());
        System.out.println(cfgReader.getAddConfig());
        System.out.println(cfgReader.getDeleteConfig());
    }
}
