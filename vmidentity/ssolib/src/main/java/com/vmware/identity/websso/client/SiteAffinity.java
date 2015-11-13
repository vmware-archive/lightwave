package com.vmware.identity.websso.client;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.vmware.identity.cdc.CdcDCEntry;
import com.vmware.identity.cdc.CdcFactory;
import com.vmware.identity.cdc.CdcSession;

/**
 * @author schai
 * Wrapper class for AFD site-affinity feature. With proper handling
 *  of installation exception,etc.
 */
public class SiteAffinity {
    final static Logger logger = LoggerFactory.getLogger(SiteAffinity.class);

    /**
     * Return affinitized DC.
     *
     * @return affinitized DC.  null if AFD client is not installed.
     * @throws WebssoClientException
     *          AFD installed but was unable to return affinitized DC.
     */
    public static String getAffinitiedDC() throws WebssoClientException {

        String affinitizedDC = null;

        try {
            CdcSession cdcSession = null;

            try {
                cdcSession = CdcFactory.createCdcSessionViaIPC();
                if (cdcSession == null) {
                    logger.error("Failed to create AFD CdcSession via IPC. ");
                    throw new WebssoClientException("AFD cdcSession is created null.");

                } else {
                    CdcDCEntry entry = cdcSession.getAffinitizedDC(null, 0);

                    if (entry == null) {
                        logger.error("AFD cdcSession getAffinitizedDC() returns null CdcDCEntry.");
                        throw new WebssoClientException(
                                "AFD cdcSession getAffinitizedDC() returns null CdcDCEntry.");
                    }else {
                        if (entry.dcName == null) {
                            logger.error("AFD CdcDCEntry has null dcName attribute.");
                            throw new WebssoClientException(
                                    "AFD cdcSession getAffinitizedDC() returns null CdcDCEntry:dcName.");
                        }
                        affinitizedDC = entry.dcName;
                    }
                }
            } catch (Exception  e ) {
                logger.error("Failed to create AFD CdcSession via IPC.", e);
                throw new WebssoClientException("AFD client fails in getting affinitized DC.");

            } finally {
                if (cdcSession != null) {
                    cdcSession.close();
                }
            }
        }catch (java.lang.UnsatisfiedLinkError e) {
            // in case it can't load libcdcjni.so
            logger.info("Failed to init CdcSession, likly due to missing libcdcjni.so.", e);
        } catch (NoClassDefFoundError e) {
            // in case vmafd jars are not found
            logger.info("Failed to init CdcSession. likly due to missing vmafd jar.", e);
        }

        return affinitizedDC;
    }

    /**
     * Return list contains all domain controllers.
     *
     * @return DC list.  null if AFD client is not installed.
     * @throws WebssoClientException
     *      AFD installed but was unable to return domain controllers.
     */
    public static List<String> enumDCEntries() throws WebssoClientException {

        List<String> dcList = null;

        try {
            CdcSession cdcSession = null;

            try {
                cdcSession = CdcFactory.createCdcSessionViaIPC();

                if (cdcSession == null) {
                    logger.error("Failed to create AFD CdcSession via IPC. ");
                    throw new WebssoClientException("AFD cdcSession is created null.");

                } else {
                    dcList = cdcSession.enumDCEntries();

                    if (dcList == null) {
                        logger.error("AFD cdcSession returns null DC list.");
                        throw new WebssoClientException(
                                "AFD cdcSession returns null DC list.");
                    }
                }
            } catch (Exception  e ) {
                logger.error("Failed to create AFD CdcSession via IPC.", e);
                throw new WebssoClientException("AFD client fails in getting affinitized DC.");
            }  finally {
                if (cdcSession != null) {
                    cdcSession.close();
                }
            }

        } catch (java.lang.UnsatisfiedLinkError e) {
            // in case it can't load libcdcjni.so
            logger.info("Failed to init CdcSession, likly due to missing libcdcjni.so.", e);
        } catch (NoClassDefFoundError e) {
            // in case vmafd jars are not found
            logger.info("Failed to init CdcSession, likly due to missing vmafd jar.", e);
        }

        return dcList;

    }

}
