bin_SCRIPTS = \
    @top_builddir@/config/scripts/configure-lightwave-server \
    @top_builddir@/config/scripts/configure-identity-server \
    @top_builddir@/config/scripts/configure-sts \
    @top_srcdir@/config/scripts/domainjoin.sh

scriptsconf_SCRIPTS = \
 setfirewallrules.py \
 lw-firewall-server.json \
 lw-firewall-post.json \
 lw-firewall-client.json \
 lightwave-syslog-logrotate.conf

systemd_SCRIPTS = firewall.service

firewall.service:
	$(MKDIR) -p $(PUBLISH_DIR)/lib/systemd/system/
	$(CP) -prf $(builddir)/scripts/firewall.service $(PUBLISH_DIR)/lib/systemd/system/
