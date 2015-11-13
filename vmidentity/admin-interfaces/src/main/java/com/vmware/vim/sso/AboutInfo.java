/* **********************************************************************
 * Copyright 2011 VMware, Inc.  All rights reserved.
 * *********************************************************************/
package com.vmware.vim.sso;

import static com.vmware.vim.sso.admin.impl.util.ValidateUtil.validateNotEmpty;

/**
 * System information (including product name, version, SSO product info, and api version) about an
 * SSO Server instance.
 */
public final class AboutInfo {

   private final String version;
   private final String build;
   private final String apiRevision;
   private final String clusterId;
   private final String deploymentId;
   private final String ssoProductVersionMajor;
   private final String ssoProductVersionMinor;
   private final String ssoProductVersionMaint;
   private final String ssoProductInfo;

   /**
    * Create and populate {@linkplain AboutInfo} instance.
    *
    * @param version
    *           Product version, up to and including the minor version number.
    * @param build
    *           Product version and build number.
    * @param apiRevision
    *           Opaque revision identifier of this server's remote API.
    * @param clusterId
    *           cannot be null or empty
    * @param deploymentId
    *           cannot be null or empty
    * @param ssoProductVersionMajor
    *           can be null;
    * @param ssoProductVersionMinor
    *           can be null;
    * @param ssoProductVersionMaint
    *           can be null;
    * @param ssoProductInfo
    *           SSO product info, can be null
    */
   public AboutInfo(String version, String build, String apiRevision,
      String clusterId, String deploymentId, String ssoProductVersionMajor,
      String ssoProductVersionMinor, String ssoProductVersionMaint, String ssoProductInfo) {

      validateNotEmpty(version, "version");
      validateNotEmpty(build, "build");
      validateNotEmpty(apiRevision, "apiRevision");
      validateNotEmpty(clusterId, "instanceId ");
      validateNotEmpty(deploymentId, "deploymentId");

      this.clusterId = clusterId;
      this.deploymentId = deploymentId;
      this.version = version;
      this.build = build;
      this.ssoProductVersionMajor = ssoProductVersionMajor;
      this.ssoProductVersionMinor = ssoProductVersionMinor;
      this.ssoProductVersionMaint = ssoProductVersionMaint;
      this.ssoProductInfo = ssoProductInfo;
      this.apiRevision = apiRevision;
   }

   /**
    * Product version, up to and including the minor version number.
    * <p>
    * A dot-separated string @{code x.y}, where {@code x} designates the major
    * version and {@code y} designated the minor version.
    */
   public String getVersion() {
      return version;
   }

   /**
    * Product version and build number.
    * <p>
    * The version string, followed by a dash, followed by the build number:
    * {@code x.y-num} indicates version {@code x.y} and build number {@code num}.
    */
   public String getBuild() {
      return build;
   }

   /**
    * @return SSO product major version, could be null
    */
   public String getSsoProductVersionMajor() {
      return ssoProductVersionMajor;
   }

   /**
    * @return SSO product minor version, could be null
    */
   public String getSsoProductVersionMinor() {
      return ssoProductVersionMinor;
   }

   /**
    * @return SSO product maintenance version, could be null
    */
   public String getSsoProductVersionMaint() {
      return ssoProductVersionMaint;
   }

   /**
    * The sso product version string, followed by a dash, followed by the build number:
    * {@code x.y.z-num} indicates version {@code x.y.z} and build number {@code num}.
    */
   public String getSsoProductInfo() {
      return ssoProductInfo;
   }

   /**
    * Opaque revision identifier of this server's remote API.
    */
   public String getApiRevision() {
      return apiRevision;
   }

   @Override
   public String toString() {
      StringBuilder toStrBuilder = new StringBuilder();
      toStrBuilder.append("{").append(getClass().getSimpleName()).append(" | ")
         .append("version: ").append(version).append("; build: ").append(build)
         .append("; ssoProductVersionMajor: ").append(ssoProductVersionMajor)
         .append("; ssoProductVersionMinor: ").append(ssoProductVersionMinor)
         .append("; ssoProductVersionMaint: ").append(ssoProductVersionMaint)
         .append("; ssoProductInfo: ").append(ssoProductInfo)
         .append("; api revision: ").append(apiRevision)
         .append("; clusterId: ").append(clusterId).append("; deploymentId: ")
         .append(deploymentId).append("}");

      return toStrBuilder.toString();
   }

   /**
    * Each node in a cluster would return the same value. One node installation is
    * considered a cluster with just one node. This value is not replicated.
    *
    * @see #getDeploymentId()
    */
   public String getClusterId() {
      return clusterId;
   }

   /**
    * Identifier of this server deployment. Each node in a cluster would return
    * the same value. Replicated in multi-site scenario, so two sites in
    * multi-site case would return the same value.
    *
    * @see #getClusterId()
    */
   public String getDeploymentId() {
      return deploymentId;
   }
}
