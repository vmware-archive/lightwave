package main

import (
	"flag"
	"fmt"
	"os"

	"github.com/vmware/lightwave/sts/internal/pkg/config"
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
	"github.com/vmware/lightwave/sts/internal/pkg/idm"
	"github.com/vmware/lightwave/sts/web/app/sts"
	"github.com/vmware/lightwave/sts/web/static"
)

func main() {

	logger, err := diag.NewLogger()
	if err != nil {
		fmt.Printf("Error: %v", err)
		os.Exit(1)
	}

	var cfgLocation string
	flag.StringVar(&cfgLocation, "config", "", "full path to config file")
	flag.Parse()

	logger.Tracef(diag.SERVER, "Parsing config")
	instanceCfg, err := config.ReadConfig(cfgLocation, logger)
	if err != nil {
		logger.Errorf(diag.SERVER, "Failed to read config: %v", err)
		os.Exit(1)
	}

	idmSvcs, err := idm.NewServices(instanceCfg, logger)
	if err != nil {
		logger.Errorf(diag.SERVER, "Failed to create IDM services: %v", err)
		os.Exit(1)
	}

	logger.Tracef(diag.SERVER, "Initializing static content")
	err = static.Init(logger)
	if err != nil {
		logger.Errorf(diag.SERVER, "Failed to initialize static content: %v", err)
		os.Exit(1)
	}

	logger.Tracef(diag.SERVER, "Instantiating server")
	stsServer, err := sts.NewServer(instanceCfg, idmSvcs.Configurator(), idmSvcs.Authenticator(), logger)
	if err != nil {
		logger.Errorf(diag.SERVER, "Failed to create server instance: %v", err)
		os.Exit(1)
	}
	defer stsServer.Close()

	logger.Infof(diag.SERVER, "Start serving...")
	err = stsServer.Serve()
	if err != nil {
		logger.Errorf(diag.SERVER, "Stopped serving: %v", err)
	} else {
		logger.Infof(diag.SERVER, "Stopped serving")
	}
}
