<%@ page language="java" contentType="text/html; charset=UTF-8"
    pageEncoding="UTF-8"%>
<html>
<head>
<link rel="icon" href="app/assets/tenant.png">
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>Lightwave Admin</title>
<style>
body{
  background-repeat:no-repeat;
  min-height: 100%;
  min-width: 100%;
  margin: 0;
  width:100%;
  height:100%;
  font-size:14px;
  font-family:Arial, Helvetica, sans-serif;
  color: white;
  background: #3075ab; /* Old browsers */
  background: -moz-linear-gradient(top,  #3a8dc8 0%, #183a62 100%); /* FF3.6+ */
  background: -webkit-gradient(linear, left top, left bottom, color-stop(0%,#3a8dc8), color-stop(100%,#183a62)); /* Chrome,Safari4+ */
  background: -webkit-linear-gradient(top,  #3a8dc8 0%,#183a62 100%); /* Chrome10+,Safari5.1+ */
  background: -o-linear-gradient(top,  #3a8dc8 0%,#183a62 100%); /* Opera 11.10+ */
  background: -ms-linear-gradient(top,  #3a8dc8 0%,#183a62 100%); /* IE10+ */
  background: linear-gradient(to bottom,  #3a8dc8 0%,#183a62 100%); /* W3C */
  filter: progid:DXImageTransform.Microsoft.gradient( startColorstr='#3a8dc8', endColorstr='#183a62',GradientType=0 ); /* IE6-9 */
}

.container{
	margin-left:10%;
	padding-top:10%;
}

.bg-image{
  position:absolute;
  top: 0;
  left: 0;
  z-index: -1;
}
</style>
</head>
<body>
<img src="app/assets/bgpattern.png" class="bg-image"/>
<img src="app/assets/vmwarelogo.png" height="10px" style=margin-left:20px;margin-top:20px;"/>
<div class="container">
<div>
	<h1>
		Lightwave Admin
		<sup><small>beta</small></sup>
	</h1> <br/><br/><br/><br/>
	Enter the tenant you want to login to 
	<input type="text" id="tenant" style="margin:10px;" placeholder="lightwave.local" /> 
	<!-- <input type="text" id="clientId" style="margin:10px;" placeholder="OIDC client ID" />  -->
	<input type="submit" name="Login" value="Take me to Lightwave Admin ..." onclick="redirect()"/>
	<br/><br/><br/><br/>
	<p style="line-height:1.5em;max-width:900px;">
	<strong>Lightwave </strong> comprises of enterprise-grade, identity and access management services targeting critical security, governance, and compliance challenges for Cloud-Native Apps within the enterprise.
	<br/><br/>
		Lightwave is made up of the following key identity infrastructure elements:<br/>
		<ul style="line-height:1.5em;max-width:900px;">
			<li><strong>Lightwave Directory Service </strong>- standards based, multi-tenant, multi-master, highly scalable LDAP v3 directory service.</li>
			<li><strong>Lightwave Certificate Authority </strong>- directory integrated certificate authority across the infrastructure.</li>
			<li><strong>Lightwave Certificate Store </strong>- endpoint certificate store to store certificate credentials.</li>
			<li><strong>Lightwave Authentication Services </strong>- cloud authentication services with support for Kerberos, OAuth 2.0/OpenID Connect, SAML and WSTrust.</li>
		</ul>
	</p>
	</div>
</div>
<script type="text/javascript">
function redirect(){
	var tenantName = document.getElementById('tenant').value;
	
	if(tenantName == null || tenantName == '')
		{
			alert('Enter a tenant name: '+ tenantName );
		}
	else
		{
			/*var clientIdStr = document.getElementById('clientId').value;
			if(clientIdStr == null || clientIdStr == '')
			{
				alert('Enter a valid OIDC client Id: '+ clientIdStr );
			}
			*/
			var server = getserver(location.href);
			
			//var uri = "https://" + server + "/openidconnect/oidc/authorize/" + tenantName + "?response_type=id_token%20token&response_mode=form_post&client_id=" + clientIdStr + "&redirect_uri=https://" + server + "/lightwaveui/Home&state=_state_lmn_&nonce=_nonce_lmn_&scope=openid%20rs_admin_server";
			var uri = "https://" + server + "/lightwaveui/Login?tenant=" + tenantName;
			alert('Redirecting to server uri: '+ uri );
			window.location = uri;
		}
}

function getserver(uri){

    var server_uri = uri.split('//')[1];
    var server_with_port = server_uri.split('/')[0];
    var server_name = server_with_port.split(':')[0];
    return server_name;
}
</script>
</body>
</html>