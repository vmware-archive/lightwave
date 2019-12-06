package types

import "github.com/vmware/lightwave/sts/internal/pkg/diag"

type DeploymentInfo interface {
	PublicEndpoint() string

	SchemaVersion() string
	// todo: additional data
}

type DeploymentInfoBuilder interface {
	PublicEndpoint(ep string)

	SchemaVersion(sv string)

	Build() (DeploymentInfo, diag.Error)
}

func NewDeploymentInfoBuilder() DeploymentInfoBuilder {
	return &deploymentInfoBuilderImpl{}
}

// idm:"ldap:cn=Deployment;oc=lightwaveSTSDeployment;t=DeploymentInfo"
type deplInfoImpl struct {
	publicEndpoint string // idm:"ldap:name=lightwaveSTSPublicEndpoint;m=PublicEndpoint"
	schemaVersion  string // idm:"ldap:name=lightwaveSTSSchemaVersion;m=SchemaVersion"
}

func (d *deplInfoImpl) PublicEndpoint() string { return d.publicEndpoint }

func (d *deplInfoImpl) SchemaVersion() string { return d.schemaVersion }

type deploymentInfoBuilderImpl deplInfoImpl

func (b *deploymentInfoBuilderImpl) PublicEndpoint(ep string) { b.publicEndpoint = ep }

func (b *deploymentInfoBuilderImpl) SchemaVersion(sv string) { b.schemaVersion = sv }

func (b *deploymentInfoBuilderImpl) Build() (DeploymentInfo, diag.Error) {
	// todo: validations
	return (*deplInfoImpl)(b), nil
}
