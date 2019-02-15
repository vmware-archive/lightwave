/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the [0m~@~\License[0m~@~]); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an [0m~@~\AS IS[0m~@~] BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

/* Common */

#define         TEST_HTTP_TOK_TYPE_BEARER               "Bearer"
#define         TEST_HTTP_TOK_TYPE_HOTK                 "hotk-pk"

#define         TEST_HTTP_TOK_TYPE_UNKNOWN              "RandomTokenType"
#define         TEST_HTTP_UNKNOWN_TOK_VALUE             "RandomTokenvalue"

#define         TEST_BUILD_AUTH_HDR(_TYPE, _DATA) _TYPE " " _DATA
#define         TEST_BUILD_BEARER_AUTH_HDR(_TOK) TEST_BUILD_AUTH_HDR(TEST_HTTP_TOK_TYPE_BEARER, _TOK)
#define         TEST_BUILD_HOTK_AUTH_HDR(_TOK, _POP) TEST_BUILD_AUTH_HDR(TEST_HTTP_TOK_TYPE_HOTK, _TOK":"_POP)

#define         TEST_DC_NAME                            "dc.a.b"
#define         TEST_TENANT                             "a.b"

#define         TEST_TENANT_OIDC_SIGNING_CERT           "-----BEGIN CERTIFICATE-----\n" \
                                                        "MIIDjDCCAnSgAwIBAgIJAMY7o6atDknUMA0GCSqGSIb3DQEBCwUAME4xFTATBgNV\n" \
                                                        "BAMMDENBLERDPWEsREM9YjELMAkGA1UEBhMCVVMxKDAmBgNVBAoMH2x3dC0yNzgt\n" \
                                                        "aS0wNjZjYmJmNDRhNTA4NGEzOC5hLmIwHhcNMTkwMTMxMjAxODI3WhcNMjkwMTI1\n" \
                                                        "MjAyODEzWjAYMRYwFAYDVQQDDA1zc29zZXJ2ZXJTaWduMIIBIjANBgkqhkiG9w0B\n" \
                                                        "AQEFAAOCAQ8AMIIBCgKCAQEAoMxGU3WI2EAXl9AGhAQOSN8z5EVcPrROeYLxSFLu\n" \
                                                        "HDtnQciE3okl2IgPZLjDNerKP5RDdNpNmejNyYJNq8fqtYz3HTJw/zDQOWYXUbvB\n" \
                                                        "8GdEeWtzztXyR4EAPhumfoSHpcD85vQgegqywy3XKQWDzC69ko0rfOgW+OQKS3TH\n" \
                                                        "3HZCJ0z21fs8VmA6NWngz9ZPT2Fy9E0lducvLy65Rb3EF4k4UZOCTYwCsO+Kxc/g\n" \
                                                        "NpTJ4QArzwhukg/8WGS7HVzulwdRyX0NP18TgL//LWQtZS6LG3i18xBDVc0Vn3He\n" \
                                                        "EDGNZAjiDVEBbudlzk88V4yF7mAmgIpElBIsnj4Lnrz5XwIDAQABo4GiMIGfMAsG\n" \
                                                        "A1UdDwQEAwIF4DAdBgNVHQ4EFgQUWRhGNvYlc9qEQSZX9mvLbhFrUhEwHwYDVR0j\n" \
                                                        "BBgwFoAUlyCDfd/yQi5TvK/ILO72+kXZ3hYwUAYIKwYBBQUHAQEERDBCMEAGCCsG\n" \
                                                        "AQUFBzAChjRodHRwczovL2x3dC0yNzgtaS0wNjZjYmJmNDRhNTA4NGEzOC5hLmIv\n" \
                                                        "YWZkL3ZlY3Mvc3NsMA0GCSqGSIb3DQEBCwUAA4IBAQCexm2zVPHVofnNt7QGRiIX\n" \
                                                        "H+e3UD1Z+ysZiSzCLR3/8arox0b+dgtMNzrZPaikQnRCbtql4M85LaOlY+y4vEJe\n" \
                                                        "qf89LTDvrRCwcKEpiA43E0sOfGp6nfAM/WfohhuHUlTt+cZhfaaPtEtpWsxPImTS\n" \
                                                        "ainWdyVpiv/U3RW91C8CAQGoRddsTDKAOb+ZkqdfsCU5Q9RTXZqP0nFS95Pq9wMj\n" \
                                                        "tXFFl7clHsP3PhTJVy006U6Sz3ZtoFhy5NNdQ+jUVmBA+b0U5JkWJKO8iu8KXFTD\n" \
                                                        "9DT6xR/ic27cIBlcPM9A7k/MmSRciHQgBxcELCetzL0sVCDE8rl47hXtaq1lqZR1\n" \
                                                        "-----END CERTIFICATE-----"

#define         TEST_TENANT2_OIDC_SIGNING_CERT          "-----BEGIN CERTIFICATE-----\n" \
                                                        "MIIDpDCCAoygAwIBAgIJAMK3GsYWRKlHMA0GCSqGSIb3DQEBCwUAMGYxCzAJBgNV\n" \
                                                        "BAMMAkNBMREwDwYKCZImiZPyLGQBGRYBYTERMA8GCgmSJomT8ixkARkWAWIxCzAJ\n" \
                                                        "BgNVBAYTAlVTMSQwIgYDVQQKDBtsd3QtMjc4LWktMDA1ZjQ0YzU1MDVmOGUyMzAw\n" \
                                                        "HhcNMTkwMjE1MDEyODE3WhcNMjkwMTI2MjI0MTExWjAYMRYwFAYDVQQDDA1zc29z\n" \
                                                        "ZXJ2ZXJTaWduMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAkrzVSj2s\n" \
                                                        "s0y3zd6jVz8/qKoh5OysNYOb6v6vwhMb3dQLWsc60ZK9Npnagf9iOCTFPBZaLAIb\n" \
                                                        "OhlDhz41ykkqHjrgWyvmHA5WSn3lpDhsOUgF0J6oL7w10qIRoJbZTAZv+mHvl7Gz\n" \
                                                        "G0iC8zu3wf2j2on9cKJBldXvz9VU2xlQzwUbZwi29735+NR7p5Kw4gwjvtk3WKiQ\n" \
                                                        "2jsWm3f5CBXUjFrI36aaVFz1NPYRYbVTDLC1DfmAhBGhS+1eI9VItpxc1ao54U5k\n" \
                                                        "/izgr08NkWvMW9DtWApW4is24Pp3MGzIcgeLvI6uqLLZGH8Iimplm43M1CIu9/ht\n" \
                                                        "pa6TmhYvCIF/QwIDAQABo4GiMIGfMAsGA1UdDwQEAwIF4DAdBgNVHQ4EFgQU7bLn\n" \
                                                        "PT+GqLRIEBU0okrCKGOlZS4wHwYDVR0jBBgwFoAU7vF+MMLpernQpqrsgw0Izl7d\n" \
                                                        "HBwwUAYIKwYBBQUHAQEERDBCMEAGCCsGAQUFBzAChjRodHRwczovL2x3dC0yNzgt\n" \
                                                        "aS0wMDVmNDRjNTUwNWY4ZTIzMC5hLmIvYWZkL3ZlY3Mvc3NsMA0GCSqGSIb3DQEB\n" \
                                                        "CwUAA4IBAQCRlrs/qlVJ1TdECAl2t+AL1c/5eYeVweQBbhj9eR2ZzHqSWJApMJ6S\n" \
                                                        "LoQXa3lTfqVsktbQohlQV+ohFy3Hlzdu2pDw7XjLdSqvhQGrpCUYv2edF9D65Rzd\n" \
                                                        "dCJhJgcV43bC8KCbLXyncvUUayBCzv878An0YTYaxunzUcksiX6cbSt6mYMo3VV9\n" \
                                                        "nse3WUQ3QkYW6bgWE96ncgLblpPp6KKvO4Jyh0/Niklwv76Fsep1439YgoGxCj1h\n" \
                                                        "cS7yEWZQW5KiXoeEuzX9vwLxaVdXsVIfALFpfzPnr/p0sonXv72zvQOkO6W5heCh\n" \
                                                        "XyQ4Vz+oJSm12ifIRlT+Mil/atzqB4ws\n" \
                                                        "-----END CERTIFICATE-----"

/* Bearer Token */

#define         TEST_UPN1                               "Administrator@A.B"
#define         TEST_UPN1_DN                            "cn=administrator,cn=users,dc=a,dc=b"
#define         TEST_UPN1_GROUPS                        { "a.b\\Users", "a.b\\Administrators", "a.b\\CAAdmins", "a.b\\Everyone" }
#define         TEST_UPN1_NUM_GROUPS                    4

// Issued:  Wed Feb 20 2019 14:06:39 GMT-0800
// Expires: Web Aug 06 116098 06:06:39 GMT-0700
#define         TEST_UPN1_BEARER_JWT_VALID              "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJBZG1pbmlzdHJhdG9yQEEuQiIsImlzcyI6Imh0dHBzOi8vbHd0LTI3OC5jYXNjYWRlLWRldi5jb20vb3BlbmlkY29ubmVjdC9hLmIiLCJncm91cHMiOlsiYS5iXFxVc2VycyIsImEuYlxcQWRtaW5pc3RyYXRvcnMiLCJhLmJcXENBQWRtaW5zIiwiYS5iXFxFdmVyeW9uZSJdLCJ0b2tlbl9jbGFzcyI6ImFjY2Vzc190b2tlbiIsInRva2VuX3R5cGUiOiJCZWFyZXIiLCJhdWQiOlsiYWRtaW5pc3RyYXRvckBhLmIiLCJyc19tdXRlbnRjYSJdLCJzY29wZSI6InJzX211dGVudGNhIGF0X2dyb3VwcyBvcGVuaWQgaWRfZ3JvdXBzIiwibXVsdGlfdGVuYW50IjpmYWxzZSwiZXhwIjozNjAxNTUwNjk2Nzk5LCJpYXQiOjE1NTA3MDAzOTksImp0aSI6Img5TlFKRmxQRXBnLWJ5aVJPODE0UHUwWkdTZ0N0OV8tbW9EYVVjVnQwTzQiLCJ0ZW5hbnQiOiJhLmIifQ.aXlT2Q166DvNiPGSDCC6SyitpMLekll9oOtWRq7Qaj1mmlt6eflv03wjry-UNOXA1r77eonYqebAJw37zIE9WkJLHFyYtGqkq3apyx40c38VMgTv-dSIXo-VAAo4c8HqaAjUgTjSTYparspvJYHwPjv9O2kjRKDunM25BNkDHdBZF_so6YHexDG2cIMWq4gpvYYJvhUlVM-df50ro4b6EO_kUDcNCPMbzpRNmkpaL6A9HrBpNocKXazGWjfO5-ugTBf_Qgi78X50enYgqObj3bNXaByrjKIBEiJiBy_4Wuqg-qFtKPXJ_r_9uK2O1h4b2wgI9FxkxzUSWBRw1TfbCA"

// Issued:  Wed Feb 20 2019 14:06:39 GMT-0800
// Expires: Web Aug 06 116098 06:06:39 GMT-0700
#define         TEST_UPN1_BEARER_JWT_BAD_AUD            "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJBZG1pbmlzdHJhdG9yQEEuQiIsImlzcyI6Imh0dHBzOi8vbHd0LTI3OC5jYXNjYWRlLWRldi5jb20vb3BlbmlkY29ubmVjdC9hLmIiLCJncm91cHMiOlsiYS5iXFxVc2VycyIsImEuYlxcQWRtaW5pc3RyYXRvcnMiLCJhLmJcXENBQWRtaW5zIiwiYS5iXFxFdmVyeW9uZSJdLCJ0b2tlbl9jbGFzcyI6ImFjY2Vzc190b2tlbiIsInRva2VuX3R5cGUiOiJCZWFyZXIiLCJhdWQiOlsiYWRtaW5pc3RyYXRvckBhLmIiXSwic2NvcGUiOiJhdF9ncm91cHMgb3BlbmlkIGlkX2dyb3VwcyIsIm11bHRpX3RlbmFudCI6ZmFsc2UsImV4cCI6MzYwMTU1MDY5Njc5OSwiaWF0IjoxNTUwNzAwMzk5LCJqdGkiOiJoOU5RSkZsUEVwZy1ieWlSTzgxNFB1MFpHU2dDdDlfLW1vRGFVY1Z0ME80IiwidGVuYW50IjoiYS5iIn0.TRDLiZzMaW3GXZ1iBQsSzTWr3SItDXTVdxNoFcvEZ7ObR_FpIfV8PG0vBHbhpqdxAC6wroYeKiKbe09t2pOqv9j790rA0FM7EBro22AU8VZzuIGXsFJr2Vq7C1sG1dsQMFX94Of3JK4q6ffmIs7nkwM_LovVK7Zc2Cop3gPYDtlIexHy_d4Vp1saN2mPIvyCYyh6iFFOsAp4-6Dsri2VcayCsV5uvMMRjWY16a-vF0a3S_kl2P3i498TqQBR6VDHxc7kvdJVfbqBGw63aL62xOOeoVlf0LbgnYbbfdeMz0knCDsGDmC_GNFwJ7owpdF8Y-iUIV63laYQRKxiGSjMbg"

// Issued:  Wed Feb 20 2019 14:06:39 GMT-0800
// Expires: Web Aug 06 116098 06:06:39 GMT-0700
#define         TEST_UPN1_BEARER_JWT_EXPIRED            "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJBZG1pbmlzdHJhdG9yQEEuQiIsImlzcyI6Imh0dHBzOi8vbHd0LTI3OC5jYXNjYWRlLWRldi5jb20vb3BlbmlkY29ubmVjdC9hLmIiLCJncm91cHMiOlsiYS5iXFxVc2VycyIsImEuYlxcQWRtaW5pc3RyYXRvcnMiLCJhLmJcXENBQWRtaW5zIiwiYS5iXFxFdmVyeW9uZSJdLCJ0b2tlbl9jbGFzcyI6ImFjY2Vzc190b2tlbiIsInRva2VuX3R5cGUiOiJCZWFyZXIiLCJhdWQiOlsiYWRtaW5pc3RyYXRvckBhLmIiLCJyc19tdXRlbnRjYSJdLCJzY29wZSI6InJzX211dGVudGNhIGF0X2dyb3VwcyBvcGVuaWQgaWRfZ3JvdXBzIiwibXVsdGlfdGVuYW50IjpmYWxzZSwiZXhwIjoxNTUwNjIwNDgzLCJpYXQiOjE1NTA1MzQwODMsImp0aSI6Img5TlFKRmxQRXBnLWJ5aVJPODE0UHUwWkdTZ0N0OV8tbW9EYVVjVnQwTzQiLCJ0ZW5hbnQiOiJhLmIifQ.jgSvnBS5TkNyCAptVEX6dbsHUOGfBNCn4nRMAA80zRNZwCuUTyvcM0TXrOcS060w8xIWDue01WNl5_Y31I9BqHOC_C-unWZgwIs9qH-D6Clls9Ia8-8pe18Y8GWEk3bN2AHxcxv0VWmW7ubR-DAf7murw4YjoJYTkzxiv7YF810GiiT6bsCK79pIylfebWEqnlRXxKuXcpIXn-gnWrfUTvE185lDf_f9_7hbrnWDk5iVmRcivLxjlvxAObbKnCqn41Y0l39VSE17Cy8G9hWcrlQFDM23lUNZkioIKaeo-OhveGhJHDYVX9247iik6ZhtcTbAATEFc7uX1_a1TQlJiA"

/* HOTK-PK Token */

#define         TEST_SRV_UPN1                           "example-svc-acct@a.b"
#define         TEST_SRV_UPN1_DN                        "cn=example-svc-acct,cn=serviceprincipals,dc=a,dc=b"
#define         TEST_SRV_UPN1_GROUPS                    { "a.b\\CAAdmins", "a.b\\SolutionUsers", "a.b\\Everyone" }
#define         TEST_SRV_UPN1_NUM_GROUPS                3

// Issued:  Wed Feb 20 2019 14:06:39 GMT-0800
// Expires: Web Aug 06 116098 06:06:39 GMT-0700
#define         TEST_SRV_UPN1_HOTK_JWT                  "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJleGFtcGxlLXN2Yy1hY2N0QGEuYiIsImlzcyI6Imh0dHBzOi8vbHd0LTI3OC5jYXNjYWRlLWRldi5jb20vb3BlbmlkY29ubmVjdC9hLmIiLCJncm91cHMiOlsiYS5iXFxDQUFkbWlucyIsImEuYlxcU29sdXRpb25Vc2VycyIsImEuYlxcRXZlcnlvbmUiXSwidG9rZW5fY2xhc3MiOiJhY2Nlc3NfdG9rZW4iLCJ0b2tlbl90eXBlIjoiaG90ay1wayIsImhvdGsiOnsia2V5cyI6W3sia3R5IjoiUlNBIiwiZSI6IkFRQUIiLCJ1c2UiOiJzaWciLCJhbGciOiJSUzI1NiIsIm4iOiJ1MFdnVHlUanJZU0gwQkZaMGdra092M2RYbG9Da0F1S18tc3UtdmtLaGwwenh4NUJuV0d0V0NMVGx0R2tlS2NyeVNVN2VwdW01cDhMaUdyYlAya1V1XzA2X0tqZmd1MkV6RUtuZzAzbVVGZDhtcVdCXzF5SnNaMndoYTE2Mjl5TFYyVGdXNll1eVJXY3lyNkNhUFlUSUpVMk15cEtDWVlJS0ZqeXhrYW9qQlg5aTllNkI0LXhqTndRS0VHTWh3U2lGYkkxTFROYVVlSEdUR0I1aWVoSmpWSWJVaTh4dDRGWDQzMWFhM1A2R0ZKaEFzLTI1Q2xwM3hkTUMya1hwOGdCbDljQnVxYTJHWEVoemx6aGdyTnhQb1lBdDJjNVVLeDFzN3ZlcEViVHhLWjhIWUdEb0s0QS1jZXVJWnc2RXpjTjNCRWR0aTVKMG1qQVltdjJtakZaaXcifV19LCJhdWQiOlsiZXhhbXBsZS1zdmMtYWNjdEBhLmIiLCJyc19tdXRlbnRjYSJdLCJzY29wZSI6Im9wZW5pZCBpZF9ncm91cHMgYXRfZ3JvdXBzIHJzX211dGVudGNhIiwibXVsdGlfdGVuYW50Ijp0cnVlLCJleHAiOjM2MDE1NTA2OTY3OTksImlhdCI6MTU1MDc3NzY0NywianRpIjoiVkthSng3U0Ffekx1eDd1Ym5TOWROM0JDQVIxcVVCMzdUOUF2NUpWZDJFWSIsInRlbmFudCI6ImEuYiJ9.W1vsu4vv0cQ0rDtGH_YRYnoyHPHU-jLPrK1VmKad216peb4PXPo6aRMIqqXEFM0xTmbYPP3c_Ojg2tLUtMi-_3JxOOPW1yCLLuLOyB3VgCfzIgJlw514kc3h9ZjlCgPPzsDzy0QI4b6vW9gWYeUW5mQjGM0SZPN7jZdYYEWdivPvSbM6bAeO52GT_uPyaDYEazyGJYV5G1wPwJjWrHj4-44rRbHoP15dMiVK_N8fdHb8QJVjVe--a8g6KjjNf1YRFCtuwMaPiyAgEj5cl7pIKrd8Z4eQvfnksjaLIp-HLzr8uuCXoR3TC8JXcaZ7F0F0NpK_1XEO2wTNrEL1hzrKKA"

// Issued:  Wed Feb 20 2019 14:06:39 GMT-0800
// Expires: Web Aug 06 116098 06:06:39 GMT-0700
#define         TEST_SRV_UPN1_HOTK_JWT_BAD_AUD          "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJleGFtcGxlLXN2Yy1hY2N0QGEuYiIsImlzcyI6Imh0dHBzOi8vbHd0LTI3OC5jYXNjYWRlLWRldi5jb20vb3BlbmlkY29ubmVjdC9hLmIiLCJncm91cHMiOlsiYS5iXFxDQUFkbWlucyIsImEuYlxcU29sdXRpb25Vc2VycyIsImEuYlxcRXZlcnlvbmUiXSwidG9rZW5fY2xhc3MiOiJhY2Nlc3NfdG9rZW4iLCJ0b2tlbl90eXBlIjoiaG90ay1wayIsImhvdGsiOnsia2V5cyI6W3sia3R5IjoiUlNBIiwiZSI6IkFRQUIiLCJ1c2UiOiJzaWciLCJhbGciOiJSUzI1NiIsIm4iOiJ1MFdnVHlUanJZU0gwQkZaMGdra092M2RYbG9Da0F1S18tc3UtdmtLaGwwenh4NUJuV0d0V0NMVGx0R2tlS2NyeVNVN2VwdW01cDhMaUdyYlAya1V1XzA2X0tqZmd1MkV6RUtuZzAzbVVGZDhtcVdCXzF5SnNaMndoYTE2Mjl5TFYyVGdXNll1eVJXY3lyNkNhUFlUSUpVMk15cEtDWVlJS0ZqeXhrYW9qQlg5aTllNkI0LXhqTndRS0VHTWh3U2lGYkkxTFROYVVlSEdUR0I1aWVoSmpWSWJVaTh4dDRGWDQzMWFhM1A2R0ZKaEFzLTI1Q2xwM3hkTUMya1hwOGdCbDljQnVxYTJHWEVoemx6aGdyTnhQb1lBdDJjNVVLeDFzN3ZlcEViVHhLWjhIWUdEb0s0QS1jZXVJWnc2RXpjTjNCRWR0aTVKMG1qQVltdjJtakZaaXcifV19LCJhdWQiOlsiZXhhbXBsZS1zdmMtYWNjdEBhLmIiXSwic2NvcGUiOiJvcGVuaWQgaWRfZ3JvdXBzIGF0X2dyb3VwcyIsIm11bHRpX3RlbmFudCI6dHJ1ZSwiZXhwIjozNjAxNTUwNjk2Nzk5LCJpYXQiOjE1NTA3Nzc2NDcsImp0aSI6IlZLYUp4N1NBX3pMdXg3dWJuUzlkTjNCQ0FSMXFVQjM3VDlBdjVKVmQyRVkiLCJ0ZW5hbnQiOiJhLmIifQ.n6so3WD_cCsWP5_NmtRCVUttsh-xgpFL7dMExulh8gmCNMs7WBNWKNXxh-g_EO_-Q3UQnSFZaZPulV3EnBh9iH3859k3JyHA3x0WAvzmQCWIK8dWMWTpHrMgXuufeNv_o_JVS7OJdwvWDZxcqXjIdLQCyWDiD2yk-0Fa54QoRMgpI4fFAPBClhDHz0PJ-4PGwyJ2Me5GCUckii1w67bNE-YtQzKy7utyJ952cyqrl54Sm_fJwZAWplrn2HisOS2_7xs8Zwj1faPZ7VrpDm6KkkPLXBVx-KLSGksFsdXIYOb8qdBC_UkK0Hapr90xLYpwUSuW62PUlhJoJmJp8KW_NQ"

// Issued:  Wed Feb 20 2019 14:06:39 GMT-0800
// Expires: Web Aug 06 116098 06:06:39 GMT-0700
#define         TEST_SRV_UPN1_HOTK_JWT_EXPIRED          "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJleGFtcGxlLXN2Yy1hY2N0QGEuYiIsImlzcyI6Imh0dHBzOi8vbHd0LTI3OC5jYXNjYWRlLWRldi5jb20vb3BlbmlkY29ubmVjdC9hLmIiLCJncm91cHMiOlsiYS5iXFxDQUFkbWlucyIsImEuYlxcU29sdXRpb25Vc2VycyIsImEuYlxcRXZlcnlvbmUiXSwidG9rZW5fY2xhc3MiOiJhY2Nlc3NfdG9rZW4iLCJ0b2tlbl90eXBlIjoiaG90ay1wayIsImhvdGsiOnsia2V5cyI6W3sia3R5IjoiUlNBIiwiZSI6IkFRQUIiLCJ1c2UiOiJzaWciLCJhbGciOiJSUzI1NiIsIm4iOiJ1MFdnVHlUanJZU0gwQkZaMGdra092M2RYbG9Da0F1S18tc3UtdmtLaGwwenh4NUJuV0d0V0NMVGx0R2tlS2NyeVNVN2VwdW01cDhMaUdyYlAya1V1XzA2X0tqZmd1MkV6RUtuZzAzbVVGZDhtcVdCXzF5SnNaMndoYTE2Mjl5TFYyVGdXNll1eVJXY3lyNkNhUFlUSUpVMk15cEtDWVlJS0ZqeXhrYW9qQlg5aTllNkI0LXhqTndRS0VHTWh3U2lGYkkxTFROYVVlSEdUR0I1aWVoSmpWSWJVaTh4dDRGWDQzMWFhM1A2R0ZKaEFzLTI1Q2xwM3hkTUMya1hwOGdCbDljQnVxYTJHWEVoemx6aGdyTnhQb1lBdDJjNVVLeDFzN3ZlcEViVHhLWjhIWUdEb0s0QS1jZXVJWnc2RXpjTjNCRWR0aTVKMG1qQVltdjJtakZaaXcifV19LCJhdWQiOlsiZXhhbXBsZS1zdmMtYWNjdEBhLmIiLCJyc19tdXRlbnRjYSJdLCJzY29wZSI6Im9wZW5pZCBpZF9ncm91cHMgYXRfZ3JvdXBzIHJzX211dGVudGNhIiwibXVsdGlfdGVuYW50Ijp0cnVlLCJleHAiOjE1NTA3Mjc2NDcsImlhdCI6MTU1MDYyMDQ4MywianRpIjoiVkthSng3U0Ffekx1eDd1Ym5TOWROM0JDQVIxcVVCMzdUOUF2NUpWZDJFWSIsInRlbmFudCI6ImEuYiJ9.ZbkgZBP7XBkeNub0mNBSU8wG5RcKWRBSe98lktx2ZWhLpslYVkkqlB3HQklKHvJPaVVKAk4wISt1u4McphST27V2P4YTEJ-pCE7rBUkxGU9kNF99yhO96aHKaHbcHMGt9r3WrsX4KWuo1iuS0ObdjfIGgsiiBAUmpfjm7NreFA8w6V7pq2cCBdIARXzpMVEMCs-J8fXL5C7LVGgYhPTlHH99i5FXO9KIVXm9xhbK7rVDAipCwQbh4PQ-h5TYNjANcGCMDZIhpqccBK3lYOqJ91CSmopWh_KIRnszYvMnVtc07wdsVvklZxnHnezqzDS7JBqpiN-adyEq_WjINvnB0A"

#define         TEST_SRV_UPN1_REQ1_POP                  "95632210b07fd4e6b6b04f250aee6bf043c95f33168fe3fd16f74d86176c4e9f3a2cb9a824ff914f16cce2c1f6984cd3e9b652efc95f32ddc3fee8f965304a5ed5ac046fdf23af1c83cf486ce41d17e913749b455522a91297afe527aef0d7910970e0d8ca3e2308a60bd19e68e928582221127ea4b90c31fb9c2257fb9b75f3b73fa8bd3afb3940d60c39e042e3f935ed1b5002b70de094b3eb2127ebc3d7d7fee40a2b6bf016a31147c0f6160e458b08dd05a29a92a89a91c8ddf657f80d0eabee3d859796f6f980b9b262c49a79e668f86767b12a7aac443d1abd4519acac1c1b5bb3d6fd8ef9f429a494c0c278cbbcb219ecfd43b9a2bc2e0dd005d40c26"
#define         TEST_SRV_UPN1_REQ1_METHOD               "GET"
#define         TEST_SRV_UPN1_REQ1_CONTENTTYPE          "application/json"
#define         TEST_SRV_UPN1_REQ1_DATE                 "Fri, 15 Feb 2019 22:53:30 GMT"
#define         TEST_SRV_UPN1_REQ1_BODY                 ""
#define         TEST_SRV_UPN1_REQ1_URI                  "/v1/mutentca/root"

#define         TEST_SRV_UPN1_REQ2_POP                  "272e01ee10004718abb74621f1561b3f84afa0346060c1f6ba9f088212f0ad1d2d93cb6407b893581dff0a52650cf343f1a3e6541dfc6feb461d8d72e15b907b215655e1364216165f9de1c7b266db693d4ccb445f2f87982702533309e0fe8f88b09cb185a2495a5be6c6b6fcf2a2b860281ee2f1189c25da3a54c6b0cc54e93617b8796c6762a8e9e0927cd4be2b5873e8677340068ca4f1cd4d07c9bd9eb34d0b3020687f8ee904c8d7d67363d8cda361654e1e0f80a7885bfb7319aa3931da8897f56011cf9805ef163b33132dd203783669ab5f85c2474d745b19ddf6774382969b996bd88fd8b4a8fdbca21dcb1dc23f3576676f029dfcaa132a841b55"
#define         TEST_SRV_UPN1_REQ2_METHOD               "POST"
#define         TEST_SRV_UPN1_REQ2_CONTENTTYPE          "application/json"
#define         TEST_SRV_UPN1_REQ2_DATE                 "Thu, 21 Feb 2019 19:28:07 GMT"
#define         TEST_SRV_UPN1_REQ2_BODY                 "{\"caId\":\"testCA\"}"
#define         TEST_SRV_UPN1_REQ2_URI                  "/v1/mutentca/intermediate"

#define         TEST_SRV_UPN1_REQ3_POP                  "78f2cc93c38080ec2281118f051ba71bcdf6938584253b4eabe8a75440c445c77be2ba6f886b63680b4b926cac30a5689cacd99c4f17880348cd2e4653c2ded8e46cb4df4266a87d243c2b1a6f3d681131224785a3f0d9284baea89f9f578331ce00ac7f5f84bda1ea5dff8ab26c1fdb3a176f63b64f34e2326615a3680bb4e64393f18ba629880a192e4218ccfe857cee3a6379227363e4d5b282da986e6c66dd082af2353d54afb13c8343533bc71bf2d42bfdb73c22ce4b3ebba2c144ef3bdcc2cb4f006c4706fe5512273c4f6038e84ce0a082a9739a20e4de578131141902054d18e98703c96c383ac3b4768120337e98622683b5c64c8d8db2b1a23ef8"
#define         TEST_SRV_UPN1_REQ3_METHOD               "POST"
#define         TEST_SRV_UPN1_REQ3_CONTENTTYPE          "application/json"
#define         TEST_SRV_UPN1_REQ3_DATE                 "Thu, 21 Feb 2019 19:34:07 GMT"
#define         TEST_SRV_UPN1_REQ3_BODY                 "{ \"caId\" : \"testCA1\", \"parentCaId\": \"testCA\" }"
#define         TEST_SRV_UPN1_REQ3_URI                  "/v1/mutentca/intermediate"

// Same as valid Req3 POP, but two numbers are incremented
#define         TEST_SRV_UPN1_REQ3_BAD_POP              "78f2cc93c38080ec2281118f051ba71bcdf6938584253b4eabe8a75440c445c77be2ba6f886b63680b4b926cac30a5689cacd99c4f17880348cd2e4653c2ded8e47cb4df4266a87d243c2b1a6f3d681131224785a3f0d9284baea89f9f578331ce00ac7f5f84bda1ea5dff8ab26c1fdb3a176f63b64f34e2326615a3680bb4e64393f18ba629880a192e4218ccfe857cee3a6379227363e4d5b282da986e6c66dd082af2353d54afb13c8343533bc71bf2d42bfdb73c22ce4b3ebba2c144ef3bdcc2cb4f006c4706fe5512273c4f6038e84ce0a082a9739a20e4de578131141902054d18e98703c96c383ac3b4768120337e98622683b5c64c8d8db2b1a23ef9"
