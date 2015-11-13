#!/usr/bin/env python
# Copyright 2015 VMware, Inc.  All rights reserved. --

"""
Base script which takes care of all VECS/VMCA/DIR_CLI operations.
"""

__author__ = 'Charudath Gopal (cgopal@vmware.com)'
__copyright__ = 'Copyright 2015, VMware Inc.'
__version__ = 1.0

import subprocess
import logging
import os
import os.path
import getpass
import tempfile

from cis.defaults import def_by_os, get_component_home_dir, get_cis_log_dir
from cis.exceptions import *
from certificateManagerUtils import *
from utils import *

isLinux = os.name == 'posix'
if not isLinux:
    import pywintypes
    import win32service as w32s
    import win32serviceutil as w32su
    __SERVICE_CTL_PREFIX = '"' + os.getenv('VMWARE_CIS_HOME',"C:\\Program Files\\VMware\\vCenter Server") +'\\bin\\service-control.bat' + '"'
else:
    __SERVICE_CTL_PREFIX = 'service-control'
global password


def cli_path(cli_name):
    """
    This function return the install path of the provided cli
    :param cli_name: Name of cli
    :return: Install directory of cli
    """
    if cli_name == 'certool':
        component_dir = get_component_home_dir(def_by_os('vmca', 'vmcad'))
    else:
        component_dir = get_component_home_dir(def_by_os('vmafd', 'vmafdd'))
    cli_rel_path = def_by_os('bin/%s', '%s.exe') % cli_name
    return os.path.join(component_dir, cli_rel_path)

def start_service(service_name):
    """
    This function starts the given service using service-control
    :param service_name: Name of service to be started
    """
    cmd = __SERVICE_CTL_PREFIX + " --start " + service_name
    try:
        execute_command(cmd)
    except InvokeCommandException as e:
        msg = 'Error while starting service : {0}'.format(service_name)
        log_error_msg(msg)
        e.appendErrorStack(msg)
        raise e


def stop_services(service):
    """
    This function stops all or non-core services using service-control
    :param service: If service is 'all' the all services will be stopped
    """
    cmd = __SERVICE_CTL_PREFIX + ' --stop --ignore '
    if 'all' in service:
        cmd = cmd + ' --all'
    try:
        execute_command(cmd)
    except InvokeCommandException as e:
        msg = 'Error while stopping services'
        log_error_msg(msg)
        e.appendErrorStack(msg)
        raise e
    logging.info('{0} services stopped successfully.'.format(service))


def start_services(service):
    """
    This function starts all or non-core services using service-control
    :param service: If service is 'all' the all services will be started
    :param service:
    """
    cmd = __SERVICE_CTL_PREFIX + ' --start '
    if 'all' in service:
        cmd = cmd + ' --all'
    try:
        execute_command(cmd)
    except InvokeCommandException as e:
        msg = 'Error while starting services, please see log for more details'
        log_error_msg(msg)
        e.appendErrorStack(msg)
        raise e
    logging.info('{0} services started successfully.'.format(service))


def vmafd_machine_id():
    """
    This function returns the machine ID of the local machine
    :return: machine ID
    """
    cli = cli_path('vmafd-cli')
    cmd = [cli, 'get-machine-id', '--server-name', 'localhost']
    logging.info('Running command : ' + str(cmd))
    try:
        (code, result, err) = run_command(cmd, None, True)
        logging.info('Output : \n {0}'.format(str(result)))
    except InvokeCommandException as e:
        msg = 'Error in retrieving machine id of local machine.'
        e.appendErrorStack(msg)
        log_error_msg(msg)
        raise e
    return result.strip()


def execute_command(cmd, quiet=False):
    """
    This function is responsible for executing command
    :param cmd: Command to be executed
    :param quiet: Flag to turnoff logging for this command
    :return:
    """
    if '--password' in cmd:
        #Donot print password
        tmp = list(cmd)
        tmp[len(tmp)-1] = '*****'
        logging.info('Running command : ' + str(tmp))
    else:
        logging.info("Running command :- " + str(cmd))

    # Service control command logs in to console even though logs are redirected to file,
    # so use default log directory and push logs instead of console
    if 'service-control' in cmd:
        quiet = True
        out_file = open(get_log_file(), 'a')
        logging.info('please see service-control.log for service status')
        p = subprocess.Popen(cmd, stdout=open(os.devnull, 'w'), stderr=out_file, shell=True)
        (output, err) = p.communicate()
        code = p.wait()
    else:
        (code, output, err) = run_command(cmd, None, True)
        logging.info("Command output :- \n {0}".format(str(output)))
    if isinstance(code, int):
        if code == 0:
            pass
        else:
            msg = (str(output))
            if quiet:
                logging.error(msg)
            else:
                log_error_msg(msg)
            raise InvokeCommandException(msg)
    else:
        for status in code:
            if status == 0:
                pass
            else:
                msg = (str(output))
                if quiet:
                    logging.error(msg)
                else:
                    log_error_msg(msg)
                raise InvokeCommandException(msg)
    logging.info("Command executed successfully")

def read_and_validate_password():
    """
    This function is to read sso password from user and authenticate which will further used for
    certificate operations
    """
    global password
    dir_cli = DirCliOps()
    log_info_msg('Please provide valid SSO password to perform certificate operations.')
    password = getpass.getpass()
    result = authenticate_password(dir_cli)
    for i in reversed(range(1, 3)):
        if result:
            logging.info('Authentication successful')
            return
        else:
            log_info_msg('Incorrect Password! Try Again! ({0} attempt/s left)'.format(i))
            password = getpass.getpass()
            result = authenticate_password(dir_cli)
            if result:
                logging.info('Authentication successful')
                return
    log_info_msg(Constants.PASSWORD_ATTEMPTS_ERROR)
    exit(1)

def authenticate_password(dir_cli):
    """
    Function to authenticate SSO password
    :param dir_cli:
    :return:
    """
    try:
        if password.strip() == '':
            logging.info('password should not be empty')
            return False
        dir_cli.get_services_list()
        return True
    except Exception as e:
        return False

class VecsOps():
    """
    This Class Implements functions that are used to perform VECS operations
    """

    def __init__(self):
        self._cli = cli_path('vecs-cli')
        self._management_node = False
        self._infra_node = False
        dir_cli = DirCliOps()
        services = dir_cli.get_services_list()
        if not check_file_exists(get_root_cert_dir() + Constants.ROOT_CERT):
            self._management_node = True

        self._solution_user_stores = []
        stores = self.list_stores()
        if 'vpxd' not in stores:
            self._infra_node = True
        if not Constants.BACKUP_STORE in stores:
            self.store_create(Constants.BACKUP_STORE)
            logging.info('Successfully created BACKUP_STORE in VECS')
        for store in stores:
            for service in services:
                if (store in service) and (store not in self._solution_user_stores):
                    self._solution_user_stores.append(store)

    def is_solution_user_store(self, store):
        """
        Function to check given store is a solution user store
        :param store: name of the store
        :return: 'True' is given store is a solution user store
        """
        return store in self._solution_user_stores

    def entry_delete(self, store_name, alias):
        """
        Function to delete entry in the VECS store
        :param store_name: Name of the store
        :param alias: entry alias which needs to deleted
        """
        cmd = [self._cli, 'entry', 'delete',
               '-y',
               '--store', store_name,
               '--alias', alias]
        try:
            execute_command(cmd)
        except InvokeCommandException as e:
            msg = 'Error in deleting entry {0} from VECS Store {1}.'.format(alias, store_name)
            e.appendErrorStack(msg)
            raise e

    def store_create(self, store_name):
        """
        Function to create store in VECS
        :param store_name: Name of the store to be created
        """
        cmd = [self._cli, 'store', 'create',
               '--name', store_name]
        try:
            execute_command(cmd)
        except InvokeCommandException as e:
            msg = 'Error in creating store {0} in vecs.'.format(store_name)
            e.appendErrorStack(msg)
            raise e

    def entry_create(self, store_name, alias, cert_path, private_key_path):
        """
        Function to create a new entry in the VECS store
        :param store_name: Name of the store where entry needs to be created
        :param alias: Alias name for new entry
        :param cert_path: certificate file path
        :param private_key_path: private key file path
        """
        cmd = [self._cli, 'entry', 'create',
               '--store', store_name,
               '--alias', alias,
               '--cert', cert_path]
        if len(private_key_path) > 0:
            cmd.append('--key')
            cmd.append(private_key_path)
        try:
            execute_command(cmd)
        except InvokeCommandException as e:
            msg = 'Error in creating a new entry for {0} in VECS Store {1}.'.format(alias, store_name)
            e.appendErrorStack(msg)
            raise e

    def get_cert_file(self, store_name, alias, outFile, quiet=False):
        """
        Function to backup cert to a file
        :param store_name: Name of the store where certificate resides
        :param alias: Alias name of certificate
        :param outFile: Certificate output file
        :param quiet: Flag to mute logging
        """
        cmd = [self._cli, 'entry', 'getcert',
               '--store', store_name,
               '--alias', alias,
               '--output', outFile]
        try:
            execute_command(cmd, quiet)
        except InvokeCommandException as e:
            msg = 'Error while creating backup cert file for ' + store_name
            e.appendErrorStack(msg)
            raise e

    def get_key_file(self, store_name, alias, outFile, quiet=False):
        """
        Function to backup key to a file
        :param store_name: Name of the store where certificate resides
        :param alias: Alias name of certificate
        :param outFile: Certificate output file
        :param quiet: Flag to mute logging
        """
        cmd = [self._cli, 'entry', 'getkey',
               '--store', store_name,
               '--alias', alias,
               '--output', outFile]
        try:
            execute_command(cmd, quiet)
        except InvokeCommandException as e:
            msg = 'Error while creating backup key file for ' + store_name
            e.appendErrorStack(msg)
            raise e

    def list_stores(self):
        """
        Function to lists all VECS stores
        :return: Returns available stores
        """
        cmd = [self._cli, 'store', 'list']
        logging.info('Running command : ' + str(cmd))
        try:
            (code, result, err) = run_command(cmd, None, True)
            logging.info('Output :\n' + str(result))
        except InvokeCommandException as e:
            msg = 'Error in generating list of VECS store instances.'
            log_error_msg(msg)
            e.appendErrorStack(msg)
            logging.error("Output : " + result)
            logging.error("StdErr : " + err)
            raise e
        return result.splitlines()

    def list_entries(self, store_name):
        """
        Function to list the entries in the VECS store
        :param store_name: Name of the store whose entries needs to be listed
        :return: Returns entries from store
        """
        cmd = [self._cli, 'entry', 'list', '--store', store_name]
        logging.info('Running command : ' + str(cmd))
        try:
            (code, result, err) = run_command(cmd, None, True)
            logging.info('Output :\n' + str(result))
        except InvokeCommandException as e:
            msg = 'Error in listing entries in VECS Store %s.'.format(store_name)
            log_error_msg(msg)
            e.appendErrorStack(msg)
            logging.error("Output : " + result)
            logging.error("StdErr : " + err)
            raise e
        # Just return the aliases
        lines = [l for l in result.splitlines() if l.startswith('Alias')]
        aliases = [l.split('\t')[1] for l in lines]
        return aliases

    def get_cert(self, store , alias):
        """
        Function to getcert from VECS store
        :param store: Name of store
        :param alias: Alias name of the entry
        :return: Returns certificate
        """
        cmd = [self._cli, 'entry', 'getcert', '--text',
               '--store',store,
               '--alias', alias]
        logging.info('Running command : ' + str(cmd))
        try:
            (code, result, err) = run_command(cmd, None, True)
            logging.info('Output :\n' + str(result))
        except InvokeCommandException as e:
            msg = 'Error in getting cert.'
            log_error_msg(msg)
            e.appendErrorStack(msg)
            logging.error("Output : " + result)
            logging.error("StdErr : " + err)
            raise e
        return result.strip()

class DirCliOps():
    """
    This Class Implements functions that are used to perform DIR-CLI operations
    """
    def __init__(self):
        self._cli = cli_path('dir-cli')

    def update_lotus(self, service, cert, ignoreError=False):
        """
        This function update lotus service for the given cert
        :param service: Service name of solution user
        :param cert: certificate to be used by update command
        :param ignoreError: Flag to suppress error in case of revert/rollback operation
        """
        logging.info("Update Lotus with the new Solution User Cert using dir-cli for : " + service)
        logging.info("Do a service update to update the account with a new cert in Lotus...")
        cmd = [self._cli, 'service', 'update',
               '--cert', cert,
               '--name', service.strip(),
               '--password', password]
        try:
            execute_command(cmd, ignoreError)
        except InvokeCommandException as e:
            if ignoreError:
                msg = 'Ignoring dir-cli update command error thrown while rollback operation'
                logging.warning(msg)
                return
            msg = 'Error while updating lotus for service : ' + service
            e.appendErrorStack(msg)
            raise e

    def trusted_cert_publish(self, cert_path):
        """
        Function to publish certificate using dir-cli
        :param cert_path: certificate file which needs to be published
        """
        cmd = [self._cli, 'trustedcert', 'publish',
               '--cert', cert_path,
               '--password', password]
        try:
            execute_command(cmd)
        except InvokeCommandException as e:
            msg = 'Error while publishing cert using dir-cli.'
            e.appendErrorStack(msg)
            raise e

    def get_services_list(self):
        """
        Function to get available services list from lotus
        :return: Returns services list from lotus
        """
        cmd = [self._cli, 'service', 'list', '--password', password]
        #Donot print password
        tmp = list(cmd)
        tmp[len(tmp)-1] = '*****'
        logging.info('Running command : ' + str(tmp))
        try:
            (code, result, err) = run_command(cmd, None, True)
            logging.info("Output : \n" + str(result))
            if result.strip() == '':
                raise InvokeCommandException('Failed to get service list using dir-cli')
        except InvokeCommandException as e:
            msg = 'Error while getting service account name using dir-cli'
            e.appendErrorStack(msg)
            logging.error("Output : " + str(result))
            logging.error("Error ({0} : {1})".format(str(code), str(err)))
            raise e
        return result.splitlines()

    def get_service_name_for_solution_user(self, solution_user):
        """
        Function to parse and return service name for a given solution user
        :param solution_user: Solution user name whose service name is required
        :return: Returns service name from lotus
        """
        machine_id = vmafd_machine_id()
        for line in self.get_services_list():
            if (solution_user + "-" + machine_id) in line:
                return line.rstrip()[3:]
        return ''


class VmcaOps():
    """
    This Class Implements functions that are used to perform VMCA operations
    """
    def __init__(self):
        self._cli = cli_path('certool')

    def generate_cert(self, service_acc_name, server):
        """
        Function to generate certificate for given service account
        :param service_acc_name: Name of the store
        :param server: Provide PSC/Infra IP in case of distributed env else 'localhost'
        """
        # Service account name would be same as VECS store name.
        logging.info("Generating cert for store : " + service_acc_name)
        logging.info("Generating key pair...")
        # Generate Private Key and Public Keys First
        cmd = [self._cli,
               '--genkey',
               '--privkey=' + get_cert_dir() + service_acc_name + Constants.KEY_EXT,
               '--pubkey=' + get_cert_dir() + service_acc_name + Constants.PUB_EXT,
               '--server=' + server]
        try:
            execute_command(cmd)
        except InvokeCommandException as e:
            msg = 'Error in generating Private and Public Keys.'
            e.appendErrorStack(msg)
            raise e
        logging.info("Generating cert...")
        cmd = [self._cli,
               '--server=' + server,
               '--gencert',
               '--privkey=' + get_cert_dir() + service_acc_name + Constants.KEY_EXT,
               '--cert=' + get_cert_dir() + service_acc_name + Constants.CERT_EXT,
               '--config=' + get_dest_config_file_loc()]
        try:
            execute_command(cmd)
        except InvokeCommandException as e:
            msg = 'Error in generating cert for store {0}'.format(service_acc_name)
            e.appendErrorStack(msg)
            raise e

    def generate_solution_user_cert(self, service_acc_name, server):
        """
        Function to generate solution user certificate
        :param service_acc_name: Name of the store
        :param server: Provide PSC/Infra IP in case of distributed env else 'localhost'
        """
        # Service account name would be same as VECS store name.
        logging.info("Generating solution user cert for : " + service_acc_name)
        logging.info("Generating key pair...")
        # Generate Private Key and Public Keys First
        cmd = [self._cli,
               '--genkey',
               '--privkey=' + get_cert_dir() + service_acc_name + Constants.KEY_EXT,
               '--pubkey=' + get_cert_dir() + service_acc_name + Constants.PUB_EXT,
               '--server=' + server]
        try:
            execute_command(cmd)
        except InvokeCommandException as e:
            msg = 'Error in generating Private and Public Keys.'
            e.appendErrorStack(msg)
            raise e
        logging.info("Generating cert...")
        cmd = [self._cli,
               '--server=' + server,
               '--genCIScert',
               '--privkey=' + get_cert_dir() + service_acc_name + Constants.KEY_EXT,
               '--cert=' + get_cert_dir() + service_acc_name + Constants.CERT_EXT,
               '--Name=' + service_acc_name]
        try:
            execute_command(cmd)
        except InvokeCommandException as e:
            msg = 'Error in generating cert for store {0}'.format(service_acc_name)
            e.appendErrorStack(msg)
            raise e

    def get_root_ca(self, server):
        """
        Function to get root signing certificate
        :param server: Provide PSC/Infra IP in case of distributed env else 'localhost'
        :return: Returns root signing certificate as text
        """
        cmd = [self._cli, '--getrootca', '--server', server]
        logging.info('Running command : ' + str(cmd))
        try:
            (code, result, err) = run_command(cmd, None, True)
            logging.info("Output : \n" + str(result))
            if result.strip() == '':
                raise InvokeCommandException('Failed to get RootCA')
        except InvokeCommandException as e:
            msg = 'Error while getting root certificate using certool getRootCa command'
            log_error_msg(msg)
            e.appendErrorStack(msg)
            logging.error("Output : " + str(result))
            logging.error("Error ({0} : {1})".format(str(code), str(err)))
            raise e
        return result


    def selfca(self, server):
        """
        Function to regenerate Root signing certificate using VMCA
        :param server: Provide PSC/Infra IP in case of distributed env else 'localhost'
        """
        cmd = [self._cli, '--selfca',
               '--config', get_dest_config_file_loc(),
               '--server', server]
        try:
            execute_command(cmd)
        except InvokeCommandException as e:
            msg = 'Error while generating root cert using selfca command.'
            e.appendErrorStack(msg)
            raise e

    def rootca(self, cert_path, key_path, server):
        """
        Function to publish custom certificate as Root signing certificate
        :param cert_path: Custom certificate file path
        :param key_path: Custom key file path
        """
        cmd = [self._cli, '--rootca',
               '--cert', cert_path,
               '--privkey', key_path,
               '--server', server]
        try:
            execute_command(cmd)
        except InvokeCommandException as e:
            msg = 'Error while performing certool rootca command'
            e.appendErrorStack(msg)
            raise e

    def generateCSR(self, cert_path, key_output_path, csr_output_path, server):
        """
        Function to generate CSR
        :param cert_path: certificate file path
        :param key_path:  output key path
        :param csr_output_path: output csr path
        """

        if not os.path.isfile(cert_path):
            raise FileNotFoundError ('Cannot find certificate file')
        pubKeyTempPath = os.path.join(tempfile.gettempdir(), 'pubkey.pub')

        logging.info("Generating key ")
        cmd = [self._cli, '--genkey',
               '--privkey', key_output_path,
               '--pubkey' , pubKeyTempPath]
        logging.info('Running command: '+ str(cmd))
        try:
            result = invoke_command(cmd)
        except InvokeCommandException as e:
            msg = 'Error in generating Private Key'
            e.appendErrorStack(msg)
            raise e
        os.remove(pubKeyTempPath)

        cmd = [self._cli, '--gencsrfromcert',
              '--privkey', key_output_path,
              '--cert',cert_path,
              '--csrfile', csr_output_path]
        logging.info('Running command: ' + str(cmd))
        try:
            result = invoke_command(cmd)
        except InvokeCommandException as e:
            msg = 'Error in generating CSR'
            e.appendErrorStack(msg)
            raise e

