#!/usr/bin/env python
# Copyright 2015 VMware, Inc.  All rights reserved. -- VMware Confidential

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


