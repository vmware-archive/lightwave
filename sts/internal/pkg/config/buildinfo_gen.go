package config

import(
	"path/filepath"
	"fmt"
)

type BuildInfo interface {
	Major() uint32
	Minor() uint32
	Release() uint32
	Patch() uint32
	GitCommit() string

	PrefixDir() string
	LwBinDir() string
	StsBinDir() string
	StsSbinDir() string

	VersionInfo() string
}

func GetBuildInfo() BuildInfo {
	return buildinfo
}

type buildInfoImpl struct {
	lwBinDir string
}

var buildinfo = &buildInfoImpl{
	lwBinDir: filepath.Join("/opt/vmware", "bin"),
}

func (bi *buildInfoImpl) Major() uint32 {
	return 1
}

func (bi *buildInfoImpl) Minor() uint32 {
	return 3
}

func (bi *buildInfoImpl) Release() uint32 {
	return 1
}

func (bi *buildInfoImpl) Patch() uint32 {
	return 35
}

func (bi *buildInfoImpl) GitCommit() string {
	return "45883943"
}

func (bi *buildInfoImpl) PrefixDir() string {
	return "/opt/vmware"
}

func (bi *buildInfoImpl) LwBinDir() string {
	return bi.lwBinDir
}

func (bi *buildInfoImpl) StsBinDir() string {
	return "/opt/vmware/bin"
}

func (bi *buildInfoImpl) StsSbinDir() string {
	return "/opt/vmware/sbin"
}

func (bi *buildInfoImpl) VersionInfo() string{
	return fmt.Sprintf(
		"Version='%v.%v.%v.%v' GitCommit='%v'",
		bi.Major(), bi.Minor(), bi.Release(), bi.Patch(),
		bi.GitCommit())
}
