#!/opt/vmware/bin/python
# Copyright 2015 VMware, Inc.  All rights reserved.

"""
Helper function responsible for verifying certificate changed, backup certs and parsing certificate string
"""

__author__ = 'Charudath Gopal (cgopal@vmware.com)'
__copyright__ = 'Copyright 2015, VMware Inc.'
__version__ = 1.0



from cis.exceptions import *
from certificateManagerUtils import *
from certificateManagerOps import *
from utils import *
import re
import platform
import StringIO
import hashlib
import base64

vmca = VmcaOps()

def verify_cert_changed(vecs, serial_num_before, thumbprint_before, store, alias, server, is_root_cert=False):
    """
    Function to verify certificate has changed by comparing serial number and thumbprint
    :param vecs: VecsOps() object
    :param serial_num_before: Serial number of previous cert
    :param thumbprint_before: Thumbprint of previous cert
    :param store: VECS Store name which needs to be verified
    :param alias: Alias name of entry which needs to be verified
    :param server: Provide PSC/Infra IP in case of distributed else 'localhost'
    :param is_root_cert: Flag to indicate root certificate verification
    """
    if is_root_cert:
        serial_num_after, thumbprint_after = parse_cert(vmca.get_root_ca(server))
        store = 'RootCert'
    else:
        serial_num_after, thumbprint_after = parse_cert(vecs.get_cert(store, alias))

    # Verify cert changed
    try:
        logging.info('Serial number before replacement: ' + serial_num_before)
        logging.info('Serial number after replacement: ' + serial_num_after)

        logging.info('Thumbprint before replacement: ' + thumbprint_before)
        logging.info('Thumbprint after replacement: ' + thumbprint_after)

        if (not serial_num_after == serial_num_before) or (not thumbprint_after == thumbprint_before):
            logging.info('{0} certificate replaced successfully. SerialNumber and Thumbprint changed.'.format(store))
        else:
            raise Exception
    except Exception as e:
        msg = '{0} certificate replacement failed. SerialNumber and Thumbprint not changed after replacement, certificates are same before and after'.format(store)
        log_info_msg(msg)
        e.appendErrorStack(msg)
        logging.error(msg)
        raise e

def lstool_communicate(argv, stdout=subprocess.PIPE, stderr=subprocess.PIPE):
    """
    Lookup service client tool
    """
    cmd = [_get_java(),
           "-Djava.security.properties=%s" % _get_java_security_properties(),
           "-cp",
           _get_classpath(node_type),
           "-Dlog4j.configuration=tool-log4j.properties"]
    cmd.append("com.vmware.vim.lookup.client.tool.LsTool")
    cmd += argv
    process = subprocess.Popen(cmd, stdout=stdout, stderr=stderr)
    stdout, stderr = process.communicate(None)
    return process.returncode, stdout

def parse_cert(cert_string):
    """
    Function to parse cert and get serial number and thumbprint
    :param cert_string: Text representation of certificate which needs to be parsed
    :return: Returns Serial Number and Thumbprint
    """
    lines = str(cert_string).splitlines()
    serial_num = ''
    thumb_print = ''
    for line in lines:
        if 'Serial Number:' in line.strip():
            serial_num = line.split('Serial Number:')[1].strip()
            #In CISWIN serial number information will be on next line
            if serial_num == '':
                serial_num = lines[lines.index(line) + 1].strip()
        if 'X509v3 Subject Key Identifier:' in line:
            thumb_print = lines[lines.index(line) + 1].strip()
            break
    return serial_num, thumb_print

def _get_java():
    java_home = ""
    if os.environ.has_key('VMWARE_JAVA_HOME'):
        java_home = os.environ['VMWARE_JAVA_HOME']
    elif os.environ.has_key('JAVA_HOME'):
        java_home = os.environ['JAVA_HOME']
    else:
        raise Exception("ERROR: VMWARE_JAVA_HOME or JAVA_HOME not set in environment." +
                        " Please set to location of Java runtime and retry.")
    ext = ""
    if platform.system() == 'Windows':
        ext = ".exe"
    return "%s/bin/java%s" % (java_home, ext)

def _get_classpath(type):
    if os.name == 'posix':
        if (type == "embedded_node"):
            ls_libdir = "/usr/lib/vmidentity/tools/lib";
        elif (type == "management_node"):
            ls_libdir = "/usr/lib/vmware-sca/lib";
            common_jar_path = "/usr/lib/vmware/common-jars";
    else:
        cis_home = os.environ["VMWARE_CIS_HOME"]
        if (type == "embedded_node"):
            ls_libdir = os.path.join(cis_home, "VMware Identity Services\\lstool\\lib");
        elif (type == "management_node"):
            ls_libdir = os.path.join(cis_home, "sca\\lib");
            common_jar_path = os.path.join(cis_home, "common-jars");

    ls_common_dir = os.path.join(ls_libdir, "*")

    if (type == "management_node"):
        ls_common_dir += os.pathsep + os.path.join(common_jar_path, "*");

    return os.path.join(ls_libdir, "lookup-client.jar") + os.pathsep + ls_common_dir

def _get_java_security_properties():
    return os.path.join(os.environ['VMWARE_CFG_DIR'],
                        "java",
                        "vmware-override-java.security")

def _get_lsurl():
    """
    Function to get lookup service url.
    """
    cli = cli_path('vmafd-cli')
    cmd = [cli, 'get-ls-location', '--server-name', 'localhost']
    try:
        (code, result, err) = run_command(cmd, None, True)
    except InvokeCommandException as e:
        msg = 'Error in retrieving lookup service URL'
        e.appendErrorStack(msg)
        log_error_msg(msg)
        raise e
    return result.strip()

def get_cert_thumbprint(cert_path):
    """
    Function to get openssl sha1 thumbprint of old machine_ssl_cert
    """
    global oldthumbprint
    openssl_bin = os.environ["VMWARE_OPENSSL_BIN"]
    cmd = [openssl_bin, 'x509', '-in', cert_path, '-noout', '-sha1', '-fingerprint']
    try:
        (code, result, err) = run_command(cmd, None, True)
    except InvokeCommandException as e:
        msg = 'Error in retrieving thumbprint of MACHINE_SSL_CERT.'
        e.appendErrorStack(msg)
        log_error_msg(msg)
        raise e
    oldthumbprint = (result.strip()[17:])
    return oldthumbprint

def backup_all_certs(vecs, backup_dir):
    """
    Function to backup all certs to BACKUP_STORE and use it in case of restore/rollback operations
    :param backup_dir: temporary backup directory where files are saved,
        which will be deleted once entries are written to BACKUPS_TORE
    """
    #Backup Machine SSL Cert
    cert = backup_dir + Constants.MACHINE_SSL_STORE + Constants.BKP_CERT_EXT
    key = backup_dir + Constants.MACHINE_SSL_STORE + Constants.BKP_KEY_EXT
    vecs.get_cert_file(Constants.MACHINE_SSL_STORE, Constants.MACHINE_SSL_ALIAS, cert)
    vecs.get_key_file(Constants.MACHINE_SSL_STORE, Constants.MACHINE_SSL_ALIAS, key)
    vecs.entry_delete(Constants.BACKUP_STORE, 'bkp_' + Constants.MACHINE_SSL_ALIAS)
    vecs.entry_create(Constants.BACKUP_STORE, 'bkp_' + Constants.MACHINE_SSL_ALIAS, cert, key)
    remove_file(cert)
    remove_file(key)

    #Backup Solution User Certs
    for store in vecs._solution_user_stores:
        cert = backup_dir + store + Constants.BKP_CERT_EXT
        key = backup_dir + store + Constants.BKP_KEY_EXT
        vecs.get_key_file(store, store, key)
        vecs.get_cert_file(store, store, cert)
        vecs.entry_delete(Constants.BACKUP_STORE, 'bkp_' + store)
        vecs.entry_create(Constants.BACKUP_STORE, 'bkp_' + store, cert, key)
        remove_file(cert)
        remove_file(key)

def update_service_endpoints(type, user, password, cert_file, oldthumbprint):
    """
    Invoke lstool to update service registrations.
    """
    global node_type
    node_type = type
    modify_svc_ep_certs(_get_lsurl(), oldthumbprint, read_pem_cert(cert_file), user, password)

def read_pem_cert(filename):
    openssl_bin = os.environ["VMWARE_OPENSSL_BIN"]
    cmd = [openssl_bin, 'x509', '-in', filename]
    try:
        (code, result, err) = run_command(cmd, None, True)
    except InvokeCommandException as e:
        msg = "Error invalid PEM certificate file: '" + filename + "'"
        e.appendErrorStack(msg)
        log_error_msg(msg)
        raise e
    pat = "-----BEGIN CERTIFICATE-----([a-zA-Z0-9/+=\r\n]+)-----END CERTIFICATE-----"
    m = re.match(pat, result)
    if not m:
        raise Exception("Failed to parse as PEM file: '" + filename + "'")
    return m.group(1).replace("\n", "").replace("\r", "")

def _modify_ep_certs(inStm, outStm, oldThumbprint, newCert):
    update_ct = 0
    lines = _readProperties(inStm)
    oldThumbprint = oldThumbprint.replace(':', '').lower()

    #Parse lines and handle back slash. Handle ssltrust anchors separately
    for i, line in enumerate(lines):
        kvTuple = _parseProperyLine(line)
        if ("ssltrust" not in line) and kvTuple:
            lines[i] = _formatPropertyLine(kvTuple[0], kvTuple[1])

    for ep_i in range(0, 10):
        line, line_i = _findFirstMatch(lines, "endpoint" + str(ep_i) + ".url\s*=")
        if (line is None):
            break

        line, line_i = _findFirstMatch(lines, "endpoint" + str(ep_i) + ".ssltrust0?\s*=")
        if line is None:
            continue
        (key, value) = _parseProperyLine(line)
        h = hashlib.sha1()
        h.update(base64.decodestring(value))
        if (h.hexdigest().lower() == oldThumbprint):
            lines[line_i] = _formatPropertyLine(key, newCert)
            update_ct = update_ct + 1
    _writeProperties(outStm, lines)
    return update_ct

def modify_svc_ep_certs(lsUrl, oldThumbprint, newCert, username, password):
    print("Get site name")
    rc, siteId = lstool_communicate(["get-site-id",
                                           "--no-check-cert",
                                           "--url",
                                           lsUrl,
                                           ])
    if(rc != 0):
        raise Exception("'lstool get-site-id' failed: %d" % rc)

    siteId = siteId.replace("\n", "").replace("\r", "").lower()
    print (siteId)
    print ("Lookup all services")
    rc, ids = lstool_communicate(["list",
                                        "--no-check-cert",
                                        "--url",
                                        lsUrl,
                                        "--id-only",
                                        ])
    if(rc != 0):
        raise Exception("'lstool list' failed: %d" % rc)
    ids55 = []
    ids60 = []
    for id in ids.splitlines():
        if not id:
            continue
        if (id.lower().startswith(siteId + ':')):
            ids55.append(id)
        else:
            ids60.append(id)

    def _update_svc_spec(oldSpec, newSpec):
        update_ct = _modify_ep_certs(oldSpec, newSpec, oldThumbprint, newCert)
        return update_ct > 0

    update55_ct = _modify_svc_eps(lsUrl, True, ids55, _update_svc_spec, username, password)
    update60_ct = _modify_svc_eps(lsUrl, False, ids60, _update_svc_spec, username, password)
    print("Updated %d service(s)" % (update55_ct + update60_ct))

def _modify_svc_eps(lsUrl, is55, ids, fnUpdateSvcSpec, username, password):
    cmdSuffix = "55" if is55 else ""
    update_ct = 0

    #to remove LsTool stdout
    if 'Successfully run command' in ids:
        ids.remove('Successfully run command')

    for id in ids:
        if not id:
            continue

        print("Get service %s" % id)
        rc, oldSpec = lstool_communicate(["get" + cmdSuffix,
                                                "--no-check-cert",
                                                "--url",
                                                lsUrl,
                                                "--id",
                                                id,
                                                "--as-spec",
                                                ])
        if(rc != 0):
            raise Exception("'lstool get' failed: %d" % rc)

        svcUpdated = False
        with tempfile.NamedTemporaryFile(prefix='svcspec_', delete=False) as newSpec:
            svcUpdated = fnUpdateSvcSpec(StringIO.StringIO(oldSpec), newSpec)
            newSpec.flush()

        if svcUpdated:
            print("Update service %s; spec: %s" % (id, newSpec.name))
            rc, _ = lstool_communicate(["reregister" + cmdSuffix,
                                       "--no-check-cert",
                                       "--url",
                                       lsUrl,
                                       "--id",
                                       id,
                                       "--spec",
                                       newSpec.name,
                                       "--user",
                                       username,
                                       "--password",
                                       password,
                                       ])
            if(rc != 0):
                raise Exception("'lstool reregister' failed: %d" % rc)
            update_ct = update_ct + 1
        else:
            print ("Don't update service %s" % id)

        #Delete spec file if successful; keep it for troubleshooting if failed
        _remove_file(newSpec.name)

    return update_ct

def _readProperties(inStm):
    return inStm.readlines()

def _writeProperties(outStm, lines):
    for line in lines:
        if line.isspace():
            continue
        if not line.endswith('\n'):
            line = line + '\n'
        outStm.write(line)
    outStm.flush()

def _findFirstMatch(lines, pat):
    idx = 0
    for line in lines:
        if re.match(pat, line):
            return (line, idx)
            break
        idx = idx + 1
    return (None, -1)

def _parseProperyLine(line):
    pat = "\s*([^=]+)=\s*(.*)"
    m = re.match(pat, line)
    if m:
        return m.group(1, 2)
    return None

def _formatPropertyLine(key, value):
    return "%s=%s" % (key, _escapePropertyValue(value))

def _escapePropertyValue(value):
    return str(value).replace('\\', '\\\\')

def _remove_file(filename):
    try:
        os.remove(filename)
    except:
        pass