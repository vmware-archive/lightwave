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

#include <iostream>
#include <aws/core/Aws.h>
#include <aws/kms/KMSClient.h>
#include <aws/kms/model/GenerateDataKeyRequest.h>
#include <aws/kms/model/GenerateDataKeyResult.h>
#include <aws/kms/model/EncryptRequest.h>
#include <aws/kms/model/EncryptResult.h>
#include <aws/kms/model/DecryptRequest.h>
#include <aws/kms/model/DecryptResult.h>
#include <aws/core/utils/Outcome.h>
#include <aws/core/utils/crypto/Factories.h>
#include <aws/core/utils/crypto/Cipher.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/utils/StringUtils.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>
#include <aws/core/utils/crypto/CryptoStream.h>
#include <aws/core/utils/crypto/ContentCryptoMaterial.h>
#include <aws/kms/model/DataKeySpec.h>

using namespace Aws;
using namespace Aws::Client;
using namespace Aws::Utils;
using namespace Aws::Utils::Crypto;

#include "aws_kms_structs.h"
#include "aws_kms_crypto_config.h"
#include "aws_kms_encrypted_data.h"
#include "aws_kms_crypto_helper.h"
