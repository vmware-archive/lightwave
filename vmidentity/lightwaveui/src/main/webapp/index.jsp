<%@ page language="java" contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<html>
<head>
	<link rel="icon" href="app/assets/tenant.png">
	<link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css">
	<link rel="stylesheet" href="app/css/lightwave-ui.0.0.2.0.min.css">
	<link rel="stylesheet" href="app/css/lightwave-ui-vendor.0.0.2.0.min.css">
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
	<title>Lightwave Admin</title>
	<style>
	.headerLogo {
		margin-left:10px;
		border-radius: 3px;
		margin-right:5px;
	}
	
	.contentRow{
		margin-top:100px;
		width:900px;
	}
	
	.formRow {
		margin-top:40px;
		width:400px;
		margin-left:100px;
	}
	
	.margin-top-bottom-10{
		margin-top:10px;
		margin-bottom: 10px;
	}
	
	.caption {
		font-size: 24px;
	    line-height: 48px;
	    margin-top: 24px;
	    margin-bottom: 0;
	    font-weight: 400;
	}
	</style>
</head>
<body>
<div class="container">
	
		<div class="title">
		 	<img src="app/assets/vm_logo.png" height="32px;" width="32px" class="headerLogo"/>
			Lightwave Admin
			<sup><small>beta</small></sup>
		</div>
		<div class="row content contentRow">
			<div class="caption">Lightwave Admin</div><br/>
			<strong>Lightwave </strong> comprises of enterprise-grade, identity and access management services targeting critical security, governance & compliance challenges for Cloud-Native Apps within the enterprise.
			<br/><br/>
			Lightwave includes ...<br/><br/>
			<div><strong>Lightwave Authentication Services </strong><br/>A cloud authentication services with support for Kerberos, OAuth 2.0/OpenID Connect, SAML and WSTrust.</div><br/>
			<div><strong>Lightwave Directory Service</strong><br/>A standards based, multi-tenant, multi-master, highly scalable LDAP v3 directory service.</div><br/>
			<div><strong>Lightwave Certificate Authority </strong><br/>A directory integrated certificate authority across the infrastructure.</div><br/>
			<div><strong>Lightwave Certificate Store </strong><br/>An endpoint certificate store to store certificate credentials.</div>
		</div>
		<div class="row content formRow">
			<span>Enter the tenant you want to login to </span>
			<input type="text" id="tenant" class="form-control pull-left margin-top-bottom-10" 
				   placeholder="lightwave.local" /> 
			<button type="submit" class="btn btn-primary pull-right"
				    onclick="redirect()">Take me to Lightwave Admin ...</button>
		</div>
</div>
<script type="text/javascript">
function redirect(){
	var tenantName = document.getElementById('tenant').value;
	
	if(tenantName == null || tenantName == '')
		{
			alert('Enter a tenant name');
		}
	else
		{
			var server = getserver(location.href);
			var uri = "https://" + server + "/lightwaveui/Login?tenant=" + tenantName;
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