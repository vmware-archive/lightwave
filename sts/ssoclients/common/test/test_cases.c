/*
 * Copyright © 2012-2016 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

#include "test_cases.h"

static const PCSTRING s_pszIDToken = "eyJhbGciOiJSUzI1NiJ9.eyJzdWIiOiJhZG1pbmlzdHJhdG9yQHZzcGhlcmUubG9jYWwiLCJhdWQiOiJhZG1pbmlzdHJhdG9yQHZzcGhlcmUubG9jYWwiLCJzY29wZSI6InJzX2FkbWluX3NlcnZlciBvcGVuaWQiLCJpc3MiOiJodHRwczpcL1wvc2MtcmRvcHMtdm0xOC1kaGNwLTYyLTY0LmVuZy52bXdhcmUuY29tXC9vcGVuaWRjb25uZWN0XC92c3BoZXJlLmxvY2FsIiwidG9rZW5fY2xhc3MiOiJpZF90b2tlbiIsInRva2VuX3R5cGUiOiJCZWFyZXIiLCJleHAiOjE0Nzc0MzQ3NzUsImdpdmVuX25hbWUiOiJBZG1pbmlzdHJhdG9yIiwiaWF0IjoxNDc3NDM0NDc1LCJmYW1pbHlfbmFtZSI6InZzcGhlcmUubG9jYWwiLCJqdGkiOiJPVTh5T3FLU3RmQVVDcS1FUTJyWThMX3JEY1VhMlJXaHRMTlNiTk16b0tFIiwidGVuYW50IjoidnNwaGVyZS5sb2NhbCJ9.BogY051jyD_78R3MHTL7WWdTINfxCwP8wBhyTSRfQjR-zcvA6FrKmf75-yUyK_9I48XCD-d8ExIIxkoOv9-t5RTAHlJkVs6J_vUbE_UAl2VpZbehtazX5w10CRHwyIogxaNCrXyILjN2tbgqjz0eG_7JabvPFyBb2DtgbK0qKd0N2qkjpP9lZZgfL5EE5bkGM6ICbphQI_YGJKmbQmED63zA7Q5HZ_mjhXzwxc9h_SQIOCGw7gX71a-YruvjQWHxROb62Z8BwH6VOCepffzFW8zY-Ifc1eQXi5uBiiNk_RLR-fys2flX7lSMgLvUWfI4AzT4JLyJ_tcEgQ2cV77eIw";
static const PCSTRING s_pszIDTokenIssuingCertificatePEM = "-----BEGIN CERTIFICATE-----\nMIIDzzCCAregAwIBAgIJAN3wKoGrohUNMA0GCSqGSIb3DQEBCwUAMIGu\nMQswCQYDVQQDDAJDQTEXMBUGCgmSJomT8ixkARkWB3ZzcGhlcmUxFTAT\nBgoJkiaJk/IsZAEZFgVsb2NhbDELMAkGA1UEBhMCVVMxEzARBgNVBAgM\nCkNhbGlmb3JuaWExMDAuBgNVBAoMJ3NjLXJkb3BzLXZtMTgtZGhjcC02\nMi02NC5lbmcudm13YXJlLmNvbTEbMBkGA1UECwwSVk13YXJlIEVuZ2lu\nZWVyaW5nMB4XDTE2MTAyMDE3Mjk0MVoXDTI2MTAxNTE3MzkyNFowGDEW\nMBQGA1UEAwwNc3Nvc2VydmVyU2lnbjCCASIwDQYJKoZIhvcNAQEBBQAD\nggEPADCCAQoCggEBANfcD1DjMhqoMwv5lCFGnQOso3sXTDHCyPDemOVL\n8Mch3rImiaKa1Tnx814mhSqP8GSOA5hmHzytzqYQBKYLGnpeF41ku2Ba\n+GwBm/1wTpjRhzqlRYtg8iggfZve6DzW7PqTLHcICC4hx8Xuoy2bwolM\nssTlczyKKtuX3Ys64FFPjse9u8lpAcVAIDbj92QsNVSzrB6fF87L3hFg\nK9VgYdcwDpp2LpXe+MZ1IV26Lpzz1blOqhrBBZxyRwjS8ovq1eS/e6hM\nruL6PCfik+EAUbb0rJ0A29utjDTWR3rst6gxHIGHaZF/anKC6ctQWt/U\nD1Nys6nVRQkvnxpc4tlW9PsCAwEAAaOBhDCBgTALBgNVHQ8EBAMCBeAw\nMgYDVR0RBCswKYInc2MtcmRvcHMtdm0xOC1kaGNwLTYyLTY0LmVuZy52\nbXdhcmUuY29tMB0GA1UdDgQWBBRoINBDReKwKk0MmSlH8RZsJkXgGzAf\nBgNVHSMEGDAWgBRbVxEsqv9GgoPYrqpm1u0YeSX2+TANBgkqhkiG9w0B\nAQsFAAOCAQEAMS7KvsRsmIrIUyqQeUK6d9IwVBi/koPeLNXgPReFxHHq\nHFcpMD7xlMqwxEzY2ZiA4C6pHnS8xGAYRhh0m7Ba9S61Ty65GBFgNSVg\nnhPFh/XRngofbVJAwN1EuLv3nOrDma52S3q677jub06JpEm1cunvJ5Ya\nRwIXwGOLXsUOVfS2Cm2Z0g250Qi/SDJw4dKwG05huCXEqhBuRuE3VN+O\nQyUnYQ4mh0kM1j351afuGyhLJ8X0J6YLMRlE/txh9b8ewDDw3uzf3pcW\nvak8I+T3LpxeH7//s8DBpggIucPttuzhPzmwM7ZoTutpy4xn5Bh2jiLA\nK6sDrnZEN/xDqywDYA==\n-----END CERTIFICATE-----";
static const PCSTRING s_pszRSAPrivateKeyPEM = "-----BEGIN RSA PRIVATE KEY-----\nMIIEpAIBAAKCAQEA4Yrat2yoe3yE3CBN77geS1CBtAbF0FcmfxXisZJ6YOWsrg4i\n3NJO7Ki11QpBgcSaxdP4XbjtZO5Cd9ylsZLicUksBmnavBKgF0BvtqG2PCJmaa3Q\nf44296rpOTuo8EcQYdLKWeRx35KY39HKrCfKb/lnxB8bh715nPwYQ+SPBFLrLXXX\n3zJzcT65EUNWggWSp5+gNoSy9mE/NVwCI6qeevLy1DtNRmh6Dl6jMdAum2gAV8x+\nOfi3Pa9gsNj7rxsvtyI/9wza46Dv4M+xRF9dqX98Q6obLgX62cf+6gzd7L4ulrNd\nUOx8/7JCPMNTJmofeplMp4ghfi7Grgqlgm2uRwIDAQABAoIBAQCIK92BgQj4BWXx\nwU9MvE0OC71DhBNKhlOnxv7nVxisB/8J118VE6SzHekqhtamEp2fcysw6sXpejCH\nTaB8ZcJxw0vhU2fMxTIC19Sa32e3Rs2uYFLRsHtfbMEC3RJSf5VUen/RZiuaYFgM\nSg2jgyKmGF6d+moUukEClvHPi98NLCB//H+ssbR8z2xShtN0pwKksVUyCLFzbOe6\nmbYgrCpPMBK0ysFjn4CeSNG74oJo6D78i4eMo1/dbKO1HufS3+qyycNwTo6olfNz\ngEUYx0QmBggXhHNKpKRhDY/7ijda4KPFeBciExA9OIatEmxBwVwtDIKTym6OmbI5\n5VANIWQBAoGBAPtEugQ8MpouDtXmrNPckcS8R5qTDiWZMJ4+FcrCp8etAdtO2hm2\nQlpK0qyi36EFduKhhI3gikxuzmPli9WYeKwEgvb7FEWrqVnGxI8sHF5SM2pRpAzR\nrZkw7MOf2+PWBOyKSYVntSVXagn0JufzgL4CfC//N8PT9pS+kbiJC7ynAoGBAOXK\nHJ5D9gG/YiNVvAx1P7AHyRpsgjopIYvnODyXV9wdK1w5uXZXudEawFfVt/CUHPTl\nG0vOcIKcsfuk4RiEG1Xs37yMeB+t8qSi+UW//AEDy7hQjtZChJcJvJVlJL4477ea\n/AhlKi4/kZbCZI/rtOkWiydr4PcfqLa5zR/+IpVhAoGARtoqWedfnXo/VLGnKlaY\nBBHTXxL4ekGoI/b9zh71kKiITX7+oSFztGJE5clHpfAUhfNlnVwjo0nbiO/Bbol/\n/YGILHzASqUNK/OQmY6msTIcrW44BCVb/kZ9fOwm5SgEW27jLGsB+XWcwRHHpsBT\nHUE2I5Kj9uReM7NJA/AhDyECgYEAnfXYxGbdZogkMzwNX9b9p8lCUfrDbq21lNGm\nU+iJihM3Kle4CQT3BCpKjFaq8iMu4J3ZrLvFldOXee/2mH6/t8p0Zs19CfHueYRa\nrLrzxdLRKOuftOPW92jHDT+RCX5kMyfEFxVL8KzcLPGGZXTXUN+hWsEKzXYi6TFC\nM7rkFSECgYAhW+fpdQMZ5yc/pe98PDAh+VJF4Zw/xsxklBBicTqSM0DCEImD6RMl\n/5ITCtETv3KEfS2kLf2hIlJCQkET2knvdMCx3BMkXO2N/pEIzofPLPWPE5Vl8vGW\nlJNm4hZ7q+0B8/95EaaWCyNn1Uk5CIY2auyknqQlkK4QcvmG1CJTNQ==\n-----END RSA PRIVATE KEY-----";
static const PCSTRING s_pszRSACertificatePEM = "-----BEGIN CERTIFICATE-----\nMIIDiDCCAnACCQCavXL+WSAq7jANBgkqhkiG9w0BAQUFADCBhTETMBEGA1UEAwwK\ndm13YXJlLmNvbTEPMA0GA1UECgwGdm13YXJlMQwwCgYDVQQLDANTU08xDTALBgNV\nBAcMBEtpbmcxEzARBgNVBAgMCldhc2hpbmd0b24xCzAJBgNVBAYTAlVTMR4wHAYJ\nKoZIhvcNAQkBFg90ZXN0QHZtd2FyZS5jb20wHhcNMTYxMDIwMTg1OTI3WhcNMTcx\nMDIwMTg1OTI3WjCBhTETMBEGA1UEAwwKdm13YXJlLmNvbTEPMA0GA1UECgwGdm13\nYXJlMQwwCgYDVQQLDANTU08xDTALBgNVBAcMBEtpbmcxEzARBgNVBAgMCldhc2hp\nbmd0b24xCzAJBgNVBAYTAlVTMR4wHAYJKoZIhvcNAQkBFg90ZXN0QHZtd2FyZS5j\nb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDhitq3bKh7fITcIE3v\nuB5LUIG0BsXQVyZ/FeKxknpg5ayuDiLc0k7sqLXVCkGBxJrF0/hduO1k7kJ33KWx\nkuJxSSwGadq8EqAXQG+2obY8ImZprdB/jjb3quk5O6jwRxBh0spZ5HHfkpjf0cqs\nJ8pv+WfEHxuHvXmc/BhD5I8EUustddffMnNxPrkRQ1aCBZKnn6A2hLL2YT81XAIj\nqp568vLUO01GaHoOXqMx0C6baABXzH45+Lc9r2Cw2PuvGy+3Ij/3DNrjoO/gz7FE\nX12pf3xDqhsuBfrZx/7qDN3svi6Ws11Q7Hz/skI8w1Mmah96mUyniCF+LsauCqWC\nba5HAgMBAAEwDQYJKoZIhvcNAQEFBQADggEBAGa76QeyUDWmZWtYriaaWlNFWmwO\nvxeC7xnc/FfHGHqtplqzgZELVlto1lEd70Zl78ESsRlLMQU3yGmA9blo27aAcOQX\nwEE92Y4JTKFMrVlfp3xchOvMttx7ON9978GmL1THwcW1lA4vGYHnb/zzeIuEjSCA\n6DWevLh0jG9SrOipWowG8dmxszWLuMfeVx3HmMetqrTKYSeCGJEM0QzyH3lKmY8t\nMxwGDSrwqdHeUwpIsMG2RL/MqQxd4rP2wL0X+NnydpXrsJY2bJS+OGjx7HElJsxP\nvNZfmq0x6PPgrjvxyaLERy9wfVwvldXwF95rLKfnhPIAdhPV8/AB/YGmD+A=\n-----END CERTIFICATE-----";
static const PCSTRING s_pszWrongRSACertificatePEM = "-----BEGIN CERTIFICATE-----\nMIIDhjCCAm4CCQCJFCY/GQuKxzANBgkqhkiG9w0BAQUFADCBhDETMBEGA1UEAwwK\ndm13YXJlLmNvbTEPMA0GA1UECgwGVk1XYXJlMQwwCgYDVQQLDANTU08xDTALBgNV\nBAcMBEtpbmcxEzARBgNVBAgMCldhc2hpbmd0b24xCzAJBgNVBAYTAlVTMR0wGwYJ\nKoZIhvcNAQkBFg5zc29Adm13YXJlLmNvbTAeFw0xNjEwMjExOTA1MDJaFw0xNzEw\nMjExOTA1MDJaMIGEMRMwEQYDVQQDDAp2bXdhcmUuY29tMQ8wDQYDVQQKDAZWTVdh\ncmUxDDAKBgNVBAsMA1NTTzENMAsGA1UEBwwES2luZzETMBEGA1UECAwKV2FzaGlu\nZ3RvbjELMAkGA1UEBhMCVVMxHTAbBgkqhkiG9w0BCQEWDnNzb0B2bXdhcmUuY29t\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAqvqORpuhbNW4HUumNAr9\ndpY3SonTVvo7IDWxt/BtlG/Rce+9E+xyqgF66LA+XyNaSpciv0z/VkOl3eUZIxkP\nj1KdkmrKPyjbhDrbDLrXxbOBYEITtGg59NXOLb9J91EHH1F/oHhGywA6m61Ce//z\n2QD1UPm8BJkdGe918OfyhTS4MjXlce564GejwvX1O0pliprncng3rUwNaSM4g0Zv\n+PNQ2my2HuWMrbPJ2A51t0bhRG/X3cZyqiO/w+eszt/eRGe74NCmcYrY2B6NtHtq\n6YKvYv95NAiXHsYTcydATEwhVVEIuyunDipr/FgBgRkQdqGuah6pNgKqQPY+H58B\nSwIDAQABMA0GCSqGSIb3DQEBBQUAA4IBAQCenaBW2cRmoaaHMet9cxTvh8hvX58v\nDRodhnJtx9FoyZ76IkC5aky+ElHUVDM29DHcJO1CveyMXNve+0OdwYY4hAfO0cK8\nMYgQiLT39HZ7WSQ60OZ1Iv8W1wpyhIg7cbmOSkZfUSXIis7bk+WKQMlxj8LQx6jH\nEjunn8dFPkBaT5ZKBOYijgdDjRJslumw+lKQeoOn/ZkyYGGO3olg3jVgOZB5eIqf\nhTfMmPwvCV/sS9RXFjncSkNHDu6sQK6lIB5v/NdPjCHnMAktMqb9Z+5mENCvO+cG\nuxRVbOCas3H2HR6NaBHcbNdUI0c7KNzSd8NWmsTNNGMKfZGP6FGcFjkU\n-----END CERTIFICATE-----";
static const PCSTRING s_pszData = "eyJhbGciOiJSUzI1NiJ9.eyJleHAiOjE0NTc1NTAwMjQsInN1YiI6IkFkbWluaXN0cmF0b3JAdnNwaGVyZS5sb2NhbCIsInRva2VuX2NsYXNzIjoiYWNjZXNzX3Rva2VuIiwic2NvcGUiOiJvZmZsaW5lX2FjY2VzcyByc19hZG1pbl9zZXJ2ZXIgb3BlbmlkIGF0X2dyb3VwcyBpZF9ncm91cHMiLCJ0ZW5hbnQiOiJ2c3BoZXJlLmxvY2FsIiwiYXVkIjpbIkFkbWluaXN0cmF0b3JAdnNwaGVyZS5sb2NhbCIsInJzX2FkbWluX3NlcnZlciJdLCJpc3MiOiJodHRwczpcL1wvc2MtcmRvcHMtdm0wNC1kaGNwLTExOS0xMTIuZW5nLnZtd2FyZS5jb21cL29wZW5pZGNvbm5lY3RcL3ZzcGhlcmUubG9jYWwiLCJhZG1pbl9zZXJ2ZXJfcm9sZSI6IkFkbWluaXN0cmF0b3IiLCJqdGkiOiJVcGFjTk94OG9sT19FeElocFlBSWd1b2xHdWU5eDFkVHBpTXN0QVc5LWc0IiwidG9rZW5fdHlwZSI6IkJlYXJlciIsImlhdCI6MTQ1NzU0OTcyNCwiZ3JvdXBzIjpbInZzcGhlcmUubG9jYWxcXFVzZXJzIiwidnNwaGVyZS5sb2NhbFxcQWRtaW5pc3RyYXRvcnMiLCJ2c3BoZXJlLmxvY2FsXFxDQUFkbWlucyIsInZzcGhlcmUubG9jYWxcXENvbXBvbmVudE1hbmFnZXIuQWRtaW5pc3RyYXRvcnMiLCJ2c3BoZXJlLmxvY2FsXFxBY3RBc1VzZXJzIiwidnNwaGVyZS5sb2NhbFxcU3lzdGVtQ29uZmlndXJhdGlvbi5BZG1pbmlzdHJhdG9ycyIsInZzcGhlcmUubG9jYWxcXExpY2Vuc2VTZXJ2aWNlLkFkbWluaXN0cmF0b3JzIiwidnNwaGVyZS5sb2NhbFxcRXZlcnlvbmUiXX0.CfNoNnCZ2XdWmXweJaoAE1r2loss1rEN_z47AXRGEHjscnInqw-Xb7keBawjuH7e3IxhfxI3SQoMe8u_aJvFsZKvknMbUlzg_8TtxFWIuPiGsH8dzYp9WyL5IFAW09RgCnyHFHD08j1pMQ9qOgn1RKlBcHW1aPZP-sBjNyShFjvd_1RQYz6XDjfzmLGyzaUBjd1JtMyoz2CFt2XDa0DXxQCsj7rMhGDy9x9B24Wq-F2-NBAH6aBZ-VkbD5ITcLOzKRfKRHznbRlmch4kg6miyeJOQOEXXc3_utlrgRa8thD0MbpdcvoKLSMaBYZbK8krsUpoHWoo6WxAbyeGfRyIMQ";
static const PCSTRING s_pszJwkPEM = "-----BEGIN PUBLIC KEY-----\nMIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCgWNmkpmuixAe3nYB7JqzjuTH6\n81VlI867VOLfqQ75ErsOKA5RnkFnZ+d7Q8S+8OXKjqnpU2b6ZCoVqD3gLQYJh01R\nV4nmpfugnO5ow1G/bHDRglubJCHjTO+ccE+TtDMf6zXAo+oPiCmqzj+vqkqwCsNw\nSQTBptof/tWXeMNUGwIDAQAB\n-----END PUBLIC KEY-----\n";
static const PCSTRING s_pszJwkCertPEM = "-----BEGIN CERTIFICATE-----\nMIID1TCCAr2gAwIBAgIJANCgpHBleTNCMA0GCSqGSIb3DQEBCwUAMIGxMQswCQYD\nVQQDDAJDQTEXMBUGCgmSJomT8ixkARkWB3ZzcGhlcmUxFTATBgoJkiaJk/IsZAEZ\nFgVsb2NhbDELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkNhbGlmb3JuaWExMzAxBgNV\nBAoMKnNjMi1yZG9wcy12bTAzLWRoY3AtMTI2LTE4Mi5lbmcudm13YXJlLmNvbTEb\nMBkGA1UECwwSVk13YXJlIEVuZ2luZWVyaW5nMB4XDTE2MTIyMjAwMDEwOFoXDTI2\nMTIxNzAwMTA1MVowGDEWMBQGA1UEAwwNc3Nvc2VydmVyU2lnbjCCASIwDQYJKoZI\nhvcNAQEBBQADggEPADCCAQoCggEBALjjA4U7BeUp58pQ4S/2dJmzfqsJthMXnc1h\nArjGYSCoat8NDVytfR0aZ7vYRC5OkyojyGh8/CkODa7u7jAisPDyomdswRHZaXod\nu3au4jv/6vD996eEq9ElWPW0NlyZn7P+90TO5tumW7LdHVXNrglDyBN6/fQRm+Aq\n7baeNISLSBBNsDfMmmUPb5B3Mz24Qz+IIIp17Wbrt4EJNmyFe+O45wdhj1k65qe0\nMDwd7sPgkGTmT+FrEOWYpD5zNKUioFhR5gJZ9Jc4Z9Bdw8EBwk3YE6gXDB5f9vtD\n7vyPleGQZL0a01daEYHgeHh47k6dXvWq0coOJejdRHd35WiLEyUCAwEAAaOBhzCB\nhDALBgNVHQ8EBAMCBeAwNQYDVR0RBC4wLIIqc2MyLXJkb3BzLXZtMDMtZGhjcC0x\nMjYtMTgyLmVuZy52bXdhcmUuY29tMB0GA1UdDgQWBBTgWtPG98E7yoMYrZYxU1rw\nE48YYzAfBgNVHSMEGDAWgBTCC+ghlpfI/ij3YPPWlzp9YR7JYDANBgkqhkiG9w0B\nAQsFAAOCAQEAFMwvCqWgH3ibxSO66i1Si1SFJSnynHr225PAykAHCg0QI2WJy5xy\nOvuKP4aetrZVrfemkfwDxfNq5jABKU9G6XsFOmXareJxxuIRJgUNnq0Hoq+phxQK\n4fbzh32wjlJZD677k6wAJQ0Nts67YZsqHXirscQttjjfnUvYRXFTdImV56BmDueR\nyEgVcmCK+PiRKo0sFeQHIt1bkSUAZqeNEuVrhSQDaKE/pP2HrtPJjRBSMKb1N2XW\nyWYeDrP1d6UFlziH1aZKD6rHSwAUaC/6Tl5OVaQnEQ1EeU7+HBz7c2oFIcSxz7Om\nwdHD5FHYwmui0SxbWk/9pEWirV/ZLXIkLQ==\n-----END CERTIFICATE-----";
static const PCSTRING s_pszJwkSetWithCert =
    "{" \
    "    \"keys\":" \
    "    [" \
    "        {" \
    "            \"use\": \"sig\"," \
    "            \"kty\": \"RSA\"," \
    "            \"alg\": \"RS256\"," \
    "            \"e\": \"AQAB\"," \
    "            \"n\": \"oFjZpKZrosQHt52Aeyas47kx-vNVZSPOu1Ti36kO-RK7DigOUZ5BZ2fne0PEvvDlyo6p6VNm-mQqFag94C0GCYdNUVeJ5qX7oJzuaMNRv2xw0YJbmyQh40zvnHBPk7QzH-s1wKPqD4gpqs4_r6pKsArDcEkEwabaH_7Vl3jDVBs\"," \
    "            \"x5c\":[\"MIID1TCCAr2gAwIBAgIJANCgpHBleTNCMA0GCSqGSIb3DQEBCwUAMIGxMQswCQYDVQQDDAJDQTEXMBUGCgmSJomT8ixkARkWB3ZzcGhlcmUxFTATBgoJkiaJk\\/IsZAEZFgVsb2NhbDELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkNhbGlmb3JuaWExMzAxBgNVBAoMKnNjMi1yZG9wcy12bTAzLWRoY3AtMTI2LTE4Mi5lbmcudm13YXJlLmNvbTEbMBkGA1UECwwSVk13YXJlIEVuZ2luZWVyaW5nMB4XDTE2MTIyMjAwMDEwOFoXDTI2MTIxNzAwMTA1MVowGDEWMBQGA1UEAwwNc3Nvc2VydmVyU2lnbjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALjjA4U7BeUp58pQ4S\\/2dJmzfqsJthMXnc1hArjGYSCoat8NDVytfR0aZ7vYRC5OkyojyGh8\\/CkODa7u7jAisPDyomdswRHZaXodu3au4jv\\/6vD996eEq9ElWPW0NlyZn7P+90TO5tumW7LdHVXNrglDyBN6\\/fQRm+Aq7baeNISLSBBNsDfMmmUPb5B3Mz24Qz+IIIp17Wbrt4EJNmyFe+O45wdhj1k65qe0MDwd7sPgkGTmT+FrEOWYpD5zNKUioFhR5gJZ9Jc4Z9Bdw8EBwk3YE6gXDB5f9vtD7vyPleGQZL0a01daEYHgeHh47k6dXvWq0coOJejdRHd35WiLEyUCAwEAAaOBhzCBhDALBgNVHQ8EBAMCBeAwNQYDVR0RBC4wLIIqc2MyLXJkb3BzLXZtMDMtZGhjcC0xMjYtMTgyLmVuZy52bXdhcmUuY29tMB0GA1UdDgQWBBTgWtPG98E7yoMYrZYxU1rwE48YYzAfBgNVHSMEGDAWgBTCC+ghlpfI\\/ij3YPPWlzp9YR7JYDANBgkqhkiG9w0BAQsFAAOCAQEAFMwvCqWgH3ibxSO66i1Si1SFJSnynHr225PAykAHCg0QI2WJy5xyOvuKP4aetrZVrfemkfwDxfNq5jABKU9G6XsFOmXareJxxuIRJgUNnq0Hoq+phxQK4fbzh32wjlJZD677k6wAJQ0Nts67YZsqHXirscQttjjfnUvYRXFTdImV56BmDueRyEgVcmCK+PiRKo0sFeQHIt1bkSUAZqeNEuVrhSQDaKE\\/pP2HrtPJjRBSMKb1N2XWyWYeDrP1d6UFlziH1aZKD6rHSwAUaC\\/6Tl5OVaQnEQ1EeU7+HBz7c2oFIcSxz7OmwdHD5FHYwmui0SxbWk\\/9pEWirV\\/ZLXIkLQ==\"]" \
    "        }" \
    "    ]" \
    "}";
static const PCSTRING s_pszJwkSetWithoutCert =
    "{" \
    "    \"keys\":" \
    "    [" \
    "        {" \
    "            \"use\": \"sig\"," \
    "            \"kty\": \"RSA\"," \
    "            \"alg\": \"RS256\"," \
    "            \"e\": \"AQAB\"," \
    "            \"n\": \"oFjZpKZrosQHt52Aeyas47kx-vNVZSPOu1Ti36kO-RK7DigOUZ5BZ2fne0PEvvDlyo6p6VNm-mQqFag94C0GCYdNUVeJ5qX7oJzuaMNRv2xw0YJbmyQh40zvnHBPk7QzH-s1wKPqD4gpqs4_r6pKsArDcEkEwabaH_7Vl3jDVBs\"" \
    "        }" \
    "    ]" \
    "}";

bool
TestStringAllocate()
{
    SSOERROR e = SSOERROR_NONE;

    PSTRING psz = NULL;
    e = SSOStringAllocate("ab", &psz);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("ab", psz);

    SSOStringFree(psz);

    return true;
}

bool
TestStringAllocateFromInt()
{
    SSOERROR e = SSOERROR_NONE;

    PSTRING psz = NULL;
    e = SSOStringAllocateFromInt(1234567, &psz);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("1234567", psz);

    SSOStringFree(psz);

    return true;
}

bool
TestStringAllocateSubstring()
{
    SSOERROR e = SSOERROR_NONE;

    PSTRING psz = NULL;

    e = SSOStringAllocateSubstring("ab", 1, 0, &psz); // startIndex > endIndex
    TEST_ASSERT_EQUAL(SSOERROR_INVALID_ARGUMENT, e);

    e = SSOStringAllocateSubstring("ab", 2, 2, &psz); // startIndex >= inputLength
    TEST_ASSERT_EQUAL(SSOERROR_INVALID_ARGUMENT, e);

    e = SSOStringAllocateSubstring("ab", 1, 2, &psz); // endIndex >= inputLength
    TEST_ASSERT_EQUAL(SSOERROR_INVALID_ARGUMENT, e);

    e = SSOStringAllocateSubstring("abcdef", 2, 3, &psz);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("cd", psz);
    SSOStringFree(psz);

    e = SSOStringAllocateSubstring("abcdef", 1, 1, &psz);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("b", psz);
    SSOStringFree(psz);

    e = SSOStringAllocateSubstring("abcdef", 0, 5, &psz);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("abcdef", psz);
    SSOStringFree(psz);

    return true;
}

bool
TestStringConcatenate()
{
    SSOERROR e = SSOERROR_NONE;
    PCSTRING lhs = "abc";
    PCSTRING rhs = "def";
    PSTRING psz = NULL;

    e = SSOStringConcatenate(lhs, rhs, &psz);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("abcdef", psz);

    SSOStringFree(psz);

    return true;
}

bool
TestStringReplace()
{
    SSOERROR e = SSOERROR_NONE;
    PSTRING psz = NULL;

    // target not found, output same as input
    e = SSOStringReplace("abcdef", "ij", "xyz", &psz);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("abcdef", psz);
    SSOStringFree(psz);

    // target in middle
    e = SSOStringReplace("abcdef", "bc", "xyz", &psz);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("axyzdef", psz);
    SSOStringFree(psz);

    // target at start
    e = SSOStringReplace("abcdef", "ab", "xyz", &psz);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("xyzcdef", psz);
    SSOStringFree(psz);

    // target at end
    e = SSOStringReplace("abcdef", "ef", "xyz", &psz);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("abcdxyz", psz);
    SSOStringFree(psz);

    // target is all of string
    e = SSOStringReplace("abcdef", "abcdef", "xyz", &psz);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("xyz", psz);
    SSOStringFree(psz);

    // long url
    e = SSOStringReplace("https://old_server/openidconnect/token/vsphere.local", "old_server", "_new_server_", &psz);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("https://_new_server_/openidconnect/token/vsphere.local", psz);
    SSOStringFree(psz);

    return true;
}

bool
TestStringBuilder()
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_STRING_BUILDER p = NULL;
    PSTRING psz = NULL;

    e = SSOStringBuilderNew(&p);
    TEST_ASSERT_SUCCESS(e);

    e = SSOStringBuilderAppend(p, "a");
    TEST_ASSERT_SUCCESS(e);
    e = SSOStringBuilderGetString(p, &psz);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("a", psz);
    SSOStringFree(psz);

    e = SSOStringBuilderAppend(p, "bcd");
    TEST_ASSERT_SUCCESS(e);
    e = SSOStringBuilderGetString(p, &psz);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("abcd", psz);
    SSOStringFree(psz);

    SSOStringBuilderDelete(p);

    return true;
}

bool
TestKeyValuePair()
{
    SSOERROR e = SSOERROR_NONE;

    PSSO_KEY_VALUE_PAIR p = NULL;
    e = SSOKeyValuePairNew(&p, "k1", "v1");
    TEST_ASSERT_SUCCESS(e);

    TEST_ASSERT_EQUAL_STRINGS("k1", SSOKeyValuePairGetKey(p));
    TEST_ASSERT_EQUAL_STRINGS("v1", SSOKeyValuePairGetValue(p));

    SSOKeyValuePairDelete(p);

    return true;
}

bool
TestBase64UrlEncodeToString()
{
    SSOERROR e = SSOERROR_NONE;

    PSTRING output = NULL;

    e = SSOBase64UrlEncodeToString((const unsigned char*) "a", SSOStringLength("a"), &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("YQ", output);
    SSOStringFree(output);

    e = SSOBase64UrlEncodeToString((const unsigned char*) "ab", SSOStringLength("ab"), &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("YWI", output);
    SSOStringFree(output);

    e = SSOBase64UrlEncodeToString((const unsigned char*) "abc", SSOStringLength("abc"), &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("YWJj", output);
    SSOStringFree(output);

    e = SSOBase64UrlEncodeToString((const unsigned char*) "abcd", SSOStringLength("abcd"), &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("YWJjZA", output);
    SSOStringFree(output);

    e = SSOBase64UrlEncodeToString((const unsigned char*) "abcde", SSOStringLength("abcde"), &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("YWJjZGU", output);
    SSOStringFree(output);

    e = SSOBase64UrlEncodeToString((const unsigned char*) "abcdef", SSOStringLength("abcdef"), &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("YWJjZGVm", output);
    SSOStringFree(output);

    e = SSOBase64UrlEncodeToString((const unsigned char*) "abcdefg", SSOStringLength("abcdefg"), &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("YWJjZGVmZw", output);
    SSOStringFree(output);

    e = SSOBase64UrlEncodeToString((const unsigned char*) "abcdefgh", SSOStringLength("abcdefgh"), &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("YWJjZGVmZ2g", output);
    SSOStringFree(output);

    e = SSOBase64UrlEncodeToString((const unsigned char*) "abcdefghi", SSOStringLength("abcdefghi"), &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("YWJjZGVmZ2hp", output);
    SSOStringFree(output);

    e = SSOBase64UrlEncodeToString((const unsigned char*) "Hello, World!", SSOStringLength("Hello, World!"), &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("SGVsbG8sIFdvcmxkIQ", output);
    SSOStringFree(output);

    return true;
}

bool
TestBase64UrlDecodeToString()
{
    SSOERROR e = SSOERROR_NONE;

    PSTRING output = NULL;

    e = SSOBase64UrlDecodeToString("YQ", &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("a", output);
    SSOStringFree(output);

    e = SSOBase64UrlDecodeToString("YWI", &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("ab", output);
    SSOStringFree(output);

    e = SSOBase64UrlDecodeToString("YWJj", &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("abc", output);
    SSOStringFree(output);

    e = SSOBase64UrlDecodeToString("YWJjZA", &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("abcd", output);
    SSOStringFree(output);

    e = SSOBase64UrlDecodeToString("YWJjZGU", &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("abcde", output);
    SSOStringFree(output);

    e = SSOBase64UrlDecodeToString("YWJjZGVm", &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("abcdef", output);
    SSOStringFree(output);

    e = SSOBase64UrlDecodeToString("YWJjZGVmZw", &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("abcdefg", output);
    SSOStringFree(output);

    e = SSOBase64UrlDecodeToString("YWJjZGVmZ2g", &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("abcdefgh", output);
    SSOStringFree(output);

    e = SSOBase64UrlDecodeToString("YWJjZGVmZ2hp", &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("abcdefghi", output);
    SSOStringFree(output);

    e = SSOBase64UrlDecodeToString("SGVsbG8sIFdvcmxkIQ", &output);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("Hello, World!", output);
    SSOStringFree(output);

    // a single char is not a valid base64url encoded string
    e = SSOBase64UrlDecodeToString("Y", &output);
    TEST_ASSERT_EQUAL(SSOERROR_INVALID_ARGUMENT, e);

    // & is not a valid base64url character
    e = SSOBase64UrlDecodeToString("Y&", &output);
    TEST_ASSERT_EQUAL(SSOERROR_INVALID_ARGUMENT, e);

    return true;
}

bool
TestJwtParseSuccess()
{
    SSOERROR e = SSOERROR_NONE;

    PSSO_JWT p = NULL;
    PSTRING psz = NULL;
    SSO_LONG longValue = 0;

    e = SSOJwtParse(&p, s_pszIDToken);
    TEST_ASSERT_SUCCESS(e);

    e = SSOJwtGetStringClaim(p, "token_class", &psz);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS("id_token", psz);
    SSOStringFree(psz);

    e = SSOJwtGetLongClaim(p, "exp", &longValue);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_TRUE(longValue > 0);

    SSOJwtDelete(p);

    return true;
}

bool
TestJwtParseFail()
{
    SSOERROR e = SSOERROR_NONE;

    PSSO_JWT p = NULL;

    e = SSOJwtParse(&p, "A.B");
    TEST_ASSERT_EQUAL(SSOERROR_INVALID_ARGUMENT, e);

    e = SSOJwtParse(&p, "A.B.C.D");
    TEST_ASSERT_EQUAL(SSOERROR_INVALID_ARGUMENT, e);

    e = SSOJwtParse(&p, "A.B.C");
    TEST_ASSERT_EQUAL(SSOERROR_INVALID_ARGUMENT, e);

    return true;
}

bool
TestJwtSignatureVerifySuccess()
{
    SSOERROR e = SSOERROR_NONE;

    PSSO_JWT p = NULL;
    bool validSignature = false;

    e = SSOJwtParse(&p, s_pszIDToken);
    TEST_ASSERT_SUCCESS(e);

    e = SSOJwtVerifySignature(p, s_pszIDTokenIssuingCertificatePEM, &validSignature);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_TRUE(validSignature);

    SSOJwtDelete(p);

    return true;
}

bool
TestJwtSignatureVerifyFail()
{
    SSOERROR e = SSOERROR_NONE;

    PSSO_JWT p = NULL;
    bool validSignature = false;

    e = SSOJwtParse(&p, s_pszIDToken);
    TEST_ASSERT_SUCCESS(e);

    e = SSOJwtVerifySignature(p, s_pszWrongRSACertificatePEM, &validSignature);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_TRUE(!validSignature);

    SSOJwtDelete(p);

    return true;
}

bool
TestJwtCreateSignedJwtString()
{
    SSOERROR e = SSOERROR_NONE;

    PSTRING psz = NULL;
    PSSO_JWT pJwt = NULL;
    bool validSignature = false;

    e = SSOJwtCreateSignedJwtString(" { \"iss\": \"issuer\", \"sub\": \"subject\" } ", s_pszRSAPrivateKeyPEM, &psz);
    TEST_ASSERT_SUCCESS(e);

    e = SSOJwtParse(&pJwt, psz);
    TEST_ASSERT_SUCCESS(e);

    e = SSOJwtVerifySignature(pJwt, s_pszRSACertificatePEM, &validSignature);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_TRUE(validSignature);

    SSOStringFree(psz);
    SSOJwtDelete(pJwt);

    return true;
}

bool
TestJwkParseSuccessWithCert()
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_JWK p = NULL;
    PSTRING pszPEM = NULL;
    PSTRING pszCertPEM = NULL;

    e = SSOJwkParseFromSet(&p, s_pszJwkSetWithCert);
    TEST_ASSERT_SUCCESS(e);

    TEST_ASSERT_TRUE(SSOJwkGetCertificate(p) != NULL);
    e = SSOJwkToCertificatePEM(p, &pszCertPEM);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS(s_pszJwkCertPEM, pszCertPEM);

    e = SSOJwkToPublicKeyPEM(p, &pszPEM);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS(s_pszJwkPEM, pszPEM);

    SSOJwkDelete(p);
    SSOStringFree(pszPEM);
    SSOStringFree(pszCertPEM);

    return true;
}

bool
TestJwkParseSuccessWithoutCert()
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_JWK p = NULL;
    PSTRING pszPEM = NULL;

    e = SSOJwkParseFromSet(&p, s_pszJwkSetWithoutCert);
    TEST_ASSERT_SUCCESS(e);

    TEST_ASSERT_TRUE(SSOJwkGetCertificate(p) == NULL);

    e = SSOJwkToPublicKeyPEM(p, &pszPEM);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_EQUAL_STRINGS(s_pszJwkPEM, pszPEM);

    SSOJwkDelete(p);
    SSOStringFree(pszPEM);

    return true;
}

bool
TestJwkParseFail()
{
    SSOERROR e = SSOERROR_NONE;
    PSSO_JWK p = NULL;

    e = SSOJwkParseFromSet(&p, s_pszIDToken); // should not be able to parse id_token
    TEST_ASSERT_EQUAL(SSOERROR_JSON_PARSE_FAILURE, e);

    SSOJwkDelete(p);

    return true;
}

bool
TestSignatureVerifySuccess()
{
    SSOERROR e = SSOERROR_NONE;

    unsigned char* pRSASignature = NULL;
    size_t rsaSignatureSize;
    bool verifySuccess = false;

    e = SSOComputeRSASignature(
        SSO_DIGEST_METHOD_SHA256,
        (const unsigned char*) s_pszData,
        SSOStringLength(s_pszData),
        s_pszRSAPrivateKeyPEM,
        &pRSASignature,
        &rsaSignatureSize);
    TEST_ASSERT_SUCCESS(e);

    e = SSOVerifyRSASignature(
        SSO_DIGEST_METHOD_SHA256,
        (const unsigned char*) s_pszData,
        SSOStringLength(s_pszData),
        pRSASignature,
        rsaSignatureSize,
        s_pszRSACertificatePEM,
        &verifySuccess);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_TRUE(verifySuccess);

    return true;
}

bool
TestSignatureVerifyFail()
{
    SSOERROR e = SSOERROR_NONE;

    unsigned char* pRSASignature = NULL;
    size_t rsaSignatureSize;
    bool verifySuccess = false;

    e = SSOComputeRSASignature(
        SSO_DIGEST_METHOD_SHA256,
        (const unsigned char*) s_pszData,
        SSOStringLength(s_pszData),
        s_pszRSAPrivateKeyPEM,
        &pRSASignature,
        &rsaSignatureSize);
    TEST_ASSERT_SUCCESS(e);

    e = SSOVerifyRSASignature(
        SSO_DIGEST_METHOD_SHA256,
        (const unsigned char*) s_pszData,
        SSOStringLength(s_pszData),
        pRSASignature,
        rsaSignatureSize,
        s_pszWrongRSACertificatePEM,
        &verifySuccess);
    TEST_ASSERT_SUCCESS(e);
    TEST_ASSERT_TRUE(!verifySuccess);

    return true;
}
