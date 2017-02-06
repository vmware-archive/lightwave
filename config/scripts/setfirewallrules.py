import json
import syslog
import os.path
import sys
import subprocess

LIGHTWAVE_CHAIN = 'LIGHTWAVE'
dir_map = {'inbound': LIGHTWAVE_CHAIN}
IPTABLES = '/sbin/iptables'
IP6TABLES = '/sbin/ip6tables'

def log_error_and_return(stderr):
    syslog.syslog(syslog.LOG_ERR, "Error: " + stderr)
    return

def set_lightwave_chain():
    #create new chain
    cmd = [IPTABLES, '-N', LIGHTWAVE_CHAIN]
    (rc, stdout, stderr) = run_command(cmd)
    if rc != 0:
        log_error_and_return(stderr)

    #add chain to input
    cmd = [IPTABLES, '-t', 'filter', '-A', 'INPUT', '-j', LIGHTWAVE_CHAIN]
    (rc, stdout, stderr) = run_command(cmd)
    if rc != 0:
        log_error_and_return(stderr)

    #end chain
    cmd = [IPTABLES, '-A', LIGHTWAVE_CHAIN, '-j', 'RETURN']
    (rc, stdout, stderr) = run_command(cmd)
    if rc != 0:
        log_error_and_return(stderr)

def set_ip6_lightwave_chain():
    #create new chain
    cmd = [IP6TABLES, '-N', LIGHTWAVE_CHAIN]
    (rc, stdout, stderr) = run_command(cmd)
    if rc != 0:
        log_error_and_return(stderr)

    #add chain to input
    cmd = [IP6TABLES, '-t', 'filter', '-A', 'INPUT', '-j', LIGHTWAVE_CHAIN]
    (rc, stdout, stderr) = run_command(cmd)
    if rc != 0:
        log_error_and_return(stderr)

    #end chain
    cmd = [IP6TABLES, '-A', LIGHTWAVE_CHAIN, '-j', 'RETURN']
    (rc, stdout, stderr) = run_command(cmd)
    if rc != 0:
        log_error_and_return(stderr)

def flush_ip4rules():
    cmd = [IPTABLES, '--list']
    (rc, stdout, stderr) = run_command(cmd)
    if rc != 0:
        log_error_and_return(stderr)

    # No lightwave chain, nothing to do
    if not 'LIGHTWAVE' in stdout:
        return

    #Remove chain from input
    cmd = [IPTABLES, '-D', 'INPUT', '-j', LIGHTWAVE_CHAIN]
    (rc, stdout, stderr) = run_command(cmd)
    if rc != 0:
        log_error_and_return(stderr)

    # Remove all rules under chain
    cmd = [IPTABLES, '-F', LIGHTWAVE_CHAIN]
    (rc, stdout, stderr) = run_command(cmd)
    if rc != 0:
        log_error_and_return(stderr)

    # Remove chain
    cmd = [IPTABLES, '-X', LIGHTWAVE_CHAIN]
    (rc, stdout, stderr) = run_command(cmd)
    if rc != 0:
        log_error_and_return(stderr)

def flush_ip6rules():
    cmd = [IP6TABLES, '--list']
    (rc, stdout, stderr) = run_command(cmd)
    if rc != 0:
        log_error_and_return(stderr)

    # No lightwave chain to remove
    if not 'LIGHTWAVE' in stdout:
        return

    #Remove chain from input
    cmd = [IP6TABLES, '-D', 'INPUT', '-j', LIGHTWAVE_CHAIN]
    (rc, stdout, stderr) = run_command(cmd)
    if rc != 0:
        log_error_and_return(stderr)

    # Remove all rules under chain
    cmd = [IP6TABLES, '-F', LIGHTWAVE_CHAIN]
    (rc, stdout, stderr) = run_command(cmd)
    if rc != 0:
        log_error_and_return(stderr)

    # Remove chain
    cmd = [IP6TABLES, '-X', LIGHTWAVE_CHAIN]
    (rc, stdout, stderr) = run_command(cmd)
    if rc != 0:
        log_error_and_return(stderr)

def run_command(cmd, stdin=None):
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE, stdin=subprocess.PIPE)
    stdout, stderr = process.communicate(stdin)
    rc = process.wait()

    if rc == 0:
        stdout = stdout.rstrip()

    if stderr is not None:
        stderr = stderr.rstrip()

    return rc, stdout, stderr

def main():
    json_file = "/opt/vmware/share/config/firewall.json"
    error = 1
    syslog.openlog()
    syslog.syslog('Setting Lightwave firewall rules')


    if not os.path.exists(json_file):
        syslog.syslog(syslog.LOG_ERR, "Error: " + json_file + " not found")
        return

    with open(json_file) as file:
        rules = json.load(file)

    # remove existing rules
    flush_ip4rules()
    flush_ip6rules()

    # create chain
    set_lightwave_chain()
    set_ip6_lightwave_chain()

    #apply rules per json file
    for service in rules:
        if 'ip4_rules' in rules[service]:
            for ip4_rules in rules[service]['ip4_rules']:
                if 'port' in ip4_rules.keys():
                    cmd = [IPTABLES, '-I', dir_map[ip4_rules['direction']],
                           '-p', ip4_rules['protocol'], '--dport', ip4_rules['port'],
                           '-j', 'ACCEPT']
                else:
                    cmd = [IPTABLES, '-I', dir_map[ip4_rules['direction']], '-p',
                           ip4_rules['protocol'], '-j', 'ACCEPT']
                (rc, stdout, stderr) = run_command(cmd)
                if rc != 0:
                    syslog.syslog(syslog.LOG_ERR, "Error: Service " + service + " port " + ip4_rules['port'] +
                                  " rule was not applied. (" + stderr + ")")


        if 'ip6_rules' in rules[service]:
            for ip6_rules in rules[service]['ip6_rules']:
                cmd = [IP6TABLES, '-I', dir_map[ip6_rules['direction']], '-p', ip6_rules['protocol'],
                       '--dport', ip6_rules['port'], '-j', 'ACCEPT']
                (rc, stdout, stderr) = run_command(cmd)
                if rc != 0:
                    syslog.syslog(syslog.LOG_ERR, "Error: Service " + service + " port " + ip6_rules['port'] +
                                  " rule was not applied. (" + stderr + ")")


if __name__ == "__main__":
    main()
