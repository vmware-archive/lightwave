/*
 * Copyright © 2018 VMware, Inc.  All Rights Reserved.
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
#include "includes.h"

#define TEST_ROOT_CA_ID "RootCA"
#define TEST_ROOT_CA_CERTIFICATE "-----BEGIN CERTIFICATE-----\n" \
        "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\n" \
        "4vBqfx74YhjZYbjYkE/6leLu9StAAd5J3b1XvA==\n" \
        "-----END CERTIFICATE-----"
#define TEST_ROOT_CA_KEY "-----BEGIN RSA PRIVATE KEY-----\n" \
        "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\n" \
        "-----END RSA PRIVATE KEY-----"
#define TEST_ROOT_CA_PASSPHRASE ""
#define TEST_ROOT_CA_ID_2 "RootCA_2"
#define TEST_ROOT_CA_CERTIFICATE_2 "-----BEGIN CERTIFICATE-----\n" \
        "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\n" \
        "DCU=\n" \
        "-----END CERTIFICATE-----"
#define TEST_ROOT_CA_KEY_2 "-----BEGIN RSA PRIVATE KEY-----\n" \
        "Proc-Type: 4,ENCRYPTED\n" \
        "DEK-Info: AES-256-CBC,7D346D2EF644F5DAA43295EA56BE6250\n" \
        "\n" \
        "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\n" \
        "-----END RSA PRIVATE KEY-----"
#define TEST_ROOT_CA_PASSPHRASE_2 "test"
// TEST_INVALID_CA_CERTIFICATE is a selfsigned certificate which does not have keyCertSign capability
#define TEST_INVALID_ROOT_CA_CERTIFICATE_2 "-----BEGIN CERTIFICATE-----\n" \
        "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\n" \
        "-----END CERTIFICATE-----"
#define TEST_DUMMY_CERTIFICATE "-----BEGIN CERTIFICATE-----\n" \
        "MIID5TCCAs2gAwIBAgIJAONKhmr803trMA0GCSqGSIb3DQEBCwUAMGkxIDAeBgNV\n" \
        "BAMMF0NBLERDPWx3LXRlc3Rkb20sREM9Y29tMQswCQYDVQQGEwJVUzE4MDYGA1UE\n" \
        "CgwvbHd0LXByYXNvb250LWktMDRhNmE5ZDQxYWU2MGU5NTAubHctdGVzdGRvbS5j\n" \
        "b20wHhcNMTgwOTI4MTczMDI5WhcNMjgwOTIyMTcxMTU4WjBGMUQwQgYDVQQDDDtt\n" \
        "dXRlbnRjYS1wb3N0LTY2LWktMGY5MGQwOTU4OWYxMjRmODYubHctdGVzdGRvbS5j\n" \
        "b20tc2VydmljZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALZKdjuu\n" \
        "EKvpnsr4qU7IPX+lJo6eoSCTX/dZs6gs+pyXshkkfLR1um+vcRNqiJF8SjsRflXv\n" \
        "Si6tzE2Cf2i8k6Tu2EVBhXvrZRVDnjlxVlUam5K9nLonHqT+w77xhcNugHJU6wet\n" \
        "xm/kFOUVqQkCn9ehRNivFSAAQgkUgTQS1b9AeMFFKq6rvjwi/vqWm5KaRbVMNYj7\n" \
        "2T+MT57i5mJpJVgzHRMoMBmEhYNC7kos0Qdej5Ud8o4UuAjRktsN0f+f9Pm9r4ku\n" \
        "4NC2p31B1N5od9fM4Jtj2fWbSoTNMhctJ1GBhCicuPgX3DEu1ASVZnIx9k0CDE/n\n" \
        "uUHSkSwBfwPQYd0CAwEAAaOBsjCBrzALBgNVHQ8EBAMCBeAwHQYDVR0OBBYEFIc1\n" \
        "FeBfbMLmSQNkVffWJUwlzKNhMB8GA1UdIwQYMBaAFK69Ssu1s5g6ixVIN/luNvxm\n" \
        "CyODMGAGCCsGAQUFBwEBBFQwUjBQBggrBgEFBQcwAoZEaHR0cHM6Ly9sd3QtcHJh\n" \
        "c29vbnQtaS0wNGE2YTlkNDFhZTYwZTk1MC5sdy10ZXN0ZG9tLmNvbS9hZmQvdmVj\n" \
        "cy9zc2wwDQYJKoZIhvcNAQELBQADggEBADzpitGGg+nWAmvjU/jxYvRU3GFZjG25\n" \
        "Mot99e+LloRrbpPS3YIVLdNkKG7uU2ndXebJmwG+McgY1ZCgk31FcBFkM/eeeWvI\n" \
        "soR6dcK63nDO7t9tSygYSriTZnMbGhsKCK9bXbPt8RrjmZN8rnRgaXKf1J9vgy6X\n" \
        "SRGV8DAEMzURA7fFPK2R36CyKaBTS+7MfQ9NRKuBU8wETzVWqgFBRYlVzYuVdcCW\n" \
        "v59zouvgwjoQ/qB3LKh4y2w7aPfPXRrI3P3WZZi+xZSsdK2uzjrU+Wh1GNCEKdDz\n" \
        "+D1z/13oztwrHTqAdq8BflLG9gpS1jZ+tVqJTiL2r2TBP27fC/9rNVM=\n" \
        "-----END CERTIFICATE-----"
#define TEST_CLIENT_KEY "-----BEGIN RSA PRIVATE KEY-----\n" \
        "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\n" \
        "-----END RSA PRIVATE KEY-----"
#define TEST_CLIENT_PASSPHRASE ""
// CSR with out extensions
#define TEST_CLIENT_CSR "-----BEGIN CERTIFICATE REQUEST-----\n" \
        "MIIEsTCCApkCAQAwbDELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAldBMREwDwYDVQQH\n" \
        "DAhCZWxsZXZ1ZTETMBEGA1UECgwKVk13YXJlIEluYzETMBEGA1UECwwKVm13YXJl\n" \
        "IEluYzETMBEGA1UEAwwKVGVzdCBVc2VyMTCCAiIwDQYJKoZIhvcNAQEBBQADggIP\n" \
        "ADCCAgoCggIBALzYURptRIAEzyqzFa+DxmikP7s9CjL5HR5NAhv6eMlHMFC8iEoe\n" \
        "wimPdVR4bqsMqmsvqhwj8bVJF4BAKF67GLNC1yD8R1DLpUoDLYmuTL8d8MFaDRAI\n" \
        "SicUQTyyq5ppF3+trz0ypyEo+upKKtdTDnrpghtRNrUNjHOO68FJUdNwyuAxerSX\n" \
        "OwLnTz7ELH+1fDqddEp9K5G2afpwDPplJgOZNHOWPpE1VJ3xNVqN3fPi6oqEiSQU\n" \
        "bQwqs1p2mEFVi1SqCs58KEJ0VlspFSKXa1FsGIuDNr4EKS4S0a2dNg9qC0l8V74Q\n" \
        "I52qYOVtDqHXmh4Tx8PWHpUPGkS9n9ovmMafhSdqsf3M6LW8xGrK5HDt/YqGobg2\n" \
        "m4t8jgy0sp1GmhpAdufsDwE2NAs+iFjybpQQnFM9stHQGJDy+T3IKOgZqsVizyn3\n" \
        "o4U4ysG8IYgaBGd/Z3Uea6Q3VBzREmrOGVpagI/czUojlfihMR6sJSFPAVVrBao3\n" \
        "Ty6mR7e94B2kr4o8zG5G4mV/zqcid2FU3r5EJLS7PIyxgVQzhW0jXd8z7LKi1bUE\n" \
        "KfxQ1ToUvVDndwrFZrUFdc4oRK831OPpHcOFi/QpPDBbAsymSK3jPu/WSDbZBUgt\n" \
        "4d//YSmKSl12Vde3qvDSMtGdVZ5TCNZQ7t5eE/kNV7Zlrs/4xMz53dqnAgMBAAGg\n" \
        "ADANBgkqhkiG9w0BAQsFAAOCAgEAEkq1h3u7SUJZwv5djxjC0vZMmjr7L2Wshz5d\n" \
        "KmG4Q01Z+oUFQ2xt6A1dubeFjf5QPx5cC6zSnl3yyuX0/cx8LTu/2AvrbgE4iazA\n" \
        "DPPvocACw09F3bXQwme/UpLxQ1w/es1/1rrxRfE6lfCHVOkcUaShkuf35IasrYN5\n" \
        "BmgLJ6BGYQ7B9m18UE/Xz5Me6ouRLc7/ktpUR1HtLcXtmMs5sHOatrMVIOWriIzh\n" \
        "bYFmt8c/CfRTHPCbXHEVH91tmhioPg6OjRyG7fhnWaqBOS3MPT0H714ctElVBJBr\n" \
        "Ri4kx0Rm+EkQeBWd6BTmUbGLndREESkKmUHixwhIo5T7Nintx+EjJD3ZUou+xb/j\n" \
        "iUrLJ+vinzc7UJhGwVgIKvrf+JEBx9zs15ofw3JPonAJPz0UF+crmaSPifwN3EZw\n" \
        "z1Vic3EJf9nX2Zz/Pvdjy4dvnOR4sm+I8aB2feGc0KM0DOEbtNGiAx6U8Xce0PqV\n" \
        "JZYqo2QDNke2nQ+iczFlwcriWDOiaPcqqkg1xz90Nm5R0W5mIKqVAU9SX5EUdBww\n" \
        "H9/8SH8E3Wh0O7jcKH33smwJkUG2DeX0x7+TFOz1nkomJzvP5KPRJi4IFq/HLQbq\n" \
        "t29x/AECMz7n9hV9jD2TZKanRFzSqyd9q/PzgCS+3eUNJ453PFdJLwuYYKb7gxLC\n" \
        "kPTsKM8=\n" \
        "-----END CERTIFICATE REQUEST-----"
// CSR with extensions
#define TEST_CLIENT_CSR_2 "-----BEGIN CERTIFICATE REQUEST-----\n" \
        "MIIDpTCCAo0CAQAwgYMxCzAJBgNVBAYTAlVTMQswCQYDVQQIDAJXQTERMA8GA1UE\n" \
        "BwwIQmVsbGV2dWUxEzARBgNVBAoMClZNd2FyZSBJbmMxEzARBgNVBAsMClZNd2Fy\n" \
        "ZSBJbmMxDjAMBgNVBAMMBVRlc3QyMRowGAYJKoZIhvcNAQkBFgthYmNAeHl6LmNv\n" \
        "bTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbDMjff74+Tiiw9OgM0\n" \
        "+js78CaVww9wEr9SuzU3s9uaoVh2Gs0DtehPwvBv4lMAQJSI0izi+GqFywCkEogd\n" \
        "dEgdIC+V1n08/ojzppwtQ6PmliP7+2W/zAIa15RedEO42wTOEhaYDaWih5mxXo/v\n" \
        "t01SUwzC89UeGC/HW3g0om7Go8uEVrIUdhuatijTWjcN+N7mykJmks9ZmUpQDx7Z\n" \
        "R4tqEacVwY3CdDj4bZgZoVYpOOYn2Mt/54D2KLq4zc1tugc1W3PWKRPAi3QjNdwp\n" \
        "m5mCA9k2RQ9N062YSnSYYm2vPV9+L+wdRj7M9M/nBvypEgPxZwP0XAvEhh+ecHkx\n" \
        "FQUCAwEAAaCB2zCB2AYJKoZIhvcNAQkOMYHKMIHHMAkGA1UdEwQCMAAwEQYJYIZI\n" \
        "AYb4QgEBBAQDAgWgMDMGCWCGSAGG+EIBDQQmFiRPcGVuU1NMIEdlbmVyYXRlZCBD\n" \
        "bGllbnQgQ2VydGlmaWNhdGUwHQYDVR0OBBYEFPXAosvvYRCm4ZXBgQi39gnxna/3\n" \
        "MA4GA1UdDwEB/wQEAwIF4DAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwQw\n" \
        "JAYDVR0RBB0wG4ILZXhhbXBsZS5jb22CDGV4YW1wbGUxLmNvbTANBgkqhkiG9w0B\n" \
        "AQsFAAOCAQEAXASj2VYm6n2iHQGYny5ThpbKp95B5nhnxq4tCObDRRiyMlaLjx2a\n" \
        "6uRcDwuEJXzPUxUMWDMc1vShi+4bcI8YzmuZk8EumDQUenaTozrhU8hxmG0QgjNs\n" \
        "4aKTfLVYhHsXTuIChzAA+oGD7TXPs+pOxHV5HyFqN7IEVT/a0E3eWQWeZ3B+ousl\n" \
        "4eVwP9EQQSyNgZYck5es9889v5+JEBl9vOdaHJ01nLICvRO18E3ByqHw7vutIIFV\n" \
        "D6q7gp0OBmrquLH79YRukb2MWJHXaDamrH0UG8b4P/a0ScY4R5SzbJmdzbSfeJkh\n" \
        "T1LtZafdHfraueXAWDkB+U/2mMS0hrh7AA==\n" \
        "-----END CERTIFICATE REQUEST-----"
#define TEST_INTERMEDIATE_CA_ID "Intermediate CA"
#define TEST_INTERMEDIATE_CA_PUBLIC_KEY "-----BEGIN PUBLIC KEY-----\n" \
        "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAyKKleR7E/pD9U3edvj6s\n" \
        "DTStacElF8+ELBq1qOYjJBSzRfL/C4B8sHgfI2z4iM+DUDZWf7i8lLcjuU86Hi5X\n" \
        "Oz39QDDtYo+UfYuCpvwLZxcc0EOTa4WaXlqMTgeIW6gVmXv842HNKNxQiW+buLmL\n" \
        "vnMtIPQdTKIxPt3p6UCoeRinUpMOPL3Wge/Pcg1Tixq0cFO2XUQ+vpbS18itTmQ1\n" \
        "FBxBFe5Kj4qbJmEcqbbxAOJMhb74GUwy0QdAMo3XM7buVArskrSzXDwgl/1oy7Bx\n" \
        "Fc5u9/wy7cEcWaDM7MparEfQe4Cdsv8mDyCMpxrU0r8Y4SOIZiP1sjt+FohGUcne\n" \
        "FQIDAQAB\n" \
        "-----END PUBLIC KEY-----"
#define TEST_INTERMEDIATE_CA_PRIVATE_KEY "-----BEGIN RSA PRIVATE KEY-----\n" \
        "MIIEowIBAAKCAQEAyKKleR7E/pD9U3edvj6sDTStacElF8+ELBq1qOYjJBSzRfL/\n" \
        "C4B8sHgfI2z4iM+DUDZWf7i8lLcjuU86Hi5XOz39QDDtYo+UfYuCpvwLZxcc0EOT\n" \
        "a4WaXlqMTgeIW6gVmXv842HNKNxQiW+buLmLvnMtIPQdTKIxPt3p6UCoeRinUpMO\n" \
        "PL3Wge/Pcg1Tixq0cFO2XUQ+vpbS18itTmQ1FBxBFe5Kj4qbJmEcqbbxAOJMhb74\n" \
        "GUwy0QdAMo3XM7buVArskrSzXDwgl/1oy7BxFc5u9/wy7cEcWaDM7MparEfQe4Cd\n" \
        "sv8mDyCMpxrU0r8Y4SOIZiP1sjt+FohGUcneFQIDAQABAoIBAB6MvXt61u4YL6qU\n" \
        "7Rz5uALuwXT35ukAPRTmIEEOgc1NpZqCDJm4v8OzFrKzNgjG6Cy/iV47R+OxGyxt\n" \
        "RuEvlzK7FqO+j3bRe/+9zXAVsrrIxydjEsBtgHrbeL+s3/Ns3ZUYTwJkcvPE0DKN\n" \
        "Hv36qq+eJAH/ibKQg8UZzzMoGD8P3+gefE6yNHm5ypxC09Fm1uLgO1ik2Yh/ie25\n" \
        "S+c5NG80isNtyfHGHDmUb2O5Rv0Wk08t/LQ0oghmCofu3i6Qsk0Vu1hHDGe3mF6i\n" \
        "35zyCmXVlBLOQ5jG36kFZl5P5I8jhQ2+Izv/gs5prEGnu2/jKK7PpmkXEPBc2Jhu\n" \
        "0b5+J4ECgYEA6ICG62r5LYKaJuHXNneiCLkEXu2dj68a+LSz1LKieXB31YcXkfUA\n" \
        "FnmmbvZTrNKb58nzCss7YAgKs610s954oI+C+LmW9wXvuMgIDxGjuG//AgRqpV/u\n" \
        "t7swJea7/wElAJ1xAsLe6EhSWFrBUoL3xeEgcMSLLHAEyGFV74oZzCECgYEA3Omj\n" \
        "j1SkIGr/P2GPuHSoMzOTAsGB8WR5JcIG7PbZ1Kq5LMifbWxja/pd1oMQfRjqnpBO\n" \
        "QGsmCHPDMEt+uXLGK40RCSqOnvGbZD2SN7BPYws/DEcdVKCC7oltkGDw6SYXzTFU\n" \
        "GgYXmdmsqFKBwfFoi0+iFt5IGp8pyAR7bB21M3UCgYAUyglGW5ZUP21RNLhtolzW\n" \
        "stR1OIHyKxIaUu4RgreMbEYKzb2Jy4JqgA4ltdjdhydxnlkhCfMRLybl0gixXJ8k\n" \
        "nDB11QJurBU3TIuL7WPaRgljQMnYZ6+Mtiwj93Gv54ZLe8hzGsGbu3vBcrv+iL9m\n" \
        "7i5cZvaazple4XUxqgOMgQKBgFn6+Fn7OdbSfiEM1gNl0NT6HtkyhIEUUZwlibif\n" \
        "W16aI06wcO+bBGwI0Iv9Wov5yCbJ1HW8rwMmU8yqE3Xp3/oBPX0kTm9DLe4IfjQc\n" \
        "pllLTWTs2m+L6YjLhUsXkUkF8Q/tFbp73MUXupHc+WlLsS99QiJaa7gcOGJ2IOeK\n" \
        "ujG5AoGBAOXHNcQmoEr+pqXn6KCkf0yAGxSqLwOA6Zg5UFbSBeeXrxTyIFz+TWpp\n" \
        "9ftIRwcy4tj2d1KMZ2lO49YgqZg0z6HBdYZgLL6R80GPk2TcEEBfgm2lQE2IIDJb\n" \
        "kscq5zBLIDLH7qAhtCJX8NlHmvNSgb77e+8fxG1Mdp5WwrywqO8w\n" \
        "-----END RSA PRIVATE KEY-----"
#define TEST_CLIENT_CERTIFICATE "-----BEGIN CERTIFICATE-----\n" \
        "MIIEQTCCAikCAhACMA0GCSqGSIb3DQEBCwUAMGkxCzAJBgNVBAYTAlVTMQswCQYD\n" \
        "VQQIDAJXQTERMA8GA1UEBwwIQmVsbGV2dWUxEzARBgNVBAoMClZNd2FyZSBJbmMx\n" \
        "EzARBgNVBAsMClZNd2FyZSBJbmMxEDAOBgNVBAMMB1Rlc3QgQ0EwHhcNMTgxMDE1\n" \
        "MDYzMDExWhcNMTkxMDI1MDYzMDExWjBjMQswCQYDVQQGEwJVUzELMAkGA1UECAwC\n" \
        "V0ExEzARBgNVBAoMClZNd2FyZSBJbmMxEzARBgNVBAsMClZNd2FyZSBJbmMxHTAb\n" \
        "BgNVBAMMFFRlc3QgSW50ZXJtZWRpYXRlIENBMIIBIjANBgkqhkiG9w0BAQEFAAOC\n" \
        "AQ8AMIIBCgKCAQEAxl2Ws9gmHJk9hofaimzsz29Aaypx2G42cHQyA2AgxuU2ScH1\n" \
        "l+l8/DLYryv5qzJTZgqIn6H7uzOj5yEFG3f0jyiNYY/w5exF6RakNMRjAT7jql7N\n" \
        "RKSKMfVGvsDUFLUo+gYklHZD4e54Ur4BVYxYzZXfjuUdBoqeySC4AS6dIfc7kkbj\n" \
        "D0q9vtZ4QlFv7+9lq/XPWviEYAt7Caw38qAdjx78twc2SLInfCglAvRgi4NVaXvL\n" \
        "hQX3rax9M6LMg9iuHgGcmBPAghJ2cleaYAmftFtLbz6nLVlt/JLjirG43edfTk7w\n" \
        "e4ARLAE6xvqgFIdeLUkZgyJtTOE3/+onANDpuQIDAQABMA0GCSqGSIb3DQEBCwUA\n" \
        "A4ICAQBrMuqo3Suf/qphltjraxWKX2l4O0EzFqxR9EhpUUm9hmjWSPyh6KENSIQl\n" \
        "Pq3ib4rsXe5or3TA9EpZzNBM55DrsGHxmT0RWyKlve0cPm1I4gN2h6l+PngbeMUY\n" \
        "oQDcVMnhsbOdwlpGxF7XANaKM5iFRylY3Ugm35RsQqVqkl20SHdRq+gIcbtqK/Es\n" \
        "ClT/wvcnY8EeTzePeaQudBlgcgQmSjUh+eWRuT5DfsM8zMhahwrWClzU0LbZWQwJ\n" \
        "/tdIKfhHDim/jF/GPHogYRFwFC5cgb05g8enJ2pOJPcJIH+3oUrpbqYmthnnSpiH\n" \
        "LTIYcm7lRD63SFBe6XinwNet5wbu4UqPR1UMrX3Ouz9BoUnTAWWrWeGbIsvZn/mI\n" \
        "e2kO7W8qrh9t7fyjOkxoJ4UAzL4Soz4ipmBq0xd7ZktFcg9q7N4dxiJVM1usVM0p\n" \
        "vlKvpqrg3wDaetrWUR+yIXhacB/MN0o7y2YsWf7oNu+RziPdWZ5frBBWMwiaP+EW\n" \
        "oWm4YDkN2dx2b710Izr17ADTc/KL0PryKn2xQRXHCQaIw8OKfUQRi7nnIsPvBKTC\n" \
        "PZ+Uzvj4jH6ZK37iKEH0FtvO/zuyyH5pQS7WYrseK8In4CYGvM2h4k6qm4ngLp1Y\n" \
        "4WjsK+nBWQ8mdipk2eOEc5hcHR1NoA/OAcAV+T/mP7dzQe4uVw==\n" \
        "-----END CERTIFICATE-----"

// defines output value of __wrap_LwCADbCheckCA
PBOOLEAN pbExistsMockArray =  NULL;
DWORD iIndex = 0;
DWORD dwMockArrayLimit = 0;

PLWCA_REQ_CONTEXT pReqCtx = NULL;

int
TestLwCACreateRequestContext(
    VOID **state
    )
{
    DWORD dwError = 0;

    if (!pReqCtx)
    {
        dwError = LwCAAllocateMemory(sizeof(LWCA_REQ_CONTEXT), (PVOID*)&pReqCtx);
        assert_int_equal(dwError, 0);
    }
    return 0;
}

int
TestLwCAFreeRequestContext(
    VOID **state
    )
{
    if (pReqCtx)
    {
        LWCA_SAFE_FREE_MEMORY(pReqCtx);
    }
    return 0;
}

DWORD
__wrap_LwCADbAddCA(
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        pCAData,
    PCSTR                   pcszParentCAId
    )
{
    assert_non_null(pcszCAId);
    assert_non_null(pCAData);
    return mock();
}

VOID
_Initialize_Output_LwCADbCheckCA(
    PBOOLEAN pbOutList,
    DWORD    dwLimit
    )
{
    pbExistsMockArray = pbOutList;
    iIndex = 0;
    dwMockArrayLimit = dwLimit;
}

DWORD
__wrap_LwCADbCheckCA(
    PCSTR                   pcszCAId,
    PBOOLEAN                pbExists
    )
{
    assert_non_null(pcszCAId);
    assert_non_null(pbExists);
    if (iIndex < dwMockArrayLimit)
    {
        *pbExists = pbExistsMockArray[iIndex++];
    }
    return mock();
}

DWORD
__wrap_LwCADbGetCACertificates(
    PCSTR                      pcszCAId,
    PLWCA_CERTIFICATE_ARRAY    *ppCertArray
    )
{
    DWORD dwError = 0;
    PSTR *ppCertificates = NULL;

    dwError = LwCAAllocateMemory(1 * sizeof(PSTR), (PVOID*)&ppCertificates);
    assert_int_equal(dwError, 0);

    if (LwCAStringCompareA(pcszCAId, "RootCA_2", TRUE) == 0)
    {
        dwError = LwCAAllocateStringA(TEST_ROOT_CA_CERTIFICATE_2, &ppCertificates[0]);
        assert_int_equal(dwError, 0);
    }
    else if (LwCAStringCompareA(pcszCAId, "RootCA", TRUE) == 0)
    {
        dwError = LwCAAllocateStringA(TEST_ROOT_CA_CERTIFICATE, &ppCertificates[0]);
        assert_int_equal(dwError, 0);
    }
    else
    {
        dwError = LwCAAllocateStringA("dummyCert", &ppCertificates[0]);
        assert_int_equal(dwError, 0);
    }

    dwError = LwCACreateCertArray(ppCertificates, 1, ppCertArray);
    assert_int_equal(dwError, 0);

    LWCA_SAFE_FREE_MEMORY(ppCertificates[0]);
    LWCA_SAFE_FREE_MEMORY(ppCertificates);

    return mock();
}

DWORD
__wrap_LwCADbGetCA(
    PCSTR                   pcszCAId,
    PLWCA_DB_CA_DATA        *ppCAData
    )
{
    DWORD dwError = 0;
    PLWCA_DB_CA_DATA pCAData = NULL;
    PLWCA_KEY pKey = NULL;
    PSTR pszKey = NULL;
    size_t keyLen = 0;
    PSTR *ppszCertificates = NULL;
    PLWCA_CERTIFICATE_ARRAY pCertArray = NULL;

    dwError = LwCAAllocateMemory(1 * sizeof(PSTR), (PVOID*)&ppszCertificates);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA(TEST_ROOT_CA_CERTIFICATE, &ppszCertificates[0]);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateCertArray(ppszCertificates, 1, &pCertArray);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateMemory(sizeof(LWCA_KEY), (PVOID*)&pKey);
    assert_int_equal(dwError, 0);

    dwError = LwCAAllocateStringA(TEST_ROOT_CA_KEY, &pszKey);
    assert_int_equal(dwError, 0);

    keyLen = LwCAStringLenA(pszKey);

    dwError = LwCAAllocateAndCopyMemory(pszKey, keyLen, (PVOID*)&pKey->pData);
    assert_int_equal(dwError, 0);

    pKey->dwLength = keyLen;

    dwError = LwCADbCreateCAData(
                    "C=US, ST=WA, L=Bellevue, O=VMware Inc, OU=VMware Inc, CN=Test CA",
                    pCertArray,
                    pKey,
                    NULL,
                    LWCA_CA_STATUS_ACTIVE,
                    &pCAData
                );
    assert_int_equal(dwError, 0);

    *ppCAData = pCAData;

    LWCA_SAFE_FREE_MEMORY(ppszCertificates[0]);
    LWCA_SAFE_FREE_MEMORY(ppszCertificates);
    LWCA_SAFE_FREE_STRINGA(pszKey);
    LwCAFreeCertificates(pCertArray);
    LwCAFreeKey(pKey);

    return mock();
}

DWORD
__wrap_LwCADbCheckCertData(
    PCSTR                   pcszCAId,
    PCSTR                   pcszSerialNumber,
    PBOOLEAN                pbExists
    )
{
    assert_string_equal(pcszCAId, TEST_ROOT_CA_ID);
    assert_non_null(pcszSerialNumber);
    assert_non_null(pbExists);
    *pbExists = FALSE;

    return mock();
}

DWORD
__wrap_LwCADbGetCACRLNumber(
    PCSTR   pcszCAId,
    PSTR    *ppszCRLNumber
    )
{
    DWORD dwError = 0;
    assert_string_equal(pcszCAId, TEST_ROOT_CA_ID);

    dwError = LwCAAllocateStringA("110000", ppszCRLNumber);
    assert_int_equal(dwError, 0);

    return mock();
}

DWORD
__wrap_LwCADbUpdateCACRLNumber(
    PCSTR   pcszCAId,
    PCSTR   pcszCRLNumber
    )
{
    assert_string_equal(pcszCAId, TEST_ROOT_CA_ID);
    assert_string_equal(pcszCRLNumber, "110001");

    return mock();
}

DWORD
__wrap_LwCADbAddCertData(
    PCSTR                   pcszCAId,
    PLWCA_DB_CERT_DATA      pCertData
    )
{
    assert_string_equal(pcszCAId, TEST_ROOT_CA_ID);
    assert_non_null(pCertData);
    assert_non_null(pCertData->pszSerialNumber);
    assert_non_null(pCertData->pszRevokedDate);
    assert_non_null(pCertData->pszTimeValidFrom);
    assert_non_null(pCertData->pszTimeValidTo);

    return mock();
}

DWORD
__wrap_LwCAKmCreateKeyPair(
    PCSTR pcszKeyId
    )
{
    return mock();
}

DWORD
__wrap_LwCAKmGetPublickey(
    PCSTR pcszKeyId,
    PSTR  *ppszPublicKey
    )
{
    DWORD dwError = 0;

    if (LwCAStringCompareA(pcszKeyId, TEST_INTERMEDIATE_CA_ID, TRUE) == 0)
    {
        dwError = LwCAAllocateStringA(TEST_INTERMEDIATE_CA_PUBLIC_KEY, ppszPublicKey);
        assert_int_equal(dwError, 0);
    }
    return mock();
}

DWORD
__wrap_LwCAKmGetEncryptedKey(
    PCSTR       pcszId,
    PLWCA_KEY   *ppKey
    )
{
    DWORD dwError = 0;
    BYTE testKey[20] = "dummyKey";

    dwError = LwCACreateKey(testKey, 20, ppKey);
    assert_int_equal(dwError, 0);
    return mock();
}

DWORD
__wrap_LwCAKmSignX509Cert(
    X509 *pCert,
    PCSTR pcszKeyId
    )
{
    DWORD dwError = 0;

    if (LwCAStringCompareA(pcszKeyId, TEST_ROOT_CA_ID, TRUE) == 0)
    {
        dwError = LwCAX509SignCertificate(pCert, TEST_ROOT_CA_KEY, NULL);
        assert_int_equal(dwError, 0);
    }
    else if (LwCAStringCompareA(pcszKeyId, TEST_INTERMEDIATE_CA_ID, TRUE) == 0)
    {
        dwError = LwCAX509SignCertificate(pCert, TEST_INTERMEDIATE_CA_PRIVATE_KEY, NULL);
        assert_int_equal(dwError, 0);
    }
    return mock();
}

DWORD
__wrap_LwCAKmSignX509Request(
    X509_REQ *pReq,
    PCSTR    pcszKeyId
    )
{
    DWORD dwError = 0;

    if (LwCAStringCompareA(pcszKeyId, TEST_ROOT_CA_ID, TRUE) == 0)
    {
        dwError = LwCAX509ReqSignRequest(pReq, TEST_ROOT_CA_KEY, NULL);
        assert_int_equal(dwError, 0);
    }
    else if (LwCAStringCompareA(pcszKeyId, TEST_INTERMEDIATE_CA_ID, TRUE) == 0)
    {
        dwError = LwCAX509ReqSignRequest(pReq, TEST_INTERMEDIATE_CA_PRIVATE_KEY, NULL);
        assert_int_equal(dwError, 0);
    }

    return mock();
}

VOID
Test_LwCACreateRootCA_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE pCertificate1 = NULL;
    PLWCA_CERTIFICATE pCertificate2 = NULL;
    BOOLEAN bCheckCAMockValues[] = {FALSE};

    will_return_always(__wrap_LwCADbAddCA, 0);
    will_return_always(__wrap_LwCADbCheckCA, 0);

    dwError = LwCACreateCertificate(TEST_ROOT_CA_CERTIFICATE, &pCertificate1);
    assert_int_equal(dwError, 0);

    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues, 1);
    dwError = LwCACreateRootCA(pReqCtx, TEST_ROOT_CA_ID, pCertificate1, TEST_ROOT_CA_KEY, TEST_ROOT_CA_PASSPHRASE);
    assert_int_equal(dwError, 0);

    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues, 1);
    dwError = LwCACreateCertificate(TEST_ROOT_CA_CERTIFICATE, &pCertificate2);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateRootCA(pReqCtx, TEST_ROOT_CA_ID, pCertificate2, TEST_ROOT_CA_KEY, TEST_ROOT_CA_PASSPHRASE);
    assert_int_equal(dwError, 0);

    LwCAFreeCertificate(pCertificate1);
    LwCAFreeCertificate(pCertificate2);
}

VOID
Test_LwCACreateRootCA_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE pCertificate1 = NULL;
    PLWCA_CERTIFICATE pCertificate2 = NULL;
    BOOLEAN bCheckCAMockValues[] = {FALSE};
    BOOLEAN bCheckCAMockValues2[] = {TRUE};

    will_return_always(__wrap_LwCADbCheckCA, 0);

    // Testcase 1: Certificate is not a CA cert

    dwError = LwCACreateCertificate(TEST_INVALID_ROOT_CA_CERTIFICATE_2, &pCertificate1);
    assert_int_equal(dwError, 0);

    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues, 1);
    dwError = LwCACreateRootCA(pReqCtx, TEST_ROOT_CA_ID_2, pCertificate1, TEST_ROOT_CA_KEY_2, TEST_ROOT_CA_PASSPHRASE_2);
    assert_int_equal(dwError, LWCA_NOT_CA_CERT);

    // Testcase 2: Certificate/Key pair mismatch

    dwError = LwCACreateCertificate(TEST_DUMMY_CERTIFICATE, &pCertificate2);
    assert_int_equal(dwError, 0);

    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues, 1);
    dwError = LwCACreateRootCA(pReqCtx, TEST_ROOT_CA_ID, pCertificate2, TEST_ROOT_CA_KEY, TEST_ROOT_CA_PASSPHRASE);
    assert_int_equal(dwError, LWCA_CERT_PRIVATE_KEY_MISMATCH);


    // Testcase 3: Invalid inputs

    dwError = LwCACreateRootCA(pReqCtx, TEST_ROOT_CA_ID, NULL, NULL, NULL);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    // Testcase 4: CA exists already

    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues2, 1);
    dwError = LwCACreateRootCA(pReqCtx, TEST_ROOT_CA_ID, pCertificate2, TEST_ROOT_CA_KEY, TEST_ROOT_CA_PASSPHRASE);
    assert_int_equal(dwError, LWCA_CA_ALREADY_EXISTS);

    LwCAFreeCertificate(pCertificate1);
    LwCAFreeCertificate(pCertificate2);
}

VOID
Test_LwCAGetCACertificates_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE_ARRAY pCertificates = NULL;
    BOOLEAN bCheckCAMockValues[] = {TRUE};

    will_return_always(__wrap_LwCADbCheckCA, 0);
    will_return(__wrap_LwCADbGetCACertificates, 0);

    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues, 1);
    dwError = LwCAGetCACertificates(pReqCtx, TEST_ROOT_CA_ID, &pCertificates);
    assert_int_equal(dwError, 0);
    assert_non_null(pCertificates);
    assert_int_equal(pCertificates->dwCount, 1);
    assert_non_null(pCertificates->ppCertificates);
    assert_string_equal(pCertificates->ppCertificates[0], TEST_ROOT_CA_CERTIFICATE);

    LwCAFreeCertificates(pCertificates);
}

VOID
Test_LwCAGetCACertificates_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE_ARRAY pCertificates = NULL;
    BOOLEAN bCheckCAMockValues[] = {FALSE};

    will_return_always(__wrap_LwCADbCheckCA, 0);

    // Testcase 1: Invalid input

    dwError = LwCAGetCACertificates(pReqCtx, NULL, &pCertificates);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
    assert_null(pCertificates);

    // Testcase 2: CA does not exist

    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues, 1);
    dwError = LwCAGetCACertificates(pReqCtx, TEST_ROOT_CA_ID, &pCertificates);
    assert_int_equal(dwError, LWCA_CA_MISSING);
    assert_null(pCertificates);

    LwCAFreeCertificates(pCertificates);
}


VOID
Test_LwCAGetSignedCertificate_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE pCertificate1 = NULL;
    PLWCA_CERTIFICATE pCertificate2 = NULL;
    PLWCA_CERTIFICATE_ARRAY pCACerts = NULL;
    PLWCA_CERT_VALIDITY pValidity = NULL;
    LWCA_SIGNING_ALGORITHM signAlgorithm;
    time_t tmNotBefore;
    time_t tmNotAfter;
    struct tm *tm = NULL;
    BOOLEAN bCheckCAMockValues[] = {TRUE};

    tmNotBefore = time(NULL);
    tm = localtime(&tmNotBefore);
    tm->tm_mday += 1;
    tmNotAfter = mktime(tm);

    dwError = LwCACreateCertValidity(tmNotBefore, tmNotAfter, &pValidity);
    assert_int_equal(dwError, 0);

    signAlgorithm = LWCA_SHA_256;

    will_return_always(__wrap_LwCADbCheckCA, 0);
    will_return_always(__wrap_LwCADbGetCACertificates, 0);
    will_return_always(__wrap_LwCADbGetCA, 0);

    dwError = LwCADbGetCACertificates(TEST_ROOT_CA_ID, &pCACerts);
    assert_int_equal(dwError, 0);

    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues, 1);
    dwError = LwCAGetSignedCertificate(pReqCtx, TEST_ROOT_CA_ID, TEST_CLIENT_CSR, pValidity, signAlgorithm, &pCertificate1);
    assert_int_equal(dwError, 0);

    dwError = LwCAVerifyCertificate(pCACerts, pCertificate1);
    assert_int_equal(dwError, 0);

    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues, 1);
    dwError = LwCAGetSignedCertificate(pReqCtx, TEST_ROOT_CA_ID, TEST_CLIENT_CSR_2, pValidity, signAlgorithm, &pCertificate2);
    assert_int_equal(dwError, 0);

    dwError = LwCAVerifyCertificate(pCACerts, pCertificate2);
    assert_int_equal(dwError, 0);

    LwCAFreeCertValidity(pValidity);
    LwCAFreeCertificate(pCertificate1);
    LwCAFreeCertificate(pCertificate2);
    LwCAFreeCertificates(pCACerts);
}

VOID
Test_LwCAGetSignedCertificate_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_CERTIFICATE pCertificate = NULL;
    PLWCA_CERT_VALIDITY pValidity = NULL;
    LWCA_SIGNING_ALGORITHM signAlgorithm;
    time_t tmNotBefore;
    time_t tmNotAfter;
    struct tm *tm = NULL;
    BOOLEAN bCheckCAMockValues[] = {TRUE};

    tmNotBefore = time(NULL);
    tm = localtime(&tmNotBefore);
    tm->tm_mday -= 1;
    tmNotAfter = mktime(tm);

    dwError = LwCACreateCertValidity(tmNotBefore, tmNotAfter, &pValidity);
    assert_int_equal(dwError, 0);

    signAlgorithm = LWCA_SHA_256;

    will_return_always(__wrap_LwCADbCheckCA, 0);
    will_return_always(__wrap_LwCADbGetCACertificates, 0);

    dwError = LwCAGetSignedCertificate(pReqCtx, NULL, TEST_CLIENT_CSR, pValidity, signAlgorithm, &pCertificate);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
    assert_null(pCertificate);

    dwError = LwCAGetSignedCertificate(pReqCtx, TEST_ROOT_CA_ID, TEST_CLIENT_CSR, NULL, signAlgorithm, &pCertificate);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
    assert_null(pCertificate);

    dwError = LwCAGetSignedCertificate(pReqCtx, TEST_ROOT_CA_ID, TEST_CLIENT_CSR, pValidity, signAlgorithm, NULL);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
    assert_null(pCertificate);

    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues, 1);
    dwError = LwCAGetSignedCertificate(pReqCtx, TEST_ROOT_CA_ID, TEST_CLIENT_CSR, pValidity, signAlgorithm, &pCertificate);
    assert_int_equal(dwError, LWCA_INVALID_TIME_SPECIFIED);
    assert_null(pCertificate);

    LwCAFreeCertValidity(pValidity);
    LwCAFreeCertificate(pCertificate);
}

VOID
Test_LwCACreateIntermediateCA_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_INT_CA_REQ_DATA pCARequest = NULL;
    PLWCA_CERT_VALIDITY pValidity = NULL;
    PLWCA_CERTIFICATE_ARRAY pCACerts =  NULL;
    PLWCA_CERTIFICATE_ARRAY pParentCACerts =  NULL;
    PSTR pOUs[] = {"VMware Inc"};
    PSTR pCountries[] = {"US"};
    PSTR pStates[] = {"WA", "CA"};
    PSTR pLocalities[] = {"Bellevue"};
    PLWCA_STRING_ARRAY pCountryList = NULL;
    PLWCA_STRING_ARRAY pStateList = NULL;
    PLWCA_STRING_ARRAY pLocalityList = NULL;
    PLWCA_STRING_ARRAY pOUList = NULL;
    time_t tmNotBefore;
    time_t tmNotAfter;
    struct tm *tm = NULL;
    BOOLEAN bCheckCAMockValues[] = {FALSE, TRUE};

    tmNotBefore = time(NULL);
    tm = localtime(&tmNotBefore);
    tm->tm_mday += 1;
    tmNotAfter = mktime(tm);

    dwError = LwCACreateCertValidity(tmNotBefore, tmNotAfter, &pValidity);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateStringArray(pOUs, 1, &pOUList);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateStringArray(pCountries, 1, &pCountryList);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateStringArray(pStates, 2, &pStateList);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateStringArray(pLocalities, 1, &pLocalityList);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateIntCARequest(pCountryList, pStateList, pLocalityList, pOUList, NULL, &pCARequest);
    assert_int_equal(dwError, 0);

    will_return_always(__wrap_LwCADbCheckCA, 0);
    will_return_always(__wrap_LwCADbGetCACertificates, 0);
    will_return_always(__wrap_LwCADbAddCA, 0);
    will_return_always(__wrap_LwCAKmGetPublickey, 0);
    will_return_always(__wrap_LwCAKmGetEncryptedKey, 0);
    will_return_always(__wrap_LwCAKmCreateKeyPair, 0);
    will_return_always(__wrap_LwCAKmSignX509Cert, 0);
    will_return_always(__wrap_LwCAKmSignX509Request, 0);

    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues, 2);
    dwError = LwCACreateIntermediateCA(
                            pReqCtx,
                            TEST_INTERMEDIATE_CA_ID,
                            TEST_ROOT_CA_ID,
                            pCARequest,
                            pValidity,
                            &pCACerts
                            );
    assert_int_equal(dwError, 0);

    dwError = LwCADbGetCACertificates(TEST_ROOT_CA_ID, &pParentCACerts);
    assert_int_equal(dwError, 0);
    assert_non_null(pCACerts);
    assert_non_null(pCACerts->ppCertificates);
    assert_int_equal(pCACerts->dwCount, 1);

    dwError = LwCAVerifyCertificate(pParentCACerts, pCACerts->ppCertificates[0]);
    assert_int_equal(dwError, 0);

    LwCAFreeCertValidity(pValidity);
    LwCAFreeIntCARequest(pCARequest);
    LwCAFreeCertificates(pCACerts);
    LwCAFreeCertificates(pParentCACerts);
    LwCAFreeStringArray(pOUList);
    LwCAFreeStringArray(pCountryList);
    LwCAFreeStringArray(pStateList);
    LwCAFreeStringArray(pLocalityList);
}

VOID
Test_LwCACreateIntermediateCA_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    PLWCA_INT_CA_REQ_DATA pCARequest = NULL;
    PLWCA_CERT_VALIDITY pValidity = NULL;
    PLWCA_CERTIFICATE_ARRAY pCACerts =  NULL;
    BOOLEAN bCheckCAMockValues[] = {FALSE, TRUE};
    BOOLEAN bCheckCAMockValues2[] = {FALSE, FALSE};
    BOOLEAN bCheckCAMockValues3[] = {TRUE, FALSE};
    time_t tmNotBefore;
    time_t tmNotAfter;
    struct tm *tm = NULL;

    tmNotBefore = time(NULL);
    tm = localtime(&tmNotBefore);
    tm->tm_mday -= 1;
    tmNotAfter = mktime(tm);

    dwError = LwCACreateCertValidity(tmNotBefore, tmNotAfter, &pValidity);
    assert_int_equal(dwError, 0);

    dwError = LwCACreateIntCARequest(NULL, NULL, NULL, NULL, NULL, &pCARequest);
    assert_int_equal(dwError, 0);

    will_return_always(__wrap_LwCADbCheckCA, 0);
    will_return_always(__wrap_LwCADbGetCACertificates, 0);
    will_return_always(__wrap_LwCAKmCreateKeyPair, 0);
    will_return_always(__wrap_LwCAKmGetPublickey, 0);
    will_return_always(__wrap_LwCAKmSignX509Request, 0);

    // Testcase 1: Invalid input

    dwError = LwCACreateIntermediateCA(
                            pReqCtx,
                            TEST_INTERMEDIATE_CA_ID,
                            TEST_ROOT_CA_ID,
                            NULL,
                            NULL,
                            &pCACerts
                            );
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);
    assert_null(pCACerts);

    // Testcase 2: Invalid Validity period

    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues, 2);
    dwError = LwCACreateIntermediateCA(
                            pReqCtx,
                            TEST_INTERMEDIATE_CA_ID,
                            TEST_ROOT_CA_ID,
                            pCARequest,
                            pValidity,
                            &pCACerts
                            );
    assert_int_equal(dwError, LWCA_INVALID_TIME_SPECIFIED);
    assert_null(pCACerts);

    // Testcase 3: Parent CA does not exist

    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues2, 2);
    dwError = LwCACreateIntermediateCA(
                            pReqCtx,
                            TEST_INTERMEDIATE_CA_ID,
                            TEST_ROOT_CA_ID,
                            pCARequest,
                            pValidity,
                            &pCACerts
                            );
    assert_int_equal(dwError, LWCA_CA_MISSING);
    assert_null(pCACerts);

    // Testcase 4: CA exists already

    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues3, 2);
    dwError = LwCACreateIntermediateCA(
                            pReqCtx,
                            TEST_INTERMEDIATE_CA_ID,
                            TEST_ROOT_CA_ID,
                            pCARequest,
                            pValidity,
                            &pCACerts
                            );
    assert_int_equal(dwError, LWCA_CA_ALREADY_EXISTS);
    assert_null(pCACerts);

    LwCAFreeCertValidity(pValidity);
    LwCAFreeIntCARequest(pCARequest);
    LwCAFreeCertificates(pCACerts);
}

VOID
Test_LwCARevokeCertificate_Valid(
    VOID **state
    )
{
    DWORD dwError = 0;
    BOOLEAN bCheckCAMockValues[] = {TRUE};

    will_return(__wrap_LwCADbCheckCA, 0);
    will_return(__wrap_LwCADbCheckCertData, 0);
    will_return(__wrap_LwCADbGetCACRLNumber, 0);
    will_return(__wrap_LwCADbUpdateCACRLNumber, 0);
    will_return(__wrap_LwCADbAddCertData, 0);
    will_return(__wrap_LwCADbGetCACertificates, 0);

    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues, 1);
    dwError = LwCARevokeCertificate(pReqCtx, TEST_ROOT_CA_ID, TEST_CLIENT_CERTIFICATE);
    assert_int_equal(dwError, 0);
}

VOID
Test_LwCARevokeCertificate_Invalid(
    VOID **state
    )
{
    DWORD dwError = 0;
    BOOLEAN bCheckCAMockValues[] = {TRUE};

    will_return_always(__wrap_LwCADbCheckCA, 0);
    will_return(__wrap_LwCADbGetCACertificates, 0);

    // Testcase1: Invalid input
    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues, 1);
    dwError = LwCARevokeCertificate(pReqCtx, TEST_ROOT_CA_ID, NULL);
    assert_int_equal(dwError, LWCA_ERROR_INVALID_PARAMETER);

    // Testcase2: Certificate not issued by requested CA
    _Initialize_Output_LwCADbCheckCA(bCheckCAMockValues, 1);
    dwError = LwCARevokeCertificate(pReqCtx, TEST_ROOT_CA_ID, TEST_DUMMY_CERTIFICATE);
    assert_int_equal(dwError, LWCA_SSL_CERT_VERIFY_ERR);
}
