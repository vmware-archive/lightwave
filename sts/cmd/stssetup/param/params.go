package param

import (
	"fmt"
	"net/url"
	"strings"

	"github.com/vmware/lightwave/sts/internal/pkg/config"
)

type UrlsParam []*url.URL

type StringsParam []string

type HttpHeadsParam []config.HttpHead

func (v *UrlsParam) String() string {
	if v == nil {
		return ""
	}

	sb := &strings.Builder{}

	for _, u := range ([]*url.URL)(*v) {
		if u != nil {
			if sb.Len() > 0 {
				sb.WriteString(",")
			}
			sb.WriteString(u.String())
		}
	}
	return sb.String()
}

func (v *UrlsParam) Set(val string) error {
	if v == nil {
		return fmt.Errorf("Unable set value to an nil parameter")
	}
	val = strings.TrimSpace(val)
	if len(val) <= 0 {
		return nil
	}
	urls := make([]*url.URL, 0)
	for _, vi := range strings.Split(val, ",") {
		u, err := url.Parse(vi)
		if err != nil {
			return err
		}
		urls = append(urls, u)
	}
	*v = urls
	return nil
}

func (v *UrlsParam) Get() interface{} {
	if v == nil {
		return []*url.URL{}
	}
	return ([]*url.URL)(*v)
}

func (v *StringsParam) String() string {
	if v == nil {
		return ""
	}
	return strings.Join(([]string)(*v), ",")
}

func (v *StringsParam) Set(val string) error {
	if v == nil {
		return fmt.Errorf("Unable set value to an nil parameter")
	}
	val = strings.TrimSpace(val)
	if len(val) <= 0 {
		return nil
	}
	*v = strings.Split(val, ",")
	return nil
}

func (v *StringsParam) Get() interface{} {
	if v == nil {
		return []string{}
	}
	return ([]string)(*v)
}

func (v *HttpHeadsParam) String() string {
	if v == nil {
		return ""
	}

	sb := &strings.Builder{}

	for _, h := range ([]config.HttpHead)(*v) {
		if sb.Len() > 0 {
			sb.WriteString(",")
		}
		sb.WriteString(h.String())
	}
	return sb.String()
}

func (v *HttpHeadsParam) Set(val string) error {
	if v == nil {
		return fmt.Errorf("Unable set value to an nil parameter")
	}
	val = strings.TrimSpace(val)
	if len(val) <= 0 {
		return nil
	}
	heads := make([]config.HttpHead, 0)
	for _, vi := range strings.Split(val, ",") {
		var h config.HttpHead
		err := h.From(vi)
		if err != nil {
			return err
		}
		heads = append(heads, h)
	}
	*v = heads
	return nil
}

func (v *HttpHeadsParam) Get() interface{} {
	if v == nil {
		return []config.HttpHead{}
	}
	return ([]config.HttpHead)(*v)
}
