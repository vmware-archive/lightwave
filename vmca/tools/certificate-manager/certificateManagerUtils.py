#!/usr/bin/env python
# Copyright 2015 VMware, Inc.  All rights reserved. -- VMware Confidential

"""
Utility functions to support certificate-manager
"""

__author__ = 'Charudath Gopal (cgopal@vmware.com)'
__copyright__ = 'Copyright 2015, VMware Inc.'
__version__ = 1.0

import logging
import os
import sys
import errno

sys.path.append(os.environ['VMWARE_PYTHON_PATH'])

from utils import *
from cis.defaults import get_cis_log_dir, get_cis_data_dir, get_cis_tmp_dir, get_env_var

cfgKeys = list()
cfgValues = list()
resultCfg = list()
global logFile
isLinux = os.name == 'posix'
if not isLinux:
    import pywintypes
    import win32service as w32s
    import win32serviceutil as w32su

def show_progress(progress, appendMsg, msg='Status'):
    """
    Function to show progress messages in console
    :param progress: Progress %
    :param appendMsg: Message which needs to presented to user
    :param msg: Status message
    """
    value = ' '
    if '100' in str(progress) or 'failed' in appendMsg:
        value = '\n'
    appendMsg = 'Completed [{0}]{1}               '.format(appendMsg, value)
    sys.stdout.write("{0} : {1}% {2}  \r".format(msg, int(progress), appendMsg))
    sys.stdout.flush()


def get_log_file():
    """
    Function to get certificate-manager log file location
    :return: Returns certificate-manager log location
    """
    return logfile


def initialize_ops():
    """
    Function to setup logging and create required directories
    """
    global logfile
    # Setup up logging to file/syslog based on operation
    log_folder = 'vmca'
    if isLinux:
        log_folder = 'vmcad'
    logDir = os.path.join(get_cis_log_dir(), log_folder)
    setupLogging('certificate-manager', logMechanism='file', logDir=logDir)
    logfile = os.path.join(logDir, 'certificate-manager.log')

    # setup certificate manager directory
    directory = os.path.dirname(get_cert_dir())
    if not os.path.exists(directory):
        create_directory(directory)
        logging.info("Created the certificate-manager directory: {0}".format(directory))

    rollback_dir = os.path.dirname(get_rollback_cert_dir())
    if not os.path.exists(rollback_dir):
        create_directory(rollback_dir)
        logging.info('temp directory of certs : {0}'.format(rollback_dir))


def create_directory(directory):
    try:
        os.makedirs(directory)
    except OSError as e:
        if e.errno != errno.EEXIST:
            log_error_msg("ERROR: Failed to create directory ({0}): {1}".format(e.errno, e.strerror))
            raise e


def get_root_cert_dir():
    if isLinux:
        return '/var/lib/vmware/vmca/'
    else:
        dir = os.path.join(get_cis_data_dir(), 'vmca')
        return dir + os.path.sep


def get_cert_dir():
    directory = os.path.join(get_cis_data_dir(), 'certmanager')
    return directory + os.path.sep


def get_rollback_cert_dir():
    directory = os.path.join(get_cert_dir(), 'rollback')
    return directory + os.path.sep


def get_src_config_file_loc():
    if isLinux:
        return "/usr/lib/vmware-vmca/share/config/certool.cfg"
    else:
        return get_env_var('VMWARE_CIS_HOME') + os.path.sep + 'vmcad' + os.path.sep + 'certool.cfg'


def get_dest_config_file_loc():
    directory = os.path.join(get_cis_tmp_dir(), 'certool.cfg')
    return directory


def log_info_msg(msg):
    """
    Method to write messages to console as well as log
    """
    #Append new line to make sure progress messages are not over written
    print('')
    print(str(msg))
    logging.info(str(msg))


def log_error_msg(msg):
    """
    Method to write messages to console as well as log
    """
    #Append new line to make sure progress messages are not over written
    print('')
    print(str(msg))
    logging.error(str(msg))


def read_cfg_file(filename):
    with open(filename) as fp:
        for line in fp:
            if '=' in line:
                cfgKeys.append((line.split('=')[0]).replace('\t', '').strip())
                cfgValues.append((line.split('=')[1]).replace('\t', '').strip())


def write_cfg_file():
    file = open(get_dest_config_file_loc(), "wb")
    logging.info('Certool.cfg file contents.')
    for item in resultCfg:
        logging.info(item)
        file.write(item + "\n")
    file.close()


def prepare_cfg_file():
    """
    Method to prepare Certool.cfg file
    """
    str = 'Default'
    #If config file exists then reconfigure else create new using default certool.cfg file
    if check_file_exists(get_dest_config_file_loc()):
        if get_user_confirmation(Constants.RECONFIGURE_CFG_MSG):
            filename = get_dest_config_file_loc()
            str = 'Previous'
        else:
            return
    else:
        log_info_msg(Constants.CONFIGURE_CFG_MSG)
        filename = get_src_config_file_loc()
    read_cfg_file(filename)
    log_info_msg('Press Enter key to skip optional parameters or use {0} value.'.format(str))
    index = 0
    for item in cfgKeys:
        #Optional param will be commented, uncomment while reconfiguring
        if '#' in item:
            item = item.replace('#','')
        if item == 'Hostname':
            value = 'Enter valid Fully Qualified Domain Name(FQDN), For Example : example.domain.com'
        elif item == 'IPAddress':
            value = 'optional'
        else:
            #Show default or previous value
            value = '{0} value : {1}'.format(str, cfgValues[index])

        var = raw_input("\nEnter proper value for '{0}' [{1}] : ".format(item, value))
        #Validate Hostname, hostname should not be empty
        if item == 'Hostname':
            while var.strip() == '':
                log_info_msg('Hostname should not be empty, please enter valid FQDN.')
                var = raw_input("\nEnter proper value for '{0}' [{1}] : ".format(item, value))
        if not len(var.strip()) > 0:
            if value == 'optional':
                item = '#' + item
            else:
                var = cfgValues[index]
        # Validate IPAddress
        if item == 'IPAddress':
            while not isIPAddress(var):
                if not len(var.strip()) > 0:
                    item = '#' + item
                    break
                log_info_msg('Please enter IP address in valid IPV4 or IPV6 format')
                var = raw_input("\nEnter proper value for '{0}' [{1}] : ".format(item, value))
        # Restrict Country code to 2 letters
        if item == 'Country':
            while not len(var.strip()) == 2:
                log_info_msg('Enter valid 2 letter country code')
                var = raw_input("\nEnter proper value for '{0}' [{1}] : ".format(item, value))
        index += 1
        resultCfg.append(item + " = " + var)
    write_cfg_file()


def check_file_exists(filename):
    if '\"' in filename:
        filename = filename.replace('\"', '')
    elif '\'' in filename:
        filename = filename.replace('\'', '')
    return os.path.isfile(filename)


def get_user_confirmation(msg):
    logging.info(msg + '\n')
    msg = msg + ' : Option[Y/N] ? : '
    var = raw_input(msg)
    if var.strip() in ['y', 'Y']:
        logging.info('Answer : Y')
        return True
    elif var.strip() in ['n', 'N']:
        logging.info('Answer : N')
        return False
    else:
        get_user_confirmation('Please enter any value with in [Y/N].')

def ask_for_file_and_validate(msg, optional=False):
    log_info_msg(msg)
    if optional:
        log_info_msg('Optional parameter, press Enter key to skip')
    var = raw_input('File : ')
    if len(var.strip()) == 0 and optional:
        return ''
    while not check_file_exists(var.strip()):
        log_info_msg("Please provide valid file location, couldn't find file : " + var.strip())
        var = raw_input("File : ")
    #Remove quotes in filename
    if '\"' in var:
        var = var.replace('\"', '')
    elif '\'' in var:
        var = var.replace('\'', '')
    return var.strip()

def ask_for_output_file_path(msg, isDir=False):
    log_info_msg(msg)

    var = raw_input('Output directory path: ')
    realPath = os.path.realpath(var.strip())
    if isDir:
        if os.path.exists(realPath) == False:
            raise IOError ('Cannot find directory: ' + realPath)
        if os.access(realPath, os.W_OK) == False:
            raise PermissionError ('You do not have permission to write to this location')
    else:
        if os.path.exists(os.path.split(realPath)[0]) == False:
            raise IOError ('Cannot find parent directory location: ' + realPath)
        if os.access(os.path.split(realPath)[0], os.W_OK) == False:
            raise PermissionError ('You do not have permission to write to this location')
    return var.strip()

class Constants():
    """
    Class maintaining all constants and messages used by cert-manager
    """

    # Progress constants
    STARTING_SERVICES = 'starting services...'
    STOPPING_SERVICES = 'stopping services...'
    REPLACE_MACHINE_SSL = 'Replacing Machine SSL Cert...'
    REPLACE_SOLUTION_CERT = 'Replace {0} Cert...'
    REPLACE_ROOT_CERT = 'Replacing Root Cert...'
    REPLACED_ROOT_CERT = 'Replaced Root Cert...'
    ROLLBACK_MACHINE_SSL = 'Rollback Machine SSL Cert...'
    ROLLBACK_SOLUTION_CERT = 'Rollback {0} Cert...'
    ROLLBACK_ROOT_CERT = 'Rollback Root Cert...'
    REVERT_MACHINE_SSL = 'Revert Machine SSL Cert...'
    REVERT_SOLUTION_CERT = 'Revert {0} Cert...'
    REVERT_ROOT_CERT = 'Revert Root Cert...'
    RESET_MACHINE_SSL = 'Reset Machine SSL Cert...'
    RESET_SOLUTION_CERT = 'Reset {0} Cert...'
    RESET_ROOT_CERT = 'Reset Root Cert...'
    PUBLISHING_ROOT_CERT = 'Publishing Root cert...'
    TASK_COMPLETED = 'All tasks completed successfully'
    REVERT_TASK_COMPLETED = 'Revert completed successfully'
    RESET_TASK_COMPLETED = 'Reset completed successfully'
    ROLLBACK_TASK_COMPLETED = 'Rollback completed successfully'
    ROLLBACK_MSG = 'Operation failed, performing automatic rollback'
    MGMT_ROLLBACK_MSG = 'Operation failed, Automatic rollback is not supported in distributed setup.\nPlease perform revert operation in Platform Services Controller machine \'{0}\' and then perform revert operation in this machine'
    REVERT_ERROR_MSG = 'Revert operation failed'
    RESET_ERROR_MSG = 'Reset operation failed'
    ROLLBACK_ERROR_MSG = 'Rollback operation failed'
    REVERT_STATUS = 'Revert status'
    RESET_STATUS = 'Reset status'
    ROLLBACK_STATUS = 'Rollback Status'

    #Load certificate messages
    READ_ROOT_CRT = 'Please provide valid custom certificate for Root.'
    READ_ROOT_SIGNING_CRT = 'Please provide the signing certificate of the '
    READ_ROOT_KEY = 'Please provide valid custom key for Root.'
    READ_MACHINE_SSL_CRT = 'Please provide valid custom certificate for Machine SSL.'
    READ_MACHINE_SSL_KEY = 'Please provide valid custom key for Machine SSL.'
    READ_SOLUTION_USER_CRT = 'Please provide valid custom certificate for solution user store : '
    READ_SOLUTION_USER_KEY = 'Please provide valid custom key for solution user store : '
    READ_PRIVATEKEY_OP_PATH = 'Please provide a location for private key: '
    READ_CSR_OP_PATH = 'Please provide a location for CSR: '
    READ_CSR_OP_DIR_PATH = 'Please provide a directory location to write the CSR(s) and PrivateKey(s) to: '

    #General constants
    IP_LOCALHOST = 'localhost'
    CONTINUE_OPERATION = 'Continue operation'
    MACHINE_SSL_STORE = 'MACHINE_SSL_CERT'
    MACHINE_SSL_ALIAS = '__MACHINE_CERT'
    TRUSTED_ROOTS_STORE = 'TRUSTED_ROOTS'
    SERVICES_ALL = 'all'
    SERVICES_NON_CORE = 'non-core'
    ROOT_CERT = 'root.cer'
    ROOT_KEY = 'privatekey.pem'
    BKP_ROOT_CERT = 'root.cer.0'
    BKP_ROOT_KEY = 'privatekey.pem.0'
    TERMINATE_OP = '\nTerminating operation.'
    PASSWORD_ATTEMPTS_ERROR = 'Reached max number of attempts, terminating operation.'
    BKP_CERT_EXT = '_bkp.crt'
    BKP_KEY_EXT = '_bkp.priv'
    SSL_ROLLBACK_MSG = 'Performing rollback of Machine SSL Cert...'
    SOL_ROLLBACK_MSG = 'Performing rollback of Solution User Certs...'
    ROOT_ROLLBACK_MSG = 'Performing rollback of Root Cert...'
    ROLLBACK_FAILED_MSG = 'Error while performing rollback operation, please try Reset operation...'
    CERT_EXT = '.crt'
    KEY_EXT = '.priv'
    PUB_EXT = '.pub'
    RECONFIGURE_CFG_MSG = 'Certool.cfg file exists, Do you wish to reconfigure'
    CONFIGURE_CFG_MSG = 'Please configure certool.cfg file with proper values before proceeding to next step.'
    ROOT_PRIVATE_KEY_OUTPUT_FILENAME = 'root_signing_cert.key'
    ROOT_CSR_FILE_OUTPUT_FILENAME = 'root_signing_cert.csr'
    MACHINE_SSL_PRIVATE_KEY_OUTPUT_FILENAME = 'machine_ssl.key'
    MACHINE_SSL_CSR_OUTPUT_FILENAME = 'machine_ssl.csr'


    #service constants
    VMCA_SERVICE_LIN = 'vmcad'
    VMAFD_SERVICE_LIN = 'vmafdd'
    VMDIRD_SERVICE_LIN = 'vmdird'
    VMCA_SERVICE_WIN = 'VMWareCertificateService'
    VMAFD_SERVICE_WIN = 'VMWareAfdService'
    VMDIRD_SERVICE_WIN = 'VMWareDirectoryService'

    #Confirm Operation
    SSL_CONFIRM_MESSAGE = 'You are going to regenerate Machine SSL cert using VMCA'
    SSL_CUSTOM_CONFIRM_MESSAGE = 'You are going to replace Machine SSL cert using custom cert'
    SOL_CONFIRM_MSG = 'You are going to regenerate Solution User Certificates using VMCA'
    SOL_CUSTOM_CONFIRM_MSG = 'You are going to replace all Solution User Certificates using custom cert'
    ROOT_CONFIRM_MSG = 'You are going to regenerate Root Certificate and all other certificates using VMCA'
    CUSTOM_ROOT_CONFIRM_MSG = 'You are going to replace Root Certificate with custom certificate and regenerate all other certificates'
    ROOT_SSL_CONFIRM_MSG = 'Do you wish to replace Machine SSL Certificate with custom cert ?'
    ROOT_SOL_CERT_CONFIRM_MSG = 'Do you wish to replace all Solution User certificates with custom cert ?'
    REVERT_MSG = 'You are going to revert all certs with certs backed up in last operation.'
    MGMT_REVERT_MSG = 'You are going to revert Machine SSL certs and Solution user certs with certs backed up in last operation.\nIf you wish to revert Root Certificate please perform from Platform Services Controller machine {0}.'
    RESET_MSG = 'You are going to reset by regenerating Root Certificate and replace all certificates using VMCA'
    MGMT_RESET_MSG = 'You are going to reset by regenerating Machine SSL cert and Solution user certs.\nIf you wish to reset Root Certificate please perform from Platform Services Controller machine {0}.'
    BACKUP_STORE = 'BACKUP_STORE'
    if isLinux:
        EXIT_MSG = 'Note : Use Ctrl-D to exit.'
    else:
        EXIT_MSG = 'Note : Use Ctrl-Z and hit Enter to exit.'
