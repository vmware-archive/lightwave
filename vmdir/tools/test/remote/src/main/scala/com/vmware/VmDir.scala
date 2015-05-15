package com.vmware

import scala.sys.process._

object VmDir {
	private val base = "/usr/lib/vmware-vmdir/bin/"
	private val vdcpromo = base + "vdcpromo"

	def CreateTenant(user: String, password: String, tenant: String) = {
		val cmd = "%s -u %s -w %s -d %s -t".format(vdcpromo, user, password, tenant)
		val ret = cmd.!
		if (ret != 0) throw new Exception("%s [Error:%s]".format("CreateTenant", ret))
	}
}