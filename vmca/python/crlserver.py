#
# Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the “License”); you may not
# use this file except in compliance with the License.  You may obtain a copy
# of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an “AS IS” BASIS, without
# warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
from twisted.web import server, resource
from twisted.internet import reactor
import os

class CrlResource(resource.Resource):
	isLeaf = True
	numberRequests = 0
 	crlfile = "vmca.crl"
	crlfilepath="/var/lib/vmware/vmca/vmca.crl"	
	def render_GET(self, request):
		if (self.crlfile.lower() ==  os.path.basename(request.path).lower()):
			if os.path.exists(self.crlfilepath):
				request.setHeader("content-type", "application/octet-stream")
				with open(self.crlfilepath) as source:
					return source.read()
			else :
				request.setResponseCode(204) # No CRLs to Return
				return ""
		else :
			request.setResponseCode(400) # Anything other than vmca.crl is error
			return ""
reactor.listenTCP(21000, server.Site(CrlResource()))
reactor.run()
