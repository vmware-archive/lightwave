/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */
using System;

namespace VMPSCHighAvailability.Common
{
    /// <summary>
    /// Constants.
    /// </summary>
    public class Constants
    {
        /// <summary>
        /// Denotes the name of the tool
        /// </summary>
        public const string ToolName = "PSC Site Management";
        public const string ToolDescription = "PSC Site Management MMC SnapIn to monitor the availability of PSC connected to the management nodes.";
        public const string Company = "Vmware";
        public const string CompanyName = "Vmware Inc.";
        public const string DateFormat = "dd-MMM-yyyy hh:mm:ss";
        public const int MilliSecsMultiplier = 1000;
        public const int TopologyTimeout = 90;
        public const int DefaultTimerRefreshInterval = 30;
        public const int CacheCycleRefreshInterval = 60;
        public const string DeleteServer = "Delete Server";
        public const string RootNodeDisplayName = "VMware Servers";
        public const string SnapIn = "SnapIn";
        public const string SuiteName = "Lightwave";

        #region Welcome Screen Text Constants

        public const string WelcomeText = "Welcome to ";
        public const string FeatureLine2 = "Set Site Affinity for Management node";
        public const string FeatureLine3 = "Monitor health of Infrastructure node";
        public const string FeatureLine1 = "Global View of the HA topology";

        #endregion

        #region Connect/Disconnect Constants

        public const string ServerLoginFailure = "Unable to connect to the server. Please check the login details.";
		public const string VMDirConnectFailure = "Please check the directory service is running on the domain controller " +
			"and the login details are correct. Server responded with error(s) - {0}";
        public const string TolologyApiIsTakenLonger = "Timed out. Unable to load the topology in the required time.";
        public const string LoginError = "Username or password is incorrect or server is unsupported or not reachable";
        public const string LoadingTopologyDetails = "Loading topology details";
        public const string FailedLoadingTopologyDetails = "Failed to load topology details";
        public const int ServerTimeOut = 20000;
        public const string DisconnectLabel = "Disconnect";
        public const string ConnectLabel = "Connect";   
        public const string DisconnectIcon = "disconnect_64x.png";
        public const string ConnectIcon = "connect.png";

        #endregion

        #region Monitor View Contants

        public const string Node = "Node";
        public const string StatusActiveImage = "NSStatusAvailable";
        public const string StatusInActiveImage = "NSStatusUnavailable";
        public const string GlobalNodeImage = "global.png";
        public const string ManagementNodeImage = "host.png";
        public const string ServiceImage = "service.png";
        public const string InfrastructureNodeImage = "servermonitor.png";
        public const string InfrastructureGroupNodeImage = "node.png";
        public const string ManagementGroupNodeImage = "node.png";
        public const string SiteNodeImage = "provider.png";
        public const string ClientAffinityOn = "ON";
        public const string ClientAffinityOff = "OFF";
        public const string HALabel = "Turn HA ";
        public const string ClientSiteAffinityDesc = "High Availability settings are {0}enabled for the host";

        #endregion

        #region Application Environment Constants

        public const string LocalDataFileName = "PSCHighAvailability.xml";
        public const string ToolsSuiteName = "LightwaveTools";
		public const string PscLogFileName = "PscSiteManagement.log";
		public const string PscTopologyFileName = "PscTopologyInfo.log";


        #endregion

        #region Error Message(s)

        public const string NoServicesAvailable = "No services available.";
        public const string ConnectForDetails = "Connect.. for details";
        public const string ConnectToServer = "Connect to server";
        public const string RefreshFailure = "Refresh Failure";

        #endregion

        #region PSC Table Constants

        public const string PscTableColumnNameId = "Host name";
        public const string PscTableColumnAffinitizedId = "Affinitized";
        public const string PscTableColumnStatusId = "Status";
        public const string PscTableColumnServicesId = "Services";
        public const string PscTableColumnLastHeartbeatId = "Last Heartbeat";
        public const string PscTableColumnSitenameId = "Sitename";
        public const string PscTableColumnLastPingId = "Last Ping";
        public const string PscTableColumnLastResponseTimeId = "Last Response Time";
        public const string PscTableColumnLastErrorId = "Last Error";
        public const string PscTableColumnNodeTypeId = "Node Type";
		public const string PscTableColumnSiteLocationId = "Site Location";
        #endregion

        #region Services Table Constants

        public const string TableColumnIconId = "Icon";
        public const string ServiceTableColumnNameId = "Service Name";
		public const string ServiceTableColumnDescId = "Description";
        public const string ServiceTableColumnPortId = "Port";
        public const string ServiceTableColumnStatusId = "Status";
        public const string ServiceTableColumnLastHeartbeatId = "LastHeartbeat";

        #endregion

        #region Filter Criteria Constants

        public const string FilterCriteriaTableColumnId = "Column";
        public const string FilterCriteriaTableColumnOpearatorId = "Operator";
        public const string FilterCriteriaTableColumnValueId = "Value";

        #endregion

        #region Service Status Constants

        public const string Active = "Active";
        public const string UnKnown = "Unknown";
        public const string InActive = "Inactive";

        #endregion

        #region Multi-Site Constants

        public const string SameSite = "Local";
        public const string RemoteSite = "Remote";
        
        #endregion

        #region State Constants

        public const string StateLegacyMode = "Legacy Mode";
        public const string StateNoKnowledgeOfAnyDcs = "No knowledge of any DCs";
        public const string StateSiteAffinitized = "Site affinitized";
        public const string StateAffinitizedToOffsiteDc = "Affinitized to an offsite DC";
        public const string StateAllKnownDcsAreDown = "All known DCs are down";
        public const string StateInvalidState = "Invalid State";

        #endregion

        #region State Descriptions Constants

        public const string StateDescHighAvailabilityDisabled = "The node is in Legacy Mode.";
        public const string StateDescNoKnowledgeOfAnyDcs = "The node had no knowledge of any domain controllers.";
        public const string StateDescSiteAffinitized = "The node is affinitized to domain controller.";
        public const string StateDescAffinitizedToOffsiteDc = "The node is affinitized to an offsite domain controller.";
        public const string StateDescAllKnownDcsAreDown = "All the domain controllers that are known to the node are in-active.";
        public const string StateDescInvalidState = "Invalid State";

        #endregion

        public const string EnableDefaultHA = "Enable HA";
        public const string EnableLegacy = "Enable Legacy";
        public const string FullHealth = "Full high availability requires multiple healthy infrastructure nodes to support failover.";
        public const string LimitedHealth = "Limited high availability indicates that the site has only one healthy infrastructure node and is in danger of site failure if that node fails.";
        public const string DownHealth = "Site down indicates that no infrastructure nodes in the site are healthy. Immediate action necessary to restore a healthy infrastructure node.";
        public const string LegacyHealth = "Legacy HA depends on a load balancer to be configured to allow failover.";
        public const string CannotDeleteTheRootNode = "Connot delete the Console Root for the window";
        public const string HA = "HA";
        public const string Legacy = "Legacy";
        public const string ModeChange = "Are you sure you want to change the mode to ";
        public const string Confirm = "Confirm?";

		#region Service Name Constants

		public const string ApplianceManagementServiceName = "applmgmt (VMware Appliance Management Service)";
		public const string LicenseServiceName = "vmware-cis-license";
		public const string ComponentManagerServiceName = "vmware-cm (VMware Component Manager)";
		public const string IdentityManagementServiceName = "vmware-sts-idmd";
		public const string PscClientServiceName = "vmware-psc-client (VMware Platform Services Controller Client)";
		public const string RHttpProxyServiceName = "vmware-rhttpproxy (VMware HTTP Reverse Proxy)";
		public const string ServiceControlAgentServiceName = "vmware-sca (VMware Service Control Agent)";
		public const string StsServiceName = "vmware-stsd";
		public const string AuthFrameworkServiceName = "vmafdd";
		public const string AuthFrameworkServiceDesc = "VMware Authentication Framework";
		public const string CertificateServiceName = "vmcad";
		public const string DirectoryServiceName = "vmdird";
		public const string DomainNameServiceName = "vmdnsd (VMware Domain Name Service)";
		public const string LifecycleManagerServiceName = "vmonapi (VMware Service Lifecycle Manager API)";
		public const string VMonServiceName = "vmware-vmon (VMware Service Lifecycle Manager)";
		public const string IdentityManagementService = "IdentityManager";
		public const string LicenseService = "License Service";
		public const string SsoAdminService = "sso-adminserver";
		public const string AdminServiceName = "sso-adminserver";
		public const string WebSsoService = "Websso";
		public const string WebSsoServiceName = "websso";
		public const string StsService = "sts";
		public const string CertificateService = "VMware Certificate-Service";
        public const string LookupService = "Lookupservice";

		public const string IdentityManagementServiceDesc = "VMware Identity Management Service";
		public const string LicenseServiceDesc = "VMware License Service";
		public const string AdminServiceDesc = "SSO Administration Server";
		public const string WebSsoServiceDesc = "Web SSO Server";
		public const string StsServiceDesc = "VMware Security Token Service";
		public const string CertificateServiceDesc = "VMware Certificate Service";
		public const string DirectoryServiceDesc = "VMware Directory Service";
        public const string LookupServiceDesc = "VMware Lookup Service";

		#endregion

        public static string GetHealthDescription()
        {
            return string.Format("FULL: Multiple healthy PSC nodes{0}LIMITED: Single healthy PSC node{0}DOWN: No healthy PSC node", Environment.NewLine);
        }
    }
}