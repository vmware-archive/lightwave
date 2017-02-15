package com.vmware.identity.ssoconfig;

import java.io.IOException;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.List;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

import org.kohsuke.args4j.CmdLineParser;
import org.kohsuke.args4j.Option;
import org.kohsuke.args4j.ParserProperties;

import com.vmware.identity.diagnostics.DiagnosticsLoggerFactory;
import com.vmware.identity.diagnostics.IDiagnosticsLogger;

/**
 * Base class of all sso-config commands.
 *
 * Sub command classes should be extended from this base class with the suffix "Command".
 * For example, "AuthnPolicyCommand" is a subclass and provide command line options for tenant's authentication policy configuration.
 * SSO config sub commands are generated from the subclass names. For example, "AuthnPolicyCommand" is converted to "authn-policy".
 */
public abstract class SSOConfigCommand {

    @Option(name = "-t", aliases = {"--tenant"}, metaVar = "[Tenant Name]", usage = "Tenant name.")
    protected String tenant = SSOConfigurationUtils.DEFAULT_TENANT;

    private static final IDiagnosticsLogger logger = DiagnosticsLoggerFactory.getLogger(SSOConfigCommand.class);

    private ParserProperties properties;
    protected CmdLineParser parser;

    protected static final String HELP_CMD = "-h";
    private static final String CLASS_FILE_SUFFIX = ".class";
    private static final String COMMAND_CLASS_SUFFIX = "Command";
    private static final String PKG_SEPARATOR = ".";
    private static final String DIR_SEPARATOR = "/";
    private static final String BASE_CLASS_NAME = "SSOConfigCommand";
    private static final int CMD_LINE_USAGE_LENGTH = 120;

    /**
     * The command to be executed as specified by the client.
     */
    protected String command;

    /**
     * Tokenized arguments, excluding command name.
     */
    protected List<String> arguments;

    /**
     * Gets the quick summary of what the command does.
     */
    public abstract String getShortDescription();

    protected void setCommandAndParseArguments(String command, List<String> arguments) {
        this.command = command;
        this.arguments = arguments;
        try {
            if (arguments.size() > 0 && arguments.get(0).equalsIgnoreCase(HELP_CMD)) {
                arguments = arguments.subList(1, arguments.size());
            }

            parser = new CmdLineParser(this, getParserProperties());
            parser.parseArgument(arguments);

            if (arguments.isEmpty()) {
                printUsage(parser);
                System.exit(1);
            }
        } catch (Exception e) {
            logger.error("Encountered an error when parsing command.", e);
            System.exit(1);
        }
    }

    /**
     * Get CommandLine Parser properties.
     *
     * @return
     */
    ParserProperties getParserProperties() {
        if (properties == null) {
            properties = ParserProperties.defaults();
            properties.withUsageWidth(CMD_LINE_USAGE_LENGTH);
        }
        return properties;
    }

    /**
     * Get all classes in the package with suffix "Command.class".
     *
     * @throws ClassNotFoundException
     * @throws IOException
     * @throws URISyntaxException
     */
    public static List<Class<?>> all() throws ClassNotFoundException, IOException, URISyntaxException {
        String pathToJar = SSOConfigCommand.class.getProtectionDomain().getCodeSource().getLocation().toURI().getPath();
        @SuppressWarnings("resource")
		JarFile jarFile = new JarFile(pathToJar);
        Enumeration<JarEntry> entries = jarFile.entries();
        ClassLoader classLoader = Thread.currentThread().getContextClassLoader();

        List<Class<?>> classes = new ArrayList<>();
        while (entries.hasMoreElements()) {
            JarEntry je = entries.nextElement();
            if (je.isDirectory() || !je.getName().endsWith(CLASS_FILE_SUFFIX)) {
                continue;
            }
            String fileName = je.getName();
            String className = fileName.substring(0, fileName.length() - CLASS_FILE_SUFFIX.length());
            if (className.endsWith(COMMAND_CLASS_SUFFIX) && !className.contains(BASE_CLASS_NAME)) {
                className = className.replace(DIR_SEPARATOR, PKG_SEPARATOR);
                classes.add(classLoader.loadClass(className));
            }
        }

        return classes;
    }

    /**
     * Execute the command.
     * @throws Exception
     *
     */
    protected abstract void execute() throws Exception;

    /**
     * Print command line usage of the current subclass.
     *
     */
    protected void printUsage(CmdLineParser parser) {
        System.out.println(getShortDescription() + "\n");
        parser.printUsage(System.out);
    }

    /**
     * Print general help message. Provide a list of sub commands.
     *
     */
    private static void printUsage() throws Exception {
        System.out.println(String.format("SSO Configuration Commands. Use %s for command line help.\n", HELP_CMD));
        // display short description for each sub command
        int displayWidth = 20;
        for (Class<?> clazz : all()) {
            SSOConfigCommand cmd = (SSOConfigCommand) clazz.newInstance();
            SSOConfigurationUtils.displayParamNameAndValue(cmd.getCommandName(), cmd.getShortDescription(), displayWidth);
        }
    }

    /**
     * Gets the command name.
     *
     * <p>
     * Converts Command class name to a command name.
     * For example, "AuthnPolicyCommand" is converted to "authn-policy".
     */
    public String getCommandName() {
        String name = getClass().getName();
        name = name.substring(name.lastIndexOf('.') + 1); // short name
        name = name.substring(0,name.length()-COMMAND_CLASS_SUFFIX.length()); // trim off the command

        return name.replaceAll("([a-z0-9])([A-Z])","$1-$2").toLowerCase();
    }

    /**
     * Creates a clone to be used to execute a command.
     */
    protected SSOConfigCommand createClone() {
        try {
            return getClass().newInstance();
        } catch (IllegalAccessException e) {
            throw new AssertionError(e);
        } catch (InstantiationException e) {
            throw new AssertionError(e);
        }
    }

    /**
     * Obtains a copy of the command for invocation.
     *
     * @throws IllegalAccessException
     * @throws InstantiationException
     */
    public static SSOConfigCommand clone(String cmdName) throws Exception {
        for (Class<?> clazz : all()) {
            SSOConfigCommand cmd = (SSOConfigCommand) clazz.newInstance();
            if(cmdName.equals(cmd.getCommandName())) {
                return cmd.createClone();
            }
        }
        return null;
    }

    /**
     * Main function.
     */
    public static void main(String[] args) {
        try {
            List<String> arguments = Arrays.asList(args);

            if (arguments.isEmpty() || arguments.get(0).equalsIgnoreCase(HELP_CMD)) {
                // print help message if arguments are not provided
                printUsage();
                System.exit(1);
            }

            String cmdName = arguments.get(0);
            SSOConfigCommand cmd = SSOConfigCommand.clone(cmdName);
            cmd.setCommandAndParseArguments(cmdName, arguments.subList(1, arguments.size()));
            cmd.execute();
        } catch (com.vmware.identity.rest.core.client.exceptions.client.UnauthorizedException ue) {
            logger.error("Token expired. Please use set-credentials command to retrieve token with admin username and password.");
            System.exit(1);
        } catch (Exception e) {
            logger.error("Encountered an error when executing the command.", e);
            System.exit(1);
        } finally {
            SSOConfigurationUtils.closeAllResources();
        }
    }
}
