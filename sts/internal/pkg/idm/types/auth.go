package types

import (
	"github.com/vmware/lightwave/sts/internal/pkg/diag"
)

type AttributeValueFunc func(v string) diag.Error

type AttributeValue interface {
	ID() AttributeID
	Len() int
	Iterate(f AttributeValueFunc) diag.Error
}

type AttributeValueBuilder interface {
	ID(attr AttributeID)
	Add(v string)

	Build() (AttributeValue, diag.Error)
}

func NewAttributeValueBuilder(capacity int) AttributeValueBuilder {
	return &attributeValueBuilderImpl{val: make([]string, 0, capacity)}
}

type attributeValueImpl struct {
	id  AttributeID
	val []string
}

func (av *attributeValueImpl) Len() int {
	return len(av.val)
}
func (av *attributeValueImpl) ID() AttributeID {
	return av.id
}
func (av *attributeValueImpl) Iterate(f AttributeValueFunc) diag.Error {
	for _, r := range av.val {
		err := f(r)
		if err != nil {
			return err
		}
	}
	return nil
}

type attributeValueBuilderImpl attributeValueImpl

func (b *attributeValueBuilderImpl) ID(attr AttributeID) {
	b.id = attr
}

func (b *attributeValueBuilderImpl) Add(v string) {
	b.val = append(b.val, v)
}

func (b *attributeValueBuilderImpl) Build() (AttributeValue, diag.Error) {
	// todo: validations
	return (*attributeValueImpl)(b), nil
}

type AttributeFunc func(v AttributeValue) diag.Error

type AttributeValues interface {
	Value(id AttributeID) (AttributeValue, bool)
	Len() int
	// for iterating over values
	Iterate(f AttributeFunc)
}

type AttributeValuesBuilder interface {
	Add(av AttributeValue)

	Build() (AttributeValues, diag.Error)
}

func NewAttributeValuesBuilder(capacity int) AttributeValuesBuilder {
	return make(attributeValuesImpl, capacity)
}

type Credentials interface {
	//Method() LoginMethod
}

type UnamePwdCredentials interface {
	Credentials
	UserName() string // username
	Password() string //password
}

type attributeValuesImpl map[AttributeID]AttributeValue

func (av attributeValuesImpl) Value(id AttributeID) (AttributeValue, bool) {
	if av == nil {
		return nil, false
	}
	val, ok := av[id]
	return val, ok
}

func (av attributeValuesImpl) Len() int {
	if av == nil {
		return 0
	}

	return len(av)
}

// for iterating over values
func (av attributeValuesImpl) Iterate(f AttributeFunc) {
	if av != nil {
		for _, v := range av {
			f(v)
		}
	}
}

func (av attributeValuesImpl) Add(v AttributeValue) {
	av[v.ID()] = v
}

func (av attributeValuesImpl) Build() (AttributeValues, diag.Error) {
	// todo: any validations
	return av, nil
}
