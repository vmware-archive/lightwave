package mutentcaclient

// TestConfig - structure to load configuration for test execution
type TestConfig struct {
	// IP/Hostname for Lightwave endpoint
	LWHost string `yaml:"lwhost"`
	// IP/Hostname for Multi Tenant CA endpoint
	MTCAHost string `yaml:"mtcahost"`
	// Lighwave system tenant domain
	LWDomain string `yaml:"lwdomain"`
	// Lighwave Admin user
	LWUser string `yaml:"lwuser"`
	// Lighwave Admin password
	LWPassword string `yaml:"lwpass"`
	// User used to get Auth token from Lightwave = User@Domain
	LWAuthUser string
}
