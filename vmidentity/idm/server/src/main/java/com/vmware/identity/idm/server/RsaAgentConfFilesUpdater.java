/*
 *
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
 *
 */
package com.vmware.identity.idm.server;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Set;

import org.apache.commons.lang.StringEscapeUtils;
import org.apache.commons.lang.SystemUtils;
import org.apache.commons.lang.Validate;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;
import com.vmware.identity.idm.IDMException;
import com.vmware.identity.idm.RSAAMInstanceInfo;
import com.vmware.identity.idm.RSAAgentConfig;
import com.vmware.identity.interop.IdmUtils;

/**
 * RSA secure agent configuration files updater class. This class allows local configurations files
 * sync to the tenant setting of the RSA agent configuration parameters and binary config/options files.
 *
 * @author schai
 *
 */
public class RsaAgentConfFilesUpdater {

    // RSA SecureID configuration constants
    private static final String RSA_API_PROPERTIES_TEMPLATE = "rsa_api.properties";

    private static final String RSA_API_PROPERTIES_NAME = "rsa_api.properties";

    private static final String RSA_AGENT_NAME = "RSA_AGENT_NAME";

    private static final String SDCONF_LOC = "SDCONF_LOC";

    private static final String SDOPTS_LOC = "SDOPTS_LOC";

    private static final String RSA_LOG_LEVEL = "RSA_LOG_LEVEL";

    private static final String RSA_LOG_FILE_SIZE = "RSA_LOG_FILE_SIZE";

    private static final String RSA_LOG_FILE_COUNT = "RSA_LOG_FILE_COUNT";

    private static final String RSA_CONNECTION_TIMEOUT = "RSA_CONNECTION_TIMEOUT";

    private static final String RSA_READ_TIMEOUT = "RSA_READ_TIMEOUT";

    private static final String RSA_ENC_ALGLIST = "RSA_ENC_ALGLIST";

    private static final String RSA_AGENT_PLATFORM = "RSA_AGENT_PLATFORM";

    private static final String RSA_CONFIG_DATA_LOC = "RSA_CONFIG_DATA_LOC";

    private static final String SDNDSCRT_LOC = "SDNDSCRT_LOC";

    private static final String RSA_LOG_FILE = "RSA_LOG_FILE";

    private static final String SD_CONF_NAME = "sdconf.rec";

    private static final String SD_OPTS_NAME = "sdopts.rec";

    private static final String RSA_SECUREID_LOG_NAME = "rsa_securid.log";

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(RsaAgentConfFilesUpdater.class);

    private final String _clusterID;


    RsaAgentConfFilesUpdater(String clusterID) {
        this._clusterID = clusterID;
    }
    /**
    * Update rsa_api.properties, sdconf.rec, sdopts.rec on local sso server conf directory
    * @param tenantInfo
    * @param rsaConfig
    * @throws Exception
    */
   public void updateRSAConfigFiles(TenantInformation tenantInfo, RSAAgentConfig rsaConfig) throws Exception {

       Validate.notNull(tenantInfo, "tenantInfo");

       String tenantName = tenantInfo.getTenant().getName();
       Validate.notEmpty(tenantName, "tenantName");

       // read from template from class resource
       ClassLoader classLoader = getClass().getClassLoader();
       InputStream templateStrm = classLoader.getResourceAsStream(RSA_API_PROPERTIES_TEMPLATE);
       Validate.notNull(templateStrm, "rsa_api.properties resource not found");

       // create tenant config directory if it does not exist
       File tenantConfigDir = ensureTenantConfigDirExist(tenantName);

       // create config file
       File tenantConfigFile = new File(tenantConfigDir, RSA_API_PROPERTIES_NAME);

       BufferedReader br = new BufferedReader(new InputStreamReader(templateStrm));

       String lines = "";
       String siteID = _clusterID;
       RSAAMInstanceInfo instInfo = rsaConfig.get_instMap().get(siteID);

       if (instInfo == null) {
           // site info are not defined.
           return;
       }
       String line;
       try {
           line = br.readLine();
           String ls = System.getProperty("line.separator");
           while (line != null)
           {
               String[] attrVal = line.split("=");

               if (attrVal == null || attrVal.length < 1) {
                   continue;
               }

               String attr = attrVal[0].replace("#", "");
               String newLine;
               switch (attr) {
               // keys that are configurable
                   case RSA_AGENT_NAME:
                       newLine = RSA_AGENT_NAME + "=" + instInfo.get_agentName() + ls;
                       lines += newLine;
                       break;

                   case SDCONF_LOC: {
                       byte[] sdConfBytes = instInfo.get_sdconfRec();
                       Validate.notNull(sdConfBytes);

                       // delete existing file.
                       File sdConfFile = new File(tenantConfigDir, SD_CONF_NAME);
                       sdConfFile.delete();

                       // write the file
                       String sdConfFilePath = sdConfFile.getAbsolutePath();
                       FileOutputStream fos;
                       try {
                           fos = new FileOutputStream(sdConfFilePath);
                       } catch (IOException e) {
                           logger.error("Can not create or open sdconf.rec: " + sdConfFilePath, e);
                           throw e;
                       }
                       try {
                           fos.write(sdConfBytes);
                       } catch (IOException e) {
                           logger.error("Can not write to sdconf.rec: " + sdConfFilePath, e);
                           throw e;
                       } finally {
                           fos.close();
                       }
                       // update the api properties
                       newLine = SDCONF_LOC + "=" + sdConfFilePath + ls;
                       lines += StringEscapeUtils.escapeJava(newLine);

                       break;
                   }
                   case SDOPTS_LOC: {
                       byte[] sdOptsBytes = instInfo.get_sdoptsRec();
                       if (sdOptsBytes == null) {
                           newLine = "#" + SDOPTS_LOC + "=" + ls;
                           lines += newLine;
                           break;
                       }

                       // delete existing file.
                       File sdOptsFile = new File(tenantConfigDir, SD_OPTS_NAME);
                       sdOptsFile.delete();

                       // write the file
                       String sdOptsFilePath = sdOptsFile.getAbsolutePath();
                       FileOutputStream fos;
                       try {
                           fos = new FileOutputStream(sdOptsFilePath);
                       } catch (IOException e) {
                           logger.error("Can not create or open sdopts.rec: " + sdOptsFilePath, e);
                           throw e;
                       }
                       try {
                           fos.write(sdOptsBytes);
                       } catch (IOException e) {
                           logger.error("Can not write to sdopts.rec: " + sdOptsFilePath, e);
                           throw e;
                       } finally {
                           fos.close();
                       }
                       // update the api properties
                       newLine = SDOPTS_LOC + "=" + sdOptsFilePath;
                       lines += StringEscapeUtils.escapeJava(newLine);

                       break;
                   }

                   case RSA_LOG_LEVEL:
                       newLine = RSA_LOG_LEVEL + "=" + rsaConfig.get_logLevel() + ls;
                       lines += newLine;
                       break;

                   case RSA_LOG_FILE_SIZE:
                       newLine = RSA_LOG_FILE_SIZE + "=" + rsaConfig.get_logFileSize() + "MB" + ls;
                       lines += newLine;
                       break;

                   case RSA_LOG_FILE_COUNT:
                       newLine = RSA_LOG_FILE_COUNT + "=" + rsaConfig.get_maxLogFileCount() + ls;
                       lines += newLine;
                       break;

                   case RSA_CONNECTION_TIMEOUT:
                       newLine = RSA_CONNECTION_TIMEOUT + "=" + rsaConfig.get_connectionTimeOut() + ls;
                       lines += newLine;
                       break;

                   case RSA_READ_TIMEOUT:
                       newLine = RSA_READ_TIMEOUT + "=" + rsaConfig.get_readTimeOut() + ls;
                       lines += newLine;
                       break;

                   case RSA_ENC_ALGLIST:
                       Set<String> encSet = rsaConfig.get_rsaEncAlgList();
                       Validate.notNull(encSet);
                       String[] encArray = encSet.toArray(new String[encSet.size()]);

                       newLine = RSA_ENC_ALGLIST + "=";
                       if (encArray.length > 0) {
                           newLine += encArray[0];
                       }
                       for (int i = 1; i < encArray.length; i++) {
                           newLine += ("," + encArray[i]);
                       }
                       newLine += ls;
                       lines += newLine;
                       break;

                   // keys that not configurable
                   case RSA_AGENT_PLATFORM:
                       newLine = RSA_AGENT_PLATFORM + "=";
                       if (SystemUtils.IS_OS_LINUX) {
                           newLine = RSA_AGENT_PLATFORM + "=linux" + ls;
                       } else {
                           newLine = RSA_AGENT_PLATFORM + "=windows" + ls;
                       }
                       lines += newLine;
                       break;

                   case RSA_CONFIG_DATA_LOC:
                       newLine = RSA_CONFIG_DATA_LOC + "=" + tenantConfigDir.getAbsolutePath() + ls;
                       lines += StringEscapeUtils.escapeJava(newLine);
                       break;

                   case SDNDSCRT_LOC:
                       File sdndscrtLoc = new File(tenantConfigDir, "secureid");
                       newLine = RSA_CONFIG_DATA_LOC + "=" + sdndscrtLoc.getAbsolutePath() + ls;
                       lines += StringEscapeUtils.escapeJava(newLine);
                       break;

                   case RSA_LOG_FILE:
                       String ssoConfigDir = IdmUtils.getIdentityServicesLogDir();
                       File rsaLogFile = new File(ssoConfigDir, RSA_SECUREID_LOG_NAME);
                       newLine = RSA_LOG_FILE + "=" + rsaLogFile.getAbsolutePath() + ls;
                       lines += StringEscapeUtils.escapeJava(newLine);
                       break;

                   default:
                       lines += (line + ls);

               }
               line = br.readLine();
           }
       } catch (IOException e) {
           logger.error("Fail to read from rsa_api.properties resource stream", e);
           throw new IDMException("Unable to generate rsa_api property file", e);
       }
       FileWriter fw;
       try {
           fw = new FileWriter(tenantConfigFile);
       } catch (IOException e) {
           logger.error("Fail to create rsa_api.properties file", e);
           throw new IDMException("Unable to generate rsa_api property file", e);
       }

       BufferedWriter out = new BufferedWriter(fw);
       try {
           tenantInfo.get_rsaConfigFilesLock().writeLock().lock();
           out.write(lines.toString());
       } catch (IOException e) {
           logger.error("Fail to write to rsa_api.properties file", e);
           throw new IDMException("Unable to generate rsa_api property file", e);
       } finally {
           try {
               out.close();
           } catch (IOException e) {
               tenantInfo.get_rsaConfigFilesLock().writeLock().unlock();
               logger.error("Fail to close to rsa_api.properties file BufferWriter", e);
               throw new IDMException("Unable to generate rsa_api property file", e);
           }
           tenantInfo.get_rsaConfigFilesLock().writeLock().unlock();
       }
   }

   private File ensureTenantConfigDirExist(String tenantName) throws IDMException {
       String ssoConfigDir = IdmUtils.getIdentityServicesConfigDir();
       Validate.notEmpty(ssoConfigDir);
       File tenantConfigPath = new File(ssoConfigDir, tenantName);

       if (!tenantConfigPath.exists()) {
           if (!tenantConfigPath.mkdir()) {
               String error = String.format("Fail to create subdirectory %s " +
                       "in sso config dir for tenant", tenantConfigPath);
               logger.error(error);
               throw new IDMException(error);
           }
       }
       return tenantConfigPath;
   }

}
