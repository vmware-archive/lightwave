"""Lotus long haul tests with multimaster replication
"""
import os
import ast
import sys
import re
import copy
import random
import logging
import unittest
import json
import ldap
import ldap.modlist as modlist
import time
import threading
import subprocess
import pickle
import sqlite3
from datetime import datetime
from optparse import OptionParser

class TestOpCode:
    AddDn  , ModDn  , DelDn      = ['AddDn  ', 'ModDn  ', 'DelDn  ']
    AddAttr, ModAttr, DelAttr    = ['AddAttr', 'ModAttr', 'DelAttr']
    AddVal , ModVal , DelVal     = ['AddVal ', 'ModVal ', 'DelVal ']

_T2E = { # TestOpCode to execution type
    TestOpCode.AddDn   : 'C',
    TestOpCode.ModDn   : 'U',
    TestOpCode.DelDn   : 'D',
    TestOpCode.AddAttr : 'U',
    TestOpCode.ModAttr : 'U',
    TestOpCode.DelAttr : 'U',
    TestOpCode.AddVal  : 'U',
    TestOpCode.ModVal  : 'U',
    TestOpCode.DelVal  : 'U'
}

_T2V = { # TestOpCode to verification type
    TestOpCode.AddDn   : 'R',
    TestOpCode.ModDn   : 'R',
    TestOpCode.DelDn   : 'R',
    TestOpCode.AddAttr : 'R',
    TestOpCode.ModAttr : 'R',
    TestOpCode.DelAttr : 'R',
    TestOpCode.AddVal  : 'R',
    TestOpCode.ModVal  : 'R',
    TestOpCode.DelVal  : 'R'
}

STAT_COUNT_MIN = 100

def db_insert_testruns(timeStamp, numVMs, buildNumber, testOS, poolName, numBegin, vms,
                              nodeIds, initial_merge_seq, additional_merge_seq, topology, comment):
    records = [(timeStamp, numVMs, buildNumber, testOS, poolName, numBegin, vms,
                              nodeIds, initial_merge_seq, additional_merge_seq, topology, comment)]
    with db_conn:
        db_conn.executemany('INSERT INTO testruns VALUES (?,?,?,?,?,?,?,?,?,?,?,?)', records)

def db_insert_worksets(testRunId, batchNum, startTime, endTime, nodeName, opCode, count, duration):
    records = [(testRunId, batchNum, startTime, endTime, nodeName, opCode, count, duration)]
    with db_conn:
        db_conn.executemany('INSERT INTO worksets VALUES (?,?,?,?,?,?,?,?)', records)

def db_insert_perfsummaries(testRunId, type, startTime, endTime, batchNum, C, R, U, D, count, duration):
    records = [(testRunId, type, startTime, endTime, batchNum, C, R, U, D, count, duration)]
    with db_conn:
        db_conn.executemany('INSERT INTO perfsummaries VALUES (?,?,?,?,?,?,?,?,?,?,?)', records)

def datetime_2_str(dt_obj):
    "Convert datetime object to a compact string format: YYYYMMDDHHMMSS.SSSSSS"
    return format(dt_obj,'%Y%m%d%H%M%S.%f')

def timestamp_2_datetime(t_obj):
    "Convert a time obj to datetime obj"
    return datetime.fromtimestamp(t_obj)

def timestamp_2_str(t_obj):
    "Convert a time obj to a compact string format:YYYYMMDDHHMMSS.SSSSSS"
    return datetime_2_str(timestamp_2_datetime(t_obj))

def create_batch_for_node(opcode, nodeSeq, nodeTotal, nodeId, server_ip, protocol, port, bind_dn, bind_pwd, test_data, batch_range):
    """Create workset for a single node"""
    work_batch = []
    for i in [seqNum for seqNum in batch_range if seqNum % nodeTotal == nodeSeq]:
        workitem = create_1_work_item(opcode, nodeSeq, nodeId, server_ip, protocol, port, bind_dn, bind_pwd, test_data, i)
        work_batch.append(workitem)
    return work_batch

def create_1_work_item(opcode, nodeSeq, nodeId, server_ip, protocol, port, bind_dn, bind_pwd, test_data, suffix):
    """Create 1 workitem for a single node, append nodeId and suffix to each workitem entry"""
    # prepare data for entry
    bdn    = test_data["baseDN"]
    prefix = test_data["rdn"]["prefix"]
    entry_name   = '%s_%s_%07d' % (prefix, nodeId, suffix)
    entry_dn     = 'cn=%s,%s' % (entry_name, bdn)
    server_uri   = '%s://%s:%s' % (protocol, server_ip, port)
    # create work item
    workitem = {}
    workitem["opcode"]       = opcode
    workitem["bind_dn"]      = bind_dn
    workitem["bind_pwd"]     = bind_pwd
    workitem["server_uri"]   = server_uri
    workitem["entry_dn"]     = entry_dn
    # add details for specific operaton
    if opcode==TestOpCode.AddDn or opcode==TestOpCode.AddAttr:
        attrs = copy.deepcopy(test_data["attrs"])
        for attr in attrs.itervalues():
            attr[0] = str("%s_%s_%07d" % (attr[0], nodeId, suffix))
        attrs_const = copy.deepcopy(test_data["attrs_const"])
        attrs.update(attrs_const)
        workitem["AddDn_Attrs"] = attrs
    if opcode==TestOpCode.AddAttr:
        attrs_add = copy.deepcopy(test_data["attrs_add"])
        for attr in attrs_add.itervalues():
            attr[0] = str("%s_%s_%d" % (attr[0], nodeId, suffix))
        workitem["AddAttr_Attrs"] = attrs_add
    elif opcode==TestOpCode.DelDn:
        pass
    return workitem

def execute_1_work_item(opcode, server_uri, bind_dn, bind_pwd, entry_dn, attrs):
    try:
        if opcode==TestOpCode.AddDn:
            ldap_add_s(server_uri, bind_dn, bind_pwd, entry_dn, attrs)
        elif opcode==TestOpCode.AddAttr:
            ldap_mod_s(server_uri, bind_dn, bind_pwd, entry_dn, attrs)
        elif opcode==TestOpCode.DelDn:
            ldap_delete_s(server_uri, bind_dn, bind_pwd, entry_dn)
    # work-around for openldap error: txn_commit fail.
    except ldap.OTHER, e:
        report_print("Error: %r, %r, %r, %r, %r, %r, %r" % ( opcode, server_uri, bind_dn, bind_pwd, entry_dn, attrs, e))
        time.sleep(5)
    # work-around for openldap Read failure.
    except ldap.SERVER_DOWN, e:
        report_print("Error: %r, %r, %r, %r, %r, %r, %r" % ( opcode, server_uri, bind_dn, bind_pwd, entry_dn, attrs, e))
        wait_for_server_up()

def failUnlessEqual(first, second, msg=None):
    if not first == second:
        raise AssertionError, (msg or '%r != %r' % (first, second))

def failUnlessRaises(excClass, callableObj, *args, **kwargs):
    """Fail unless an exception of class excClass is thrown
       by callableObj when invoked with arguments args and keyword
       arguments kwargs. If a different type of exception is
       thrown, it will not be caught, and the test case will be
       deemed to have suffered an error, exactly as for an
       unexpected exception.
    """
    try:
        callableObj(*args, **kwargs)
    except excClass:
        return
    else:
        if hasattr(excClass,'__name__'): excName = excClass.__name__
        else: excName = str(excClass)
        raise Exception("%s not raised" % excName)

assertEqual = assertEquals = failUnlessEqual
assertRaises = failUnlessRaises

def verify_1_work_item(opcode, server_ip, protocol, port, bind_dn, bind_pwd, entry_dn, attrs):
    success = True
    server_uri  = '%s://%s:%s' % (protocol, server_ip, port)
    if opcode==TestOpCode.AddDn or opcode==TestOpCode.AddAttr:
        try:
            result = ldap_search_s(server_uri, bind_dn, bind_pwd, entry_dn, ldap.SCOPE_BASE)
        except (ldap.NO_SUCH_OBJECT) as e:
            success = False
            report_print("FAILED:%r: %r, %r, %r, %r, %r" % ( opcode, server_ip, bind_dn, bind_pwd, entry_dn, e))
        except (ldap.SERVER_DOWN) as e:
            # work-around for openldap Read failure.
            success = False
            report_print("FAILED:%r: %r, %r, %r, %r, %r" % ( opcode, server_ip, bind_dn, bind_pwd, entry_dn, e))
            wait_for_server_up()
        else:
            assertEqual(len(result), 1)
            assertEqual(len(result[0]), 2)
            assertEqual(result[0][0].lower(), entry_dn.lower())
            # no verification on AD
            # assertEqual(dict_sort(result[0][1]), dict_sort(attrs))
    elif opcode==TestOpCode.DelDn:
        #assertRaises(ldap.NO_SUCH_OBJECT, ldap_delete_s, server_uri, bind_dn, bind_pwd, entry_dn)
        try:
            ldap_delete_s(server_uri, bind_dn, bind_pwd, entry_dn)
        except (ldap.NO_SUCH_OBJECT) as e:
            pass
        except (ldap.SERVER_DOWN) as e:
            # work-around for openldap Read failure.
            success = False
            report_print("FAILED:%r: %r, %r, %r, %r, %r" % ( opcode, server_ip, bind_dn, bind_pwd, entry_dn, e))
            wait_for_server_up()
        else:
            success = False
            report_print("FAILED:%r: %r, %r, %r, %r" % ( opcode, server_uri, bind_dn, bind_pwd, entry_dn ))
    return success

def execute_batch_on_node(batch_num, workset, nodeId):
    execution_report = []
    work_batch = workset[nodeId]
    msgs = []
    msgs.append('----------------------------------------------\n')
    msgs.append('Execute-%s start: %s\n' % (work_batch[0]["opcode"], work_batch[0]["entry_dn"] ))
    msgs.append('Execute-%s end:   %s\n' % (work_batch[0]["opcode"], work_batch[-1]["entry_dn"]))
    msgs.append('Execute-%s count: %d\n' % (work_batch[0]["opcode"], len(work_batch)))
    if len(work_batch) >= STAT_COUNT_MIN:
        report_print(''.join(msgs))
    stats  = { 'C':0, 'R':0, 'U':0, 'D':0 }
    t_begin = time.time()
    for i, workitem in enumerate(work_batch):
        attrs       = {}
        opcode      = workitem["opcode"]
        server_uri  = workitem["server_uri"]
        bind_dn     = workitem["bind_dn"]
        bind_pwd    = workitem["bind_pwd"]
        entry_dn    = workitem["entry_dn"]
        if opcode==TestOpCode.AddDn:
            attrs = workitem["AddDn_Attrs"]
            stats['C'] += 1
        elif opcode==TestOpCode.AddAttr:
            attrs = workitem["AddAttr_Attrs"]
            stats['U'] += 1
        elif opcode==TestOpCode.DelDn:
            stats['D'] += 1
        else:
            raise ValueError, 'opcode is not recognized in workitem: %s' % (str(workitem))
        #report_print("%r, %r, %r, %r, %r, %r"  %(opcode, server_uri, bind_dn, bind_pwd, entry_dn, attrs))
        execute_1_work_item(opcode, server_uri, bind_dn, bind_pwd, entry_dn, attrs)
    t_end = time.time()
    t_elapsed = t_end - t_begin
    # Add database entry
    if len(work_batch) >= STAT_COUNT_MIN:
        db_insert_worksets(test_run_id, batch_num, timestamp_2_str(t_begin), timestamp_2_str(t_end),
                            nodeId, _T2E[opcode], stats[_T2E[opcode]], t_elapsed)
        msg = 'It takes %8.2f seconds to execute on node %s: [C:%d R:%d U:%d D:%d]\n' % (
            t_elapsed, nodeId, stats['C'], stats['R'], stats['U'], stats['D'] )
        report_print(msg)
        execution_report.append(msg)
    return execution_report, stats

def verify_batch_on_other_node(batch_num, workset, nodeSeq, nodeId, nodeIds, test_vms, protocol, port):
    verification_report = []
    work_batch = workset[nodeId]
    msgs = []
    msgs.append('----------------------------------------------\n')
    msgs.append('Verify-%s start: %s\n' % (work_batch[0]["opcode"], work_batch[0]["entry_dn"] ))
    msgs.append('Verify-%s end:   %s\n' % (work_batch[0]["opcode"], work_batch[-1]["entry_dn"]))
    msgs.append('Verify-%s count: %d\n' % (work_batch[0]["opcode"], len(work_batch)))
    if len(work_batch) >= STAT_COUNT_MIN:
        report_print(''.join(msgs))
    # find a random server to verify the results on
    min = 0
    max = len(nodeIds) - 1
    if max > min:
        rand = random.randint(min, max)
        while rand == nodeSeq:
            rand = random.randint(min, max)
    else:
        rand = min
    test_nodeId = nodeIds[rand]
    server_ip   = test_vms[test_nodeId]
    stats  = { 'C':0, 'R':0, 'U':0, 'D':0 }
    succs  = { 'C':0, 'R':0, 'U':0, 'D':0 }
    fails  = { 'C':0, 'R':0, 'U':0, 'D':0 }
    t_begin = time.time()
    for i, workitem in enumerate(work_batch):
        attrs       = {}
        opcode      = workitem["opcode"]
        server_uri  = workitem["server_uri"]
        bind_dn     = workitem["bind_dn"]
        bind_pwd    = workitem["bind_pwd"]
        entry_dn    = workitem["entry_dn"]
        if opcode==TestOpCode.AddDn:
            attrs = workitem["AddDn_Attrs"]
            stats['R'] += 1 # verify is always a "read" operation.
        if opcode==TestOpCode.AddAttr:
            attrs = workitem["AddDn_Attrs"]
            attrs.update(workitem["AddAttr_Attrs"])
            stats['R'] += 1 # verify is always a "read" operation.
        elif opcode==TestOpCode.DelDn:
            stats['R'] += 1 # verify is always a "read" operation.
        success = verify_1_work_item(opcode, server_ip, protocol, port, bind_dn, bind_pwd, entry_dn, attrs)
        if success:
            succs[_T2E[opcode]] += 1
        else:
            fails[_T2E[opcode]] += 1
    t_end = time.time()
    t_elapsed = t_end - t_begin
    # Add database entry if verifying read happened
    if stats['R'] >= STAT_COUNT_MIN:
        db_insert_worksets(test_run_id, batch_num, timestamp_2_str(t_begin), timestamp_2_str(t_end),
                            test_nodeId, 'R', stats['R'], t_elapsed)
        msg = 'It takes %8.2f seconds to verify  on node %s: [C:%d R:%d U:%d D:%d]\n' % (
            t_elapsed, test_nodeId, stats['C'], stats['R'], stats['U'], stats['D'] )
        msg+= '                                         succ : [C:%d R:%d U:%d D:%d]\n' % (
            succs['C'], succs['R'], succs['U'], succs['D'] )
        msg+= '                                         fail : [C:%d R:%d U:%d D:%d]\n' % (
            fails['C'], fails['R'], fails['U'], fails['D'] )
        report_print(msg)
        verification_report.append(msg)
    return verification_report, stats

def append_to_debug_log(message):
    with open('debug.log','a+') as f:
        f.write(message+'\n')

def report_print(message):
    """Print messages that need to be in the final report
    """
    m = message.lstrip('\n').rstrip('\n')
    pad = '[REPORT][%s]: ' % datetime.now()
    m1 = m.replace('\n', '\n%s' %pad)
    m2 =  '%s%s' % (pad, m1)
    print m2
    append_to_debug_log(m2)

def report_build_number(buildNumber):
    report_print('Build number: %s' % buildNumber)

def report_vm_names(vms):
    report_print('Test VMs created: ')
    report_print('{')
    keys = dict_keys_sorted(vms)
    for key in keys[0:-1]:
        report_print('"%s":"%s",' %(key, vms[key]))
    key = keys[-1]
    report_print('"%s":"%s"' %(key, vms[key]))
    report_print('}')

def report_vm_topology(topology):
    """topology should be a string already formated for printing
    """
    report_print('Test VMs topology setup is:\n%s' % topology)

def runCmd(args):
    report_print ('running %s' % args)
    p = subprocess.Popen(args,
                      stdout=subprocess.PIPE,
                      stderr=subprocess.STDOUT)
    if p.returncode:
        report_print (p.communicate()[0].rstrip())
    return p

class FuncThread(threading.Thread):
    def __init__(self, target, *args):
        self._target = target
        self._args = args
        threading.Thread.__init__(self)

    def run(self):
        self._target(*self._args)

def run_ssh_command_without_password(username, host, command):
    uri = '%s@%s' % (username, host)
    #print "ssh ran command %s on %s@%s" % (command, username, host)
    p = runCmd(['ssh', uri, command])
    output = p.communicate()[0].rstrip()
    report_print(output)
    if p.returncode:
        raise Exception('Failed to run command %s on host %s\n%s' % (command, host, output))
    return output

def run_cmd_on_pdc(command, pdcUser='pgu', pdcHost='pa-dbc1025.eng.vmware.com'):
    """Run a command on a machine in Palo-alto data center
    """
    output = run_ssh_command_without_password(pdcUser, pdcHost, command)
    return output

def create_cloud_vm(vmName, buildNumber, poolName):
    """Create cloudvm on a NIMBUS cluster
    'vmName' is required. For example, vmName='pgu-repTest-01'
    'buildNumber' should be kept up-to-date, e.g., buildNumber='sb-1549829'
    """
    command = 'NIMBUS=%s /mts/git/bin/nimbus-vcvadeploy --lease 100 -v %s %s' % (poolName, buildNumber, vmName)
    output = run_cmd_on_pdc(command)
    #print output
    return vmName

def create_vcenter_windows(vmName, buildNumber,poolName):
    """Create vcenterWindows VM on a NIMBUS cluster
    'vmName' is required. For example, vmName='pgu-repTest-01'
    'buildNumber' should be kept up-to-date, e.g., buildNumber='sb-1549829'
    """
    command = 'NIMBUS=%s /mts/git/bin/nimbus-vcdeploy-cat --lease 100 --vcenterwindowsBuild %s %s' % (poolName, buildNumber, vmName)
    output = run_cmd_on_pdc(command)
    print output
    return vmName

def get_vm_ip(vmName, poolName):
    """Get IP address for a VM on a NIMBUS cluster
    'vmName' is required. For example, vmName='pgu-repTest-01'
    """
    command = 'NIMBUS=%s /mts/git/bin/nimbus-ctl ip %s' % (poolName, vmName)
    output = run_cmd_on_pdc(command)
    print 'Got IP address on %s for %s: %s' % (poolName, vmName, output)
    return output

def create_test_vms_on_nimbus(buildNumber, testOS, poolName, numVMs, numBegin=0, doSetup=False):
    cloudVms = {}
    vm_create_threads = []
    if testOS == "Windows":
        raise Exception("Test OS type [%s] not supported!!!" % testOS)
    elif testOS == "Linux":
        for num in xrange(numVMs):
            vmName = "pgu-repTest-%03d" % (num+numBegin)
            if doSetup:
                t = FuncThread(create_cloud_vm, vmName, buildNumber, poolName)
                vm_create_threads.append(t)
                print '%s is being created on %s' % (vmName, poolName)
                t.start()
        for num in xrange(numVMs):
            vmName = "pgu-repTest-%03d" % (num+numBegin)
            if doSetup:
                t = vm_create_threads.pop()
                t.join()
                print '%s has been created on %s' % (vmName, poolName)
            vmIp = get_vm_ip(vmName, poolName)
            cloudVms[vmName] = vmIp
            print '%s:%s' % (vmName, vmIp)
        print "All cloud vms are created successfully!"
        print cloudVms
        return cloudVms
    else:
        raise Exception("Test OS type [%s] not supported!!!" % testOS)

def wait_for_server_up():
    time.sleep(30)

def ldap_init(ldap_server_uri):
    l = ldap.ldapobject.ReconnectLDAPObject(ldap_server_uri,trace_level=0,retry_max=30,retry_delay=1.0)
    return l

def ldap_add_s(ldap_server_uri, bind_dn, bind_pwd, entry_dn, attrs):
    #return # TODO: remove this line and the other line says: return True in verify_1_work_item
    try:
        l = ldap_init(ldap_server_uri)
        l.simple_bind_s(bind_dn, bind_pwd)
        ldif = modlist.addModlist(attrs)
        l.add_s(entry_dn, ldif)
    except (ldap.ALREADY_EXISTS, ldap.INVALID_CREDENTIALS, ldap.INVALID_SYNTAX, ldap.LDAPError) as e:
        report_print ( "%r, %r, %r, %r, %r, %r" % ( ldap_server_uri, bind_dn, bind_pwd, entry_dn, attrs, e) )
        raise
    finally:
        if l is not None: l.unbind_s()

def ldap_mod_s(ldap_server_uri, bind_dn, bind_pwd, entry_dn, attrs):
    try:
        l = ldap_init(ldap_server_uri)
        l.simple_bind_s(bind_dn, bind_pwd)
        mod_attrs = [(ldap.MOD_ADD, attr, attrs[attr]) for attr in attrs.keys()]
        l.modify_s(entry_dn, mod_attrs)
    except (ldap.ALREADY_EXISTS, ldap.INVALID_CREDENTIALS, ldap.INVALID_SYNTAX, ldap.LDAPError) as e:
        report_print ( "%r, %r, %r, %r, %r, %r" % ( ldap_server_uri, bind_dn, bind_pwd, entry_dn, attrs, e) )
        raise
    finally:
        if l is not None: l.unbind_s()

def ldap_search_s(ldap_server_uri, bind_dn, bind_pwd, base_dn, scope):
    result = []
    try:
        l = ldap_init(ldap_server_uri)
        l.simple_bind_s(bind_dn, bind_pwd)
        result = l.search_s(base_dn, scope)
    except (ldap.NO_SUCH_OBJECT, ldap.INVALID_CREDENTIALS, ldap.INVALID_SYNTAX, ldap.LDAPError) as e:
        report_print ( "%r, %r, %r, %r, %r, %r" % ( ldap_server_uri, bind_dn, bind_pwd, base_dn, scope, e) )
        raise
    finally:
        if l is not None: l.unbind_s()
    return result

def ldap_delete_s(ldap_server_uri, bind_dn, bind_pwd, delete_dn):
    try:
        l = ldap_init(ldap_server_uri)
        l.simple_bind_s(bind_dn, bind_pwd)
        l.delete_s(delete_dn)
    except (ldap.NO_SUCH_OBJECT, ldap.INVALID_CREDENTIALS, ldap.LDAPError) as e:
        report_print ("%r, %r, %r, %r, %r" % ( ldap_server_uri, bind_dn, bind_pwd, delete_dn, e))
        raise
    except (ldap.OPERATIONS_ERROR) as e:
        report_print ("%r, %r, %r, %r, %r" % ( ldap_server_uri, bind_dn, bind_pwd, delete_dn, e))
        #raise  # workaround for MDB_PAGE_FULL
    finally:
        if l is not None: l.unbind_s()

def change_user_password(ldap_server_uri, bind_dn, bind_pwd, user_dn, new_password):
    """Change the password for a user
    """
    PASSWORD_ATTR = "unicodePwd"
    # LDAP connection
    try:
        l = ldap_init(ldap_server_uri)
        l.simple_bind_s(bind_dn, bind_pwd)
    except ldap.LDAPError, error_message:
        raise ldap.LDAPError, "Error connecting to LDAP server: %s" % error_message
    # Prep the password
    unicode_pass = unicode('\"' + new_password + '\"', 'iso-8859-1')
    password_value = unicode_pass.encode('utf-16-le')
    add_pass = [(ldap.MOD_REPLACE, PASSWORD_ATTR, [password_value])]
    # Change the password
    try:
        l.modify_s(user_dn, add_pass)
    except ldap.LDAPError, error_message:
        raise ldap.LDAPError, "Error setting password: %s" % error_message
    # LDAP unbind
    l.unbind_s()

def create_user(ldap_server_uri, bind_dn, bind_pwd, username, password, base_dn, group_dn, fname, lname, domain, employee_id):
    """Create a new user account in lotus
    """
    PASSWORD_ATTR = "unicodePwd"
    # LDAP connection
    try:
        l = ldap_init(ldap_server_uri)
        l.simple_bind_s(bind_dn, bind_pwd)
    except ldap.LDAPError, error_message:
        raise ldap.LDAPError, "Error connecting to LDAP server: %s" % error_message
    # Check and see if user exists
    try:
        user_results = l.search_s(base_dn, ldap.SCOPE_SUBTREE,
                                '(&(sAMAccountName=' +
                                username +
                                ')(objectClass=person))',
                                ['distinguishedName'])
    except ldap.LDAPError, error_message:
        raise ldap.LDAPError, "Error finding username: %s" % error_message
    # Check the results
    if len(user_results) != 0:
        raise ldap.LDAPError, "User %s already exists in lotus:" % username
    # Lets build our user: Disabled to start (514)
    user_dn = 'cn=' + username + ',' + base_dn
    user_attrs = {}
    user_attrs['objectClass'] = \
              ['top', 'person', 'organizationalPerson', 'user']
    user_attrs['cn'] = fname + ' ' + lname
    user_attrs['userPrincipalName'] = username + '@' + domain
    user_attrs['sAMAccountName'] = username
    user_attrs['givenName'] = fname
    user_attrs['sn'] = lname
    user_attrs['displayName'] = fname + ' ' + lname
    user_attrs['userAccountControl'] = '514'
    user_attrs['mail'] = username + '@' + domain
    user_attrs['employeeID'] = employee_id
    user_attrs['homeDirectory'] = '\\\\server\\' + username
    user_attrs['homeDrive'] = 'H:'
    user_attrs['scriptPath'] = 'logon.vbs'
    user_ldif = modlist.addModlist(user_attrs)
    # Prep the password
    unicode_pass = unicode('\"' + password + '\"', 'iso-8859-1')
    password_value = unicode_pass.encode('utf-16-le')
    add_pass = [(ldap.MOD_REPLACE, PASSWORD_ATTR, [password_value])]
    # 512 will set user account to enabled
    mod_acct = [(ldap.MOD_REPLACE, 'userAccountControl', '512')]
    # New group membership
    add_member = [(ldap.MOD_ADD, 'member', user_dn)]
    # Add the new user account
    try:
        l.add_s(user_dn, user_ldif)
    except ldap.LDAPError, error_message:
        raise ldap.LDAPError, "Error adding new user: %s" % error_message
    # Add the password
    try:
        l.modify_s(user_dn, add_pass)
    except ldap.LDAPError, error_message:
        raise ldap.LDAPError, "Error setting password: %s" % error_message
    # Change the account back to enabled
    try:
        l.modify_s(user_dn, mod_acct)
    except ldap.LDAPError, error_message:
        raise ldap.LDAPError, "Error enabling user: %s" % error_message
    # Add user to their primary group
    try:
        l.modify_s(group_dn, add_member)
    except ldap.LDAPError, error_message:
        raise ldap.LDAPError, "Error adding user to group: %s" % error_message
    # LDAP unbind
    l.unbind_s()
    # All is good
    return user_dn

def _decode_list(data):
    rv = []
    for item in data:
        if isinstance(item, unicode):
            item = item.encode('utf-8')
        elif isinstance(item, list):
            item = _decode_list(item)
        elif isinstance(item, dict):
            item = _decode_dict(item)
        rv.append(item)
    return rv

def _decode_dict(data):
    rv = {}
    for key, value in data.iteritems():
        if isinstance(key, unicode):
           key = key.encode('utf-8')
        if isinstance(value, unicode):
           value = value.encode('utf-8')
        elif isinstance(value, list):
           value = _decode_list(value)
        elif isinstance(value, dict):
           value = _decode_dict(value)
        rv[key] = value
    return rv

def dict_sort(mydict):
    """Sort a dictionary by key and return as an array of sorted list
    """
    return ["%s:%s" % (key, mydict[key]) for key in sorted(mydict.iterkeys())]

def dict_keys_sorted(mydict):
    """Return all the keys as a sorted list
    """
    return ["%s" % (key) for key in sorted(mydict.iterkeys())]

def dict_vals_sorted(mydict):
    """Sort by key and return all the corresponding values in that order
    """
    return ["%s" % (mydict[key]) for key in sorted(mydict.iterkeys())]

def get_function_name(skip_num=0):
    return sys._getframe(skip_num+1).f_code.co_name

def update_progress(progress):
    sys.stdout.write( '\r%d%%' % progress )
    sys.stdout.flush()

def domain_dn_2_name(dn):
    """Example:
    Input" "dc=vsphere,dc=local"
    Output: "vsphere.local"
    """
    domain_name = dn.replace(" ","").replace("dc=","").replace(",",".")
    return str(domain_name)

class vdcreptest_helper:
    """Handles cmdline parameter parsing and logging/debugging
    """

    def error_print(self, message):
        self.logger.error(message)
        #print '[%s][ERROR]:%s' % (datetime.now(), message)

    def warn_print(self, message):
        self.logger.warn(message)
        #print '[%s][WARN]:%s' % (datetime.now(), message)

    def verbose_print(self, message):
        self.logger.info(message)
        #if self.options.verbose:
            #print message
        #print '[%s][INFO]:%s' % (datetime.now(), message)

    def debug_print(self, message):
        self.logger.debug("[%s]:%s" % (get_function_name(1), message))
        #if self.options.debug:
        #print '\n[%s][DEBUG][%s]:%s' % (datetime.now(), get_function_name(1), message)

    def perf_print(self, message):
        self.logger.info(message)
        #print '[%s][PERF]:%s' % (datetime.now(), message)

    def __init__(self):
        self.parse_options()
        self.load_config()
        self.log_init()
        self.verbose_print( "Log file name = %s" % self.options.logFile)
        self.verbose_print( "Command line arguments:" )
        self.verbose_print( "\nINFO\t" + str(self.options).strip('{}').replace("'","").replace(': ',' = ').replace(',','\nINFO\t'))
        self.verbose_print( "Suggested usage: python %s -v -n" % sys.argv[0] )
        # setup VMs on Nimbus, overloads default VMs info in config file
        if self.options.doSetup:
            self.setup_vms()
        elif self.options.doMerge:
            self.merge_vms()
        # output VMs Info
        #self.verbose_print("Test Vms = ")
        #self.verbose_print(json.dumps(self.test_vms, sort_keys=True, indent=2))
        #self.verbose_print("VM topology %s\n" % self.topology)
        #self.debug_print(json.dumps(self.cfg, sort_keys=True, indent=2))
        # report key test factors
        comment = self.options.comment
        db_insert_testruns (test_run_id, self.numVMs, self.buildNumber, self.testOS, self.poolName,
                            self.numBegin, str(self.test_vms), str(self.nodeIds),
                            str(self.initial_merge_seq), str(self.additional_merge_seq), str(self.topology), comment)
        report_build_number(self.buildNumber)
        report_vm_names(self.test_vms)
        report_vm_topology(self.topology)

    def parse_options(self):
        """
        Parse command line options.
        If a config file name is specified, command line options overrides config file options.
        """
        parser = OptionParser()
        parser.add_option("-f","--cfgFile",dest="cfgFile",default="vdcreplication_test.data",action="store",help="config file")
        parser.add_option("-s","--setup",dest="doSetup",default=False,action="store_true",help="Do setup")
        parser.add_option("-m","--merge",dest="doMerge",default=False,action="store_true",help="Do merge")
        parser.add_option("-c","--cleanup",dest="doCleanup",default=False,action="store_true",help="Do cleanup")
        parser.add_option("-C","--comment",dest="comment",default="",action="store",help="comment for this testrun")
        parser.add_option("-n","--notrun",dest="notRun",default=False,action="store_true",help="Don't really run anything")
        parser.add_option("-v","--verify",dest="doVerify",default=False,action="store_true",help="Verify")
        parser.add_option("-V","--verbose",dest="verbose",default=False,action="store_true",help="Verbose")
        parser.add_option("-D","--debug",dest="debug",default=False,action="store_true",help="Print debug info")
        parser.add_option("-l","--logFile",dest="logFile",default="/dev/null",action="store",help="log file")
        parser.add_option("-L","--logLevel",dest="logLevel",default="DEBUG",action="store",help="logging level")
        parser.add_option("-b","--buildNum",dest="buildNum",default="sb-1549829",action="store",help="sandbox build number")
        parser.add_option("-o","--testOS",dest="testOS",default="Linux",action="store",help="either Linux or Windows")
        (self.options, self.args) = parser.parse_args()

    def load_config(self):
        """ get OS specific constants dictionary
        """
        # load test config data, convert everyting to ASCII
        with open(self.options.cfgFile,"r") as f:
            da = json.load(f, object_hook=_decode_dict)
        self.cfg = copy.deepcopy(da)
        # test case related const
        self.test_const = self.cfg["const"]["test"]
        self.admin_user_name = 'cn=%s,cn=Users,%s' % ( self.test_const["lotus_admin_username"],
                                                     self.test_const["lotus_domain_dn"] )
        self.admin_password = self.test_const["lotus_admin_password"]
        self.lotus_domain_name = domain_dn_2_name(self.test_const["lotus_domain_dn"])
        # test VM related const
        self.opCodes          = self.cfg["opCodes"]
        self.numVMs           = self.cfg["numVMs"]
        numStr = "%d" % self.numVMs

        if not self.options.doSetup: # Even if doMerge, our new config should still be valid, so use new, not old.
            self.buildNumber  = self.cfg[numStr]["0-buildNumber"]
            self.testOS       = self.cfg[numStr]["0-testOS"]
            self.poolName     = self.cfg[numStr]["0-poolName"]
            self.numBegin     = self.cfg[numStr]["0-numBegin"]
            self.test_vms     = self.cfg[numStr]["0-vms"]
        else:
            self.buildNumber  = self.cfg[numStr]["buildNumber"]
            self.testOS       = self.cfg[numStr]["testOS"]
            self.poolName     = self.cfg[numStr]["poolName"]
            self.numBegin     = self.cfg[numStr]["numBegin"]
            self.test_vms     = self.cfg[numStr]["vms"]
        self.nodeIds      = self.cfg[numStr]["nodeIds"]
        self.initial_merge_seq    = self.cfg[numStr]["initial_merge_seq"]
        self.additional_merge_seq = self.cfg[numStr]["additional_merge_seq"]
        self.topology     = "\n".join(self.cfg[numStr]["topology"])
        self.sleeptime    = self.cfg[numStr]["sleeptime"]

        # test os specific const
        if self.testOS == 'Windows':
            self.os_const = self.cfg["const"]["os"]["win32"]
        else:
            self.os_const = self.cfg["const"]["os"]["linux"]

        # special case for AD, OpenLdap and local Lotus testing
        if numStr=="1":
            testTarget = self.cfg["testTarget"]
            if testTarget in ["AD"] :
                self.admin_password = self.cfg['const']['os']['win32']['root_login_password']
            if testTarget in ["AD", "OpenLdap", "LotusLinux","LotusWindows"] :
                self.options.doSetup = False
                self.buildNumber  = self.cfg[numStr][testTarget]["buildNumber"]
                self.testOS       = self.cfg[numStr][testTarget]["testOS"]
                self.poolName     = self.cfg[numStr][testTarget]["poolName"]
                self.numBegin     = self.cfg[numStr][testTarget]["numBegin"]
                self.test_vms     = self.cfg[numStr][testTarget]["vms"]

    def log_init(self):
        logger = logging.getLogger()
        logger.setLevel(logging._levelNames[self.options.logLevel])
        # log format
        formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
        #formatter = logging.Formatter('%(asctime)s - LOG - %(message)s')
        # Output to file
        fh = logging.FileHandler(self.options.logFile)
        fh.setLevel(logger.level)
        fh.setFormatter(formatter)
        logger.addHandler(fh)
        # Output to screen
        #ch = logging.StreamHandler()
        #ch.setLevel(logger.level)
        #ch.setFormatter(formatter)
        #logger.addHandler(ch)
        self.logger = logger

    def run_ssh_command_on_remote_host(self, host, username, password, command):
        """Use myssh.tcl to run a command remotely.
        Internally expect language is used.
        No return value can be retried.
        """
        cmdline = './myssh.tcl %s %s %s "%s"' % ( host, username, password, command)
        self.verbose_print( cmdline )
        self.run_command(cmdline)

    def run_command(self, cmdline):
        import commands
        if not self.options.notRun:
            status, output = commands.getstatusoutput(cmdline)
            m = '%s returned %d\nOutput:\n%s' % (cmdline, status, output)
            report_print(m)

    def _test_vm_name_to_id(self):
        vm_names = dict_keys_sorted(self.test_vms_names)
        vms = {}
        for i in xrange(0, len(vm_names)):
            vms[self.nodeIds[i]] = self.test_vms_names[vm_names[i]]
        return vms

    def setup_vms(self):
        """Setup 2-6 CloudVMs for testing
        The steps include:
        1. Deploy 2-6 CloudVMs using nimbus, say a,b,c,d,e,f.
        2. Initial merge of these 6 CloudVMs using vdcmerge
        3. Add additional replicaiton links using vdcrepadmin
        The final replication relationship should look like:
        a-----d
        |\   /|
        | b-e |
        |/   \|
        c-----f
        """
        self.debug_print('----<')
        if len(self.nodeIds) < self.numVMs:
            raise Exception("Not enough nodeIds for the number of VMs to be created!!!, #nodeIds=%d #Vms=%d"
                            % (len(self.nodeIds), self.numVMs))
        #Setup cloudvm using nimbus deployment tools
        self.test_vms_names = create_test_vms_on_nimbus(
                        self.buildNumber, self.testOS, self.poolName, self.numVMs, self.numBegin, self.options.doSetup)
        self.test_vms = self._test_vm_name_to_id()
        #Merge vms defined in test_vms based on merge sequence defined in initial_merge_seq
        if self.options.doSetup:
            self.merge_vms()
        #TODO: Run vdcrepadmin to setup redundant replication links
        # self.add_rep_partner_vms()
        self.debug_print('---->\n')

    def teardown_vms(self):
        """cleanup on CloudVMs that has been setup
        currently this has to be done manually using
        /mts/git/bin/nimbus-ctl kill <VM_NAME>
        """
        self.debug_print('----<\n')
        if self.options.doCleanup:
            # TODO: Run vdcrepadmin to remove redundant replication links
            # Run vdcsplit to tear down all replication partnership
            self.split_vms()
            # TODO: Destroy all test vms using nimbus ctl tool
        self.debug_print('---->\n')

    def merge_vms(self):
        """Run vdcmerge to setup initial replication partnership
        """
        for (vm1, vm2) in self.initial_merge_seq:
            self.verbose_print('Merging %s to %s' % (vm1, vm2))
            self.merge_vm(self.test_vms[vm1], self.test_vms[vm2])

    def split_vms(self):
        """Run vdcsplit to destroy final replication partnership
        """
        for (vm1, vm2) in reversed(self.initial_merge_seq):
            self.verbose_print('Spliting %s and %s' % (vm1, vm2))
            self.split_vm(self.test_vms[vm1])

    def merge_vm(self, vm1, vm2):
        """Merge 2 vms using vdcmerge tool
        """
        vdcmerge_bin = self.get_vdcmerge_bin()
        self.debug_print( "vdcmerge=" + vdcmerge_bin )

        source_host = vm1
        source_username = self.test_const["lotus_admin_username"]
        source_password = self.test_const["lotus_admin_password"]
        target_host = vm2
        target_username = self.test_const["lotus_admin_username"]
        target_password = self.test_const["lotus_admin_password"]

        #call vdcmerge
        cmdline = '%s -u %s -w %s -H %s -U %s -W %s' % (vdcmerge_bin,
                source_username, source_password,
                target_host, target_username, target_password)
        self.debug_print( 'On %s, run' % (source_host) )
        self.debug_print( '%s' % (cmdline) )
        self.run_ssh_command_on_remote_host(source_host, "root", "vmware", cmdline)

    def split_vm(self,vm1):
        """Split 2 or more vms using vdcsplit tool
        """
        vdcsplit_bin = self.get_vdcsplit_bin()
        self.debug_print( "vdcsplit=" + vdcsplit_bin )

        source_host = vm1
        source_username = self.test_const["lotus_admin_username"]
        source_password = self.test_const["lotus_admin_password"]
        target_username = self.test_const["lotus_admin_username"]
        target_password = self.test_const["lotus_admin_password"]
        domain_name     = domain_dn_2_name[self.test_const["lotus_domain_dn"]]

        #call vdcsplit
        cmdline = '%s -u %s -w %s -D %s -U %s -W %s' % (vdcsplit_bin,
                source_username, source_password,
                domain_name, target_username, target_password)
        self.debug_print( 'On %s, run' % (source_host) )
        self.debug_print( '%s' % (cmdline) )
        self.run_ssh_command_on_remote_host(source_host, "root", "vmware", cmdline)

    def kill_cpu_hog_on_vms(self):
        """Stop the major CPU hog services on a cloudvm machine (identified by 'top' command output)
        """
        command = ";".join("service %s stop" % s for s in self.cfg["cpu_hog_services"])
        for (name, ip) in self.test_vms.items():
            host=ip
            username=self.os_const["root_login_username"]
            password=self.os_const["root_login_password"]
            self.run_ssh_command_on_remote_host(host, username, password, command)

    def get_vdcpromo_bin(self):
        """Get vdcpromo binary absolute path
        """
        return self.os_const["vmdir_bin_path"] + "vdcpromo" + self.os_const["bin_name_suffix"]

    def get_vdcmerge_bin(self):
        """Get vdcmerge binary absolute path
        """
        return self.os_const["vmdir_bin_path"] + "vdcmerge" + self.os_const["bin_name_suffix"]

    def get_vdcsplit_bin(self):
        """Get vdcsplit binary absolute path
        """
        return self.os_const["vmdir_bin_path"] + "vdcsplit" + self.os_const["bin_name_suffix"]

    def get_vdcsetupldu_bin(self):
        """Get vdcsetupldu binary absolute path
        """
        return self.os_const["vmdir_bin_path"] + "vdcsetupldu" + self.os_const["bin_name_suffix"]

    def get_vdcrepadmin_bin(self):
        """Get vdcrepadmin binary absolute path
        """
        return self.os_const["vmdir_bin_path"] + "vdcrepadmin" + self.os_const["bin_name_suffix"]

    def sleep(self, seconds=0):
        if seconds < 1:
            sleeptime = self.sleeptime
        else:
            sleeptime = seconds
        self.verbose_print('sleeping %d seconds (most likely waiting for replication)...' % sleeptime)
        try:
            for i in range(sleeptime):
                if not self.options.notRun:
                    time.sleep(1)
                if self.options.verbose:
                    update_progress((i+1)*100/sleeptime)
        except KeyboardInterrupt:
            self.warn_print('KeyboardInterrupt: Ctrl+C during sleep')

class vdcreptest (unittest.TestCase):
    """ All replication tests go here
    """
    # class variables are defined here
    # instance variable are defined in __init__
    def __init__(self, testname, helper):
        super(vdcreptest, self).__init__(testname)
        if helper is not None:
            self.helper = helper
            self.cfg = helper.cfg
        else:
            print "[vdcreptest.__init__] Test environment not setup yet, exit..."
            sys.exit(2)

    def test_scenario_1_0(self):
        """CREATE NEW ENTRIES and verify on the replication servers """
        report_print(get_function_name() + ' - ' + eval('self.' + get_function_name() + '.__doc__'))
        self.helper.verbose_print('')
        self.helper.debug_print('----<')
        #setup
        src_node_id = dict_keys_sorted(self.helper.test_vms)[0]
        src_host = self.helper.test_vms[src_node_id]
        src_server_uri = '%s://%s:%s' % (self.helper.test_const["protocol"], src_host, self.helper.test_const["port"] )
        test_data = copy.deepcopy(self.helper.cfg["1_0"])
        bdn    = test_data["baseDN"]
        prefix = test_data["rdn"]["prefix"]
        min    = test_data["rdn"]["suffix"]["range"]["min"]
        max    = test_data["rdn"]["suffix"]["range"]["max"]
        self.helper.verbose_print('Adding %d entries on node %s ...' % (max-min, src_node_id) )
        report_print('Adding %d entries on node %s ...' % (max-min, src_node_id) )
        suffixs = xrange(min, max)
        attrs_const = copy.deepcopy(test_data["attrs_const"])
        #doTest: add new entries
        try:
            l = ldap_init(src_server_uri)
            l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
        except ldap.LDAPError, e:
            self.helper.error_print(e)
        else:
            start = time.time()
            batch = 10000
            start_batch = time.time()
            for suffix in suffixs:
                new_user_name    = "%s_%d" % (prefix, suffix)
                new_user_dn  = "cn=%s,%s" % (new_user_name,bdn)
                attrs = copy.deepcopy(test_data["attrs"])
                for attr in attrs.itervalues():
                    attr[0] = str("%s_%d" % (attr[0], suffix))
                    self.helper.debug_print(attr[0])
                attrs.update(attrs_const)
                self.helper.verbose_print('Adding entry [%s] to server %s' % (new_user_dn, src_server_uri) )
                self.helper.debug_print('attrs = %s' % str(attrs) )
                if not self.helper.options.notRun:
                    try:
                        ldif = modlist.addModlist(attrs)
                        l.add_s(new_user_dn, ldif)
                    except ldap.LDAPError, e:
                        self.helper.error_print(e)
                if (suffix % batch == 0):
                    end_batch = time.time()
                    self.helper.perf_print('It takes %.2f seconds to add the %d st %d entries to ldap server'
                                        % (end_batch-start_batch, suffix/10000, batch))
                    start_batch = end_batch
            if l is not None: l.unbind_s()
            end = time.time()
            self.helper.perf_print('It takes %.2f seconds to add %d entries to ldap server' % (end-start, max-min))
        #verify: make sure the newly added entry exists on a remote replication partner.
        if self.helper.options.doVerify:
            self.helper.sleep() #wait for 3-hop replication a->b->e->f, that is 30s x 3, plus 10s for the replication to finish.
            tgt_node_ids = dict_keys_sorted(self.helper.test_vms)[1:]
            for tgt_node_id in tgt_node_ids:
                tgt_host = self.helper.test_vms[tgt_node_id]
                tgt_server_uri = '%s://%s:%s' % (self.helper.test_const["protocol"], tgt_host, self.helper.test_const["port"])
                self.helper.verbose_print('verifying results on node %s ...' % tgt_node_id)
                report_print('Verifying %d entries on node %s ...' % (max-min, tgt_node_id) )
                try:
                    l = ldap_init(tgt_server_uri)
                    l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
                except ldap.LDAPError, e:
                    self.helper.error_print(e)
                else:
                    for suffix in suffixs:
                        new_user_name    = "%s_%d" % (prefix, suffix)
                        new_user_dn  = "cn=%s,%s" % (new_user_name,bdn)
                        attrs = copy.deepcopy(test_data["attrs"])
                        for attr in attrs.itervalues():
                            attr[0] = str("%s_%d" % (attr[0], suffix))
                            self.helper.debug_print(attr[0])
                        attrs.update(attrs_const)
                        self.helper.debug_print('Verifying entry [%s] on server %s' % (new_user_dn, tgt_server_uri) )
                        if not self.helper.options.notRun:
                            try:
                                result = None
                                result = l.search_s(new_user_dn, ldap.SCOPE_BASE )
                            except ldap.LDAPError, e:
                                report_print('Failed at %s on node %s' % (new_user_dn, tgt_node_id))
                                self.helper.error_print(e)
                            self.assertEqual(len(result), 1)
                            self.assertEqual(len(result[0]), 2)
                            self.assertEqual(result[0][0], new_user_dn)
                            self.assertEqual(dict_sort(result[0][1]), dict_sort(attrs))
                    if l is not None: l.unbind_s()
        report_print('Done')
        self.helper.verbose_print('Done')
        self.helper.debug_print('---->')

    def test_scenario_1_1(self):
        """CREATE NEW ATTRIBUTES and verify on the replication servers
        """
        self.helper.verbose_print('')
        self.helper.debug_print('----<')
        #setup
        src_host = self.helper.test_vms["a"]
        src_server_uri = '%s://%s:%s' % (self.helper.test_const["protocol"], src_host, self.helper.test_const["port"] )
        test_data = copy.deepcopy(self.helper.cfg["1_0"])
        test_data.update(copy.deepcopy(self.helper.cfg["1_1"]))
        bdn    = test_data["baseDN"]
        prefix = test_data["rdn"]["prefix"]
        min    = test_data["rdn"]["suffix"]["range"]["min"]
        max    = test_data["rdn"]["suffix"]["range"]["max"]
        suffixs = xrange(min, max)
        attrs_const = copy.deepcopy(test_data["attrs_const"])
        #doTest: add new attributes
        try:
            l = ldap_init(src_server_uri)
            l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
        except ldap.LDAPError, e:
            self.helper.error_print(e)
        else:
            for suffix in suffixs:
                new_user_name    = "%s_%d" % (prefix, suffix)
                new_user_dn  = "cn=%s,%s" % (new_user_name,bdn)
                attrs_add = copy.deepcopy(test_data["attrs_add"])
                for attr in attrs_add.itervalues():
                    attr[0] = str("%s_%d" % (attr[0], suffix))
                    self.helper.debug_print(attr[0])
                self.helper.verbose_print('Add attribute for [%s] on server %s' % (new_user_dn, src_server_uri) )
                self.helper.debug_print('attrs_add = %s' % str(attrs_add) )
                if not self.helper.options.notRun:
                    mod_attrs = [(ldap.MOD_ADD, attr, attrs_add[attr]) for attr in attrs_add.keys()]
                    try:
                        l.modify_s(new_user_dn, mod_attrs)
                    except ldap.LDAPError, e:
                        self.helper.error_print(e)
            if l is not None: l.unbind_s()
        #verify: make sure the new attributes replicates to replication partners.
        self.helper.sleep() #wait for 3-hop replication a->b->e->f, that is 30s x 3, plus 10s for the replication to finish.
        tgt_hosts = [self.helper.test_vms[key] for key in sorted(self.helper.test_vms.iterkeys())]
        tgt_server_uris = ['%s://%s:%s' % (self.helper.test_const["protocol"], tgt_host, self.helper.test_const["port"] )
                            for tgt_host in tgt_hosts[:] ]
        for tgt_server_uri in tgt_server_uris:
            self.helper.verbose_print('verify result on %s' % tgt_server_uri)
            try:
                l = ldap_init(tgt_server_uri)
                l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
            except ldap.LDAPError, e:
                self.helper.error_print(e)
            else:
                for suffix in suffixs:
                    new_user_name    = "%s_%d" % (prefix, suffix)
                    new_user_dn  = "cn=%s,%s" % (new_user_name,bdn)
                    attrs = copy.deepcopy(test_data["attrs"])
                    attrs_add = copy.deepcopy(test_data["attrs_add"])
                    for attr in attrs.itervalues():
                        attr[0] = str("%s_%d" % (attr[0], suffix))
                        self.helper.debug_print(attr[0])
                    for attr in attrs_add.itervalues():
                        attr[0] = str("%s_%d" % (attr[0], suffix))
                        self.helper.debug_print(attr[0])
                    attrs.update(attrs_const)
                    attrs.update(attrs_add) # update the expectation for future validation
                    self.helper.debug_print('Verifying entry [%s] on server %s' % (new_user_dn, tgt_server_uri) )
                    if not self.helper.options.notRun:
                        try:
                            result = l.search_s(new_user_dn, ldap.SCOPE_BASE )
                        except ldap.LDAPError, e:
                            self.helper.error_print(e)
                        self.assertEqual(len(result), 1)
                        self.assertEqual(len(result[0]), 2)
                        self.assertEqual(result[0][0], new_user_dn)
                        self.assertEqual(dict_sort(result[0][1]), dict_sort(attrs))
                if l is not None: l.unbind_s()
        self.helper.verbose_print('Done')
        self.helper.debug_print('---->')

    def test_scenario_1_2(self):
        """CREATE NEW ATTRIBUTE VALUES and verify on the replication servers
        """
        self.helper.verbose_print('')
        self.helper.debug_print('----<')
        #setup
        src_host = self.helper.test_vms["a"]
        src_server_uri = '%s://%s:%s' % (self.helper.test_const["protocol"], src_host, self.helper.test_const["port"] )
        test_data = copy.deepcopy(self.helper.cfg["1_0"])
        test_data.update(copy.deepcopy(self.helper.cfg["1_1"]))
        test_data.update(copy.deepcopy(self.helper.cfg["1_2"]))
        bdn    = test_data["baseDN"]
        prefix = test_data["rdn"]["prefix"]
        min    = test_data["rdn"]["suffix"]["range"]["min"]
        max    = test_data["rdn"]["suffix"]["range"]["max"]
        suffixs = xrange(min, max)
        attrs_const = copy.deepcopy(test_data["attrs_const"])
        #doTest: add new attribute values
        try:
            l = ldap_init(src_server_uri)
            l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
        except ldap.LDAPError, e:
            self.helper.error_print(e)
        else:
            for suffix in suffixs:
                new_user_name    = "%s_%d" % (prefix, suffix)
                new_user_dn  = "cn=%s,%s" % (new_user_name,bdn)
                attr_values_add = copy.deepcopy(test_data["attr_values_add"])
                for attr in attr_values_add.itervalues():
                    attr[0] = str("%s_%d" % (attr[0], suffix))
                    self.helper.debug_print(attr[0])
                self.helper.verbose_print('Add attribute values for [%s] on server %s' % (new_user_dn, src_server_uri) )
                self.helper.debug_print('attr_values_add = %s' % str(attr_values_add) )
                if not self.helper.options.notRun:
                    mod_attrs = [(ldap.MOD_ADD, attr, attr_values_add[attr]) for attr in attr_values_add.keys()]
                    try:
                        l.modify_s(new_user_dn, mod_attrs)
                    except ldap.LDAPError, e:
                        self.helper.error_print(e)
            if l is not None: l.unbind_s()
        #verify: make sure the added attribute values replicates to replication partners.
        self.helper.sleep() #wait for 3-hop replication a->b->e->f, that is 30s x 3, plus 10s for the replication to finish.
        tgt_hosts = [self.helper.test_vms[key] for key in sorted(self.helper.test_vms.iterkeys())]
        tgt_server_uris = ['%s://%s:%s' % (self.helper.test_const["protocol"], tgt_host, self.helper.test_const["port"] )
                            for tgt_host in tgt_hosts[:] ]
        for tgt_server_uri in tgt_server_uris:
            self.helper.verbose_print('verify result on %s' % tgt_server_uri)
            try:
                l = ldap_init(tgt_server_uri)
                l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
            except ldap.LDAPError, e:
                self.helper.error_print(e)
            else:
                for suffix in suffixs:
                    new_user_name    = "%s_%d" % (prefix, suffix)
                    new_user_dn  = "cn=%s,%s" % (new_user_name,bdn)
                    attrs = copy.deepcopy(test_data["attrs"])
                    attrs_add = copy.deepcopy(test_data["attrs_add"])
                    attr_values_add = copy.deepcopy(test_data["attr_values_add"])
                    for attr in attrs.itervalues():
                        attr[0] = str("%s_%d" % (attr[0], suffix))
                        self.helper.debug_print(attr[0])
                    for attr in attrs_add.itervalues():
                        attr[0] = str("%s_%d" % (attr[0], suffix))
                        self.helper.debug_print(attr[0])
                    for attr in attr_values_add.itervalues():
                        attr[0] = str("%s_%d" % (attr[0], suffix))
                        self.helper.debug_print(attr[0])
                    attrs.update(attrs_const)
                    attrs.update(attrs_add) # update the expectation for future validation
                    for attr in attr_values_add.keys():
                        attrs[attr].extend(attr_values_add[attr])
                    self.helper.debug_print('Verifying entry [%s] on server %s' % (new_user_dn, tgt_server_uri) )
                    if not self.helper.options.notRun:
                        try:
                            result = l.search_s(new_user_dn, ldap.SCOPE_BASE )
                        except ldap.LDAPError, e:
                            self.helper.error_print(e)
                        self.assertEqual(len(result), 1)
                        self.assertEqual(len(result[0]), 2)
                        self.assertEqual(result[0][0], new_user_dn)
                        self.assertEqual(dict_sort(result[0][1]), dict_sort(attrs))
                if l is not None: l.unbind_s()
        self.helper.verbose_print('Done')
        self.helper.debug_print('---->')

    def test_scenario_1_3(self):
        """MODIFY ATTRIBUTE VALUES and verify on the replication servers
        """
        self.helper.verbose_print('')
        self.helper.debug_print('----<')
        #setup
        src_host = self.helper.test_vms["a"]
        src_server_uri = '%s://%s:%s' % (self.helper.test_const["protocol"], src_host, self.helper.test_const["port"] )
        test_data = copy.deepcopy(self.helper.cfg["1_0"])
        test_data.update(copy.deepcopy(self.helper.cfg["1_1"]))
        test_data.update(copy.deepcopy(self.helper.cfg["1_2"]))
        test_data.update(copy.deepcopy(self.helper.cfg["1_3"]))
        bdn    = test_data["baseDN"]
        prefix = test_data["rdn"]["prefix"]
        min    = test_data["rdn"]["suffix"]["range"]["min"]
        max    = test_data["rdn"]["suffix"]["range"]["max"]
        suffixs = xrange(min, max)
        attrs_const = copy.deepcopy(test_data["attrs_const"])
        #doTest: modify attribute values
        try:
            l = ldap_init(src_server_uri)
            l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
        except ldap.LDAPError, e:
            self.helper.error_print(e)
        else:
            for suffix in suffixs:
                new_user_name    = "%s_%d" % (prefix, suffix)
                new_user_dn  = "cn=%s,%s" % (new_user_name,bdn)
                attr_values_mod = copy.deepcopy(test_data["attr_values_mod"])
                for attr in attr_values_mod.itervalues():
                    attr[0] = str("%s_%d" % (attr[0], suffix))
                    self.helper.debug_print(attr[0])
                self.helper.verbose_print('Modify attribute values for [%s] on server %s' % (new_user_dn, src_server_uri) )
                self.helper.debug_print('attr_values_mod = %s' % str(attr_values_mod) )
                if not self.helper.options.notRun:
                    mod_attrs = [(ldap.MOD_REPLACE, attr, attr_values_mod[attr]) for attr in attr_values_mod.keys()]
                    try:
                        l.modify_s(new_user_dn, mod_attrs)
                    except ldap.LDAPError, e:
                        self.helper.error_print(e)
            if l is not None: l.unbind_s()
        #verify: make sure the modified attribute values replicates to replication partners.
        self.helper.sleep() #wait for 3-hop replication a->b->e->f, that is 30s x 3, plus 10s for the replication to finish.
        tgt_hosts = [self.helper.test_vms[key] for key in sorted(self.helper.test_vms.iterkeys())]
        tgt_server_uris = ['%s://%s:%s' % (self.helper.test_const["protocol"], tgt_host, self.helper.test_const["port"] )
                            for tgt_host in tgt_hosts[:] ]
        for tgt_server_uri in tgt_server_uris:
            self.helper.verbose_print('verify result on %s' % tgt_server_uri)
            try:
                l = ldap_init(tgt_server_uri)
                l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
            except ldap.LDAPError, e:
                self.helper.error_print(e)
            else:
                for suffix in suffixs:
                    new_user_name    = "%s_%d" % (prefix, suffix)
                    new_user_dn  = "cn=%s,%s" % (new_user_name,bdn)
                    attrs = copy.deepcopy(test_data["attrs"])
                    attrs_add = copy.deepcopy(test_data["attrs_add"])
                    attr_values_add = copy.deepcopy(test_data["attr_values_add"])
                    attr_values_mod = copy.deepcopy(test_data["attr_values_mod"])
                    for attr in attrs.itervalues():
                        attr[0] = str("%s_%d" % (attr[0], suffix))
                        self.helper.debug_print(attr[0])
                    for attr in attrs_add.itervalues():
                        attr[0] = str("%s_%d" % (attr[0], suffix))
                        self.helper.debug_print(attr[0])
                    for attr in attr_values_add.itervalues():
                        attr[0] = str("%s_%d" % (attr[0], suffix))
                        self.helper.debug_print(attr[0])
                    for attr in attr_values_mod.itervalues():
                        attr[0] = str("%s_%d" % (attr[0], suffix))
                        self.helper.debug_print(attr[0])
                    attrs.update(attrs_const)
                    attrs.update(attrs_add) # update the expectation for future validation
                    for attr in attr_values_add.keys():
                        attrs[attr].extend(attr_values_add[attr])
                    attrs.update(attr_values_mod)
                    self.helper.debug_print('Verifying entry [%s] on server %s' % (new_user_dn, tgt_server_uri) )
                    if not self.helper.options.notRun:
                        try:
                            result = l.search_s(new_user_dn, ldap.SCOPE_BASE )
                        except ldap.LDAPError, e:
                            self.helper.error_print(e)
                        self.assertEqual(len(result), 1)
                        self.assertEqual(len(result[0]), 2)
                        self.assertEqual(result[0][0], new_user_dn)
                        self.assertEqual(dict_sort(result[0][1]), dict_sort(attrs))
                if l is not None: l.unbind_s()
        self.helper.verbose_print('Done')
        self.helper.debug_print('---->')

    def test_scenario_1_4(self):
        """DELETE ATTRIBUTE VALUES and verify on the replication servers
        """
        self.helper.verbose_print('')
        self.helper.debug_print('----<')
        #setup
        src_host = self.helper.test_vms["a"]
        src_server_uri = '%s://%s:%s' % (self.helper.test_const["protocol"], src_host, self.helper.test_const["port"] )
        test_data = copy.deepcopy(self.helper.cfg["1_0"])
        test_data.update(copy.deepcopy(self.helper.cfg["1_1"]))
        test_data.update(copy.deepcopy(self.helper.cfg["1_2"]))
        test_data.update(copy.deepcopy(self.helper.cfg["1_3"]))
        bdn    = test_data["baseDN"]
        prefix = test_data["rdn"]["prefix"]
        min    = test_data["rdn"]["suffix"]["range"]["min"]
        max    = test_data["rdn"]["suffix"]["range"]["max"]
        suffixs = xrange(min, max)
        attrs_const = copy.deepcopy(test_data["attrs_const"])
        #doTest: delete attribute values
        try:
            l = ldap_init(src_server_uri)
            l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
        except ldap.LDAPError, e:
            self.helper.error_print(e)
        else:
            for suffix in suffixs:
                new_user_name    = "%s_%d" % (prefix, suffix)
                new_user_dn  = "cn=%s,%s" % (new_user_name,bdn)
                attr_values_del = copy.deepcopy(test_data["attr_values_add"])
                for attr in attr_values_del.itervalues():
                    attr[0] = str("%s_%d" % (attr[0], suffix))
                    self.helper.debug_print(attr[0])
                self.helper.verbose_print('Delete attribute values for [%s] on server %s' % (new_user_dn, src_server_uri) )
                self.helper.debug_print('attr_values_del = %s' % str(attr_values_del) )
                if not self.helper.options.notRun:
                    mod_attrs = [(ldap.MOD_DELETE, attr, attr_values_del[attr]) for attr in attr_values_del.keys()]
                    try:
                        l.modify_s(new_user_dn, mod_attrs)
                    except ldap.LDAPError, e:
                        self.helper.error_print(e)
            if l is not None: l.unbind_s()
        #verify: make sure the deleted attribute values no longer exists on replication partners.
        self.helper.sleep() #wait for 3-hop replication a->b->e->f, that is 30s x 3, plus 10s for the replication to finish.
        tgt_hosts = [self.helper.test_vms[key] for key in sorted(self.helper.test_vms.iterkeys())]
        tgt_server_uris = ['%s://%s:%s' % (self.helper.test_const["protocol"], tgt_host, self.helper.test_const["port"] )
                            for tgt_host in tgt_hosts[:] ]
        for tgt_server_uri in tgt_server_uris:
            self.helper.verbose_print('verify result on %s' % tgt_server_uri)
            try:
                l = ldap_init(tgt_server_uri)
                l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
            except ldap.LDAPError, e:
                self.helper.error_print(e)
            else:
                for suffix in suffixs:
                    new_user_name    = "%s_%d" % (prefix, suffix)
                    new_user_dn  = "cn=%s,%s" % (new_user_name,bdn)
                    attrs = copy.deepcopy(test_data["attrs"])
                    attrs_add = copy.deepcopy(test_data["attrs_add"])
                    attr_values_mod = copy.deepcopy(test_data["attr_values_mod"])
                    for attr in attrs.itervalues():
                        attr[0] = str("%s_%d" % (attr[0], suffix))
                        self.helper.debug_print(attr[0])
                    for attr in attrs_add.itervalues():
                        attr[0] = str("%s_%d" % (attr[0], suffix))
                        self.helper.debug_print(attr[0])
                    for attr in attr_values_mod.itervalues():
                        attr[0] = str("%s_%d" % (attr[0], suffix))
                        self.helper.debug_print(attr[0])
                    attrs.update(attrs_const)
                    attrs.update(attrs_add) # update the expectation for future validation
                    attrs.update(attr_values_mod)
                    self.helper.debug_print('Verifying entry [%s] on server %s' % (new_user_dn, tgt_server_uri) )
                    if not self.helper.options.notRun:
                        try:
                            result = l.search_s(new_user_dn, ldap.SCOPE_BASE )
                        except ldap.LDAPError, e:
                            self.helper.error_print(e)
                        self.assertEqual(len(result), 1)
                        self.assertEqual(len(result[0]), 2)
                        self.assertEqual(result[0][0], new_user_dn)
                        self.assertEqual(dict_sort(result[0][1]), dict_sort(attrs))
                if l is not None: l.unbind_s()
        self.helper.verbose_print('Done')
        self.helper.debug_print('---->')

    def test_scenario_1_5(self):
        """DELETE an ATTRIBUTE and verify on the replication servers
        """
        self.helper.verbose_print('')
        self.helper.debug_print('----<')
        #setup
        src_host = self.helper.test_vms["a"]
        src_server_uri = '%s://%s:%s' % (self.helper.test_const["protocol"], src_host, self.helper.test_const["port"] )
        test_data = copy.deepcopy(self.helper.cfg["1_0"])
        test_data.update(copy.deepcopy(self.helper.cfg["1_1"]))
        test_data.update(copy.deepcopy(self.helper.cfg["1_2"]))
        test_data.update(copy.deepcopy(self.helper.cfg["1_3"]))
        bdn    = test_data["baseDN"]
        prefix = test_data["rdn"]["prefix"]
        min    = test_data["rdn"]["suffix"]["range"]["min"]
        max    = test_data["rdn"]["suffix"]["range"]["max"]
        suffixs = xrange(min, max)
        attrs_const = copy.deepcopy(test_data["attrs_const"])
        #doTest: delete attributes
        try:
            l = ldap_init(src_server_uri)
            l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
        except ldap.LDAPError, e:
            self.helper.error_print(e)
        else:
            for suffix in suffixs:
                new_user_name    = "%s_%d" % (prefix, suffix)
                new_user_dn  = "cn=%s,%s" % (new_user_name,bdn)
                attrs_del = copy.deepcopy(test_data["attrs_add"])
                for attr in attrs_del.itervalues():
                    attr[0] = str("%s_%d" % (attr[0], suffix))
                    self.helper.debug_print(attr[0])
                self.helper.verbose_print('Delete attributes for [%s] on server %s' % (new_user_dn, src_server_uri) )
                self.helper.debug_print('attrs_del = %s' % str(attrs_del) )
                if not self.helper.options.notRun:
                    mod_attrs = [(ldap.MOD_DELETE, attr, attrs_del[attr]) for attr in attrs_del.keys()]
                    try:
                        l.modify_s(new_user_dn, mod_attrs)
                    except ldap.LDAPError, e:
                        self.helper.error_print(e)
            if l is not None: l.unbind_s()
        #verify: make sure the deleted attributes no longer exists on replication partners.
        self.helper.sleep() #wait for 3-hop replication a->b->e->f, that is 30s x 3, plus 10s for the replication to finish.
        tgt_hosts = [self.helper.test_vms[key] for key in sorted(self.helper.test_vms.iterkeys())]
        tgt_server_uris = ['%s://%s:%s' % (self.helper.test_const["protocol"], tgt_host, self.helper.test_const["port"] )
                            for tgt_host in tgt_hosts[:] ]
        for tgt_server_uri in tgt_server_uris:
            self.helper.verbose_print('verify result on %s' % tgt_server_uri)
            try:
                l = ldap_init(tgt_server_uri)
                l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
            except ldap.LDAPError, e:
                self.helper.error_print(e)
            else:
                for suffix in suffixs:
                    new_user_name    = "%s_%d" % (prefix, suffix)
                    new_user_dn  = "cn=%s,%s" % (new_user_name,bdn)
                    attrs = copy.deepcopy(test_data["attrs"])
                    attr_values_mod = copy.deepcopy(test_data["attr_values_mod"])
                    for attr in attrs.itervalues():
                        attr[0] = str("%s_%d" % (attr[0], suffix))
                        self.helper.debug_print(attr[0])
                    for attr in attr_values_mod.itervalues():
                        attr[0] = str("%s_%d" % (attr[0], suffix))
                        self.helper.debug_print(attr[0])
                    attrs.update(attrs_const)
                    attrs.update(attr_values_mod) # update the expectation for future validation
                    self.helper.debug_print('Verifying entry [%s] on server %s' % (new_user_dn, tgt_server_uri) )
                    if not self.helper.options.notRun:
                        try:
                            result = l.search_s(new_user_dn, ldap.SCOPE_BASE )
                        except ldap.LDAPError, e:
                            self.helper.error_print(e)
                        self.assertEqual(len(result), 1)
                        self.assertEqual(len(result[0]), 2)
                        self.assertEqual(result[0][0], new_user_dn)
                        self.assertEqual(dict_sort(result[0][1]), dict_sort(attrs))
                if l is not None: l.unbind_s()
        self.helper.verbose_print('Done')
        self.helper.debug_print('---->')

    def test_scenario_1_6(self):
        """DELETE ENTRIES and verify on the replication servers
        """
        self.helper.verbose_print('')
        self.helper.debug_print('----<')
        #setup
        src_host = self.helper.test_vms["a"]
        src_server_uri = '%s://%s:%s' % (self.helper.test_const["protocol"], src_host, self.helper.test_const["port"] )
        test_data = copy.deepcopy(self.helper.cfg["1_0"])
        bdn    = test_data["baseDN"]
        prefix = test_data["rdn"]["prefix"]
        min    = test_data["rdn"]["suffix"]["range"]["min"]
        max    = test_data["rdn"]["suffix"]["range"]["max"]
        suffixs = xrange(min, max)
        attrs_const = copy.deepcopy(test_data["attrs_const"])
        #doTest: delete entries
        try:
            l = ldap_init(src_server_uri)
            l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
        except ldap.LDAPError, e:
            self.helper.error_print(e)
        else:
            for suffix in suffixs:
                new_user_name    = "%s_%d" % (prefix, suffix)
                new_user_dn  = "cn=%s,%s" % (new_user_name,bdn)
                self.helper.verbose_print('Deleting entry [%s] from server %s' % (new_user_dn, src_server_uri) )
                if not self.helper.options.notRun:
                    try:
                        result = l.delete_s(new_user_dn)
                    except ldap.LDAPError, e:
                        self.helper.error_print(e)
            if l is not None: l.unbind_s()
        #verify: make sure the deleted entry no longer exists on a remote replication partner.
        # TODO: remove the "not" word
        if self.helper.options.doVerify:
            self.helper.sleep() #wait for 3-hop replication a->b->e->f, that is 30s x 3, plus 10s for the replication to finish.
            tgt_hosts = [self.helper.test_vms[key] for key in sorted(self.helper.test_vms.iterkeys())]
            tgt_server_uris = ['%s://%s:%s' % (self.helper.test_const["protocol"], tgt_host, self.helper.test_const["port"] )
                                for tgt_host in tgt_hosts[:] ]
            for tgt_server_uri in tgt_server_uris:
                self.helper.verbose_print('verify result on %s' % tgt_server_uri)
                try:
                    l = ldap_init(tgt_server_uri)
                    l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
                except ldap.LDAPError, e:
                    self.helper.error_print(e)
                else:
                    for suffix in suffixs:
                        new_user_name    = "%s_%d" % (prefix, suffix)
                        new_user_dn  = "cn=%s,%s" % (new_user_name,bdn)
                        self.helper.debug_print('Verifying entry [%s] deleted from server %s' % (new_user_dn, tgt_server_uri) )
                        if not self.helper.options.notRun:
                            self.assertRaises(ldap.NO_SUCH_OBJECT, l.delete_s, new_user_dn)
                    if l is not None: l.unbind_s()
        self.helper.verbose_print('Done')
        self.helper.debug_print('---->')

    def test_scenario_1_7(self):
        """OUT-OF-ORDER REPLICATION test: 1. create parent; 2. create child; 3. modify parent;
        And verify on replication partners.
        """
        self.helper.verbose_print('')
        self.helper.debug_print('----<')
        #setup
        src_host = self.helper.test_vms["a"]
        src_server_uri = '%s://%s:%s' % (self.helper.test_const["protocol"], src_host, self.helper.test_const["port"] )
        test_data = copy.deepcopy(self.helper.cfg["1_0"])
        test_data.update(copy.deepcopy(self.helper.cfg["1_7"]))
        #doTest: 1. create parent
        pdn          = test_data["parentDN"]
        parent_attrs = test_data["parent_attrs"]
        self.helper.verbose_print('Adding parent [%s] on server %s' % (pdn, src_server_uri) )
        self.helper.debug_print('attrs = %s' % str(parent_attrs) )
        try:
            l = ldap_init(src_server_uri)
            l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
        except ldap.LDAPError, e:
            self.helper.error_print(e)
        else:
            if not self.helper.options.notRun:
                try:
                    ldif = modlist.addModlist(parent_attrs)
                    l.add_s(pdn, ldif)
                except ldap.LDAPError, e:
                    self.helper.error_print(e)
            if l is not None: l.unbind_s()
        #doTest: 2. create child
        bdn    = pdn
        prefix = test_data["rdn"]["prefix"]
        min    = test_data["rdn"]["suffix"]["range"]["min"]
        max    = test_data["rdn"]["suffix"]["range"]["max"]
        suffixs = xrange(min, max)
        attrs_const = copy.deepcopy(test_data["attrs_const"])
        try:
            l = ldap_init(src_server_uri)
            l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
        except ldap.LDAPError, e:
            self.helper.error_print(e)
        else:
            for suffix in suffixs:
                new_user_name    = "%s_%d" % (prefix, suffix)
                new_user_dn  = "cn=%s,%s" % (new_user_name,bdn)
                attrs = copy.deepcopy(test_data["attrs"])
                for attr in attrs.itervalues():
                    attr[0] = str("%s_%d" % (attr[0], suffix))
                    self.helper.debug_print(attr[0])
                attrs.update(attrs_const)
                self.helper.verbose_print('Adding child [%s] on server %s' % (new_user_dn, src_server_uri) )
                self.helper.debug_print('attrs = %s' % str(attrs) )
                if not self.helper.options.notRun:
                    try:
                        ldif = modlist.addModlist(attrs)
                        l.add_s(new_user_dn, ldif)
                    except ldap.LDAPError, e:
                        self.helper.error_print(e)
            if l is not None: l.unbind_s()
        #doTest: 3. modify parent
        parent_attrs_mod = test_data["parent_attrs_mod"]
        self.helper.verbose_print('Modifying parent [%s] on server %s' % (pdn, src_server_uri) )
        self.helper.debug_print('attrs = %s' % str(parent_attrs) )
        try:
            l = ldap_init(src_server_uri)
            l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
        except ldap.LDAPError, e:
            self.helper.error_print(e)
        else:
            if not self.helper.options.notRun:
                mod_attrs = [(ldap.MOD_REPLACE, attr, parent_attrs_mod[attr]) for attr in parent_attrs_mod.keys()]
                try:
                    l.modify_s(pdn, mod_attrs)
                except ldap.LDAPError, e:
                    self.helper.error_print(e)
            if l is not None: l.unbind_s()
        #verify: modified parent info replicates to replication partners.
        self.helper.sleep() #wait for 3-hop replication a->b->e->f, that is 30s x 3, plus 10s for the replication to finish.
        parent_attrs.update(parent_attrs_mod)
        tgt_hosts = [self.helper.test_vms[key] for key in sorted(self.helper.test_vms.iterkeys())]
        tgt_server_uris = ['%s://%s:%s' % (self.helper.test_const["protocol"], tgt_host, self.helper.test_const["port"] )
                            for tgt_host in tgt_hosts[:] ]
        for tgt_server_uri in tgt_server_uris:
            self.helper.verbose_print('verify result on %s' % tgt_server_uri)
            if not self.helper.options.notRun:
                try:
                    l = ldap_init(tgt_server_uri)
                    l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
                    result = l.search_s(pdn, ldap.SCOPE_BASE )
                    self.assertEqual(len(result), 1)
                    self.assertEqual(len(result[0]), 2)
                    self.assertEqual(result[0][0], pdn)
                    self.assertEqual(dict_sort(result[0][1]), dict_sort(parent_attrs))
                except ldap.LDAPError, e:
                    self.helper.error_print(e)
                else:
                    if l is not None: l.unbind_s()
        #cleanup: delete all children entry
        if not self.helper.options.notRun:
            try:
                l = ldap_init(src_server_uri)
                l.simple_bind_s(self.helper.admin_user_name, self.helper.admin_password)
                for suffix in suffixs:
                    new_user_name    = "%s_%d" % (prefix, suffix)
                    new_user_dn  = "cn=%s,%s" % (new_user_name,bdn)
                    self.helper.verbose_print('Deleting child [%s] on server %s' % (new_user_dn, src_server_uri) )
                    l.delete_s(new_user_dn)
                self.helper.verbose_print('Deleting parent [%s] on server %s' % (pdn, src_server_uri) )
                self.helper.debug_print('attrs = %s' % str(parent_attrs) )
                l.delete_s(pdn)
            except ldap.LDAPError, e:
                self.helper.error_print(e)
            else:
                if l is not None: l.unbind_s()
        self.helper.verbose_print('Done')
        self.helper.debug_print('---->')

    def test_scenario_1_8(self):
        """Special case 1: update GROUP MEMBERSHIPS
        And verify on the replication servers
        """
        self.helper.verbose_print('')
        self.helper.debug_print('----<')
        #setup
        src_host = self.helper.test_vms["a"]
        src_server_uri = '%s://%s:%s' % (self.helper.test_const["protocol"], src_host, self.helper.test_const["port"] )
        test_data = copy.deepcopy(self.helper.cfg["1_8"])
        bind_dn     = self.helper.admin_user_name
        bind_pwd   = self.helper.admin_password
        username    = test_data['username']
        password    = test_data['password']
        base_dn     = test_data['base_dn']
        group_dn    = test_data['group_dn']
        fname       = test_data['fname']
        lname       = test_data['lname']
        domain      = test_data['domain']
        employee_id = test_data['employee_id']
        user_dn     = 'cn=%s,%s' % (username, base_dn)
        #doTest
        self.helper.verbose_print('Create user %s and add to group [%s] on server %s' % (user_dn, group_dn, src_server_uri) )
        self.helper.debug_print('group member attrs = %s' % str(test_data) )
        create_user(src_server_uri, bind_dn, bind_pwd, username, password,
                              base_dn, group_dn, fname, lname, domain, employee_id)
        #verify
        self.helper.sleep()
        tgt_hosts = [self.helper.test_vms[key] for key in sorted(self.helper.test_vms.iterkeys())]
        tgt_server_uris = ['%s://%s:%s' % (self.helper.test_const["protocol"], tgt_host, self.helper.test_const["port"] )
                            for tgt_host in tgt_hosts[:] ]
        for tgt_server_uri in tgt_server_uris:
            self.helper.verbose_print('verify result on %s' % tgt_server_uri)
            result = ldap_search_s(tgt_server_uri, bind_dn, bind_pwd, group_dn, ldap.SCOPE_BASE)
            self.assertEqual(len(result), 1)
            self.assertEqual(len(result[0]), 2)
            self.assertEqual(result[0][0], group_dn)
            self.assertTrue(result[0][1].has_key('member'))
            self.assertTrue(user_dn in result[0][1]['member'])
        #cleanup
        self.helper.verbose_print('Done')
        self.helper.debug_print('---->')

    def test_scenario_1_9(self):
        """Special case 2: change PASSWORD
        And verify on the replicaiton servers.
        """
        self.helper.verbose_print('')
        self.helper.debug_print('----<')
        #setup
        src_host = self.helper.test_vms["a"]
        src_server_uri = '%s://%s:%s' % (self.helper.test_const["protocol"], src_host, self.helper.test_const["port"] )
        test_data = copy.deepcopy(self.helper.cfg["1_8"])
        test_data.update(copy.deepcopy(self.helper.cfg["1_10"]))
        bind_dn     = self.helper.admin_user_name
        bind_pwd   = self.helper.admin_password
        username    = test_data['username']
        base_dn     = test_data['base_dn']
        user_dn     = 'cn=%s,%s' % (username, base_dn)
        new_password= test_data['new_password']
        #doTest
        self.helper.verbose_print('Change user password for [%s] on server %s' % (user_dn, src_server_uri) )
        change_user_password(src_server_uri, bind_dn, bind_pwd, user_dn, new_password)
        #verify
        self.helper.sleep()
        tgt_hosts = [self.helper.test_vms[key] for key in sorted(self.helper.test_vms.iterkeys())]
        tgt_server_uris = ['%s://%s:%s' % (self.helper.test_const["protocol"], tgt_host, self.helper.test_const["port"] )
                            for tgt_host in tgt_hosts[:] ]
        for tgt_server_uri in tgt_server_uris:
            self.helper.verbose_print('verify result on %s' % tgt_server_uri)
            bind_dn = user_dn
            bind_pwd = new_password
            # TODO: how to do the verification?
            #result = ldap_search_s(tgt_server_uri, bind_dn, bind_pwd, user_dn, ldap.SCOPE_BASE)
            #self.assertEqual(len(result), 1)
            #self.assertEqual(len(result[0]), 2)
            #self.assertEqual(result[0][0], user_dn)
        #cleanup
        self.helper.verbose_print('Done')
        self.helper.debug_print('---->')

    def test_scenario_1_10(self):
        """Special case 3: DELETE an object that's a MEMBER OF A GROUP, and
        verify that the group membership is removed by vmdir "business logic".
        And verify on replicaiton servers.
        """
        self.helper.verbose_print('')
        self.helper.debug_print('----<')
        #setup
        src_host = self.helper.test_vms["a"]
        src_server_uri = '%s://%s:%s' % (self.helper.test_const["protocol"], src_host, self.helper.test_const["port"] )
        test_data = copy.deepcopy(self.helper.cfg["1_8"])
        bind_dn     = self.helper.admin_user_name
        bind_pwd   = self.helper.admin_password
        username    = test_data['username']
        base_dn     = test_data['base_dn']
        group_dn    = test_data['group_dn']
        user_dn     = 'cn=%s,%s' % (username, base_dn)
        #doTest
        self.helper.verbose_print('Delete user %s from group [%s] on server %s' % (user_dn, group_dn, src_server_uri) )
        ldap_delete_s(src_server_uri, bind_dn, bind_pwd, user_dn)
        #verify
        self.helper.sleep()
        tgt_hosts = [self.helper.test_vms[key] for key in sorted(self.helper.test_vms.iterkeys())]
        tgt_server_uris = ['%s://%s:%s' % (self.helper.test_const["protocol"], tgt_host, self.helper.test_const["port"] )
                            for tgt_host in tgt_hosts[:] ]
        for tgt_server_uri in tgt_server_uris:
            self.helper.verbose_print('verify result on %s' % tgt_server_uri)
            self.assertRaises(ldap.NO_SUCH_OBJECT, ldap_delete_s, src_server_uri, bind_dn, bind_pwd, user_dn)
        #cleanup
        self.helper.verbose_print('Done')
        self.helper.debug_print('---->')

    def test_scenario_2_1(self):
        """long-haul stress replication test on multiple nodes WITHOUT conflicts """
        report_print(get_function_name() + ' - ' + eval('self.' + get_function_name() + '.__doc__'))
        self.helper.debug_print('----<')
        #setup
        reportDuration     = self.helper.cfg["2_1"]["reportDuration"]
        checkpointDuration = self.helper.cfg["2_1"]["checkpointDuration"]
        timer = {
                    "batch": {"start":time.time(), "end":time.time()},
                    "daily": {"start":time.time(), "end":time.time()},
                    "total": {"start":time.time(), "end":time.time()}
                }
        summary={
                    "batch": {"C":0, "R":0, "U":0, "D":0},
                    "daily": {"C":0, "R":0, "U":0, "D":0},
                    "total": {"C":0, "R":0, "U":0, "D":0}
                }
        protocol = self.helper.test_const["protocol"]
        port     = self.helper.test_const["port"]
        bind_dn  = self.helper.admin_user_name
        bind_pwd = self.helper.admin_password
        random_access = eval(self.helper.cfg["random_access"])
        batch_start_num = self.helper.cfg["2_1"]["rdn"]["suffix"]["range"]["batch_start_num"]
        num_batches     = self.helper.cfg["2_1"]["rdn"]["suffix"]["range"]["num_batches"]
        per_batch_count = self.helper.cfg["2_1"]["rdn"]["suffix"]["range"]["per_node_batch_count"] * self.helper.numVMs
        daily_report = []
        batch_num = batch_start_num
        #while True: # do CRUD operations and verify on all nodes, one batch at a time
        while batch_num < batch_start_num + num_batches:
            batch_num_old = batch_num
            if (random_access):
                batch_num = random.randint(batch_start_num, batch_start_num + num_batches)
            if per_batch_count >= STAT_COUNT_MIN:
                report_print('Batch ops start: %d/%d/%d' % (batch_num_old, batch_num, batch_start_num + num_batches ))
            timer["batch"]["start"] = time.time()
            batch_report=[]
            msg=''
            workset = {}
            batch_min = batch_num * per_batch_count
            batch_max = (batch_num+1) * per_batch_count
            batch_range = xrange(batch_min, batch_max)
            test_data = copy.deepcopy(self.helper.cfg["2_1"])
            test_data.update(copy.deepcopy(self.helper.cfg["1_1"]))
            if (self.helper.cfg["testTarget"] == "OpenLdap") :
                test_data["attrs_const"] = test_data["attrs_const_openldap"]
                test_data["attrs"]       = test_data["attrs_openldap"]
            #1.1 create "AddDn" work batch on each node
            for i, nodeId in enumerate(self.helper.nodeIds):
                server_ip = self.helper.test_vms[nodeId]
                workset[nodeId] = []
                workset[nodeId] = create_batch_for_node( TestOpCode.AddDn,
                    i, self.helper.numVMs, nodeId, server_ip, protocol, port, bind_dn, bind_pwd, test_data, batch_range)
                # workset >> workset.txt, just in case the test crashed so we can recover
                #with open('workset/AddDn-%s.txt' % datetime_2_str(datetime.now()), 'w') as f:
                #    pickle.dump(workset, f)
            #1.2 execute "AddDn" work batch on each node
            if "C" in self.helper.cfg["opCodes"]:
                for i, nodeId in enumerate(self.helper.nodeIds):
                    execution_report, stats = execute_batch_on_node(batch_num, workset, nodeId)
                    batch_report.extend(execution_report)
                    for dur in ["batch", "daily", "total"]:
                        for op in ["C","R","U","D"]:
                            summary[dur][op] += stats[op]
            #1.3 verify "AddDn" work batch has been replicated to other nodes
            if "R" in self.helper.cfg["opCodes"]:
                for i, nodeId in enumerate(self.helper.nodeIds):
                    verification_report, stats = verify_batch_on_other_node(batch_num,
                                        workset, i, nodeId, self.helper.nodeIds, self.helper.test_vms, protocol, port)
                    batch_report.extend(verification_report)
                    for dur in ["batch", "daily", "total"]:
                        for op in ["C","R","U","D"]:
                            summary[dur][op] += stats[op]
            #2.1 create "AddAttr" work batch on each node
            for i, nodeId in enumerate(self.helper.nodeIds):
                server_ip = self.helper.test_vms[nodeId]
                workset[nodeId] = []
                workset[nodeId] = create_batch_for_node( TestOpCode.AddAttr,
                    i, self.helper.numVMs, nodeId, server_ip, protocol, port, bind_dn, bind_pwd, test_data, batch_range)
                # workset >> workset.txt, just in case the test crashed so we can recover
                #with open('workset/AddAttr-%s.txt' % datetime_2_str(datetime.now()), 'w') as f:
                #    pickle.dump(workset, f)
            #2.2 execute "AddAttr" work batch on each node
            if "U" in self.helper.cfg["opCodes"]:
                for i, nodeId in enumerate(self.helper.nodeIds):
                    execution_report, stats = execute_batch_on_node(batch_num, workset, nodeId)
                    batch_report.extend(execution_report)
                    for dur in ["batch", "daily", "total"]:
                        for op in ["C","R","U","D"]:
                            summary[dur][op] += stats[op]
            #2.3 verify "AddAttr" work batch has been replicated to other nodes
            if "RU" in self.helper.cfg["opCodes"]:
                for i, nodeId in enumerate(self.helper.nodeIds):
                    verification_report, stats = verify_batch_on_other_node(batch_num,
                                        workset, i, nodeId, self.helper.nodeIds, self.helper.test_vms, protocol, port)
                    batch_report.extend(verification_report)
                    for dur in ["batch", "daily", "total"]:
                        for op in ["C","R","U","D"]:
                            summary[dur][op] += stats[op]
            #3.1 create "DelDn" work batch on each node
            for i, nodeId in enumerate(self.helper.nodeIds):
                server_ip = self.helper.test_vms[nodeId]
                workset[nodeId] = []
                workset[nodeId] = create_batch_for_node( TestOpCode.DelDn,
                    i, self.helper.numVMs, nodeId, server_ip, protocol, port, bind_dn, bind_pwd, test_data, batch_range)
                # workset >> workset.txt
                #with open('workset/DelDn-%s.txt' % datetime_2_str(datetime.now()), 'w') as f:
                #    pickle.dump(workset, f)
            #3.2 execute "DelDn" work batch on each node
            if "D" in self.helper.cfg["opCodes"]:
                for i, nodeId in enumerate(self.helper.nodeIds):
                    execution_report, stats = execute_batch_on_node(batch_num, workset, nodeId)
                    batch_report.extend(execution_report)
                    for dur in ["batch", "daily", "total"]:
                        for op in ["C","R","U","D"]:
                            summary[dur][op] += stats[op]
            #3.3 verify "DelDn" work batch has been replicated to other nodes
            if "DD" in self.helper.cfg["opCodes"]:
                for i, nodeId in enumerate(self.helper.nodeIds):
                    verification_report, stats = verify_batch_on_other_node(batch_num,
                                        workset, i, nodeId, self.helper.nodeIds, self.helper.test_vms, protocol, port)
                    batch_report.extend(verification_report)
                    for dur in ["batch", "daily", "total"]:
                        for op in ["C","R","U","D"]:
                            summary[dur][op] += stats[op]
            # generate batch_report for the batch, reset batch couters
            timer["batch"]["end"] = time.time()
            timer["daily"]["end"] = timer["batch"]["end"]
            timer["total"]["end"] = timer["batch"]["end"]
            # add to database
            count = reduce(int.__add__, summary["batch"].itervalues(), 0)
            if count >= STAT_COUNT_MIN:
                db_insert_perfsummaries(test_run_id, 'batch', timestamp_2_str(timer["batch"]["start"]),
                    timestamp_2_str(timer["batch"]["end"]), str(batch_num),
                    summary["batch"]['C'], summary["batch"]['R'], summary["batch"]['U'], summary["batch"]['D'],
                    count, timer["batch"]["end"] - timer["batch"]["start"])
                msg = 'It takes %8.2f seconds to finish batch %d on all nodes: [C:%d R:%d U:%d D:%d]\n' % (
                    timer["batch"]["end"] - timer["batch"]["start"], batch_num,
                    summary["batch"]['C'], summary["batch"]['R'], summary["batch"]['U'], summary["batch"]['D'] )
                report_print(msg)
                batch_report.append(msg)
            # reset timer and stats for batch summary
            timer["batch"]["start"] = timer["batch"]["end"]
            for dur in ["batch"]:
                for op in ["C","R","U","D"]:
                    summary[dur][op] = 0
            # batch_report > batch_timestamp.txt
            now = datetime.now()
            timestamp = str(now).replace(':', '-').replace(' ', '_')
            #with open('report/batch_%s.txt' % timestamp, 'w') as f:
            #    for r in batch_report:
            #        f.write(str(r))
            # if reportDuration is met, generate summary for the day and re-initiate daily summary counters
            if timer["daily"]["end"] - timer["daily"]["start"] >= reportDuration:
                count = reduce(int.__add__, summary["daily"].itervalues(), 0)
                db_insert_perfsummaries(test_run_id, 'daily', timestamp_2_str(timer["daily"]["start"]),
                    timestamp_2_str(timer["daily"]["end"]), -1,
                    summary["daily"]['C'], summary["daily"]['R'], summary["daily"]['U'], summary["daily"]['D'],
                    count, timer["daily"]["end"] - timer["daily"]["start"])
                msg = 'It takes %8.2f seconds to perform daily: [C:%d R:%d U:%d D:%d]\n' % (
                    timer["daily"]["end"] - timer["daily"]["start"],
                    summary["daily"]['C'], summary["daily"]['R'], summary["daily"]['U'], summary["daily"]['D'] )
                report_print(msg)
                daily_report.append(msg)
                # reset timer and stats for daily summary
                timer["daily"]["start"] = timer["daily"]["end"]
                for dur in ["daily"]:
                    for op in ["C","R","U","D"]:
                        summary[dur][op] = 0
                # daily_report > daily_timestamp.txt
                #with open('report/daily_%s.txt' % timestamp[:10], 'a') as f:
                #    for r in daily_report:
                #        f.write(str(r))
                daily_report = []
                # also generate checkpoint summary to db
                count = reduce(int.__add__, summary["total"].itervalues(), 0)
                db_insert_perfsummaries(test_run_id, 'total', timestamp_2_str(timer["total"]["start"]),
                    timestamp_2_str(timer["total"]["end"]), -1,
                    summary["total"]['C'], summary["total"]['R'], summary["total"]['U'], summary["total"]['D'],
                    count, timer["total"]["end"] - timer["total"]["start"])
            # forward to the next batch
            if per_batch_count >= STAT_COUNT_MIN:
                report_print('Batch ops end:   %d/%d/%d' % (batch_num_old, batch_num, batch_start_num + num_batches ))
            batch_num = batch_num_old + 1

        # Test finished successfull, add final summary to db
        count = reduce(int.__add__, summary["total"].itervalues(), 0)
        db_insert_perfsummaries(test_run_id, 'total', timestamp_2_str(timer["total"]["start"]),
            timestamp_2_str(timer["total"]["end"]), -1,
            summary["total"]['C'], summary["total"]['R'], summary["total"]['U'], summary["total"]['D'],
            count, timer["total"]["end"] - timer["total"]["start"])
        report_print('Done')
        self.helper.verbose_print('Done')
        self.helper.debug_print('---->')

    def test_scenario_3_1(self):
        """Add the same attribute on multiple servers at the same time and expect conflict logging.
        """
        self.helper.debug_print('----<')
        #setup
        #doTest
        #verify
        #cleanup
        self.helper.debug_print('---->\n')

    def test_scenario_3_2(self):
        """Add the same attribute value on multiple servers at the same time and expect conflict logging.
        """
        self.helper.debug_print('----<')
        #setup
        #doTest
        #verify
        #cleanup
        self.helper.debug_print('---->\n')

    def test_scenario_3_3(self):
        """Modify the same attribute value on multiple servers with different values at the same time and expect conflict logging.
        """
        self.helper.debug_print('----<')
        #setup
        #doTest
        #verify
        #cleanup
        self.helper.debug_print('---->\n')

    def test_scenario_3_4(self):
        """Delete the same attribute value on mulpiple servers and expect conflict logging.
        """
        self.helper.debug_print('----<')
        #setup
        #doTest
        #verify
        #cleanup
        self.helper.debug_print('---->\n')

    def test_scenario_3_5(self):
        """Delete the same attribute on mulpiple servers and expect conflict logging.
        """
        self.helper.debug_print('----<')
        #setup
        #doTest
        #verify
        #cleanup
        self.helper.debug_print('---->\n')

def main():
    helper = vdcreptest_helper()
    #unittest.main() # run all tests

    """Suite 1, single node updates:
    """
    #suite_1_server_update = unittest.TestSuite()
    #suite_1_server_update.addTest(vdcreptest("test_scenario_1_0", helper))
    #suite_1_server_update.addTest(vdcreptest("test_scenario_1_1", helper))
    #suite_1_server_update.addTest(vdcreptest("test_scenario_1_2", helper))
    #suite_1_server_update.addTest(vdcreptest("test_scenario_1_3", helper))
    #suite_1_server_update.addTest(vdcreptest("test_scenario_1_4", helper))
    #suite_1_server_update.addTest(vdcreptest("test_scenario_1_5", helper))
    #suite_1_server_update.addTest(vdcreptest("test_scenario_1_6", helper))
    #suite_1_server_update.addTest(vdcreptest("test_scenario_1_7", helper))
    #suite_1_server_update.addTest(vdcreptest("test_scenario_1_8", helper))
    #suite_1_server_update.addTest(vdcreptest("test_scenario_1_9", helper))
    #suite_1_server_update.addTest(vdcreptest("test_scenario_1_10", helper))
    #unittest.TextTestRunner(verbosity=2).run(suite_1_server_update)

    """Suite 2, parallel (multi-thread) updates on multiple servers WITHOUT conflicts
    """
    #***   [] Perform all tests specified in Scenario 1 on EACH server, and verify on each server. Make sure each server works on a different set of objects to prevent conflicts.
    suite_2_n_server_no_conflict = unittest.TestSuite()
    suite_2_n_server_no_conflict.addTest(vdcreptest("test_scenario_2_1", helper))
    unittest.TextTestRunner(verbosity=2).run(suite_2_n_server_no_conflict)

    """Scenario 3, parallet (multi-thread) updates on multiple servers WITH conflicts:
    """
    #suite_3_n_server_with_conflict = unittest.TestSuite()
    #suite_3_n_server_with_conflict.addTest(vdcreptest("test_scenario_3_1", helper))
    #suite_3_n_server_with_conflict.addTest(vdcreptest("test_scenario_3_2", helper))
    #suite_3_n_server_with_conflict.addTest(vdcreptest("test_scenario_3_3", helper))
    #suite_3_n_server_with_conflict.addTest(vdcreptest("test_scenario_3_4", helper))
    #suite_3_n_server_with_conflict.addTest(vdcreptest("test_scenario_3_5", helper))
    #unittest.TextTestRunner(verbosity=2).run(suite_3_n_server_with_conflict)

    helper.teardown_vms()

test_run_id = str(datetime.now())[:-3]
db_conn = sqlite3.connect('testreport.db', isolation_level="IMMEDIATE" )

if __name__ == "__main__":
    main()

db_conn.close()
