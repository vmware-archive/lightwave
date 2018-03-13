package oidc

import (
	"bytes"
	"crypto/rand"
	"encoding/base64"
	"encoding/json"
	"strings"
)

func generateRandom(n int) (string, error) {
	b := make([]byte, n)
	_, err := rand.Read(b)
	if err != nil {
		return "", err
	}

	return base64.RawURLEncoding.EncodeToString(b), nil
}

func encodeJSON(entity interface{}) (string, error) {
	jsonBuffer := new(bytes.Buffer)
	if err := json.NewEncoder(jsonBuffer).Encode(entity); err != nil {
		return "", err
	}
	json := jsonBuffer.String()
	json = strings.TrimSpace(json)
	return json, nil
}

func contains(array []string, val string) bool {
	for _, s := range array {
		if s == val {
			return true
		}
	}
	return false
}
