package main

import (
	"fmt"
	"io/ioutil"
	"os"
	"strings"
	"time"

	"github.com/vmware/lightwave/sts/cmd/stssetup/install"
	"github.com/vmware/lightwave/sts/cmd/stssetup/param"
	"github.com/vmware/lightwave/sts/internal/pkg/config"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

var lgLvl int
var lgLoc string

func main() {

	err := run()
	if err != nil {
		os.Exit(1)
	}
}

func run() error {

	if len(os.Args) < 2 {
		fmt.Println("Invalid parameters")
		usage()
		fmt.Println("")
		return fmt.Errorf("Invalid parameters")
	}

	cmd, ok := commands[strings.ToLower(os.Args[1])]
	if !ok {
		fmt.Println("Invalid parameters")
		usage()
		fmt.Println("")
		return fmt.Errorf("Invalid parameters")
	}

	err := cmd.Parse(os.Args[2:])
	if err != nil {
		return fmt.Errorf("Invalid parameters for '%s' command: %v", cmd.Name(), err)
	}

	var ctxt diag.RequestContext
	var logger diag.Logger

	if cmd.LogLvl() > -1 {
		var logFile *os.File
		if len(cmd.LogLocation()) > 0 {
			logFile, err = os.Create(cmd.LogLocation())
			if err != nil {
				fmt.Printf("Unable to create log file '%s': %v", lgLoc, err)
				fmt.Println("")
				return err
			}
		} else {
			logFile, err = ioutil.TempFile("", "stssetup_*.log")
			if err != nil {
				fmt.Printf("Unable to create temp lof file '%s': %v", "stssetup_*.log", err)
				fmt.Println("")
				return err
			}
			fmt.Printf("Setup log will be saved to '%s'", logFile.Name())
			fmt.Println()
		}
		defer logFile.Close()

		logger, err = diag.NewCmdLogger(cmd.LogLvl(), logFile)
		if err != nil {
			fmt.Printf("Failed to initialize logging: %v", err)
			fmt.Println()
			return err
		}

		ctxt = diag.NewRequestContext(
			diag.NoneTenantID, fmt.Sprintf("stssetup-%v", time.Now().UTC()), logger)
	}

	err = cmd.Process(ctxt)
	if err != nil {
		if logger != nil {
			logger.Errorf(
				diag.SETUP,
				"Failed processing command '%s' command: '%v'", cmd.Name(), err)
		}
		return fmt.Errorf("command execution failed with: %v", err)
	}

	return nil
}

var commands map[string]param.Command

func init() {
	commands = make(map[string]param.Command)

	setupCmd := install.NewSetupCmd()
	commands[strings.ToLower(setupCmd.Name())] = setupCmd
	versionCmd := &versionCmd{}
	commands[strings.ToLower(versionCmd.Name())] = versionCmd

	for _, c := range commands {
		c.RegisterParams()
	}
}

func usage() {
	fmt.Println("Usage:")
	fmt.Printf("%s <command> [<parameters>]", os.Args[0])
	fmt.Println("")
	fmt.Println("Where <command> is one of the following:")
	for _, c := range commands {
		fmt.Printf("    %s", c.Name())
		fmt.Println("")
		fmt.Printf("        %s", c.ShortDescription())
		fmt.Println("")
	}
	fmt.Printf("for more info on specific command run '%s <command> -help'", os.Args[0])
	fmt.Println()
}

type versionCmd struct{}

func (c *versionCmd) Name() string { return "version" }

func (c *versionCmd) ShortDescription() string {
	return "Show binary version information"
}
func (c *versionCmd) Parse(args []string) error { return nil }
func (c *versionCmd) LogLvl() int32             { return -1 }
func (c *versionCmd) LogLocation() string       { return "" }

func (c *versionCmd) RegisterParams() {}

func (c *versionCmd) Process(ctxt diag.RequestContext) error {
	bld := config.GetBuildInfo()
	fmt.Println(bld.VersionInfo())
	return nil
}
