package postclient

type PostClientInterface interface {
	Login(username string, password string) error

	DeleteCA(caID string) error
	DeleteCert(caID string, crlNumber string) error
}
