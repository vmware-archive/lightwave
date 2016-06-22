/* **********************************************************************
 * Copyright 2015 VMware, Inc.  All rights reserved. VMware Confidential
 * *********************************************************************/

package com.vmware.identity.configure;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.attribute.PosixFilePermission;
import java.util.EnumSet;
import java.util.Set;

import org.apache.commons.lang.SystemUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class HostnameWriter {
    private static final String _filename = InstallerUtils.HOSTNAME_FILE;
    private static final String _configPath = InstallerUtils
            .getInstallerHelper().getConfigFolderPath();
    private String hostname;

    private static final Logger log = LoggerFactory
            .getLogger(HostnameWriter.class);

    public HostnameWriter(String hostname) {
        Validate.validateNotEmpty(hostname, "Hostname");

        this.hostname = hostname;
    }

    public void write() throws HostnameCreationFailedException {

        createConfigDirectory();

        String filePath = Paths.get(_configPath, _filename).toString();
        String tempFilePath = String.format("%s.tmp", filePath);
        File oldPath = null;

        try {
            oldPath = createTemporaryFile(tempFilePath);
            File newPath = new File(filePath);

            boolean success = oldPath.renameTo(newPath);
            if (!success) {
                throw new HostnameCreationFailedException();
            }
        } catch (IOException exception) {
            log.debug(exception.getStackTrace().toString());

            throw new HostnameCreationFailedException(exception);
        } finally {
            if (oldPath != null && oldPath.exists()) {
                oldPath.delete();
            }
        }

    }

    private void createConfigDirectory() throws HostnameCreationFailedException {
        Path configDirPath = Paths.get(_configPath);
        if (Files.notExists(configDirPath)) {
            try {
                Files.createDirectories(configDirPath);
                if (SystemUtils.IS_OS_LINUX) {
                    Set<PosixFilePermission> perms = EnumSet.of(
                            PosixFilePermission.OWNER_READ,
                            PosixFilePermission.OWNER_WRITE,
                            PosixFilePermission.OWNER_EXECUTE,
                            PosixFilePermission.GROUP_READ,
                            PosixFilePermission.GROUP_EXECUTE,
                            PosixFilePermission.OTHERS_READ,
                            PosixFilePermission.OTHERS_EXECUTE);

                    Files.setPosixFilePermissions(configDirPath, perms);
                }
            } catch (IOException e) {
                throw new HostnameCreationFailedException(e);
            }
        }
    }

    private File createTemporaryFile(String path) throws IOException {
        File file = new File(path);
        Writer writer = null;

        try {
            writer = new BufferedWriter(new OutputStreamWriter(
                    new FileOutputStream(file), "utf-8"));
            writer.write(hostname);

            return file;
        } finally {
            writer.close();
        }
    }
}
