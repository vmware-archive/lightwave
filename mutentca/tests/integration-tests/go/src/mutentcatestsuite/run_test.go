package mutentcatestsuite

import (
	"flag"
	"fmt"
	"log"
	"os"
	"testing"

	"mutentcaclient"
	"postclient"
)

var lwhost = flag.String("lwhost", "", "IP/Hostname of Lightwave endpoint")
var mtcahost = flag.String("mtcahost", "", "IP/Hostname of Multi Tenant CA endpoint")
var lwdomain = flag.String("lwdomain", "", "System tenant domain")
var lwuser = flag.String("lwuser", "", "System tenant administrator username")
var lwpass = flag.String("lwpass", "", "System tenant password")

var mtcaclient mutentcaclient.MutentCAClientInterface
var postdbclient postclient.PostClientInterface
var config *mutentcaclient.TestConfig

func setup() (err error) {
	flag.Parse()
	config = &mutentcaclient.TestConfig{
		LWHost:     *lwhost,
		MTCAHost:   *mtcahost,
		LWDomain:   *lwdomain,
		LWUser:     *lwuser,
		LWPassword: *lwpass,
		LWAuthUser: fmt.Sprintf("%s@%s", *lwuser, *lwdomain),
	}

	mtcaclient = mutentcaclient.NewMutentCAClient(config.LWHost, config.MTCAHost)

	err = mtcaclient.Login(config.LWAuthUser, config.LWPassword)
	if err != nil {
		return fmt.Errorf("Failed to login. Lightwave Server: '%s', AuthUser: '%s', Error: '%+v'", config.LWHost, config.LWAuthUser, err)
	}

	// MTCAHost is the same POST host
	postdbclient = postclient.NewDbRestClient(config.LWHost, config.MTCAHost, config.LWDomain)
	err = postdbclient.Login(config.LWAuthUser, config.LWPassword)
	if err != nil {
		return fmt.Errorf("Failed to login. Lightwave Server: '%s', AuthUser: '%s', Error: '%+x'", config.LWHost, config.LWAuthUser, err)
	}

	return nil
}

// TestMain Entry point for the API tests
func TestMain(m *testing.M) {
	println("Starting Multi Tenant CA Test Suite")

	err := setup()
	if err != nil {
		log.Fatalf("Failed to setup Multi Tenant CA Test Suite. Error : '%+v'", err)
	}

	result := m.Run()

	println("Finished running Multi Tenant CA Test Suite")
	os.Exit(result)
}
